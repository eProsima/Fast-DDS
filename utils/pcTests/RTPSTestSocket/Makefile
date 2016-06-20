# Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

BASEDIR=.
OUTDIR= $(BASEDIR)/output
CC=gcc
CPP=g++
LN=g++
AR=ar
CP=cp

COMMON_CFLAGS= -c -Wall -D__LITTLE_ENDIAN__

## CHOOSE HERE BETWEEN 32 and 64 bit architecture

##32 BIT ARCH:
#COMMON_CFLAGS+= -m32 -fpic
#LDFLAGS=-m32
#EPROSIMA_TARGET=i86Linux2.6gcc

#64BIT ARCH:
COMMON_CFLAGS+= -m64 -fpic
LDFLAGS=-m64
EPROSIMA_TARGET=x64Linux2.6gcc

CFLAGS = $(COMMON_CFLAGS) -O2
CFLAGS_DEBUG= $(COMMON_CFLAGS) -g -D__DEBUG

STATETEST_OUTDIR= $(OUTDIR)/LatencyTest
STATETEST_OUTDIR_DEBUG = $(STATETEST_OUTDIR)/debug
STATETEST_OUTDIR_RELEASE = $(STATETEST_OUTDIR)/release

STATETEST_SED_OUTPUT_DIR_DEBUG= $(subst /,\\/,$(STATETEST_OUTDIR_DEBUG))
STATETEST_SED_OUTPUT_DIR_RELEASE= $(subst /,\\/,$(STATETEST_OUTDIR_RELEASE))

STATETEST_CFLAGS = $(CFLAGS) -std=c++0x 
STATETEST_CFLAGS_DEBUG = $(CFLAGS_DEBUG) -std=c++0x  

STATETEST_TARGET_DEBUG= $(BASEDIR)/bin/$(EPROSIMA_TARGET)/RTPSTestd
STATETEST_TARGET= $(BASEDIR)/bin/$(EPROSIMA_TARGET)/RTPSTest

STATETEST_INCLUDE_DIRS= $(INCLUDE_DIRS) -I$(BASEDIR)/../../../include \
                         -I$(BASEDIR)/../../../thirdparty/eprosima-common-code

STATETEST_LIBS_DEBUG=  $(LIBS_DEBUG) -L$(BASEDIR)/../../../lib/$(EPROSIMA_TARGET)  -lfastrtpsd -lboost_system -lboost_thread -lboost_date_time
STATETEST_LIBS_RELEASE=  $(LIBS) -L$(BASEDIR)/../../../lib/$(EPROSIMA_TARGET)  -lfastrtps -lboost_system -lboost_thread -lboost_date_time

STATETEST_SRC_CFILES=

STATETEST_SRC_CPPFILES= $(BASEDIR)/../../useTests/RTPSTest_as_socket/TestWriterSocket.cpp \
					$(BASEDIR)/../../useTests/RTPSTest_as_socket/TestReaderSocket.cpp \
					$(BASEDIR)/../../useTests/RTPSTest_as_socket/main_RTPSTest.cpp 

# Project sources are copied to the current directory
STATETEST_SRCS= $(STATETEST_SRC_CFILES) $(STATETEST_SRC_CPPFILES)

# Source directories
STATETEST_SOURCES_DIRS_AUX= $(foreach srcdir, $(dir $(STATETEST_SRCS)), $(srcdir))
STATETEST_SOURCES_DIRS= $(shell echo $(STATETEST_SOURCES_DIRS_AUX) | tr " " "\n" | sort | uniq | tr "\n" " ")

STATETEST_OBJS_DEBUG = $(foreach obj,$(notdir $(addsuffix .o, $(STATETEST_SRCS))), $(STATETEST_OUTDIR_DEBUG)/$(obj))
STATETEST_DEPS_DEBUG = $(foreach dep,$(notdir $(addsuffix .d, $(STATETEST_SRCS))), $(STATETEST_OUTDIR_DEBUG)/$(dep))
STATETEST_OBJS_RELEASE = $(foreach obj,$(notdir $(addsuffix .o, $(STATETEST_SRCS))), $(STATETEST_OUTDIR_RELEASE)/$(obj))
STATETEST_DEPS_RELEASE = $(foreach dep,$(notdir $(addsuffix .d, $(STATETEST_SRCS))), $(STATETEST_OUTDIR_RELEASE)/$(dep))

