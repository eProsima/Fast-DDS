ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

NAME=Fast-DDS

#$(INSTALL_ROOT_$(OS)) is pointing to $QNX_TARGET
#by default, unless it was manually re-routed to
#a staging area by setting both INSTALL_ROOT_nto
#and USE_INSTALL_ROOT
FAST-DDS_INSTALL_ROOT ?= $(INSTALL_ROOT_$(OS))

# These commands require GNU Make
FAST-DDS_CMAKE_VERSION = $(shell bash -c "grep VERSION $(PROJECT_ROOT)/../CMakeLists.txt | grep fastdds ")
FAST-DDS_VERSION = .$(subst $\",,$(word 3,$(FAST-DDS_CMAKE_VERSION)))

#choose Release or Debug
CMAKE_BUILD_TYPE ?= Release

#override 'all' target to bypass the default QNX build system
ALL_DEPENDENCIES = Fast-DDS_all
.PHONY: Fast-DDS_all install check clean

CFLAGS += $(FLAGS)
LDFLAGS += -Wl,--build-id=md5

include $(MKFILES_ROOT)/qtargets.mk

FAST-DDS_DIST_DIR = $(PROJECT_ROOT)/../

ASIO_ROOT = $(PROJECT_ROOT)/../thirdparty/asio/asio
FOONATHAN_MEMORY_ROOT = $(PROJECT_ROOT)/../foonathan_memory_vendor
FASTCDR_ROOT = $(PROJECT_ROOT)/../thirdparty/fastcdr
GOOGLETEST_ROOT = $(PROJECT_ROOT)/../googletest
TINYXML2_ROOT = $(PROJECT_ROOT)/../thirdparty/tinyxml2

CMAKE_ARGS += -DBUILD_SHARED_LIBS=ON \
             -DCMAKE_NO_SYSTEM_FROM_IMPORTED=TRUE \
             -DCMAKE_TOOLCHAIN_FILE=$(PROJECT_ROOT)/qnx.nto.toolchain.cmake \
             -DCMAKE_INSTALL_INCLUDEDIR=$(FAST-DDS_INSTALL_ROOT)/usr/include \
             -DCMAKE_INSTALL_PREFIX=$(FAST-DDS_INSTALL_ROOT)/$(CPUVARDIR)/usr \
             -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) \
             -DEXTRA_CMAKE_C_FLAGS="$(CFLAGS)" \
             -DEXTRA_CMAKE_CXX_FLAGS="$(CFLAGS)" \
             -DEXTRA_CMAKE_ASM_FLAGS="$(FLAGS)" \
             -DEXTRA_CMAKE_LINKER_FLAGS="$(LDFLAGS)" \
             -DCPUVARDIR=$(CPUVARDIR) \
             -DCMAKE_PREFIX_PATH=$(FAST-DDS_INSTALL_ROOT) \
             -DCMAKE_MODULE_PATH=$(PWD)/cmake \
             -DCMAKE_INSTALL_BINDIR=$(FAST-DDS_INSTALL_ROOT)/$(CPUVARDIR)/usr/bin \
             -DCMAKE_INSTALL_LIBDIR=$(FAST-DDS_INSTALL_ROOT)/$(CPUVARDIR)/usr/lib \
             -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON \
             -DINCLUDE_INSTALL_DIR=$(FAST-DDS_INSTALL_ROOT)/usr/include \
             -DLIB_INSTALL_DIR=$(FAST-DDS_INSTALL_ROOT)/$(CPUVARDIR)/usr/lib

FAST-DDS_CMAKE_ARGS = $(CMAKE_ARGS) \
                     -DQNX_INSTALL_ROOT=$(FAST-DDS_INSTALL_ROOT) \
                     -DSECURITY=ON \
                     -DCOMPILE_EXAMPLES=OFF \
                     -DEPROSIMA_BUILD_TESTS=OFF

CONFIGURE_ASIO = $(ASIO_ROOT)/configure --exec-prefix $(FAST-DDS_INSTALL_ROOT)/$(CPUVARDIR) --prefix $(FAST-DDS_INSTALL_ROOT)

MAKE_ARGS_ASIO = CXX=$(QNX_HOST)/usr/bin/q++ CC=$(QNX_HOST)/usr/bin/qcc

ifndef NO_TARGET_OVERRIDE
dependencies:
	@cd $(ASIO_ROOT) && aclocal && autoconf && automake --add-missing
	@cd $(ASIO_ROOT) && $(CONFIGURE_ASIO) && JLEVEL=8 make VERBOSE=1 install $(MAKE_ARGS_ASIO)
	@rm -rf $(FAST-DDS_INSTALL_ROOT)/usr/include/asio $(FAST-DDS_INSTALL_ROOT)/usr/include/asio.hpp
	@mkdir -p $(FAST-DDS_INSTALL_ROOT)/usr/include
	@mv $(FAST-DDS_INSTALL_ROOT)/include/asio.hpp $(FAST-DDS_INSTALL_ROOT)/usr/include
	@mv $(FAST-DDS_INSTALL_ROOT)/include/asio $(FAST-DDS_INSTALL_ROOT)/usr/include

	@mkdir -p build/build_fastcdr
	@cd build/build_fastcdr && cmake $(CMAKE_ARGS) $(FASTCDR_ROOT)
	@cd build/build_fastcdr && JLEVEL=8 make VERBOSE=1 all
	@cd build/build_fastcdr && JLEVEL=8 make VERBOSE=1 install

	@mkdir -p build/build_foonathan_memory
	@cd build/build_foonathan_memory && cmake $(CMAKE_ARGS) $(FOONATHAN_MEMORY_ROOT)
	@cd build/build_foonathan_memory && JLEVEL=8 make VERBOSE=1 all
	@cd build/build_foonathan_memory && JLEVEL=8 make VERBOSE=1 install

	@mkdir -p build/build_googletest
	@cd build/build_googletest && cmake $(CMAKE_ARGS) $(GOOGLETEST_ROOT)
	@cd build/build_googletest && JLEVEL=8 make VERBOSE=1 all
	@cd build/build_googletest && JLEVEL=8 make VERBOSE=1 install

	@mkdir -p build/build_tinyxml2
	@cd build/build_tinyxml2 && cmake $(CMAKE_ARGS) $(TINYXML2_ROOT)
	@cd build/build_tinyxml2 && JLEVEL=8 make VERBOSE=1 all
	@cd build/build_tinyxml2 && JLEVEL=8 make VERBOSE=1 install
	@cp $(FAST-DDS_INSTALL_ROOT)/$(CPUVARDIR)/usr/lib/cmake/tinyxml2/tinyxml2Config.cmake $(FAST-DDS_INSTALL_ROOT)/$(CPUVARDIR)/usr/lib/cmake/tinyxml2/TinyXML2Config.cmake

Fast-DDS_all: dependencies
	@mkdir -p build/build_fast-dds
	@cd build/build_fast-dds && cmake $(FAST-DDS_CMAKE_ARGS) $(FAST-DDS_DIST_DIR)
	@cd build/build_fast-dds && JLEVEL=8 make VERBOSE=1 install

install check: Fast-DDS_all
	@echo Done.

clean iclean spotless:
	@cd $(ASIO_ROOT) && make clean
	@rm -rf build

endif
