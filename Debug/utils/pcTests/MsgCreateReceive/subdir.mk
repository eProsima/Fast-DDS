################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../utils/pcTests/MsgCreateReceive/MessageCreatorTest.cpp \
../utils/pcTests/MsgCreateReceive/aux_test.cpp 

OBJS += \
./utils/pcTests/MsgCreateReceive/MessageCreatorTest.o \
./utils/pcTests/MsgCreateReceive/aux_test.o 

CPP_DEPS += \
./utils/pcTests/MsgCreateReceive/MessageCreatorTest.d \
./utils/pcTests/MsgCreateReceive/aux_test.d 


# Each subdirectory must supply rules for building sources it contributes
utils/pcTests/MsgCreateReceive/%.o: ../utils/pcTests/MsgCreateReceive/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


