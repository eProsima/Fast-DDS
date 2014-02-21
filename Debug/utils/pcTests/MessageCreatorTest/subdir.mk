################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../utils/pcTests/MessageCreatorTest/MessageCreatorTest.cpp 

OBJS += \
./utils/pcTests/MessageCreatorTest/MessageCreatorTest.o 

CPP_DEPS += \
./utils/pcTests/MessageCreatorTest/MessageCreatorTest.d 


# Each subdirectory must supply rules for building sources it contributes
utils/pcTests/MessageCreatorTest/%.o: ../utils/pcTests/MessageCreatorTest/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


