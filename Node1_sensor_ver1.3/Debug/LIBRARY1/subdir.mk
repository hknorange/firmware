################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../LIBRARY1/flash.c 

OBJS += \
./LIBRARY1/flash.o 

C_DEPS += \
./LIBRARY1/flash.d 


# Each subdirectory must supply rules for building sources it contributes
LIBRARY1/%.o LIBRARY1/%.su: ../LIBRARY1/%.c LIBRARY1/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"F:/KIN_SOURCE/Software/Node1_sensor_ver1.2/LIBRARY1" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-LIBRARY1

clean-LIBRARY1:
	-$(RM) ./LIBRARY1/flash.d ./LIBRARY1/flash.o ./LIBRARY1/flash.su

.PHONY: clean-LIBRARY1

