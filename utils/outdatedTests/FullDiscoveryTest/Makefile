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

DISCOVERYTEST_OUTDIR= $(OUTDIR)/DiscoveryTest
DISCOVERYTEST_OUTDIR_DEBUG = $(DISCOVERYTEST_OUTDIR)/debug
DISCOVERYTEST_OUTDIR_RELEASE = $(DISCOVERYTEST_OUTDIR)/release

DISCOVERYTEST_SED_OUTPUT_DIR_DEBUG= $(subst /,\\/,$(DISCOVERYTEST_OUTDIR_DEBUG))
DISCOVERYTEST_SED_OUTPUT_DIR_RELEASE= $(subst /,\\/,$(DISCOVERYTEST_OUTDIR_RELEASE))

DISCOVERYTEST_CFLAGS = $(CFLAGS) -std=c++0x 
DISCOVERYTEST_CFLAGS_DEBUG = $(CFLAGS_DEBUG) -std=c++0x 

DISCOVERYTEST_TARGET_DEBUG= $(BASEDIR)/bin/$(EPROSIMA_TARGET)/DiscoveryTestd
DISCOVERYTEST_TARGET= $(BASEDIR)/bin/$(EPROSIMA_TARGET)/DiscoveryTest

DISCOVERYTEST_INCLUDE_DIRS= $(INCLUDE_DIRS) -I$(BASEDIR)/../../../include \
                         -I$(EPROSIMADIR)/code

DISCOVERYTEST_LIBS_DEBUG= $(LIBS_DEBUG) -L$(BASEDIR)/../../../lib/$(EPROSIMA_TARGET) -lfastrtpsd -lboost_system -lboost_thread
DISCOVERYTEST_LIBS_RELEASE= $(LIBS) -L$(BASEDIR)/../../../lib/$(EPROSIMA_TARGET) -lfastrtps -lboost_system -lboost_thread

DISCOVERYTEST_SRC_CFILES=

DISCOVERYTEST_SRC_CPPFILES= $(BASEDIR)/FullDiscoveryTest.cpp 


# Project sources are copied to the current directory
DISCOVERYTEST_SRCS= $(DISCOVERYTEST_SRC_CFILES) $(DISCOVERYTEST_SRC_CPPFILES)

# Source directories
DISCOVERYTEST_SOURCES_DIRS_AUX= $(foreach srcdir, $(dir $(DISCOVERYTEST_SRCS)), $(srcdir))
DISCOVERYTEST_SOURCES_DIRS= $(shell echo $(DISCOVERYTEST_SOURCES_DIRS_AUX) | tr " " "\n" | sort | uniq | tr "\n" " ")

DISCOVERYTEST_OBJS_DEBUG = $(foreach obj,$(notdir $(addsuffix .o, $(DISCOVERYTEST_SRCS))), $(DISCOVERYTEST_OUTDIR_DEBUG)/$(obj))
DISCOVERYTEST_DEPS_DEBUG = $(foreach dep,$(notdir $(addsuffix .d, $(DISCOVERYTEST_SRCS))), $(DISCOVERYTEST_OUTDIR_DEBUG)/$(dep))
DISCOVERYTEST_OBJS_RELEASE = $(foreach obj,$(notdir $(addsuffix .o, $(DISCOVERYTEST_SRCS))), $(DISCOVERYTEST_OUTDIR_RELEASE)/$(obj))
DISCOVERYTEST_DEPS_RELEASE = $(foreach dep,$(notdir $(addsuffix .d, $(DISCOVERYTEST_SRCS))), $(DISCOVERYTEST_OUTDIR_RELEASE)/$(dep))

OBJS+= $(DISCOVERYTEST_OBJS_DEBUG) $(DISCOVERYTEST_OBJS_RELEASE)
DEPS+= $(DISCOVERYTEST_DEPS_DEBUG) $(DISCOVERYTEST_DEPS_RELEASE)

.PHONY: DiscoveryTest checkDiscoveryTestDirectories

DiscoveryTest: checkDiscoveryTestDirectories $(DISCOVERYTEST_TARGET_DEBUG) $(DISCOVERYTEST_TARGET)

