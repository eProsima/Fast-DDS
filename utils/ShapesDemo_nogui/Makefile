BASEDIR=.
OUTDIR= $(BASEDIR)/output
CC=arm-unknown-linux-gnueabi-g++
CPP=arm-unknown-linux-gnueabi-g++
LN=arm-unknown-linux-gnueabi-g++
AR=ar
CP=cp



COMMON_CFLAGS= -c -D__LITTLE_ENDIAN__

## CHOOSE HERE BETWEEN 32 and 64 bit architecture

##32 BIT ARCH:
COMMON_CFLAGS+= -fpic
LDFLAGS=
EPROSIMA_TARGET=armelf_linux_eabi
EPROSIMADIR= $(BASEDIR)/../../thirdparty/dev-env

#64BIT ARCH:
#COMMON_CFLAGS+= -m64 -fpic
#LDFLAGS=-m64
#EPROSIMA_TARGET=x64Linux2.6gcc

CFLAGS = $(COMMON_CFLAGS) -O2
CFLAGS_DEBUG= $(COMMON_CFLAGS) -g -D__DEBUG


HELLOWORLDTEST_OUTDIR= $(OUTDIR)/ShapesDemo
HELLOWORLDTEST_OUTDIR_RELEASE = $(HELLOWORLDTEST_OUTDIR)/release


HELLOWORLDTEST_SED_OUTPUT_DIR_RELEASE= $(subst /,\\/,$(HELLOWORLDTEST_OUTDIR_RELEASE))

HELLOWORLDTEST_CFLAGS = $(CFLAGS) -std=c++0x -O2 -Wno-unknown-pragmas



HELLOWORLDTEST_TARGET= $(BASEDIR)/bin/$(EPROSIMA_TARGET)/ShapesDemo

HELLOWORLDTEST_INCLUDE_DIRS= $(INCLUDE_DIRS) -I$(BASEDIR)/../../include -I$(BASEDIR)/include


HELLOWORLDTEST_LIBS_RELEASE= $(LIBS) -L/home/eprosima/raspi/boost_inst_dir/lib/ -L$(BASEDIR)/../../lib/$(EPROSIMA_TARGET) -lfastrtps -lboost_system -lboost_thread 

HELLOWORLDTEST_SRC_CFILES=

HELLOWORLDTEST_SRC_CPPFILES= \
	$(BASEDIR)/src/main.cpp \
	$(BASEDIR)/src/utils/md5.cpp \
	$(BASEDIR)/src/shapesdemo/ShapesDemo.cpp \
	$(BASEDIR)/src/shapesdemo/ShapeTopicDataType.cpp \
	$(BASEDIR)/src/shapesdemo/ShapePublisher.cpp

# Project sources are copied to the current directory
HELLOWORLDTEST_SRCS= $(HELLOWORLDTEST_SRC_CFILES) $(HELLOWORLDTEST_SRC_CPPFILES)

# Source directories
HELLOWORLDTEST_SOURCES_DIRS_AUX= $(foreach srcdir, $(dir $(HELLOWORLDTEST_SRCS)), $(srcdir))
HELLOWORLDTEST_SOURCES_DIRS= $(shell echo $(HELLOWORLDTEST_SOURCES_DIRS_AUX) | tr " " "\n" | sort | uniq | tr "\n" " ")


HELLOWORLDTEST_OBJS_RELEASE = $(foreach obj,$(notdir $(addsuffix .o, $(HELLOWORLDTEST_SRCS))), $(HELLOWORLDTEST_OUTDIR_RELEASE)/$(obj))
HELLOWORLDTEST_DEPS_RELEASE = $(foreach dep,$(notdir $(addsuffix .d, $(HELLOWORLDTEST_SRCS))), $(HELLOWORLDTEST_OUTDIR_RELEASE)/$(dep))

OBJS+=  $(HELLOWORLDTEST_OBJS_RELEASE)
DEPS+=  $(HELLOWORLDTEST_DEPS_RELEASE)

.PHONY: shapesdemo checkHelloWorldTestDirectories

shapesdemo: checkHelloWorldTestDirectories  $(HELLOWORLDTEST_TARGET)

checkHelloWorldTestDirectories:
	@mkdir -p $(OUTDIR)
	@mkdir -p $(HELLOWORLDTEST_OUTDIR)
	@mkdir -p $(HELLOWORLDTEST_OUTDIR_RELEASE)
	@mkdir -p $(BASEDIR)/bin
	@mkdir -p $(BASEDIR)/bin/$(EPROSIMA_TARGET)



$(HELLOWORLDTEST_TARGET): $(HELLOWORLDTEST_OBJS_RELEASE)
	$(LN) $(LDFLAGS) -o $(HELLOWORLDTEST_TARGET) $(HELLOWORLDTEST_OBJS_RELEASE) $(LIBRARY_PATH) $(HELLOWORLDTEST_LIBS_RELEASE) $(HELLOWORLDTEST_STATIC_LIBS)

vpath %.c $(HELLOWORLDTEST_SOURCES_DIRS)
vpath %.cpp $(HELLOWORLDTEST_SOURCES_DIRS)



$(HELLOWORLDTEST_OUTDIR_RELEASE)/%.cpp.o:%.cpp
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CPP) $(HELLOWORLDTEST_CFLAGS) -MM $(HELLOWORLDTEST_CFLAGS) $(HELLOWORLDTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(HELLOWORLDTEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CPP) $(HELLOWORLDTEST_CFLAGS) $(HELLOWORLDTEST_INCLUDE_DIRS) $< -o $@

.PHONY: all

all: State

clean:
	@rm -f $(OBJS)
	@rm -f $(DEPS)

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPS)
endif
