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

-include $(EPROSIMADIR)/building/makefiles/eProsima.mk

ZEROMQTEST_OUTDIR= $(OUTDIR)/ZeroMQTest
ZEROMQTEST_OUTDIR_DEBUG = $(ZEROMQTEST_OUTDIR)/debug
ZEROMQTEST_OUTDIR_RELEASE = $(ZEROMQTEST_OUTDIR)/release

ZEROMQTEST_SED_OUTPUT_DIR_DEBUG= $(subst /,\\/,$(ZEROMQTEST_OUTDIR_DEBUG))
ZEROMQTEST_SED_OUTPUT_DIR_RELEASE= $(subst /,\\/,$(ZEROMQTEST_OUTDIR_RELEASE))

ZEROMQTEST_CFLAGS = $(CFLAGS) -std=c++0x 
ZEROMQTEST_CFLAGS_DEBUG = $(CFLAGS_DEBUG) -std=c++0x  

ZEROMQTEST_TARGET_DEBUG= $(BASEDIR)/bin/$(EPROSIMA_TARGET)/ZeroMQTestd
ZEROMQTEST_TARGET= $(BASEDIR)/bin/$(EPROSIMA_TARGET)/ZeroMQTest

ZEROMQTEST_INCLUDE_DIRS= $(INCLUDE_DIRS) -I$(BASEDIR)/../../../include \
                         -I$(EPROSIMADIR)/code

ZEROMQTEST_LIBS_DEBUG=  $(LIBS_DEBUG) -L$(BASEDIR)/../../../lib/$(EPROSIMA_TARGET)  -lfastrtpsd -lboost_system -lboost_thread -lzmq
ZEROMQTEST_LIBS_RELEASE=  $(LIBS) -L$(BASEDIR)/../../../lib/$(EPROSIMA_TARGET)  -lfastrtps -lboost_system -lboost_thread -lzmq

ZEROMQTEST_SRC_CFILES=

ZEROMQTEST_SRC_CPPFILES= $(BASEDIR)/main_ZeroMQTest.cpp \
						$(BASEDIR)/ZeroMQPublisher.cpp \
						$(BASEDIR)/ZeroMQSubscriber.cpp

# Project sources are copied to the current directory
ZEROMQTEST_SRCS= $(ZEROMQTEST_SRC_CFILES) $(ZEROMQTEST_SRC_CPPFILES)

# Source directories
ZEROMQTEST_SOURCES_DIRS_AUX= $(foreach srcdir, $(dir $(ZEROMQTEST_SRCS)), $(srcdir))
ZEROMQTEST_SOURCES_DIRS= $(shell echo $(ZEROMQTEST_SOURCES_DIRS_AUX) | tr " " "\n" | sort | uniq | tr "\n" " ")

ZEROMQTEST_OBJS_DEBUG = $(foreach obj,$(notdir $(addsuffix .o, $(ZEROMQTEST_SRCS))), $(ZEROMQTEST_OUTDIR_DEBUG)/$(obj))
ZEROMQTEST_DEPS_DEBUG = $(foreach dep,$(notdir $(addsuffix .d, $(ZEROMQTEST_SRCS))), $(ZEROMQTEST_OUTDIR_DEBUG)/$(dep))
ZEROMQTEST_OBJS_RELEASE = $(foreach obj,$(notdir $(addsuffix .o, $(ZEROMQTEST_SRCS))), $(ZEROMQTEST_OUTDIR_RELEASE)/$(obj))
ZEROMQTEST_DEPS_RELEASE = $(foreach dep,$(notdir $(addsuffix .d, $(ZEROMQTEST_SRCS))), $(ZEROMQTEST_OUTDIR_RELEASE)/$(dep))

OBJS+= $(ZEROMQTEST_OBJS_DEBUG) $(ZEROMQTEST_OBJS_RELEASE)
DEPS+= $(ZEROMQTEST_DEPS_DEBUG) $(ZEROMQTEST_DEPS_RELEASE)

.PHONY: LatencyTest checkLatencyTestDirectories

LatencyTest: checkLatencyTestDirectories $(ZEROMQTEST_TARGET_DEBUG) $(ZEROMQTEST_TARGET)