checkDiscoveryTestDirectories:
	@mkdir -p $(OUTDIR)
	@mkdir -p $(DISCOVERYTEST_OUTDIR)
	@mkdir -p $(DISCOVERYTEST_OUTDIR_DEBUG)
	@mkdir -p $(DISCOVERYTEST_OUTDIR_RELEASE)
	@mkdir -p $(BASEDIR)/bin
	@mkdir -p $(BASEDIR)/bin/$(EPROSIMA_TARGET)

$(DISCOVERYTEST_TARGET_DEBUG): $(DISCOVERYTEST_OBJS_DEBUG)
	$(LN) $(LDFLAGS) -o $(DISCOVERYTEST_TARGET_DEBUG) $(DISCOVERYTEST_OBJS_DEBUG) $(LIBRARY_PATH) $(DISCOVERYTEST_LIBS_DEBUG) $(DISCOVERYTEST_STATIC_LIBS)

$(DISCOVERYTEST_TARGET): $(DISCOVERYTEST_OBJS_RELEASE)
	$(LN) $(LDFLAGS) -o $(DISCOVERYTEST_TARGET) $(DISCOVERYTEST_OBJS_RELEASE) $(LIBRARY_PATH) $(DISCOVERYTEST_LIBS_RELEASE) $(DISCOVERYTEST_STATIC_LIBS)

vpath %.c $(DISCOVERYTEST_SOURCES_DIRS)
vpath %.cpp $(DISCOVERYTEST_SOURCES_DIRS)

$(DISCOVERYTEST_OUTDIR_DEBUG)/%.o:%.c
	@echo Calculating dependencies \(DEBUG mode\) $<
	@$(CC) $(DISCOVERYTEST_CFLAGS_DEBUG) -MM $(DISCOVERYTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(DISCOVERYTEST_SED_OUTPUT_DIR_DEBUG)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(DEBUG mode\) $<  
	@$(CC) $(DISCOVERYTEST_CFLAGS_DEBUG) $(DISCOVERYTEST_INCLUDE_DIRS) $< -o $@

$(DISCOVERYTEST_OUTDIR_RELEASE)/%.o:%.c
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CC) $(DISCOVERYTEST_CFLAGS) -MM $(DISCOVERYTEST_CFLAGS) $(DISCOVERYTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(DISCOVERYTEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CC) $(DISCOVERYTEST_CFLAGS) $(DISCOVERYTEST_INCLUDE_DIRS) $< -o $@

$(DISCOVERYTEST_OUTDIR_DEBUG)/%.c.o:%.c
	@echo Calculating dependencies \(DEBUG mode\) $<
	@$(CC) $(DISCOVERYTEST_CFLAGS_DEBUG) -MM $(DISCOVERYTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(DISCOVERYTEST_SED_OUTPUT_DIR_DEBUG)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(DEBUG mode\) $<  
	@$(CC) $(DISCOVERYTEST_CFLAGS_DEBUG) $(DISCOVERYTEST_INCLUDE_DIRS) $< -o $@

$(DISCOVERYTEST_OUTDIR_RELEASE)/%.c.o:%.c
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CC) $(DISCOVERYTEST_CFLAGS) -MM $(DISCOVERYTEST_CFLAGS) $(DISCOVERYTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(DISCOVERYTEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CC) $(DISCOVERYTEST_CFLAGS) $(DISCOVERYTEST_INCLUDE_DIRS) $< -o $@

$(DISCOVERYTEST_OUTDIR_DEBUG)/%.cpp.o:%.cpp
	@echo Calculating dependencies \(DEBUG mode\) $<
	@$(CPP) $(DISCOVERYTEST_CFLAGS_DEBUG) -MM $(DISCOVERYTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(DISCOVERYTEST_SED_OUTPUT_DIR_DEBUG)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(DEBUG mode\) $<
	@$(CPP) $(DISCOVERYTEST_CFLAGS_DEBUG) $(DISCOVERYTEST_INCLUDE_DIRS) $< -o $@

$(DISCOVERYTEST_OUTDIR_RELEASE)/%.cpp.o:%.cpp
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CPP) $(DISCOVERYTEST_CFLAGS) -MM $(DISCOVERYTEST_CFLAGS) $(DISCOVERYTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(DISCOVERYTEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CPP) $(DISCOVERYTEST_CFLAGS) $(DISCOVERYTEST_INCLUDE_DIRS) $< -o $@

.PHONY: all

all: Discovery

clean:
	@rm -f $(OBJS)
	@rm -f $(DEPS)

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPS)
endif
