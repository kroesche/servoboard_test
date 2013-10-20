
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"

#include "driverlib/fpu.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"


#include "utils/uartstdio.h"
#include "utils/ustdlib.h"

#include "stellaris_drivers/servo-wt.h"

#define NUM_SERVOS 12
static ServoHandle_t hServo[NUM_SERVOS];
static uint32_t gSysClock;
static volatile uint32_t gInMotion;

typedef struct
{
    int32_t timer;
    int32_t half;
} ServoInfo_t;

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

#define S0 (1 << 0)
#define S1 (1 << 1)
#define S2 (1 << 2)
#define S3 (1 << 3)
#define S4 (1 << 4)
#define S5 (1 << 5)
#define S6 (1 << 6)
#define S7 (1 << 7)
#define S8 (1 << 8)
#define S9 (1 << 9)
#define S10 (1 << 10)
#define S11 (1 << 11)

#define HIP_A1 0
#define HIP_A2 4
#define HIP_A3 8
#define HIP_B1 2
#define HIP_B2 6
#define HIP_B3 10
#define HIP_ALL (S0|S4|S8|S2|S6|S10)
#define FEET_A (S1|S5|S9)
#define FEET_B (S3|S7|S11)
#define FEET_ALL (FEET_A|FEET_B)
#define FOOT_UP 450
#define FOOT_DOWN -50
#define HIP_A1_FORWARD 0
#define HIP_A2_FORWARD 500
#define HIP_A3_FORWARD 250
#define HIP_A1_BACK -500
#define HIP_A2_BACK 0
#define HIP_A3_BACK -250
#define HIP_B1_FORWARD 250
#define HIP_B2_FORWARD 500
#define HIP_B3_FORWARD 0
#define HIP_B1_BACK -250
#define HIP_B2_BACK 0
#define HIP_B3_BACK -500
#define FOOT_A1 1
#define FOOT_B1 3
#define FOOT_A2 5
#define FOOT_B2 7
#define FOOT_A3 9
#define FOOT_B3 11

#define STATE_IDLE 1
#define STATE_WALKING 2
#define STATE_ENDWALK 3
#define STATE_TRICK1 4
#define STATE_TRICK2 5
#define STATE_TRICK2_CONTINUE 6



static void
MotionCallback(void *pData)
{
    uint32_t servo = (uint32_t)pData;
    HWREGBITW(&gInMotion, servo) = 0;
}

static void
WaitOne(uint32_t servo)
{
    while(HWREGBITW(&gInMotion, servo))
    {}
}

static void
WaitAll(uint32_t mask)
{
    while(gInMotion & mask)
    {}
}

static void
MoveOne(uint32_t servo, int32_t cdeg)
{
    WaitOne(servo);
    HWREGBITW(&gInMotion, servo) = 1;
    Servo_Move(hServo[servo], cdeg, MotionCallback, (void *)servo);
}

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

void
TripodGait(uint32_t count)
{
    while(count--)
    {
        // wait for all done
        WaitAll(0xFFF);
        
        // A foot down, wait
        MoveAll(FEET_A, FOOT_DOWN);
        WaitAll(FEET_A);
        
        // B foot up, wait
        MoveAll(FEET_B, FOOT_UP);
        WaitAll(FEET_B);
        
        // A hip back
        MoveOne(HIP_A1, HIP_A1_BACK);
        MoveOne(HIP_A2, HIP_A2_BACK);
        MoveOne(HIP_A3, HIP_A3_BACK);
        //                WaitAll(0xFFF);
        // B hip forward
        MoveOne(HIP_B1, HIP_B1_FORWARD);
        MoveOne(HIP_B2, HIP_B2_FORWARD);
        MoveOne(HIP_B3, HIP_B3_FORWARD);
        
        WaitAll(0xFFF);
        
        // B foot down, wait
        MoveAll(FEET_B, FOOT_DOWN);
        WaitAll(FEET_B);
        
        // A foot up, wait
        MoveAll(FEET_A, FOOT_UP);
        WaitAll(FEET_A);
        
        // B hip back
        MoveOne(HIP_B1, HIP_B1_BACK);
        MoveOne(HIP_B2, HIP_B2_BACK);
        MoveOne(HIP_B3, HIP_B3_BACK);
        //                WaitAll(0xFFF);
        // A hip forward
        MoveOne(HIP_A1, HIP_A1_FORWARD);
        MoveOne(HIP_A2, HIP_A2_FORWARD);
        MoveOne(HIP_A3, HIP_A3_FORWARD);
    }
    WaitAll(0xFFF);
    MoveAll(0xFFF, 0);
    WaitAll(0xFFF);
}

void
LeftRight(uint32_t count)
{
    MoveAll(FEET_ALL, 850);
    while(count--)
    {
        WaitAll(0xFFF);
        
        MoveAll(HIP_ALL, 450);
        WaitAll(HIP_ALL);
        MoveAll(HIP_ALL, -450);
    }
    WaitAll(0xFFF);
    MoveAll(0xFFF, 0);
    WaitAll(0xFFF);
}

void
UpDown(uint32_t count)
{
    MoveAll(0xFFF, 0);
    while(count--)
    {
        WaitAll(0xFFF);
        
        MoveAll(FEET_ALL, 1050);
        WaitAll(FEET_ALL);
        MoveAll(FEET_ALL, -450);
    }
    WaitAll(0xFFF);
    MoveAll(0xFFF, 0);
    WaitAll(0xFFF);
}

