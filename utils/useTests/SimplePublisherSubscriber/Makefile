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


FASTCDR_DIR = /home/grcanosa/workspace/fastcdr


SIMPLEPUBSUB_OUTDIR= $(OUTDIR)/SimplePubSub
SIMPLEPUBSUB_OUTDIR_RELEASE = $(SIMPLEPUBSUB_OUTDIR)/release


SIMPLEPUBSUB_SED_OUTPUT_DIR_RELEASE= $(subst /,\\/,$(SIMPLEPUBSUB_OUTDIR_RELEASE))

SIMPLEPUBSUB_CFLAGS = $(CFLAGS) -std=c++0x 



SIMPLEPUBSUB_TARGET= $(BASEDIR)/bin/$(EPROSIMA_TARGET)/SimplePubSub

SIMPLEPUBSUB_INCLUDE_DIRS= $(INCLUDE_DIRS) -I$(BASEDIR)/../../../include \
                         						-I$(EPROSIMADIR)/code \
                         						-I$(FASTCDR_DIR)/include


SIMPLEPUBSUB_LIBS_RELEASE= $(LIBS) -L$(BASEDIR)/../../../lib/$(EPROSIMA_TARGET) -leprosimartps -lboost_system -lboost_thread \
										-L$(FASTCDR_DIR)/lib/$(EPROSIMA_TARGET) -lfastcdr

SIMPLEPUBSUB_SRC_CFILES=

SIMPLEPUBSUB_SRC_CPPFILES= $(BASEDIR)/MyType.cpp\
				$(BASEDIR)/SimplePubSubType.cpp \
				$(BASEDIR)/SimplePublisher.cpp \
				$(BASEDIR)/SimpleSubscriber.cpp \
				$(BASEDIR)/SimplePubSub_main.cpp 


# Project sources are copied to the current directory
SIMPLEPUBSUB_SRCS= $(SIMPLEPUBSUB_SRC_CFILES) $(SIMPLEPUBSUB_SRC_CPPFILES)

# Source directories
SIMPLEPUBSUB_SOURCES_DIRS_AUX= $(foreach srcdir, $(dir $(SIMPLEPUBSUB_SRCS)), $(srcdir))
SIMPLEPUBSUB_SOURCES_DIRS= $(shell echo $(SIMPLEPUBSUB_SOURCES_DIRS_AUX) | tr " " "\n" | sort | uniq | tr "\n" " ")


SIMPLEPUBSUB_OBJS_RELEASE = $(foreach obj,$(notdir $(addsuffix .o, $(SIMPLEPUBSUB_SRCS))), $(SIMPLEPUBSUB_OUTDIR_RELEASE)/$(obj))
SIMPLEPUBSUB_DEPS_RELEASE = $(foreach dep,$(notdir $(addsuffix .d, $(SIMPLEPUBSUB_SRCS))), $(SIMPLEPUBSUB_OUTDIR_RELEASE)/$(dep))

OBJS+=  $(SIMPLEPUBSUB_OBJS_RELEASE)
DEPS+=  $(SIMPLEPUBSUB_DEPS_RELEASE)

.PHONY: SIMPLEPUBSUB checkSIMPLEPUBSUBDirectories

SIMPLEPUBSUB: checkSIMPLEPUBSUBDirectories  $(SIMPLEPUBSUB_TARGET)

checkSIMPLEPUBSUBDirectories:
	@mkdir -p $(OUTDIR)
	@mkdir -p $(SIMPLEPUBSUB_OUTDIR)
	@mkdir -p $(SIMPLEPUBSUB_OUTDIR_RELEASE)
	@mkdir -p $(BASEDIR)/bin
	@mkdir -p $(BASEDIR)/bin/$(EPROSIMA_TARGET)



$(SIMPLEPUBSUB_TARGET): $(SIMPLEPUBSUB_OBJS_RELEASE)
	$(LN) $(LDFLAGS) -o $(SIMPLEPUBSUB_TARGET) $(SIMPLEPUBSUB_OBJS_RELEASE) $(LIBRARY_PATH) $(SIMPLEPUBSUB_LIBS_RELEASE) $(SIMPLEPUBSUB_STATIC_LIBS)

vpath %.c $(SIMPLEPUBSUB_SOURCES_DIRS)
vpath %.cpp $(SIMPLEPUBSUB_SOURCES_DIRS)



$(SIMPLEPUBSUB_OUTDIR_RELEASE)/%.o:%.c
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CC) $(SIMPLEPUBSUB_CFLAGS) -MM $(SIMPLEPUBSUB_CFLAGS) $(SIMPLEPUBSUB_INCLUDE_DIRS) $< | sed "s/^.*:/$(SIMPLEPUBSUB_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CC) $(SIMPLEPUBSUB_CFLAGS) $(SIMPLEPUBSUB_INCLUDE_DIRS) $< -o $@



$(SIMPLEPUBSUB_OUTDIR_RELEASE)/%.c.o:%.c
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CC) $(SIMPLEPUBSUB_CFLAGS) -MM $(SIMPLEPUBSUB_CFLAGS) $(SIMPLEPUBSUB_INCLUDE_DIRS) $< | sed "s/^.*:/$(SIMPLEPUBSUB_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CC) $(SIMPLEPUBSUB_CFLAGS) $(SIMPLEPUBSUB_INCLUDE_DIRS) $< -o $@



$(SIMPLEPUBSUB_OUTDIR_RELEASE)/%.cpp.o:%.cpp
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CPP) $(SIMPLEPUBSUB_CFLAGS) -MM $(SIMPLEPUBSUB_CFLAGS) $(SIMPLEPUBSUB_INCLUDE_DIRS) $< | sed "s/^.*:/$(SIMPLEPUBSUB_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CPP) $(SIMPLEPUBSUB_CFLAGS) $(SIMPLEPUBSUB_INCLUDE_DIRS) $< -o $@

.PHONY: all

all: State

clean:
	@rm -f $(OBJS)
	@rm -f $(DEPS)

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPS)
endif
