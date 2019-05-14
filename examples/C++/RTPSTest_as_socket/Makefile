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

all: $(DIRECTORIES) RTPSTest_as_socket

RTPSTESTASSOCKET_TARGET= bin/RTPSTest_as_socket

RTPSTESTASSOCKET_SRC_CXXFILES=

RTPSTESTASSOCKET_SRC_CPPFILES= RTPSTest_as_socket/TestReaderSocket.cpp \
								RTPSTest_as_socket/TestWriterSocket.cpp \
								RTPSTest_as_socket/main_RTPSTest.cpp


# Project sources are copied to the current directory
RTPSTESTASSOCKET_SRCS= $(RTPSTESTASSOCKET_SRC_CXXFILES) $(RTPSTESTASSOCKET_SRC_CPPFILES)

# Source directories
RTPSTESTASSOCKET_SOURCES_DIRS_AUX= $(foreach srcdir, $(dir $(RTPSTESTASSOCKET_SRCS)), $(srcdir))
RTPSTESTASSOCKET_SOURCES_DIRS= $(shell echo $(RTPSTESTASSOCKET_SOURCES_DIRS_AUX) | tr " " "\n" | sort | uniq | tr "\n" " ")

RTPSTESTASSOCKET_OBJS = $(foreach obj,$(notdir $(addsuffix .o, $(RTPSTESTASSOCKET_SRCS))), output/$(obj))
RTPSTESTASSOCKET_DEPS = $(foreach dep,$(notdir $(addsuffix .d, $(RTPSTESTASSOCKET_SRCS))), output/$(dep))

OBJS+=  $(RTPSTESTASSOCKET_OBJS)
DEPS+=  $(RTPSTESTASSOCKET_DEPS)

RTPSTest_as_socket: $(RTPSTESTASSOCKET_TARGET)

$(RTPSTESTASSOCKET_TARGET): $(RTPSTESTASSOCKET_OBJS)
	$(LN) $(LDFLAGS) -o $(RTPSTESTASSOCKET_TARGET) $(RTPSTESTASSOCKET_OBJS) $(LIBS)

vpath %.cxx $(RTPSTESTASSOCKET_SOURCES_DIRS)
vpath %.cpp $(RTPSTESTASSOCKET_SOURCES_DIRS)

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

.PHONY: RTPSTest_as_socket

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
