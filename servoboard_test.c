/******************************************************************************
 *
 * servo-wt.c - Servo (wide timer) driver for TI Stellaris microcontroller
 *
 * Copyright (c) 2013, Joseph Kroesche (kroesche.org)
 * All rights reserved.
 *
 * This software is released under the FreeBSD license, found in the
 * accompanying file LICENSE.txt and at the following URL:
 *      http://www.freebsd.org/copyright/freebsd-license.html
 *
 * This software is provided as-is and without warranty.
 *
 *****************************************************************************/

/* Library headers
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/* StellarisWare headers
 */
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"

#include "driverlib/fpu.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"

#include "utils/uartstdio.h"
#include "utils/ustdlib.h"

/* Servo driver header
 */
#include "stellaris_drivers/servo-wt.h"

/******************************************************************************
 *
 * This file contains a simple test program to exercise the 12-servo
 * booster pack.  It is meant to be used as part of the test procedure
 * for the board.
 */

/******************************************************************************
 *
 * Macros and locals
 *
 *****************************************************************************/

/* Define the number of servos used
 */
#define NUM_SERVOS 12

/* Holds a servo handle for each servo index
 */
static ServoHandle_t hServo[NUM_SERVOS];

/* This is a bit mask to track which servos are in motion.  Each bit
 * represents one servo.  If the bit is 1 it means the servo is in
 * motion.
 */
static volatile uint32_t gInMotion;

/* This structure holds the timer and timer half used for each servo
 */
typedef struct
{
    int32_t timer;
    int32_t half;
} ServoInfo_t;

/* Table that maps each servo index (0, 1, 2, etc) to a specific
 * hardware timer instance
 */
static const ServoInfo_t servoInfo[NUM_SERVOS] =
{
    {   4, SERVO_TIMER_A },
    {   4, SERVO_TIMER_B },
    {   2, SERVO_TIMER_A },
    {   2, SERVO_TIMER_B },
    {   3, SERVO_TIMER_A },
    {   3, SERVO_TIMER_B },
    {   5, SERVO_TIMER_A },
    {   5, SERVO_TIMER_B },
    {   1, SERVO_TIMER_A },
    {   1, SERVO_TIMER_B },
    {   0, SERVO_TIMER_A },
    {   0, SERVO_TIMER_B },
};

/* Defines the RGB pins and color combinations
 */
#define RGB_RED GPIO_PIN_1
#define RGB_BLU GPIO_PIN_2
#define RGB_GRN GPIO_PIN_3
#define RGB_YEL (RGB_GRN | RGB_RED)
#define RGB_PINS (RGB_RED | RGB_BLU | RGB_GRN)

/* Holds the system clock frequency
 */
static uint32_t gSysClock;

/******************************************************************************
 *
 * Private Functions
 *
 *****************************************************************************/

/*
 * Read the battery ADC and update the RGB color to indicate if
 * the battery voltage is in range of 8V +/- 0.1V
 */
static void
UpdateRGB(void)
{
    uint32_t bat = Servo_ReadBatteryMv();
    if(bat < 7900)
    {
        GPIOPinWrite(GPIO_PORTF_BASE, RGB_PINS, RGB_BLU);
    }
    else if(bat > 8100)
    {
        GPIOPinWrite(GPIO_PORTF_BASE, RGB_PINS, RGB_RED);
    }
    else
    {
        GPIOPinWrite(GPIO_PORTF_BASE, RGB_PINS, RGB_GRN);
    }
}

/*
 * Motion callback for servo move commands
 *
 * @param pData is the callback data, which is the servo index
 *
 * This function is called back whenever a servo move is complete.  It
 * is used to clear the "in-motion" bit for the servo that finished moving.
 */
static void
MotionCallback(void *pData)
{
    uint32_t servo = (uint32_t)pData;
    HWREGBITW(&gInMotion, servo) = 0;
}

/*
 * Wait until a specific servo has stopped moving.
 *
 * @param servo is the servo number to wait on
 *
 * This function will poll-wait until the specified servo has stoped
 * moving.
 */
static void
WaitOne(uint32_t servo)
{
    while(HWREGBITW(&gInMotion, servo))
    {}
}

/*
 * Wait until multiple servos have stopped moving
 *
 * @param mask is a bit mask of servos to wait on
 *
 * This function will poll-wait until all of the servos specified in the
 * bit mask have finished moving.
 */
#if 0 // set to 1 if needed
static void
WaitAll(uint32_t mask)
{
    while(gInMotion & mask)
    {}
}
#endif

