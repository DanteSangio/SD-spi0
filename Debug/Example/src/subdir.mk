################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Example/src/SD.c \
../Example/src/cr_startup_lpc175x_6x.c \
../Example/src/crp.c \
../Example/src/delay.c \
../Example/src/fat32.c \
../Example/src/gpio.c \
../Example/src/sdcard.c \
../Example/src/spi.c \
../Example/src/sysinit.c \
../Example/src/uart.c 

OBJS += \
./Example/src/SD.o \
./Example/src/cr_startup_lpc175x_6x.o \
./Example/src/crp.o \
./Example/src/delay.o \
./Example/src/fat32.o \
./Example/src/gpio.o \
./Example/src/sdcard.o \
./Example/src/spi.o \
./Example/src/sysinit.o \
./Example/src/uart.o 

C_DEPS += \
./Example/src/SD.d \
./Example/src/cr_startup_lpc175x_6x.d \
./Example/src/crp.d \
./Example/src/delay.d \
./Example/src/fat32.d \
./Example/src/gpio.d \
./Example/src/sdcard.d \
./Example/src/spi.d \
./Example/src/sysinit.d \
./Example/src/uart.d 


# Each subdirectory must supply rules for building sources it contributes
Example/src/%.o: ../Example/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -D__CODE_RED -DCORE_M3 -D__USE_LPCOPEN -DNO_BOARD_LIB -D__LPC17XX__ -D__REDLIB__ -I"D:\Facultad\Digitales II\Repositorio MCU\lpc_chip_175x_6x" -I"C:\Users\kevin\git\SD-spi0\Example\inc" -I"D:\Facultad\Digitales II\Repositorio MCU\lpc_chip_175x_6x\inc" -I"C:\Users\kevin\git\SD-spi0\freeRTOS\inc" -O0 -fno-common -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m3 -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


