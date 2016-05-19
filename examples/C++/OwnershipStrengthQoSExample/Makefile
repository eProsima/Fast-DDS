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

all: $(DIRECTORIES) OwnershipStrength

OWNERSHIPSTRENGTH_TARGET= bin/OwnershipStrength

OWNERSHIPSTRENGTH_SRC_CXXFILES= OwnershipStrength.cxx \
								OwnershipStrengthPubSubTypes.cxx \
								OwnershipStrengthPublisher.cxx \
								OwnershipStrengthSubscriber.cxx \
								OwnershipStrengthPubSubMain.cxx

OWNERSHIPSTRENGTH_SRC_CPPFILES= 

# Project sources are copied to the current directory
OWNERSHIPSTRENGTH_SRCS= $(OWNERSHIPSTRENGTH_SRC_CXXFILES) $(OWNERSHIPSTRENGTH_SRC_CPPFILES)

# Source directories
OWNERSHIPSTRENGTH_SOURCES_DIRS_AUX= $(foreach srcdir, $(dir $(OWNERSHIPSTRENGTH_SRCS)), $(srcdir))
OWNERSHIPSTRENGTH_SOURCES_DIRS= $(shell echo $(OWNERSHIPSTRENGTH_SOURCES_DIRS_AUX) | tr " " "\n" | sort | uniq | tr "\n" " ")

OWNERSHIPSTRENGTH_OBJS = $(foreach obj,$(notdir $(addsuffix .o, $(OWNERSHIPSTRENGTH_SRCS))), output/$(obj))
OWNERSHIPSTRENGTH_DEPS = $(foreach dep,$(notdir $(addsuffix .d, $(OWNERSHIPSTRENGTH_SRCS))), output/$(dep))

OBJS+=  $(OWNERSHIPSTRENGTH_OBJS)
DEPS+=  $(OWNERSHIPSTRENGTH_DEPS)

OwnershipStrength: $(OWNERSHIPSTRENGTH_TARGET)

$(OWNERSHIPSTRENGTH_TARGET): $(OWNERSHIPSTRENGTH_OBJS)
	$(LN) $(LDFLAGS) -o $(OWNERSHIPSTRENGTH_TARGET) $(OWNERSHIPSTRENGTH_OBJS) $(LIBS)

vpath %.cxx $(OWNERSHIPSTRENGTH_SOURCES_DIRS)
vpath %.cpp $(OWNERSHIPSTRENGTH_SOURCES_DIRS)

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

.PHONY: OwnershipStrength

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
