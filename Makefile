BASEDIR=.
OUTDIR= $(BASEDIR)/output
CC=arm-unknown-linux-gnueabi-g++
CPP=arm-unknown-linux-gnueabi-g++
LN=arm-unknown-linux-gnueabi-g++
AR=ar
CP=cp
LNK=ln

EPROSIMA_TARGET=armelf_linux_eabi

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
