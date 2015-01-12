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


STATETEST_OUTDIR= $(OUTDIR)/RTPSTest

STATETEST_OUTDIR_RELEASE = $(STATETEST_OUTDIR)/release


STATETEST_SED_OUTPUT_DIR_RELEASE= $(subst /,\\/,$(STATETEST_OUTDIR_RELEASE))

STATETEST_CFLAGS = $(CFLAGS) -std=c++0x 



STATETEST_TARGET= $(BASEDIR)/bin/$(EPROSIMA_TARGET)/RTPSTest

STATETEST_INCLUDE_DIRS= $(INCLUDE_DIRS) -I$(BASEDIR)/../../../include \
								-I$(BASEDIR)/../../../thirdparty/eprosima-common-code 
STATETEST_LIBS_RELEASE=  $(LIBS) -L$(BASEDIR)/../../../lib/$(EPROSIMA_TARGET)  -lfastrtps -lboost_system -lboost_thread -lboost_date_time

STATETEST_SRC_CFILES=

STATETEST_SRC_CPPFILES= $(BASEDIR)/TestWriterRegistered.cpp \
					$(BASEDIR)/TestReaderRegistered.cpp \
					$(BASEDIR)/main_RTPSTest.cpp 

# Project sources are copied to the current directory
STATETEST_SRCS= $(STATETEST_SRC_CFILES) $(STATETEST_SRC_CPPFILES)

# Source directories
STATETEST_SOURCES_DIRS_AUX= $(foreach srcdir, $(dir $(STATETEST_SRCS)), $(srcdir))
STATETEST_SOURCES_DIRS= $(shell echo $(STATETEST_SOURCES_DIRS_AUX) | tr " " "\n" | sort | uniq | tr "\n" " ")


STATETEST_OBJS_RELEASE = $(foreach obj,$(notdir $(addsuffix .o, $(STATETEST_SRCS))), $(STATETEST_OUTDIR_RELEASE)/$(obj))
STATETEST_DEPS_RELEASE = $(foreach dep,$(notdir $(addsuffix .d, $(STATETEST_SRCS))), $(STATETEST_OUTDIR_RELEASE)/$(dep))

OBJS+=  $(STATETEST_OBJS_RELEASE)
DEPS+=  $(STATETEST_DEPS_RELEASE)

.PHONY: LatencyTest checkLatencyTestDirectories

LatencyTest: checkLatencyTestDirectories  $(STATETEST_TARGET)

checkLatencyTestDirectories:
	@mkdir -p $(OUTDIR)
	@mkdir -p $(STATETEST_OUTDIR)

	@mkdir -p $(STATETEST_OUTDIR_RELEASE)
	@mkdir -p $(BASEDIR)/bin
	@mkdir -p $(BASEDIR)/bin/$(EPROSIMA_TARGET)


$(STATETEST_TARGET): $(STATETEST_OBJS_RELEASE)
	$(LN) $(LDFLAGS) -o $(STATETEST_TARGET) $(STATETEST_OBJS_RELEASE) $(LIBRARY_PATH) $(STATETEST_LIBS_RELEASE) $(STATETEST_STATIC_LIBS)

vpath %.c $(STATETEST_SOURCES_DIRS)
vpath %.cpp $(STATETEST_SOURCES_DIRS)



$(STATETEST_OUTDIR_RELEASE)/%.o:%.c
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CC) $(STATETEST_CFLAGS) -MM $(STATETEST_CFLAGS) $(STATETEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(STATETEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CC) $(STATETEST_CFLAGS) $(STATETEST_INCLUDE_DIRS) $< -o $@



$(STATETEST_OUTDIR_RELEASE)/%.c.o:%.c
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CC) $(STATETEST_CFLAGS) -MM $(STATETEST_CFLAGS) $(STATETEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(STATETEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CC) $(STATETEST_CFLAGS) $(STATETEST_INCLUDE_DIRS) $< -o $@



$(STATETEST_OUTDIR_RELEASE)/%.cpp.o:%.cpp
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CPP) $(STATETEST_CFLAGS) -MM $(STATETEST_CFLAGS) $(STATETEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(STATETEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CPP) $(STATETEST_CFLAGS) $(STATETEST_INCLUDE_DIRS) $< -o $@

.PHONY: all

all: State

clean:
	@rm -f $(OBJS)
	@rm -f $(DEPS)

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPS)
endif
