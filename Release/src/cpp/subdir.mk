################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/cpp/CDRMessageCreator.cpp \
../src/cpp/MessageReceiver.cpp 

OBJS += \
./src/cpp/CDRMessageCreator.o \
./src/cpp/MessageReceiver.o 

CPP_DEPS += \
./src/cpp/CDRMessageCreator.d \
./src/cpp/MessageReceiver.d 


# Each subdirectory must supply rules for building sources it contributes
src/cpp/%.o: ../src/cpp/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


