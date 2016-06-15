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

CFLAGS = $(COMMON_CFLAGS) -O2 -Wno-unknown-pragmas
CFLAGS_DEBUG= $(COMMON_CFLAGS) -g -D__DEBUG -Wno-unknown-pragmas
RELIABLETEST_OUTDIR= $(OUTDIR)/ReliableTest
RELIABLETEST_OUTDIR_DEBUG = $(RELIABLETEST_OUTDIR)/debug
RELIABLETEST_OUTDIR_RELEASE = $(RELIABLETEST_OUTDIR)/release

RELIABLETEST_SED_OUTPUT_DIR_DEBUG= $(subst /,\\/,$(RELIABLETEST_OUTDIR_DEBUG))
RELIABLETEST_SED_OUTPUT_DIR_RELEASE= $(subst /,\\/,$(RELIABLETEST_OUTDIR_RELEASE))

RELIABLETEST_CFLAGS = $(CFLAGS) -std=c++0x 
RELIABLETEST_CFLAGS_DEBUG = $(CFLAGS_DEBUG) -std=c++0x 

RELIABLETEST_TARGET_DEBUG= $(BASEDIR)/bin/$(EPROSIMA_TARGET)/ReliableTestd
RELIABLETEST_TARGET= $(BASEDIR)/bin/$(EPROSIMA_TARGET)/ReliableTest

RELIABLETEST_INCLUDE_DIRS= $(INCLUDE_DIRS) -I$(BASEDIR)/../../../include \
                         -I$(EPROSIMADIR)/code

RELIABLETEST_LIBS_DEBUG= $(LIBS_DEBUG) -L$(BASEDIR)/../../../lib/$(EPROSIMA_TARGET) -lfastrtpsd -lboost_system -lboost_thread -lboost_date_time
RELIABLETEST_LIBS_RELEASE= $(LIBS) -L$(BASEDIR)/../../../lib/$(EPROSIMA_TARGET) -lfastrtps -lboost_system -lboost_thread -lboost_date_time

RELIABLETEST_SRC_CFILES=

RELIABLETEST_SRC_CPPFILES= $(BASEDIR)/ReliableCommunicationTest.cpp 


# Project sources are copied to the current directory
RELIABLETEST_SRCS= $(RELIABLETEST_SRC_CFILES) $(RELIABLETEST_SRC_CPPFILES)

# Source directories
RELIABLETEST_SOURCES_DIRS_AUX= $(foreach srcdir, $(dir $(RELIABLETEST_SRCS)), $(srcdir))
RELIABLETEST_SOURCES_DIRS= $(shell echo $(RELIABLETEST_SOURCES_DIRS_AUX) | tr " " "\n" | sort | uniq | tr "\n" " ")

RELIABLETEST_OBJS_DEBUG = $(foreach obj,$(notdir $(addsuffix .o, $(RELIABLETEST_SRCS))), $(RELIABLETEST_OUTDIR_DEBUG)/$(obj))
RELIABLETEST_DEPS_DEBUG = $(foreach dep,$(notdir $(addsuffix .d, $(RELIABLETEST_SRCS))), $(RELIABLETEST_OUTDIR_DEBUG)/$(dep))
RELIABLETEST_OBJS_RELEASE = $(foreach obj,$(notdir $(addsuffix .o, $(RELIABLETEST_SRCS))), $(RELIABLETEST_OUTDIR_RELEASE)/$(obj))
RELIABLETEST_DEPS_RELEASE = $(foreach dep,$(notdir $(addsuffix .d, $(RELIABLETEST_SRCS))), $(RELIABLETEST_OUTDIR_RELEASE)/$(dep))

OBJS+= $(RELIABLETEST_OBJS_DEBUG) $(RELIABLETEST_OBJS_RELEASE)
DEPS+= $(RELIABLETEST_DEPS_DEBUG) $(RELIABLETEST_DEPS_RELEASE)

.PHONY: ReliableTest checkReliableTestDirectories

ReliableTest: checkReliableTestDirectories $(RELIABLETEST_TARGET_DEBUG) $(RELIABLETEST_TARGET)

