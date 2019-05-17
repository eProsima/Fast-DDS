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

CPP=g++
LN=g++
AR=ar
CP=cp
SYSLIBS= -ldl -lnsl -lm -lpthread -lrt
DEFINES= 
COMMON_CFLAGS= -c -Wall -D__LITTLE_ENDIAN__ -std=c++11

## CHOOSE HERE BETWEEN 32 and 64 bit architecture

##32 BIT ARCH:
#COMMON_CFLAGS+= -m32 -fpic
#LDFLAGS=-m32

#64BIT ARCH:
COMMON_CFLAGS+= -m64 -fpic
LDFLAGS=-m64

CFLAGS = $(COMMON_CFLAGS) -O2

INCLUDES= -I.

LIBS = -lfastcdr -lfastrtps $(shell test -x "$$(which pkg-config)" && pkg-config libssl libcrypto --libs --silence-errors) $(SYSLIBS)

DIRECTORIES= output.dir bin.dir

all: $(DIRECTORIES) FlowControlExample

FLOWCONTROLEXAMPLE_TARGET= bin/FlowControlExample

FLOWCONTROLEXAMPLE_SRC_CXXFILES=

FLOWCONTROLEXAMPLE_SRC_CPPFILES= FlowControlExample/FlowControlExample.cxx \
								FlowControlExample/FlowControlExamplePubSubTypes.cxx \
								FlowControlExample/FlowControlExamplePublisher.cxx \
								FlowControlExample/FlowControlExampleSubscriber.cxx \
								FlowControlExample/FlowControlExamplePubSubMain.cxx 


# Project sources are copied to the current directory
FLOWCONTROLEXAMPLE_SRCS= $(FLOWCONTROLEXAMPLE_SRC_CXXFILES) $(FLOWCONTROLEXAMPLE_SRC_CPPFILES)

# Source directories
FLOWCONTROLEXAMPLE_SOURCES_DIRS_AUX= $(foreach srcdir, $(dir $(FLOWCONTROLEXAMPLE_SRCS)), $(srcdir))
FLOWCONTROLEXAMPLE_SOURCES_DIRS= $(shell echo $(FLOWCONTROLEXAMPLE_SOURCES_DIRS_AUX) | tr " " "\n" | sort | uniq | tr "\n" " ")

FLOWCONTROLEXAMPLE_OBJS = $(foreach obj,$(notdir $(addsuffix .o, $(FLOWCONTROLEXAMPLE_SRCS))), output/$(obj))
FLOWCONTROLEXAMPLE_DEPS = $(foreach dep,$(notdir $(addsuffix .d, $(FLOWCONTROLEXAMPLE_SRCS))), output/$(dep))

OBJS+=  $(FLOWCONTROLEXAMPLE_OBJS)
DEPS+=  $(FLOWCONTROLEXAMPLE_DEPS)

FlowControlExample: $(FLOWCONTROLEXAMPLE_TARGET)

$(FLOWCONTROLEXAMPLE_TARGET): $(FLOWCONTROLEXAMPLE_OBJS)
	$(LN) $(LDFLAGS) -o $(FLOWCONTROLEXAMPLE_TARGET) $(FLOWCONTROLEXAMPLE_OBJS) $(LIBS)

vpath %.cxx $(FLOWCONTROLEXAMPLE_SOURCES_DIRS)
vpath %.cpp $(FLOWCONTROLEXAMPLE_SOURCES_DIRS)

output/%.cxx.o:%.cxx
	@echo Calculating dependencies $<
	@$(CC) $(CFLAGS) -MM $(CFLAGS) $(INCLUDES) $< | sed "s/^.*:/output\/&/g" > $(@:%.cxx.o=%.cxx.d)
	@echo Compiling $<
	@$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

output/%.cpp.o:%.cpp
	@echo Calculating dependencies $<
	@$(CPP) $(CFLAGS) -MM $(CFLAGS) $(INCLUDES) $< | sed "s/^.*:/output\/&/g" > $(@:%.cpp.o=%.cpp.d)
	@echo Compiling $<
	@$(CPP) $(CFLAGS) $(INCLUDES) $< -o $@

.PHONY: FlowControlExample

clean:
	@rm -f $(OBJS)
	@rm -f $(DEPS)

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPS)
endif

%.dir : 
	@echo "Checking directory $*"
	@if [ ! -d $* ]; then \
		echo "Making directory $*"; \
		mkdir -p $* ; \
	fi;
