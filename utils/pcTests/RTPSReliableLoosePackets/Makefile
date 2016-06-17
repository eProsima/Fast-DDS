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


FASTCDR_DIR = $(BASEDIR)/../../../thirdparty/fastcdr


HELLOWORLDTEST_OUTDIR= $(OUTDIR)/RTPSTest
HELLOWORLDTEST_OUTDIR_DEBUG = $(HELLOWORLDTEST_OUTDIR)/debug
HELLOWORLDTEST_OUTDIR_RELEASE = $(HELLOWORLDTEST_OUTDIR)/release

HELLOWORLDTEST_SED_OUTPUT_DIR_DEBUG= $(subst /,\\/,$(HELLOWORLDTEST_OUTDIR_DEBUG))
HELLOWORLDTEST_SED_OUTPUT_DIR_RELEASE= $(subst /,\\/,$(HELLOWORLDTEST_OUTDIR_RELEASE))

HELLOWORLDTEST_CFLAGS = $(CFLAGS) -std=c++0x 
HELLOWORLDTEST_CFLAGS_DEBUG = $(CFLAGS_DEBUG) -std=c++0x 

HELLOWORLDTEST_TARGET_DEBUG= $(BASEDIR)/bin/$(EPROSIMA_TARGET)/RTPSTestd
HELLOWORLDTEST_TARGET= $(BASEDIR)/bin/$(EPROSIMA_TARGET)/RTPSTest

HELLOWORLDTEST_INCLUDE_DIRS= $(INCLUDE_DIRS) -I$(BASEDIR)/../../../include \
                         						-I$(BASEDIR)/../../../thirdparty/eprosima-common-code \
                         						-I$(FASTCDR_DIR)/include

HELLOWORLDTEST_LIBS_DEBUG= $(LIBS_DEBUG) -L$(BASEDIR)/../../../lib/$(EPROSIMA_TARGET) -lfastrtpsd -lboost_system -lboost_thread -lboost_date_time \
										 -L$(FASTCDR_DIR)/lib/$(EPROSIMA_TARGET) -lfastcdrd
HELLOWORLDTEST_LIBS_RELEASE= $(LIBS) -L$(BASEDIR)/../../../lib/$(EPROSIMA_TARGET) -lfastrtps -lboost_system -lboost_thread -lboost_date_time\
										-L$(FASTCDR_DIR)/lib/$(EPROSIMA_TARGET) -lfastcdr

HELLOWORLDTEST_SRC_CFILES=

HELLOWORLDTEST_SRC_CPPFILES= $(BASEDIR)/main_RTPSTest.cpp \
								$(BASEDIR)/TestReaderRegistered.cpp \
								$(BASEDIR)/TestWriterRegistered.cpp 


# Project sources are copied to the current directory
HELLOWORLDTEST_SRCS= $(HELLOWORLDTEST_SRC_CFILES) $(HELLOWORLDTEST_SRC_CPPFILES)

# Source directories
HELLOWORLDTEST_SOURCES_DIRS_AUX= $(foreach srcdir, $(dir $(HELLOWORLDTEST_SRCS)), $(srcdir))
HELLOWORLDTEST_SOURCES_DIRS= $(shell echo $(HELLOWORLDTEST_SOURCES_DIRS_AUX) | tr " " "\n" | sort | uniq | tr "\n" " ")

HELLOWORLDTEST_OBJS_DEBUG = $(foreach obj,$(notdir $(addsuffix .o, $(HELLOWORLDTEST_SRCS))), $(HELLOWORLDTEST_OUTDIR_DEBUG)/$(obj))
HELLOWORLDTEST_DEPS_DEBUG = $(foreach dep,$(notdir $(addsuffix .d, $(HELLOWORLDTEST_SRCS))), $(HELLOWORLDTEST_OUTDIR_DEBUG)/$(dep))
HELLOWORLDTEST_OBJS_RELEASE = $(foreach obj,$(notdir $(addsuffix .o, $(HELLOWORLDTEST_SRCS))), $(HELLOWORLDTEST_OUTDIR_RELEASE)/$(obj))
HELLOWORLDTEST_DEPS_RELEASE = $(foreach dep,$(notdir $(addsuffix .d, $(HELLOWORLDTEST_SRCS))), $(HELLOWORLDTEST_OUTDIR_RELEASE)/$(dep))

OBJS+= $(HELLOWORLDTEST_OBJS_DEBUG) $(HELLOWORLDTEST_OBJS_RELEASE)
DEPS+= $(HELLOWORLDTEST_DEPS_DEBUG) $(HELLOWORLDTEST_DEPS_RELEASE)

.PHONY: HelloWorldTest checkHelloWorldTestDirectories