void
Wave(uint32_t count)
{
    MoveAll(HIP_ALL, 0);
    MoveAll(FEET_ALL, 700);
    WaitAll(0xFFF);
    SysCtlDelay(gSysClock);
    
    while(count--)
    {
        MoveOne(FOOT_A1, 1050);
        MoveOne(FOOT_B3, 700);
        WaitOne(FOOT_A1);
        
        MoveOne(FOOT_B1, 1050);
        MoveOne(FOOT_A1, 700);
        WaitOne(FOOT_B1);
        
        MoveOne(FOOT_A2, 1050);
        MoveOne(FOOT_B1, 700);
        WaitOne(FOOT_A2);
        
        MoveOne(FOOT_B2, 1050);
        MoveOne(FOOT_A2, 700);
        WaitOne(FOOT_B2);
        
        MoveOne(FOOT_A3, 1050);
        MoveOne(FOOT_B2, 700);
        WaitOne(FOOT_A3);
        
        MoveOne(FOOT_B3, 1050);
        MoveOne(FOOT_A3, 700);
        WaitOne(FOOT_B3);
    }
    WaitAll(0xFFF);
    MoveAll(0xFFF, 0);
    WaitAll(0xFFF);
}

void
HipsLeft(void)
{
    MoveAll(HIP_ALL, 450);
    WaitAll(HIP_ALL);
}

void
HipsRight(void)
{
    MoveAll(HIP_ALL, -450);
    WaitAll(HIP_ALL);
}

void
HipsCenter(void)
{
    MoveAll(HIP_ALL, 0);
    WaitAll(HIP_ALL);
}

void
FeetUp(void)
{
    MoveAll(FEET_ALL, 500);
    WaitAll(FEET_ALL);
}

void
FeetUpHigh(void)
{
    MoveAll(FEET_ALL, 1050);
    WaitAll(FEET_ALL);
}

void
FeetDown(void)
{
    MoveAll(FEET_ALL, 100);
    WaitAll(FEET_ALL);
}

void
FeetIn(void)
{
    MoveAll(FEET_ALL, -450);
    WaitAll(FEET_ALL);
}

void
Delay(uint32_t secs)
{
    SysCtlDelay((gSysClock * secs) / 3);
}

int
main(void)
{
    int ret;
    
    FPUStackingDisable();
    
    // 40 MHz
    SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
    gSysClock = SysCtlClockGet();
    
    //
    // Initialize the UART.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTStdioInit(0);
    
    UARTprintf("\n\nServoBoard-Test\n---------------\n");
    
    Servo_BatteryInit(0, 0);
    
    /* Initialize servos for 20 msec
     */
    ret = Servo_Init(gSysClock, 20000);
    if(ret)
    {
        UARTprintf("error calling ServoInit\n");
        return 0;
    }
    
    for(int servo = 0; servo < NUM_SERVOS; servo++)
    {
        hServo[servo] = Servo_Config(servoInfo[servo].timer, servoInfo[servo].half);
        if(hServo[servo] == 0)
        {
            UARTprintf("error config servo %d\n", servo);
            return 0;
        }
        Servo_SetMotionParameters(hServo[servo], 140);
        SysCtlDelay(22000);
    }
    
    for(int servo = 1; servo < NUM_SERVOS; servo += 2)
    {
        Servo_Calibrate(hServo[servo], 1000, 1800, 1);
    }
#if 0
    Servo_Calibrate(hServo[5], 1000, 1900, 1);
    
    Servo_Calibrate(hServo[6], 1000, 1500, 1);
    Servo_Calibrate(hServo[8], 1000, 1500, 1);
    Servo_Calibrate(hServo[10], 1000, 1500, 1);
#endif
    
    for(int servo = 0; servo < NUM_SERVOS; servo++)
    {
        Servo_SetPosition(hServo[servo], 0);
        SysCtlDelay((gSysClock / 10) / 3);
    }
    
    MoveAll(0xFFF, 0);
    
    /*
     * PrepStep
     * -foot up
     * -wait
     * -hip forward
     *
     * MakeStep
     * -foot down
     * -wait
     * -hip back
     */
    while(1)
    {
        Delay(5);
        FeetUp();
        Delay(2);
        FeetUpHigh();
        Delay(2);
        FeetDown();
        Delay(5);
        FeetIn();
        Delay(5);
        FeetDown();
        Delay(2);
        HipsLeft();
        Delay(2);
        HipsCenter();
        Delay(2);
        HipsRight();
        Delay(2);
        HipsCenter();
        Delay(15);

        UpDown(3);
        Delay(15);
        Wave(4);
        Delay(15);
        LeftRight(2);
        Delay(10);
        
        TripodGait(5);
        Delay(10);
        
        uint32_t bat = Servo_ReadBatteryMv();
        UARTprintf("%u.%02u V\n", bat / 1000, (bat % 1000) / 10);
//        usnprintf(gDispStringBuf, sizeof(gDispStringBuf), "  BAT: %u.%02u V  ", bat / 1000, (bat % 1000) / 10);
        //        uint32_t bat = BatmonReadRaw();
        //        usnprintf(gDispStringBuf, sizeof(gDispStringBuf), "   %u   ", bat);
//        usnprintf(gDispStringBuf, sizeof(gDispStringBuf), "   %s   ", stateString);
    }
    
    return(0);
}
