################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/cpp/CDRMessage.cpp \
../src/cpp/CDRMessageCreator.cpp \
../src/cpp/Endpoint.cpp \
../src/cpp/HistoryCache.cpp \
../src/cpp/MessageReceiver.cpp \
../src/cpp/RTPSWriter.cpp \
../src/cpp/ReaderLocator.cpp \
../src/cpp/StatelessWriter.cpp 

OBJS += \
./src/cpp/CDRMessage.o \
./src/cpp/CDRMessageCreator.o \
./src/cpp/Endpoint.o \
./src/cpp/HistoryCache.o \
./src/cpp/MessageReceiver.o \
./src/cpp/RTPSWriter.o \
./src/cpp/ReaderLocator.o \
./src/cpp/StatelessWriter.o 

CPP_DEPS += \
./src/cpp/CDRMessage.d \
./src/cpp/CDRMessageCreator.d \
./src/cpp/Endpoint.d \
./src/cpp/HistoryCache.d \
./src/cpp/MessageReceiver.d \
./src/cpp/RTPSWriter.d \
./src/cpp/ReaderLocator.d \
./src/cpp/StatelessWriter.d 


# Each subdirectory must supply rules for building sources it contributes
src/cpp/%.o: ../src/cpp/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


