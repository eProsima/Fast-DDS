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

LIBS = -lfastcdr -lfastrtps -lboost_system -lboost_thread -lboost_date_time $(SYSLIBS)

DIRECTORIES= output.dir bin.dir

all: $(DIRECTORIES) HelloWorldExample

HELLOWORLDEXAMPLE_TARGET= bin/HelloWorldExample

HELLOWORLDEXAMPLE_SRC_CXXFILES=

HELLOWORLDEXAMPLE_SRC_CPPFILES= HelloWorldExample/HelloWorld.cpp \
								HelloWorldExample/HelloWorldType.cpp \
								HelloWorldExample/HelloWorldPublisher.cpp \
								HelloWorldExample/HelloWorldSubscriber.cpp \
								HelloWorldExample/HelloWorld_main.cpp 


# Project sources are copied to the current directory
HELLOWORLDEXAMPLE_SRCS= $(HELLOWORLDEXAMPLE_SRC_CXXFILES) $(HELLOWORLDEXAMPLE_SRC_CPPFILES)

# Source directories
HELLOWORLDEXAMPLE_SOURCES_DIRS_AUX= $(foreach srcdir, $(dir $(HELLOWORLDEXAMPLE_SRCS)), $(srcdir))
HELLOWORLDEXAMPLE_SOURCES_DIRS= $(shell echo $(HELLOWORLDEXAMPLE_SOURCES_DIRS_AUX) | tr " " "\n" | sort | uniq | tr "\n" " ")

HELLOWORLDEXAMPLE_OBJS = $(foreach obj,$(notdir $(addsuffix .o, $(HELLOWORLDEXAMPLE_SRCS))), output/$(obj))
HELLOWORLDEXAMPLE_DEPS = $(foreach dep,$(notdir $(addsuffix .d, $(HELLOWORLDEXAMPLE_SRCS))), output/$(dep))

OBJS+=  $(HELLOWORLDEXAMPLE_OBJS)
DEPS+=  $(HELLOWORLDEXAMPLE_DEPS)

HelloWorldExample: $(HELLOWORLDEXAMPLE_TARGET)

$(HELLOWORLDEXAMPLE_TARGET): $(HELLOWORLDEXAMPLE_OBJS)
	$(LN) $(LDFLAGS) -o $(HELLOWORLDEXAMPLE_TARGET) $(HELLOWORLDEXAMPLE_OBJS) $(LIBS)

vpath %.cxx $(HELLOWORLDEXAMPLE_SOURCES_DIRS)
vpath %.cpp $(HELLOWORLDEXAMPLE_SOURCES_DIRS)

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

.PHONY: HelloWorldExample

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