HelloWorldTest: checkHelloWorldTestDirectories $(HELLOWORLDTEST_TARGET_DEBUG) $(HELLOWORLDTEST_TARGET)

checkHelloWorldTestDirectories:
	@mkdir -p $(OUTDIR)
	@mkdir -p $(HELLOWORLDTEST_OUTDIR)
	@mkdir -p $(HELLOWORLDTEST_OUTDIR_DEBUG)
	@mkdir -p $(HELLOWORLDTEST_OUTDIR_RELEASE)
	@mkdir -p $(BASEDIR)/bin
	@mkdir -p $(BASEDIR)/bin/$(EPROSIMA_TARGET)

$(HELLOWORLDTEST_TARGET_DEBUG): $(HELLOWORLDTEST_OBJS_DEBUG)
	$(LN) $(LDFLAGS) -o $(HELLOWORLDTEST_TARGET_DEBUG) $(HELLOWORLDTEST_OBJS_DEBUG) $(LIBRARY_PATH) $(HELLOWORLDTEST_LIBS_DEBUG) $(HELLOWORLDTEST_STATIC_LIBS)

$(HELLOWORLDTEST_TARGET): $(HELLOWORLDTEST_OBJS_RELEASE)
	$(LN) $(LDFLAGS) -o $(HELLOWORLDTEST_TARGET) $(HELLOWORLDTEST_OBJS_RELEASE) $(LIBRARY_PATH) $(HELLOWORLDTEST_LIBS_RELEASE) $(HELLOWORLDTEST_STATIC_LIBS)

vpath %.c $(HELLOWORLDTEST_SOURCES_DIRS)
vpath %.cpp $(HELLOWORLDTEST_SOURCES_DIRS)

$(HELLOWORLDTEST_OUTDIR_DEBUG)/%.o:%.c
	@echo Calculating dependencies \(DEBUG mode\) $<
	@$(CC) $(HELLOWORLDTEST_CFLAGS_DEBUG) -MM $(HELLOWORLDTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(HELLOWORLDTEST_SED_OUTPUT_DIR_DEBUG)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(DEBUG mode\) $<  
	@$(CC) $(HELLOWORLDTEST_CFLAGS_DEBUG) $(HELLOWORLDTEST_INCLUDE_DIRS) $< -o $@

$(HELLOWORLDTEST_OUTDIR_RELEASE)/%.o:%.c
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CC) $(HELLOWORLDTEST_CFLAGS) -MM $(HELLOWORLDTEST_CFLAGS) $(HELLOWORLDTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(HELLOWORLDTEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CC) $(HELLOWORLDTEST_CFLAGS) $(HELLOWORLDTEST_INCLUDE_DIRS) $< -o $@

$(HELLOWORLDTEST_OUTDIR_DEBUG)/%.c.o:%.c
	@echo Calculating dependencies \(DEBUG mode\) $<
	@$(CC) $(HELLOWORLDTEST_CFLAGS_DEBUG) -MM $(HELLOWORLDTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(HELLOWORLDTEST_SED_OUTPUT_DIR_DEBUG)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(DEBUG mode\) $<  
	@$(CC) $(HELLOWORLDTEST_CFLAGS_DEBUG) $(HELLOWORLDTEST_INCLUDE_DIRS) $< -o $@

$(HELLOWORLDTEST_OUTDIR_RELEASE)/%.c.o:%.c
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CC) $(HELLOWORLDTEST_CFLAGS) -MM $(HELLOWORLDTEST_CFLAGS) $(HELLOWORLDTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(HELLOWORLDTEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CC) $(HELLOWORLDTEST_CFLAGS) $(HELLOWORLDTEST_INCLUDE_DIRS) $< -o $@

$(HELLOWORLDTEST_OUTDIR_DEBUG)/%.cpp.o:%.cpp
	@echo Calculating dependencies \(DEBUG mode\) $<
	@$(CPP) $(HELLOWORLDTEST_CFLAGS_DEBUG) -MM $(HELLOWORLDTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(HELLOWORLDTEST_SED_OUTPUT_DIR_DEBUG)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(DEBUG mode\) $<
	@$(CPP) $(HELLOWORLDTEST_CFLAGS_DEBUG) $(HELLOWORLDTEST_INCLUDE_DIRS) $< -o $@

$(HELLOWORLDTEST_OUTDIR_RELEASE)/%.cpp.o:%.cpp
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CPP) $(HELLOWORLDTEST_CFLAGS) -MM $(HELLOWORLDTEST_CFLAGS) $(HELLOWORLDTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(HELLOWORLDTEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CPP) $(HELLOWORLDTEST_CFLAGS) $(HELLOWORLDTEST_INCLUDE_DIRS) $< -o $@

.PHONY: all

all: State

clean:
	@rm -f $(OBJS)
	@rm -f $(DEPS)

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPS)
endif
