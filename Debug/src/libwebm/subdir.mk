################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/libwebm/mkvmuxer.cpp \
../src/libwebm/mkvmuxerutil.cpp \
../src/libwebm/mkvparser.cpp \
../src/libwebm/mkvreader.cpp \
../src/libwebm/mkvwriter.cpp 

CC_SRCS += \
../src/libwebm/vttdemux.cc \
../src/libwebm/vttreader.cc \
../src/libwebm/webvttparser.cc 

OBJS += \
./src/libwebm/mkvmuxer.o \
./src/libwebm/mkvmuxerutil.o \
./src/libwebm/mkvparser.o \
./src/libwebm/mkvreader.o \
./src/libwebm/mkvwriter.o \
./src/libwebm/vttdemux.o \
./src/libwebm/vttreader.o \
./src/libwebm/webvttparser.o 

CC_DEPS += \
./src/libwebm/vttdemux.d \
./src/libwebm/vttreader.d \
./src/libwebm/webvttparser.d 

CPP_DEPS += \
./src/libwebm/mkvmuxer.d \
./src/libwebm/mkvmuxerutil.d \
./src/libwebm/mkvparser.d \
./src/libwebm/mkvreader.d \
./src/libwebm/mkvwriter.d 


# Each subdirectory must supply rules for building sources it contributes
src/libwebm/%.o: ../src/libwebm/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	pnacl-clang++ -std=gnu++11 -I$(NACL_SDK_ROOT)/include -I$(NACL_SDK_ROOT)/toolchain/linux_pnacl/include/c++/v1 -O0 -Wall -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/libwebm/%.o: ../src/libwebm/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	pnacl-clang++ -std=gnu++11 -I$(NACL_SDK_ROOT)/include -I$(NACL_SDK_ROOT)/toolchain/linux_pnacl/include/c++/v1 -O0 -Wall -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


