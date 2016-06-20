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

CLIENTSERVERTEST_OUTDIR= $(OUTDIR)/ClientServerThrift
CLIENTSERVERTEST_OUTDIR_DEBUG = $(CLIENTSERVERTEST_OUTDIR)/debug
CLIENTSERVERTEST_OUTDIR_RELEASE = $(CLIENTSERVERTEST_OUTDIR)/release

CLIENTSERVERTEST_SED_OUTPUT_DIR_DEBUG= $(subst /,\\/,$(CLIENTSERVERTEST_OUTDIR_DEBUG))
CLIENTSERVERTEST_SED_OUTPUT_DIR_RELEASE= $(subst /,\\/,$(CLIENTSERVERTEST_OUTDIR_RELEASE))

CLIENTSERVERTEST_CFLAGS = $(CFLAGS) -std=c++0x 
CLIENTSERVERTEST_CFLAGS_DEBUG = $(CFLAGS_DEBUG) -std=c++0x  

CLIENTSERVERTEST_TARGET_DEBUG= $(BASEDIR)/bin/$(EPROSIMA_TARGET)/ClientServerThriftd
CLIENTSERVERTEST_TARGET= $(BASEDIR)/bin/$(EPROSIMA_TARGET)/ClientServerThrift

CLIENTSERVERTEST_INCLUDE_DIRS= $(INCLUDE_DIRS) -I$(BASEDIR)/../../../include \
                         -I$(BASEDIR)/../../../thirdparty/eprosima-common-code

CLIENTSERVERTEST_LIBS_DEBUG=  $(LIBS_DEBUG) -L$(BASEDIR)/../../../lib/$(EPROSIMA_TARGET)  -lthrift
CLIENTSERVERTEST_LIBS_RELEASE=  $(LIBS) -L$(BASEDIR)/../../../lib/$(EPROSIMA_TARGET)  -lthrift

CLIENTSERVERTEST_SRC_CFILES=

CLIENTSERVERTEST_SRC_CPPFILES= $(BASEDIR)/apache/gen-cpp/Calculator.cpp \
						$(BASEDIR)/apache/gen-cpp/shared_constants.cpp \
						$(BASEDIR)/apache/gen-cpp/shared_types.cpp \
						$(BASEDIR)/apache/gen-cpp/SharedService.cpp \
						$(BASEDIR)/apache/gen-cpp/tutorial_constants.cpp \
						$(BASEDIR)/apache/gen-cpp/tutorial_types.cpp \
						$(BASEDIR)/apache/ApacheServer.cpp \
						$(BASEDIR)/apache/ApacheClientTest.cpp \
						$(BASEDIR)/main_ClientServerTestThrift.cpp

# Project sources are copied to the current directory
CLIENTSERVERTEST_SRCS= $(CLIENTSERVERTEST_SRC_CFILES) $(CLIENTSERVERTEST_SRC_CPPFILES)

# Source directories
CLIENTSERVERTEST_SOURCES_DIRS_AUX= $(foreach srcdir, $(dir $(CLIENTSERVERTEST_SRCS)), $(srcdir))
CLIENTSERVERTEST_SOURCES_DIRS= $(shell echo $(CLIENTSERVERTEST_SOURCES_DIRS_AUX) | tr " " "\n" | sort | uniq | tr "\n" " ")

CLIENTSERVERTEST_OBJS_DEBUG = $(foreach obj,$(notdir $(addsuffix .o, $(CLIENTSERVERTEST_SRCS))), $(CLIENTSERVERTEST_OUTDIR_DEBUG)/$(obj))
CLIENTSERVERTEST_DEPS_DEBUG = $(foreach dep,$(notdir $(addsuffix .d, $(CLIENTSERVERTEST_SRCS))), $(CLIENTSERVERTEST_OUTDIR_DEBUG)/$(dep))
CLIENTSERVERTEST_OBJS_RELEASE = $(foreach obj,$(notdir $(addsuffix .o, $(CLIENTSERVERTEST_SRCS))), $(CLIENTSERVERTEST_OUTDIR_RELEASE)/$(obj))
CLIENTSERVERTEST_DEPS_RELEASE = $(foreach dep,$(notdir $(addsuffix .d, $(CLIENTSERVERTEST_SRCS))), $(CLIENTSERVERTEST_OUTDIR_RELEASE)/$(dep))

OBJS+= $(CLIENTSERVERTEST_OBJS_DEBUG) $(CLIENTSERVERTEST_OBJS_RELEASE)
DEPS+= $(CLIENTSERVERTEST_DEPS_DEBUG) $(CLIENTSERVERTEST_DEPS_RELEASE)

.PHONY: LatencyTest checkLatencyTestDirectories

LatencyTest: checkLatencyTestDirectories $(CLIENTSERVERTEST_TARGET_DEBUG) $(CLIENTSERVERTEST_TARGET)