OBJS+= $(STATETEST_OBJS_DEBUG) $(STATETEST_OBJS_RELEASE)
DEPS+= $(STATETEST_DEPS_DEBUG) $(STATETEST_DEPS_RELEASE)

.PHONY: LatencyTest checkLatencyTestDirectories

LatencyTest: checkLatencyTestDirectories $(STATETEST_TARGET_DEBUG) $(STATETEST_TARGET)

checkLatencyTestDirectories:
	@mkdir -p $(OUTDIR)
	@mkdir -p $(STATETEST_OUTDIR)
	@mkdir -p $(STATETEST_OUTDIR_DEBUG)
	@mkdir -p $(STATETEST_OUTDIR_RELEASE)
	@mkdir -p $(BASEDIR)/bin
	@mkdir -p $(BASEDIR)/bin/$(EPROSIMA_TARGET)

$(STATETEST_TARGET_DEBUG): $(STATETEST_OBJS_DEBUG)
	$(LN) $(LDFLAGS) -o $(STATETEST_TARGET_DEBUG) $(STATETEST_OBJS_DEBUG) $(LIBRARY_PATH) $(STATETEST_LIBS_DEBUG) $(STATETEST_STATIC_LIBS)

$(STATETEST_TARGET): $(STATETEST_OBJS_RELEASE)
	$(LN) $(LDFLAGS) -o $(STATETEST_TARGET) $(STATETEST_OBJS_RELEASE) $(LIBRARY_PATH) $(STATETEST_LIBS_RELEASE) $(STATETEST_STATIC_LIBS)

vpath %.c $(STATETEST_SOURCES_DIRS)
vpath %.cpp $(STATETEST_SOURCES_DIRS)

$(STATETEST_OUTDIR_DEBUG)/%.o:%.c
	@echo Calculating dependencies \(DEBUG mode\) $<
	@$(CC) $(STATETEST_CFLAGS_DEBUG) -MM $(STATETEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(STATETEST_SED_OUTPUT_DIR_DEBUG)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(DEBUG mode\) $<  
	@$(CC) $(STATETEST_CFLAGS_DEBUG) $(STATETEST_INCLUDE_DIRS) $< -o $@

$(STATETEST_OUTDIR_RELEASE)/%.o:%.c
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CC) $(STATETEST_CFLAGS) -MM $(STATETEST_CFLAGS) $(STATETEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(STATETEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CC) $(STATETEST_CFLAGS) $(STATETEST_INCLUDE_DIRS) $< -o $@

$(STATETEST_OUTDIR_DEBUG)/%.c.o:%.c
	@echo Calculating dependencies \(DEBUG mode\) $<
	@$(CC) $(STATETEST_CFLAGS_DEBUG) -MM $(STATETEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(STATETEST_SED_OUTPUT_DIR_DEBUG)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(DEBUG mode\) $<  
	@$(CC) $(STATETEST_CFLAGS_DEBUG) $(STATETEST_INCLUDE_DIRS) $< -o $@

$(STATETEST_OUTDIR_RELEASE)/%.c.o:%.c
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CC) $(STATETEST_CFLAGS) -MM $(STATETEST_CFLAGS) $(STATETEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(STATETEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CC) $(STATETEST_CFLAGS) $(STATETEST_INCLUDE_DIRS) $< -o $@

$(STATETEST_OUTDIR_DEBUG)/%.cpp.o:%.cpp
	@echo Calculating dependencies \(DEBUG mode\) $<
	@$(CPP) $(STATETEST_CFLAGS_DEBUG) -MM $(STATETEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(STATETEST_SED_OUTPUT_DIR_DEBUG)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(DEBUG mode\) $<
	@$(CPP) $(STATETEST_CFLAGS_DEBUG) $(STATETEST_INCLUDE_DIRS) $< -o $@

$(STATETEST_OUTDIR_RELEASE)/%.cpp.o:%.cpp
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CPP) $(STATETEST_CFLAGS) -MM $(STATETEST_CFLAGS) $(STATETEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(STATETEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CPP) $(STATETEST_CFLAGS) $(STATETEST_INCLUDE_DIRS) $< -o $@

.PHONY: all

all: State

clean:
	@rm -f $(OBJS)
	@rm -f $(DEPS)

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPS)
endif
