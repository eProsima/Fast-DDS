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

METATEST_OUTDIR= $(OUTDIR)/MetaTest
METATEST_OUTDIR_DEBUG = $(METATEST_OUTDIR)/debug
METATEST_OUTDIR_RELEASE = $(METATEST_OUTDIR)/release

METATEST_SED_OUTPUT_DIR_DEBUG= $(subst /,\\/,$(METATEST_OUTDIR_DEBUG))
METATEST_SED_OUTPUT_DIR_RELEASE= $(subst /,\\/,$(METATEST_OUTDIR_RELEASE))

METATEST_CFLAGS = $(CFLAGS) -std=c++0x 
METATEST_CFLAGS_DEBUG = $(CFLAGS_DEBUG) -std=c++0x  

METATEST_TARGET_DEBUG= $(BASEDIR)/bin/$(EPROSIMA_TARGET)/MetaTestd
METATEST_TARGET= $(BASEDIR)/bin/$(EPROSIMA_TARGET)/MetaTest

METATEST_INCLUDE_DIRS= $(INCLUDE_DIRS) -I$(BASEDIR)/../../include \
                         -I$(BASEDIR)/../../thirdparty/eprosima-common-code

METATEST_LIBS_DEBUG=  $(LIBS_DEBUG) -L$(BASEDIR)/../../lib/$(EPROSIMA_TARGET)  -lfastrtpsd -lboost_system -lboost_thread -lboost_date_time -lfastcdr
METATEST_LIBS_RELEASE=  $(LIBS) -L$(BASEDIR)/../../lib/$(EPROSIMA_TARGET)  -lfastrtps -lboost_system -lboost_thread -lboost_date_time -lfastcdr

METATEST_SRC_CFILES=

METATEST_SRC_CPPFILES=  $(BASEDIR)/main_MetaTest.cpp \
						$(BASEDIR)/MetaTestPublisher.cpp \
						$(BASEDIR)/MetaTestSubscriber.cpp \
						$(BASEDIR)/types/MetaTestTypes.cxx \
						$(BASEDIR)/MetaTestTypesPubSubType.cpp \
						$(BASEDIR)/../useTests/RTPSTest_as_socket/TestReaderSocket.cpp \
						$(BASEDIR)/../useTests/RTPSTest_as_socket/TestWriterSocket.cpp \
						$(BASEDIR)/../useTests/RTPSTest_registered/TestWriterRegistered.cpp \
						$(BASEDIR)/../useTests/RTPSTest_registered/TestReaderRegistered.cpp \
						$(BASEDIR)/../useTests/ClientServerTest/EprosimaClient.cpp \
						$(BASEDIR)/../useTests/ClientServerTest/EprosimaServer.cpp \
						$(BASEDIR)/../useTests/ClientServerTest/ClientServerTypes.cpp \
						$(BASEDIR)/../useTests/HelloWorldTest/HelloWorldPublisher.cpp \
						$(BASEDIR)/../useTests/HelloWorldTest/HelloWorldSubscriber.cpp \
						$(BASEDIR)/../useTests/HelloWorldTest/HelloWorldType.cpp \
						$(BASEDIR)/../useTests/HelloWorldTest/HelloWorld.cpp 

# Project sources are copied to the current directory
METATEST_SRCS= $(METATEST_SRC_CFILES) $(METATEST_SRC_CPPFILES)

# Source directories
METATEST_SOURCES_DIRS_AUX= $(foreach srcdir, $(dir $(METATEST_SRCS)), $(srcdir))
METATEST_SOURCES_DIRS= $(shell echo $(METATEST_SOURCES_DIRS_AUX) | tr " " "\n" | sort | uniq | tr "\n" " ")

METATEST_OBJS_DEBUG = $(foreach obj,$(notdir $(addsuffix .o, $(METATEST_SRCS))), $(METATEST_OUTDIR_DEBUG)/$(obj))
METATEST_DEPS_DEBUG = $(foreach dep,$(notdir $(addsuffix .d, $(METATEST_SRCS))), $(METATEST_OUTDIR_DEBUG)/$(dep))
METATEST_OBJS_RELEASE = $(foreach obj,$(notdir $(addsuffix .o, $(METATEST_SRCS))), $(METATEST_OUTDIR_RELEASE)/$(obj))
METATEST_DEPS_RELEASE = $(foreach dep,$(notdir $(addsuffix .d, $(METATEST_SRCS))), $(METATEST_OUTDIR_RELEASE)/$(dep))

OBJS+= $(METATEST_OBJS_DEBUG) $(METATEST_OBJS_RELEASE)
DEPS+= $(METATEST_DEPS_DEBUG) $(METATEST_DEPS_RELEASE)

.PHONY: LatencyTest checkLatencyTestDirectories

