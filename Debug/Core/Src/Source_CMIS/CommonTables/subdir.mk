################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Source_CMIS/CommonTables/arm_common_tables.c \
../Core/Src/Source_CMIS/CommonTables/arm_common_tables_f16.c \
../Core/Src/Source_CMIS/CommonTables/arm_const_structs.c \
../Core/Src/Source_CMIS/CommonTables/arm_const_structs_f16.c \
../Core/Src/Source_CMIS/CommonTables/arm_mve_tables.c \
../Core/Src/Source_CMIS/CommonTables/arm_mve_tables_f16.c \
../Core/Src/Source_CMIS/CommonTables/arm_neon_tables.c \
../Core/Src/Source_CMIS/CommonTables/arm_neon_tables_f16.c 

OBJS += \
./Core/Src/Source_CMIS/CommonTables/arm_common_tables.o \
./Core/Src/Source_CMIS/CommonTables/arm_common_tables_f16.o \
./Core/Src/Source_CMIS/CommonTables/arm_const_structs.o \
./Core/Src/Source_CMIS/CommonTables/arm_const_structs_f16.o \
./Core/Src/Source_CMIS/CommonTables/arm_mve_tables.o \
./Core/Src/Source_CMIS/CommonTables/arm_mve_tables_f16.o \
./Core/Src/Source_CMIS/CommonTables/arm_neon_tables.o \
./Core/Src/Source_CMIS/CommonTables/arm_neon_tables_f16.o 

C_DEPS += \
./Core/Src/Source_CMIS/CommonTables/arm_common_tables.d \
./Core/Src/Source_CMIS/CommonTables/arm_common_tables_f16.d \
./Core/Src/Source_CMIS/CommonTables/arm_const_structs.d \
./Core/Src/Source_CMIS/CommonTables/arm_const_structs_f16.d \
./Core/Src/Source_CMIS/CommonTables/arm_mve_tables.d \
./Core/Src/Source_CMIS/CommonTables/arm_mve_tables_f16.d \
./Core/Src/Source_CMIS/CommonTables/arm_neon_tables.d \
./Core/Src/Source_CMIS/CommonTables/arm_neon_tables_f16.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Source_CMIS/CommonTables/%.o Core/Src/Source_CMIS/CommonTables/%.su Core/Src/Source_CMIS/CommonTables/%.cyclo: ../Core/Src/Source_CMIS/CommonTables/%.c Core/Src/Source_CMIS/CommonTables/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_NUCLEO_64 -DUSE_HAL_DRIVER -DSTM32F446xx -DARM_MATH_CM4 -c -I../Core/Inc -I../Core/Include_CMIS -I../Core/PrivateInclude_CMIS -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/BSP/STM32F4xx-Nucleo -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Source_CMIS-2f-CommonTables

clean-Core-2f-Src-2f-Source_CMIS-2f-CommonTables:
	-$(RM) ./Core/Src/Source_CMIS/CommonTables/arm_common_tables.cyclo ./Core/Src/Source_CMIS/CommonTables/arm_common_tables.d ./Core/Src/Source_CMIS/CommonTables/arm_common_tables.o ./Core/Src/Source_CMIS/CommonTables/arm_common_tables.su ./Core/Src/Source_CMIS/CommonTables/arm_common_tables_f16.cyclo ./Core/Src/Source_CMIS/CommonTables/arm_common_tables_f16.d ./Core/Src/Source_CMIS/CommonTables/arm_common_tables_f16.o ./Core/Src/Source_CMIS/CommonTables/arm_common_tables_f16.su ./Core/Src/Source_CMIS/CommonTables/arm_const_structs.cyclo ./Core/Src/Source_CMIS/CommonTables/arm_const_structs.d ./Core/Src/Source_CMIS/CommonTables/arm_const_structs.o ./Core/Src/Source_CMIS/CommonTables/arm_const_structs.su ./Core/Src/Source_CMIS/CommonTables/arm_const_structs_f16.cyclo ./Core/Src/Source_CMIS/CommonTables/arm_const_structs_f16.d ./Core/Src/Source_CMIS/CommonTables/arm_const_structs_f16.o ./Core/Src/Source_CMIS/CommonTables/arm_const_structs_f16.su ./Core/Src/Source_CMIS/CommonTables/arm_mve_tables.cyclo ./Core/Src/Source_CMIS/CommonTables/arm_mve_tables.d ./Core/Src/Source_CMIS/CommonTables/arm_mve_tables.o ./Core/Src/Source_CMIS/CommonTables/arm_mve_tables.su ./Core/Src/Source_CMIS/CommonTables/arm_mve_tables_f16.cyclo ./Core/Src/Source_CMIS/CommonTables/arm_mve_tables_f16.d ./Core/Src/Source_CMIS/CommonTables/arm_mve_tables_f16.o ./Core/Src/Source_CMIS/CommonTables/arm_mve_tables_f16.su ./Core/Src/Source_CMIS/CommonTables/arm_neon_tables.cyclo ./Core/Src/Source_CMIS/CommonTables/arm_neon_tables.d ./Core/Src/Source_CMIS/CommonTables/arm_neon_tables.o ./Core/Src/Source_CMIS/CommonTables/arm_neon_tables.su ./Core/Src/Source_CMIS/CommonTables/arm_neon_tables_f16.cyclo ./Core/Src/Source_CMIS/CommonTables/arm_neon_tables_f16.d ./Core/Src/Source_CMIS/CommonTables/arm_neon_tables_f16.o ./Core/Src/Source_CMIS/CommonTables/arm_neon_tables_f16.su

.PHONY: clean-Core-2f-Src-2f-Source_CMIS-2f-CommonTables