checkLatencyTestDirectories:
	@mkdir -p $(OUTDIR)
	@mkdir -p $(CLIENTSERVERTEST_OUTDIR)
	@mkdir -p $(CLIENTSERVERTEST_OUTDIR_DEBUG)
	@mkdir -p $(CLIENTSERVERTEST_OUTDIR_RELEASE)
	@mkdir -p $(BASEDIR)/bin
	@mkdir -p $(BASEDIR)/bin/$(EPROSIMA_TARGET)

$(CLIENTSERVERTEST_TARGET_DEBUG): $(CLIENTSERVERTEST_OBJS_DEBUG)
	$(LN) $(LDFLAGS) -o $(CLIENTSERVERTEST_TARGET_DEBUG) $(CLIENTSERVERTEST_OBJS_DEBUG) $(LIBRARY_PATH) $(CLIENTSERVERTEST_LIBS_DEBUG) $(CLIENTSERVERTEST_STATIC_LIBS)

$(CLIENTSERVERTEST_TARGET): $(CLIENTSERVERTEST_OBJS_RELEASE)
	$(LN) $(LDFLAGS) -o $(CLIENTSERVERTEST_TARGET) $(CLIENTSERVERTEST_OBJS_RELEASE) $(LIBRARY_PATH) $(CLIENTSERVERTEST_LIBS_RELEASE) $(CLIENTSERVERTEST_STATIC_LIBS)

vpath %.c $(CLIENTSERVERTEST_SOURCES_DIRS)
vpath %.cpp $(CLIENTSERVERTEST_SOURCES_DIRS)

$(CLIENTSERVERTEST_OUTDIR_DEBUG)/%.o:%.c
	@echo Calculating dependencies \(DEBUG mode\) $<
	@$(CC) $(CLIENTSERVERTEST_CFLAGS_DEBUG) -MM $(CLIENTSERVERTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(CLIENTSERVERTEST_SED_OUTPUT_DIR_DEBUG)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(DEBUG mode\) $<  
	@$(CC) $(CLIENTSERVERTEST_CFLAGS_DEBUG) $(CLIENTSERVERTEST_INCLUDE_DIRS) $< -o $@

$(CLIENTSERVERTEST_OUTDIR_RELEASE)/%.o:%.c
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CC) $(CLIENTSERVERTEST_CFLAGS) -MM $(CLIENTSERVERTEST_CFLAGS) $(CLIENTSERVERTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(CLIENTSERVERTEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CC) $(CLIENTSERVERTEST_CFLAGS) $(CLIENTSERVERTEST_INCLUDE_DIRS) $< -o $@

$(CLIENTSERVERTEST_OUTDIR_DEBUG)/%.c.o:%.c
	@echo Calculating dependencies \(DEBUG mode\) $<
	@$(CC) $(CLIENTSERVERTEST_CFLAGS_DEBUG) -MM $(CLIENTSERVERTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(CLIENTSERVERTEST_SED_OUTPUT_DIR_DEBUG)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(DEBUG mode\) $<  
	@$(CC) $(CLIENTSERVERTEST_CFLAGS_DEBUG) $(CLIENTSERVERTEST_INCLUDE_DIRS) $< -o $@

$(CLIENTSERVERTEST_OUTDIR_RELEASE)/%.c.o:%.c
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CC) $(CLIENTSERVERTEST_CFLAGS) -MM $(CLIENTSERVERTEST_CFLAGS) $(CLIENTSERVERTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(CLIENTSERVERTEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CC) $(CLIENTSERVERTEST_CFLAGS) $(CLIENTSERVERTEST_INCLUDE_DIRS) $< -o $@

$(CLIENTSERVERTEST_OUTDIR_DEBUG)/%.cpp.o:%.cpp
	@echo Calculating dependencies \(DEBUG mode\) $<
	@$(CPP) $(CLIENTSERVERTEST_CFLAGS_DEBUG) -MM $(CLIENTSERVERTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(CLIENTSERVERTEST_SED_OUTPUT_DIR_DEBUG)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(DEBUG mode\) $<
	@$(CPP) $(CLIENTSERVERTEST_CFLAGS_DEBUG) $(CLIENTSERVERTEST_INCLUDE_DIRS) $< -o $@

$(CLIENTSERVERTEST_OUTDIR_RELEASE)/%.cpp.o:%.cpp
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CPP) $(CLIENTSERVERTEST_CFLAGS) -MM $(CLIENTSERVERTEST_CFLAGS) $(CLIENTSERVERTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(CLIENTSERVERTEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CPP) $(CLIENTSERVERTEST_CFLAGS) $(CLIENTSERVERTEST_INCLUDE_DIRS) $< -o $@

.PHONY: all

all: State

clean:
	@rm -f $(OBJS)
	@rm -f $(DEPS)

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPS)
endif
