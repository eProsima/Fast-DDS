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

UNITTEST_OUTDIR= $(OUTDIR)/unitTest
UNITTEST_OUTDIR_DEBUG = $(UNITTEST_OUTDIR)/debug
UNITTEST_OUTDIR_RELEASE = $(UNITTEST_OUTDIR)/release

UNITTEST_SED_OUTPUT_DIR_DEBUG= $(subst /,\\/,$(UNITTEST_OUTDIR_DEBUG))
UNITTEST_SED_OUTPUT_DIR_RELEASE= $(subst /,\\/,$(UNITTEST_OUTDIR_RELEASE))

UNITTEST_CFLAGS = $(CFLAGS) -std=c++0x 
UNITTEST_CFLAGS_DEBUG = $(CFLAGS_DEBUG) -std=c++0x  

UNITTEST_TARGET_DEBUG= $(BASEDIR)/bin/$(EPROSIMA_TARGET)/unitTestd
UNITTEST_TARGET= $(BASEDIR)/bin/$(EPROSIMA_TARGET)/unitTest

UNITTEST_INCLUDE_DIRS= $(INCLUDE_DIRS) -I$(BASEDIR)/../include \
                         -I$(EPROSIMADIR)/code

UNITTEST_LIBS_DEBUG=  $(LIBS_DEBUG) -L$(BASEDIR)/../lib/$(EPROSIMA_TARGET)  -lfastrtpsd -lboost_system -lboost_thread -lgtest -lpthread -ldl
UNITTEST_LIBS_RELEASE=  $(LIBS) -L$(BASEDIR)/../lib/$(EPROSIMA_TARGET)  -lfastrtps -lboost_system -lboost_thread -lgtest -lpthread -ldl

UNITTEST_SRC_CFILES=

UNITTEST_SRC_CPPFILES= $(BASEDIR)/unit_test.cpp

# Project sources are copied to the current directory
UNITTEST_SRCS= $(UNITTEST_SRC_CFILES) $(UNITTEST_SRC_CPPFILES)

# Source directories
UNITTEST_SOURCES_DIRS_AUX= $(foreach srcdir, $(dir $(UNITTEST_SRCS)), $(srcdir))
UNITTEST_SOURCES_DIRS= $(shell echo $(UNITTEST_SOURCES_DIRS_AUX) | tr " " "\n" | sort | uniq | tr "\n" " ")

UNITTEST_OBJS_DEBUG = $(foreach obj,$(notdir $(addsuffix .o, $(UNITTEST_SRCS))), $(UNITTEST_OUTDIR_DEBUG)/$(obj))
UNITTEST_DEPS_DEBUG = $(foreach dep,$(notdir $(addsuffix .d, $(UNITTEST_SRCS))), $(UNITTEST_OUTDIR_DEBUG)/$(dep))
UNITTEST_OBJS_RELEASE = $(foreach obj,$(notdir $(addsuffix .o, $(UNITTEST_SRCS))), $(UNITTEST_OUTDIR_RELEASE)/$(obj))
UNITTEST_DEPS_RELEASE = $(foreach dep,$(notdir $(addsuffix .d, $(UNITTEST_SRCS))), $(UNITTEST_OUTDIR_RELEASE)/$(dep))

OBJS+= $(UNITTEST_OBJS_DEBUG) $(UNITTEST_OBJS_RELEASE)
DEPS+= $(UNITTEST_DEPS_DEBUG) $(UNITTEST_DEPS_RELEASE)

.PHONY: unitTest checkunitTestDirectories

unitTest: checkunitTestDirectories $(UNITTEST_TARGET_DEBUG) $(UNITTEST_TARGET)

checkunitTestDirectories:
	@mkdir -p $(OUTDIR)
	@mkdir -p $(UNITTEST_OUTDIR)
	@mkdir -p $(UNITTEST_OUTDIR_DEBUG)
	@mkdir -p $(UNITTEST_OUTDIR_RELEASE)
	@mkdir -p $(BASEDIR)/bin
	@mkdir -p $(BASEDIR)/bin/$(EPROSIMA_TARGET)

$(UNITTEST_TARGET_DEBUG): $(UNITTEST_OBJS_DEBUG)
	$(LN) $(LDFLAGS) -o $(UNITTEST_TARGET_DEBUG) $(UNITTEST_OBJS_DEBUG) $(LIBRARY_PATH) $(UNITTEST_LIBS_DEBUG) $(UNITTEST_STATIC_LIBS)

$(UNITTEST_TARGET): $(UNITTEST_OBJS_RELEASE)
	$(LN) $(LDFLAGS) -o $(UNITTEST_TARGET) $(UNITTEST_OBJS_RELEASE) $(LIBRARY_PATH) $(UNITTEST_LIBS_RELEASE) $(UNITTEST_STATIC_LIBS)

vpath %.c $(UNITTEST_SOURCES_DIRS)
vpath %.cpp $(UNITTEST_SOURCES_DIRS)

$(UNITTEST_OUTDIR_DEBUG)/%.o:%.c
	@echo Calculating dependencies \(DEBUG mode\) $<
	@$(CC) $(UNITTEST_CFLAGS_DEBUG) -MM $(UNITTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(UNITTEST_SED_OUTPUT_DIR_DEBUG)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(DEBUG mode\) $<  
	@$(CC) $(UNITTEST_CFLAGS_DEBUG) $(UNITTEST_INCLUDE_DIRS) $< -o $@

$(UNITTEST_OUTDIR_RELEASE)/%.o:%.c
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CC) $(UNITTEST_CFLAGS) -MM $(UNITTEST_CFLAGS) $(UNITTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(UNITTEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CC) $(UNITTEST_CFLAGS) $(UNITTEST_INCLUDE_DIRS) $< -o $@

$(UNITTEST_OUTDIR_DEBUG)/%.c.o:%.c
	@echo Calculating dependencies \(DEBUG mode\) $<
	@$(CC) $(UNITTEST_CFLAGS_DEBUG) -MM $(UNITTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(UNITTEST_SED_OUTPUT_DIR_DEBUG)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(DEBUG mode\) $<  
	@$(CC) $(UNITTEST_CFLAGS_DEBUG) $(UNITTEST_INCLUDE_DIRS) $< -o $@

$(UNITTEST_OUTDIR_RELEASE)/%.c.o:%.c
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CC) $(UNITTEST_CFLAGS) -MM $(UNITTEST_CFLAGS) $(UNITTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(UNITTEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CC) $(UNITTEST_CFLAGS) $(UNITTEST_INCLUDE_DIRS) $< -o $@

$(UNITTEST_OUTDIR_DEBUG)/%.cpp.o:%.cpp
	@echo Calculating dependencies \(DEBUG mode\) $<
	@$(CPP) $(UNITTEST_CFLAGS_DEBUG) -MM $(UNITTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(UNITTEST_SED_OUTPUT_DIR_DEBUG)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(DEBUG mode\) $<
	@$(CPP) $(UNITTEST_CFLAGS_DEBUG) $(UNITTEST_INCLUDE_DIRS) $< -o $@

$(UNITTEST_OUTDIR_RELEASE)/%.cpp.o:%.cpp
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CPP) $(UNITTEST_CFLAGS) -MM $(UNITTEST_CFLAGS) $(UNITTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(UNITTEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CPP) $(UNITTEST_CFLAGS) $(UNITTEST_INCLUDE_DIRS) $< -o $@

.PHONY: all

all: State

clean:
	@rm -f $(OBJS)
	@rm -f $(DEPS)

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPS)
endif
