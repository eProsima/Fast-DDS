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

CLIENTSERVERTEST_OUTDIR= $(OUTDIR)/ClientServerTestFastRTPS
CLIENTSERVERTEST_OUTDIR_RELEASE = $(CLIENTSERVERTEST_OUTDIR)/release

CLIENTSERVERTEST_SED_OUTPUT_DIR_RELEASE= $(subst /,\\/,$(CLIENTSERVERTEST_OUTDIR_RELEASE))

CLIENTSERVERTEST_CFLAGS = $(CFLAGS) -std=c++0x 

CLIENTSERVERTEST_TARGET= $(BASEDIR)/bin/$(EPROSIMA_TARGET)/ClientServerFastRTPS

CLIENTSERVERTEST_INCLUDE_DIRS= $(INCLUDE_DIRS) -I$(BASEDIR)/../../../include \
                         -I$(BASEDIR)/../../../thirdparty/eprosima-common-code

CLIENTSERVERTEST_LIBS_RELEASE=  $(LIBS) -L$(BASEDIR)/../../../lib/$(EPROSIMA_TARGET)  -lfastrtps -lboost_system -lboost_thread -lboost_date_time

CLIENTSERVERTEST_SRC_CFILES=

CLIENTSERVERTEST_SRC_CPPFILES= 	$(BASEDIR)/ClientServerTypes.cpp \
						$(BASEDIR)/EprosimaClient.cpp \
						$(BASEDIR)/EprosimaServer.cpp \
						$(BASEDIR)/EprosimaClientTest.cpp \
						$(BASEDIR)/main_ClientServerTestFastRTPS.cpp

# Project sources are copied to the current directory
CLIENTSERVERTEST_SRCS= $(CLIENTSERVERTEST_SRC_CFILES) $(CLIENTSERVERTEST_SRC_CPPFILES)

# Source directories
CLIENTSERVERTEST_SOURCES_DIRS_AUX= $(foreach srcdir, $(dir $(CLIENTSERVERTEST_SRCS)), $(srcdir))
CLIENTSERVERTEST_SOURCES_DIRS= $(shell echo $(CLIENTSERVERTEST_SOURCES_DIRS_AUX) | tr " " "\n" | sort | uniq | tr "\n" " ")

CLIENTSERVERTEST_OBJS_RELEASE = $(foreach obj,$(notdir $(addsuffix .o, $(CLIENTSERVERTEST_SRCS))), $(CLIENTSERVERTEST_OUTDIR_RELEASE)/$(obj))
CLIENTSERVERTEST_DEPS_RELEASE = $(foreach dep,$(notdir $(addsuffix .d, $(CLIENTSERVERTEST_SRCS))), $(CLIENTSERVERTEST_OUTDIR_RELEASE)/$(dep))

OBJS+=  $(CLIENTSERVERTEST_OBJS_RELEASE)
DEPS+=  $(CLIENTSERVERTEST_DEPS_RELEASE)

.PHONY: ClientServerTest checkClientServerTestDirectories

ClientServerTest: checkClientServerTestDirectories $(CLIENTSERVERTEST_TARGET)

checkClientServerTestDirectories:
	@mkdir -p $(OUTDIR)
	@mkdir -p $(CLIENTSERVERTEST_OUTDIR)
	@mkdir -p $(CLIENTSERVERTEST_OUTDIR_RELEASE)
	@mkdir -p $(BASEDIR)/bin
	@mkdir -p $(BASEDIR)/bin/$(EPROSIMA_TARGET)


$(CLIENTSERVERTEST_TARGET): $(CLIENTSERVERTEST_OBJS_RELEASE)
	$(LN) $(LDFLAGS) -o $(CLIENTSERVERTEST_TARGET) $(CLIENTSERVERTEST_OBJS_RELEASE) $(LIBRARY_PATH) $(CLIENTSERVERTEST_LIBS_RELEASE) $(CLIENTSERVERTEST_STATIC_LIBS)

vpath %.c $(CLIENTSERVERTEST_SOURCES_DIRS)
vpath %.cpp $(CLIENTSERVERTEST_SOURCES_DIRS)



$(CLIENTSERVERTEST_OUTDIR_RELEASE)/%.o:%.c
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CC) $(CLIENTSERVERTEST_CFLAGS) -MM $(CLIENTSERVERTEST_CFLAGS) $(CLIENTSERVERTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(CLIENTSERVERTEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CC) $(CLIENTSERVERTEST_CFLAGS) $(CLIENTSERVERTEST_INCLUDE_DIRS) $< -o $@



$(CLIENTSERVERTEST_OUTDIR_RELEASE)/%.c.o:%.c
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CC) $(CLIENTSERVERTEST_CFLAGS) -MM $(CLIENTSERVERTEST_CFLAGS) $(CLIENTSERVERTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(CLIENTSERVERTEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CC) $(CLIENTSERVERTEST_CFLAGS) $(CLIENTSERVERTEST_INCLUDE_DIRS) $< -o $@



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