checkLatencyTestDirectories:
	@mkdir -p $(OUTDIR)
	@mkdir -p $(ZEROMQTEST_OUTDIR)
	@mkdir -p $(ZEROMQTEST_OUTDIR_DEBUG)
	@mkdir -p $(ZEROMQTEST_OUTDIR_RELEASE)
	@mkdir -p $(BASEDIR)/bin
	@mkdir -p $(BASEDIR)/bin/$(EPROSIMA_TARGET)

$(ZEROMQTEST_TARGET_DEBUG): $(ZEROMQTEST_OBJS_DEBUG)
	$(LN) $(LDFLAGS) -o $(ZEROMQTEST_TARGET_DEBUG) $(ZEROMQTEST_OBJS_DEBUG) $(LIBRARY_PATH) $(ZEROMQTEST_LIBS_DEBUG) $(ZEROMQTEST_STATIC_LIBS)

$(ZEROMQTEST_TARGET): $(ZEROMQTEST_OBJS_RELEASE)
	$(LN) $(LDFLAGS) -o $(ZEROMQTEST_TARGET) $(ZEROMQTEST_OBJS_RELEASE) $(LIBRARY_PATH) $(ZEROMQTEST_LIBS_RELEASE) $(ZEROMQTEST_STATIC_LIBS)

vpath %.c $(ZEROMQTEST_SOURCES_DIRS)
vpath %.cpp $(ZEROMQTEST_SOURCES_DIRS)

$(ZEROMQTEST_OUTDIR_DEBUG)/%.o:%.c
	@echo Calculating dependencies \(DEBUG mode\) $<
	@$(CC) $(ZEROMQTEST_CFLAGS_DEBUG) -MM $(ZEROMQTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(ZEROMQTEST_SED_OUTPUT_DIR_DEBUG)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(DEBUG mode\) $<  
	@$(CC) $(ZEROMQTEST_CFLAGS_DEBUG) $(ZEROMQTEST_INCLUDE_DIRS) $< -o $@

$(ZEROMQTEST_OUTDIR_RELEASE)/%.o:%.c
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CC) $(ZEROMQTEST_CFLAGS) -MM $(ZEROMQTEST_CFLAGS) $(ZEROMQTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(ZEROMQTEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CC) $(ZEROMQTEST_CFLAGS) $(ZEROMQTEST_INCLUDE_DIRS) $< -o $@

$(ZEROMQTEST_OUTDIR_DEBUG)/%.c.o:%.c
	@echo Calculating dependencies \(DEBUG mode\) $<
	@$(CC) $(ZEROMQTEST_CFLAGS_DEBUG) -MM $(ZEROMQTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(ZEROMQTEST_SED_OUTPUT_DIR_DEBUG)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(DEBUG mode\) $<  
	@$(CC) $(ZEROMQTEST_CFLAGS_DEBUG) $(ZEROMQTEST_INCLUDE_DIRS) $< -o $@

$(ZEROMQTEST_OUTDIR_RELEASE)/%.c.o:%.c
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CC) $(ZEROMQTEST_CFLAGS) -MM $(ZEROMQTEST_CFLAGS) $(ZEROMQTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(ZEROMQTEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CC) $(ZEROMQTEST_CFLAGS) $(ZEROMQTEST_INCLUDE_DIRS) $< -o $@

$(ZEROMQTEST_OUTDIR_DEBUG)/%.cpp.o:%.cpp
	@echo Calculating dependencies \(DEBUG mode\) $<
	@$(CPP) $(ZEROMQTEST_CFLAGS_DEBUG) -MM $(ZEROMQTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(ZEROMQTEST_SED_OUTPUT_DIR_DEBUG)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(DEBUG mode\) $<
	@$(CPP) $(ZEROMQTEST_CFLAGS_DEBUG) $(ZEROMQTEST_INCLUDE_DIRS) $< -o $@

$(ZEROMQTEST_OUTDIR_RELEASE)/%.cpp.o:%.cpp
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CPP) $(ZEROMQTEST_CFLAGS) -MM $(ZEROMQTEST_CFLAGS) $(ZEROMQTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(ZEROMQTEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CPP) $(ZEROMQTEST_CFLAGS) $(ZEROMQTEST_INCLUDE_DIRS) $< -o $@

.PHONY: all

all: State

clean:
	@rm -f $(OBJS)
	@rm -f $(DEPS)

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPS)
endif
