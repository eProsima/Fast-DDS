BASEDIR=.
OUTDIR= $(BASEDIR)/output
CC=gcc
CPP=g++
LN=g++
AR=ar
CP=cp
LNK=ln

EPROSIMA_TARGET=x64Linux2.6gcc

-include $(BASEDIR)/thirdparty/dev-env/building/makefiles/eProsima.mk
-include $(BASEDIR)/building/makefiles/eprosimartps.mk

.PHONY: all

all: eprosimartps

clean:
	@rm -f $(OBJS)
	@rm -f $(DEPS)

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPS)
endif
