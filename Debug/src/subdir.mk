################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/video_track.cpp \
../src/video_encoder.cpp \
../src/webm_muxer.cpp \
../src/video_encoder_instance.cpp \
../src/video_encoder_module.cpp 

OBJS += \
./src/video_track.o \
./src/video_encoder.o \
./src/webm_muxer.o \
./src/video_encoder_instance.o \
./src/video_encoder_module.o 

CPP_DEPS += \
./src/video_track.d \
./src/video_encoder.d \
./src/webm_muxer.d \
./src/video_encoder_instance.d \
./src/video_encoder_module.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	pnacl-clang++ -std=gnu++11 -I$(NACL_SDK_ROOT)/include -I$(NACL_SDK_ROOT)/toolchain/linux_pnacl/include/c++/v1 -O0  -Wall -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