/*
 * Move one servo to a specified position in centi-deg
 *
 * @param servo is the servo index to move
 * @param cdeg is the new position in centi-deg
 *
 * This function will command the specified servo to move to the
 * new position.  It does not wait for the motion to complete.
 */
static void
MoveOne(uint32_t servo, int32_t cdeg)
{
    WaitOne(servo);
    HWREGBITW(&gInMotion, servo) = 1;
    Servo_Move(hServo[servo], cdeg, MotionCallback, (void *)servo);
}

/*
 * Move multiple servos to a specified position
 *
 * @param mask is a bit mask of all the servos to move
 * @param cdeg is the new position in centi-deg
 *
 * This function will command multiple servos to move to a specific
 * position.  It does not wait for the motion to complete.
 */
#if 0 // set to 1 if function is used
static void
MoveAll(uint32_t mask, int32_t cdeg)
{
    for(int servo = 0; servo < NUM_SERVOS; servo++)
    {
        if(mask & (1 << servo))
        {
            MoveOne(servo, cdeg);
        }
    }
}
#endif

/*
 * Poll-wait delay in milliseconds
 *
 * @param msecs is the delay time in milliseconds
 *
 * This funtion will wait a specified number of milliseconds, using
 * a polling loop.
 */
void
DelayMs(uint32_t msecs)
{
    SysCtlDelay(((gSysClock / 1000) * msecs) / 3);
}

/******************************************************************************
 *
 * Test program
 *
 *****************************************************************************/
int
main(void)
{
    int ret;
    int servo;  // lame TI compiler cant handle loop var declaration

    FPUStackingDisable();
    
    /* Initialize the clock to run at 40 MHz
     */
    SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
    gSysClock = SysCtlClockGet();

    /* Initialize the UART.
     */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
#ifdef STELLARISWARE
    UARTStdioInit(0);
#else
    UARTStdioConfig(0, 115200, gSysClock);
#endif
    
    UARTprintf("\n\nServoBoard-Test\n---------------\n");

    /* Initialize the GPIO port for the RGB LED
     */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, RGB_PINS);
    GPIOPinWrite(GPIO_PORTF_BASE, RGB_PINS, 0);

    /* Initialize the battery monitor
     * Use zeroes for parameter so default calibration will be used
     */
    Servo_BatteryInit(0, 0);
    
    /* Initialize servos for 20 msec
     */
    ret = Servo_Init(gSysClock, 20000);
    if(ret)
    {
        UARTprintf("error calling ServoInit\n");
        return 0;
    }

    /* Enter loop to initialize all the servos in the system
     */
    for(servo = 0; servo < NUM_SERVOS; servo++)
    {
        /* Associate each servo ID with a hardware timer (and A or B half)
         */
        hServo[servo] = Servo_Config(servoInfo[servo].timer, servoInfo[servo].half);
        if(hServo[servo] == 0)
        {
            UARTprintf("error config servo %d\n", servo);
            return 0;
        }

        /* Delay a bit before initting the next servo.  This is done to
         * spread out the servo pulses so they do not all happen at the
         * same time and load down the power supply.
         * The delay value was determined experimentally.  If the
         * system clock frequency is changed then the delay value needs to
         * be changed
         */
        SysCtlDelay(22000);
    }
    
    /* Set each servo position to 0 to start, with 100 ms delay
     */
    for(servo = 0; servo < NUM_SERVOS; servo++)
    {
        /* Set the servo motion rate */
        Servo_SetMotionParameters(hServo[servo], 200);
        Servo_SetPosition(hServo[servo], 0);
        SysCtlDelay((gSysClock / 10) / 3);
    }


    // MoveAll(0xFFF, 0);
    
    /* In this loop we just move all the servos between +45 and
     * -45 deg (uncalibrated).  There is a 100 ms delay between each
     * servo, so that if observed with a scope each servo does not have
     * the exact same timing.
     */
    while(1)
    {
        /* Move all servos to -45 deg, with 100 ms between each servo
         */
        for(servo = 0; servo < NUM_SERVOS; servo++)
        {
            UpdateRGB();
            MoveOne(servo, -450);
            DelayMs(100);
        }

        /* Now move all servos to +45 deg, with 100 ms delay
         */
        for(servo = 0; servo < NUM_SERVOS; servo++)
        {
            UpdateRGB();
            MoveOne(servo, 450);
            DelayMs(100);
        }

        /* Read the battery voltage and print to the terminal
         */
        uint32_t bat = Servo_ReadBatteryMv();
        UARTprintf("%u.%02u V\n", bat / 1000, (bat % 1000) / 10);
    }
#ifndef ccs // prevent warning from TI ccs compiler
    return(0);
#endif
}
