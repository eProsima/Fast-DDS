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

all: $(DIRECTORIES) ConfigurabilityPub ConfigurabilitySub

CONFPUB_TARGET= bin/UseCasePublisher
CONFSUB_TARGET= bin/UseCaseSubscriber

CONFIGURABILITY_SRC_CXXFILES=

CONFIGURABILITY_SRC_CPPFILES= Configurability/samplePubSubTypes.cxx \
				Configurability/sample.cxx

CONFIGURABILITY_SRCS= $(CONFIGURABILITY_SRC_CXXFILES) $(CONFIGURABILITY_SRC_CPPFILES)

PUB_SRC_CPPFILE= Configurability/UseCasePublisher.cpp
SUB_SRC_CPPFILE= Configurability/UseCaseSubscriber.cpp

# Project sources are copied to the current directory
CONFPUB_SRCS= $(CONFIGURABILITY_SRC_CPPFILES) $(PUB_SRC_CPPFILE)
CONFSUB_SRCS= $(CONFIGURABILITY_SRC_CPPFILES) $(SUB_SRC_CPPFILE)

# Source directories
CONFPUB_SOURCES_DIRS_AUX= $(foreach srcdir, $(dir $(CONFPUB_SRCS)), $(srcdir))
CONFPUB_SOURCES_DIRS= $(shell echo $(CONFPUB_SOURCES_DIRS_AUX) | tr " " "\n" | sort | uniq | tr "\n" " ")

CONFPUB_OBJS = $(foreach obj,$(notdir $(addsuffix .o, $(CONFPUB_SRCS))), output/$(obj))
CONFPUB_DEPS = $(foreach dep,$(notdir $(addsuffix .d, $(CONFPUB_SRCS))), output/$(dep))

CONFSUB_SOURCES_DIRS_AUX= $(foreach srcdir, $(dir $(CONFSUB_SRCS)), $(srcdir))
CONFSUB_SOURCES_DIRS= $(shell echo $(CONFSUB_SOURCES_DIRS_AUX) | tr " " "\n" | sort | uniq | tr "\n" " ")

CONFSUB_OBJS = $(foreach obj,$(notdir $(addsuffix .o, $(CONFSUB_SRCS))), output/$(obj))
CONFSUB_DEPS = $(foreach dep,$(notdir $(addsuffix .d, $(CONFSUB_SRCS))), output/$(dep))

ALL_SOURCE_DIRS = $(CONFPUB_SOURCES_DIRS) $(CONFSUB_SOURCES_DIRS)

OBJS+=  $(CONFPUB_OBJS)
DEPS+=  $(CONFPUB_DEPS)

OBJS+=  $(CONFSUB_OBJS)
DEPS+=  $(CONFSUB_DEPS)

ConfigurabilityPub: $(CONFPUB_TARGET)

$(CONFPUB_TARGET): $(CONFPUB_OBJS)
	$(LN) $(LDFLAGS) -o $(CONFPUB_TARGET) $(CONFPUB_OBJS) $(LIBS)

ConfigurabilitySub: $(CONFSUB_TARGET)

$(CONFSUB_TARGET): $(CONFSUB_OBJS)
	$(LN) $(LDFLAGS) -o $(CONFSUB_TARGET) $(CONFSUB_OBJS) $(LIBS)


vpath %.cxx $(ALL_SOURCE_DIRS)
vpath %.cpp $(ALL_SOURCE_DIRS)

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

.PHONY: ConfigurabilityPub ConfigurabilitySub

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