checkReliableTestDirectories:
	@mkdir -p $(OUTDIR)
	@mkdir -p $(RELIABLETEST_OUTDIR)
	@mkdir -p $(RELIABLETEST_OUTDIR_DEBUG)
	@mkdir -p $(RELIABLETEST_OUTDIR_RELEASE)
	@mkdir -p $(BASEDIR)/bin
	@mkdir -p $(BASEDIR)/bin/$(EPROSIMA_TARGET)

$(RELIABLETEST_TARGET_DEBUG): $(RELIABLETEST_OBJS_DEBUG)
	$(LN) $(LDFLAGS) -o $(RELIABLETEST_TARGET_DEBUG) $(RELIABLETEST_OBJS_DEBUG) $(LIBRARY_PATH) $(RELIABLETEST_LIBS_DEBUG) $(RELIABLETEST_STATIC_LIBS)

$(RELIABLETEST_TARGET): $(RELIABLETEST_OBJS_RELEASE)
	$(LN) $(LDFLAGS) -o $(RELIABLETEST_TARGET) $(RELIABLETEST_OBJS_RELEASE) $(LIBRARY_PATH) $(RELIABLETEST_LIBS_RELEASE) $(RELIABLETEST_STATIC_LIBS)

vpath %.c $(RELIABLETEST_SOURCES_DIRS)
vpath %.cpp $(RELIABLETEST_SOURCES_DIRS)

$(RELIABLETEST_OUTDIR_DEBUG)/%.o:%.c
	@echo Calculating dependencies \(DEBUG mode\) $<
	@$(CC) $(RELIABLETEST_CFLAGS_DEBUG) -MM $(RELIABLETEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(RELIABLETEST_SED_OUTPUT_DIR_DEBUG)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(DEBUG mode\) $<  
	@$(CC) $(RELIABLETEST_CFLAGS_DEBUG) $(RELIABLETEST_INCLUDE_DIRS) $< -o $@

$(RELIABLETEST_OUTDIR_RELEASE)/%.o:%.c
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CC) $(RELIABLETEST_CFLAGS) -MM $(RELIABLETEST_CFLAGS) $(RELIABLETEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(RELIABLETEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CC) $(RELIABLETEST_CFLAGS) $(RELIABLETEST_INCLUDE_DIRS) $< -o $@

$(RELIABLETEST_OUTDIR_DEBUG)/%.c.o:%.c
	@echo Calculating dependencies \(DEBUG mode\) $<
	@$(CC) $(RELIABLETEST_CFLAGS_DEBUG) -MM $(RELIABLETEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(RELIABLETEST_SED_OUTPUT_DIR_DEBUG)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(DEBUG mode\) $<  
	@$(CC) $(RELIABLETEST_CFLAGS_DEBUG) $(RELIABLETEST_INCLUDE_DIRS) $< -o $@

$(RELIABLETEST_OUTDIR_RELEASE)/%.c.o:%.c
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CC) $(RELIABLETEST_CFLAGS) -MM $(RELIABLETEST_CFLAGS) $(RELIABLETEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(RELIABLETEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CC) $(RELIABLETEST_CFLAGS) $(RELIABLETEST_INCLUDE_DIRS) $< -o $@

$(RELIABLETEST_OUTDIR_DEBUG)/%.cpp.o:%.cpp
	@echo Calculating dependencies \(DEBUG mode\) $<
	@$(CPP) $(RELIABLETEST_CFLAGS_DEBUG) -MM $(RELIABLETEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(RELIABLETEST_SED_OUTPUT_DIR_DEBUG)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(DEBUG mode\) $<
	@$(CPP) $(RELIABLETEST_CFLAGS_DEBUG) $(RELIABLETEST_INCLUDE_DIRS) $< -o $@

$(RELIABLETEST_OUTDIR_RELEASE)/%.cpp.o:%.cpp
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CPP) $(RELIABLETEST_CFLAGS) -MM $(RELIABLETEST_CFLAGS) $(RELIABLETEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(RELIABLETEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CPP) $(RELIABLETEST_CFLAGS) $(RELIABLETEST_INCLUDE_DIRS) $< -o $@

.PHONY: all

all: State

clean:
	@rm -f $(OBJS)
	@rm -f $(DEPS)

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPS)
endif