LatencyTest: checkLatencyTestDirectories $(METATEST_TARGET_DEBUG) $(METATEST_TARGET)

checkLatencyTestDirectories:
	@mkdir -p $(OUTDIR)
	@mkdir -p $(METATEST_OUTDIR)
	@mkdir -p $(METATEST_OUTDIR_DEBUG)
	@mkdir -p $(METATEST_OUTDIR_RELEASE)
	@mkdir -p $(BASEDIR)/bin
	@mkdir -p $(BASEDIR)/bin/$(EPROSIMA_TARGET)

$(METATEST_TARGET_DEBUG): $(METATEST_OBJS_DEBUG)
	$(LN) $(LDFLAGS) -o $(METATEST_TARGET_DEBUG) $(METATEST_OBJS_DEBUG) $(LIBRARY_PATH) $(METATEST_LIBS_DEBUG) $(METATEST_STATIC_LIBS)

$(METATEST_TARGET): $(METATEST_OBJS_RELEASE)
	$(LN) $(LDFLAGS) -o $(METATEST_TARGET) $(METATEST_OBJS_RELEASE) $(LIBRARY_PATH) $(METATEST_LIBS_RELEASE) $(METATEST_STATIC_LIBS)

vpath %.c $(METATEST_SOURCES_DIRS)
vpath %.cpp $(METATEST_SOURCES_DIRS)
vpath %.cxx $(METATEST_SOURCES_DIRS)

$(METATEST_OUTDIR_DEBUG)/%.o:%.c
	@echo Calculating dependencies \(DEBUG mode\) $<
	@$(CC) $(METATEST_CFLAGS_DEBUG) -MM $(METATEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(METATEST_SED_OUTPUT_DIR_DEBUG)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(DEBUG mode\) $<  
	@$(CC) $(METATEST_CFLAGS_DEBUG) $(METATEST_INCLUDE_DIRS) $< -o $@

$(METATEST_OUTDIR_RELEASE)/%.o:%.c
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CC) $(METATEST_CFLAGS) -MM $(METATEST_CFLAGS) $(METATEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(METATEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CC) $(METATEST_CFLAGS) $(METATEST_INCLUDE_DIRS) $< -o $@

$(METATEST_OUTDIR_DEBUG)/%.c.o:%.c
	@echo Calculating dependencies \(DEBUG mode\) $<
	@$(CC) $(METATEST_CFLAGS_DEBUG) -MM $(METATEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(METATEST_SED_OUTPUT_DIR_DEBUG)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(DEBUG mode\) $<  
	@$(CC) $(METATEST_CFLAGS_DEBUG) $(METATEST_INCLUDE_DIRS) $< -o $@

$(METATEST_OUTDIR_RELEASE)/%.c.o:%.c
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CC) $(METATEST_CFLAGS) -MM $(METATEST_CFLAGS) $(METATEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(METATEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CC) $(METATEST_CFLAGS) $(METATEST_INCLUDE_DIRS) $< -o $@

$(METATEST_OUTDIR_DEBUG)/%.cpp.o:%.cpp
	@echo Calculating dependencies \(DEBUG mode\) $<
	@$(CPP) $(METATEST_CFLAGS_DEBUG) -MM $(METATEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(METATEST_SED_OUTPUT_DIR_DEBUG)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(DEBUG mode\) $<
	@$(CPP) $(METATEST_CFLAGS_DEBUG) $(METATEST_INCLUDE_DIRS) $< -o $@

$(METATEST_OUTDIR_RELEASE)/%.cpp.o:%.cpp
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CPP) $(METATEST_CFLAGS) -MM $(METATEST_CFLAGS) $(METATEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(METATEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CPP) $(METATEST_CFLAGS) $(METATEST_INCLUDE_DIRS) $< -o $@
	
$(METATEST_OUTDIR_DEBUG)/%.cxx.o:%.cxx
	@echo Calculating dependencies \(DEBUG mode\) $<
	@$(CPP) $(METATEST_CFLAGS_DEBUG) -MM $(METATEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(METATEST_SED_OUTPUT_DIR_DEBUG)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(DEBUG mode\) $<
	@$(CPP) $(METATEST_CFLAGS_DEBUG) $(METATEST_INCLUDE_DIRS) $< -o $@

$(METATEST_OUTDIR_RELEASE)/%.cxx.o:%.cxx
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CPP) $(METATEST_CFLAGS) -MM $(METATEST_CFLAGS) $(METATEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(METATEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CPP) $(METATEST_CFLAGS) $(METATEST_INCLUDE_DIRS) $< -o $@

.PHONY: all

all: State

clean:
	@rm -f $(OBJS)
	@rm -f $(DEPS)

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPS)
endif
