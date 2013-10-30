
# name of the program
APPNAME=servoboard_test

# list of C source files
SRCS=servoboard_test.c
SRCS+=startup_gcc.c
SRCS+=../stellaris_drivers/servo-wt.c
SRCS+=${STELLARISWARE_ROOT}/utils/uartstdio.c
SRCS+=${STELLARISWARE_ROOT}/utils/ustdlib.c

# make search path for source files
VPATH=../stellaris_drivers
VPATH+=${STELLARISWARE_ROOT}/utils

# list of all the include paths
INCLUDES=-I.
INCLUDES+=-I${STELLARISWARE_ROOT}
INCLUDES+=-I..

# list of libraries to include in the link
LIBS=${STELLARISWARE_ROOT}/driverlib/gcc-${CORTEXM}/libdriver-${CORTEXM}.a

# defines the part number
PART=LM4F120H5QR

# defines the part class.  this is used for ROM calls.  this can be left
# blank if ROM not used
PART_CLASS=-DTARGET_IS_BLIZZARD_RA2

# the type of CORTEX-M, either cm3 or cm4f
CORTEXM=cm4f

# path to StellarisWare installed on the system
STELLARISWARE_ROOT=/Users/joe/Dev/StellarisWare

# the name of the linker script
LDSCRIPT=lm4f120.ld

# name of entry point
ENTRY=ResetISR

# name of the output directory for objs and binaries
OUT=gcc/

# name of map file
MAPFILE=${OUT}${APPNAME}.map

#
# Set this to the path of the toolchain.  If it is already on your path
# you may be able to leave this blank.  However, if using xcode to invoke
# the makefile, it is hard to make xcode find your compiler even when on the
# path so I defined it here.
#
TOOLPATH=/usr/local/gcc-arm-none-eabi-4_7-2012q4/bin/

# prefix name for tool chain
TOOLCHAIN=arm-none-eabi

# part definition macro needed by some StellarisWare files
PARTMACRO=PART_${PART}

# define the tools program names
CC=${TOOLPATH}${TOOLCHAIN}-gcc
AR=${TOOLPATH}${TOOLCHAIN}-ar
LD=${TOOLPATH}${TOOLCHAIN}-ld
OBJCOPY=${TOOLPATH}${TOOLCHAIN}-objcopy
CC=${TOOLPATH}${TOOLCHAIN}-gcc

# set up processor and floating point options per the core type
ifeq (${CORTEXM}, cm4f)
CPU=-mcpu=cortex-m4
FPU=-mfpu=fpv4-sp-d16 -mfloat-abi=softfp
else
CPU=-mcpu=cortex-m3
FPU=
endif

# stellaris standard C flags
CFLAGS=-g -mthumb ${CPU} ${FPU} -Os -ffunction-sections -fdata-sections -MD -std=c99 -Wall -pedantic

# stellaris standard linker flags
LDFLAG=--gc-sections

# some StellarisWare files need the part macro defined and expect gcc to be defined
CFLAGS+=-D${PARTMACRO}
CFLAGS+=-Dgcc
CFLAGS+=${PART_CLASS}

# get gcc library locations
LIBM=${shell ${CC} ${CFLAGS} -print-file-name=libm.a}
LIBC=${shell ${CC} ${CFLAGS} -print-file-name=libc.a}
LIBGCC=${shell ${CC} ${CFLAGS} -print-libgcc-file-name}

# derive the object file list from the C sources
OBJS=$(addprefix ${OUT}, $(notdir $(SRCS:%.c=%.o)))

# rule for compiling
gcc/%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -o ${@} -c ${<}

# rule for linking
%.axf:
	${LD} -T ${LDSCRIPT} --entry ${ENTRY} ${LDFLAGS} -Map ${MAPFILE} -o ${@} ${OBJS} ${LIBS} '${LIBM}' '${LIBC}' '${LIBGCC}'
	${OBJCOPY} -O binary ${@} ${@:.axf=.bin}

# rule to build all
all: ${OUT} ${OUT}${APPNAME}.axf

#rule for cleaning
clean:
	rm -rf ${OUT}
	
# rule to make the output directory
${OUT}:
	mkdir ${OUT}

# dependency rule for building app
${OUT}${APPNAME}.axf: ${OBJS} ${LIBS} ${LDSCRIPT} Makefile ${OUT}

# include compiler generated dependencies
ifneq ($(MAKECMDGOALS), clean)
-include $(SRCS:%.c=$(OUT)%.d)
endif
