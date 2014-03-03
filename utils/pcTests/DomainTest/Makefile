BASEDIR=.
OUTDIR= $(BASEDIR)/output
CC=gcc
CPP=g++
LN=g++
AR=ar
CP=cp

-include $(EPROSIMADIR)/building/makefiles/eProsima.mk

DOMAINTEST_OUTDIR= $(OUTDIR)/DomainTest
DOMAINTEST_OUTDIR_DEBUG = $(DOMAINTEST_OUTDIR)/debug
DOMAINTEST_OUTDIR_RELEASE = $(DOMAINTEST_OUTDIR)/release

DOMAINTEST_SED_OUTPUT_DIR_DEBUG= $(subst /,\\/,$(DOMAINTEST_OUTDIR_DEBUG))
DOMAINTEST_SED_OUTPUT_DIR_RELEASE= $(subst /,\\/,$(DOMAINTEST_OUTDIR_RELEASE))

DOMAINTEST_CFLAGS = $(CFLAGS) -std=c++0x 
DOMAINTEST_CFLAGS_DEBUG = $(CFLAGS_DEBUG) -std=c++0x 

DOMAINTEST_TARGET_DEBUG= $(BASEDIR)/bin/$(EPROSIMA_TARGET)/DomainTestd
DOMAINTEST_TARGET= $(BASEDIR)/bin/$(EPROSIMA_TARGET)/DomainTest

DOMAINTEST_INCLUDE_DIRS= $(INCLUDE_DIRS) -I$(BASEDIR)/../../../include \
                         -I$(EPROSIMADIR)/code

DOMAINTEST_LIBS_DEBUG= $(LIBS_DEBUG) -L$(BASEDIR)/../../../lib/$(EPROSIMA_TARGET) -leprosimartpsd -lboost_system -lboost_thread
DOMAINTEST_LIBS_RELEASE= $(LIBS) -L$(BASEDIR)/../../../lib/$(EPROSIMA_TARGET) -leprosimartps -lboost_system -lboost_thread

DOMAINTEST_SRC_CFILES=

DOMAINTEST_SRC_CPPFILES= $(BASEDIR)/DomainTest.cpp

# Project sources are copied to the current directory
DOMAINTEST_SRCS= $(DOMAINTEST_SRC_CFILES) $(DOMAINTEST_SRC_CPPFILES)

# Source directories
DOMAINTEST_SOURCES_DIRS_AUX= $(foreach srcdir, $(dir $(DOMAINTEST_SRCS)), $(srcdir))
DOMAINTEST_SOURCES_DIRS= $(shell echo $(DOMAINTEST_SOURCES_DIRS_AUX) | tr " " "\n" | sort | uniq | tr "\n" " ")

DOMAINTEST_OBJS_DEBUG = $(foreach obj,$(notdir $(addsuffix .o, $(DOMAINTEST_SRCS))), $(DOMAINTEST_OUTDIR_DEBUG)/$(obj))
DOMAINTEST_DEPS_DEBUG = $(foreach dep,$(notdir $(addsuffix .d, $(DOMAINTEST_SRCS))), $(DOMAINTEST_OUTDIR_DEBUG)/$(dep))
DOMAINTEST_OBJS_RELEASE = $(foreach obj,$(notdir $(addsuffix .o, $(DOMAINTEST_SRCS))), $(DOMAINTEST_OUTDIR_RELEASE)/$(obj))
DOMAINTEST_DEPS_RELEASE = $(foreach dep,$(notdir $(addsuffix .d, $(DOMAINTEST_SRCS))), $(DOMAINTEST_OUTDIR_RELEASE)/$(dep))

OBJS+= $(DOMAINTEST_OBJS_DEBUG) $(DOMAINTEST_OBJS_RELEASE)
DEPS+= $(DOMAINTEST_DEPS_DEBUG) $(DOMAINTEST_DEPS_RELEASE)

.PHONY: DomainTest checkDomainTestDirectories

DomainTest: checkDomainTestDirectories $(DOMAINTEST_TARGET_DEBUG) $(DOMAINTEST_TARGET)

checkDomainTestDirectories:
	@mkdir -p $(OUTDIR)
	@mkdir -p $(DOMAINTEST_OUTDIR)
	@mkdir -p $(DOMAINTEST_OUTDIR_DEBUG)
	@mkdir -p $(DOMAINTEST_OUTDIR_RELEASE)
	@mkdir -p $(BASEDIR)/bin
	@mkdir -p $(BASEDIR)/bin/$(EPROSIMA_TARGET)

$(DOMAINTEST_TARGET_DEBUG): $(DOMAINTEST_OBJS_DEBUG)
	$(LN) $(LDFLAGS) -o $(DOMAINTEST_TARGET_DEBUG) $(DOMAINTEST_OBJS_DEBUG) $(LIBRARY_PATH) $(DOMAINTEST_LIBS_DEBUG) $(DOMAINTEST_STATIC_LIBS)

$(DOMAINTEST_TARGET): $(DOMAINTEST_OBJS_RELEASE)
	$(LN) $(LDFLAGS) -o $(DOMAINTEST_TARGET) $(DOMAINTEST_OBJS_RELEASE) $(LIBRARY_PATH) $(DOMAINTEST_LIBS_RELEASE) $(DOMAINTEST_STATIC_LIBS)

vpath %.c $(DOMAINTEST_SOURCES_DIRS)
vpath %.cpp $(DOMAINTEST_SOURCES_DIRS)

$(DOMAINTEST_OUTDIR_DEBUG)/%.o:%.c
	@echo Calculating dependencies \(DEBUG mode\) $<
	@$(CC) $(DOMAINTEST_CFLAGS_DEBUG) -MM $(DOMAINTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(DOMAINTEST_SED_OUTPUT_DIR_DEBUG)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(DEBUG mode\) $<  
	@$(CC) $(DOMAINTEST_CFLAGS_DEBUG) $(DOMAINTEST_INCLUDE_DIRS) $< -o $@

$(DOMAINTEST_OUTDIR_RELEASE)/%.o:%.c
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CC) $(DOMAINTEST_CFLAGS) -MM $(DOMAINTEST_CFLAGS) $(DOMAINTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(DOMAINTEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CC) $(DOMAINTEST_CFLAGS) $(DOMAINTEST_INCLUDE_DIRS) $< -o $@

$(DOMAINTEST_OUTDIR_DEBUG)/%.c.o:%.c
	@echo Calculating dependencies \(DEBUG mode\) $<
	@$(CC) $(DOMAINTEST_CFLAGS_DEBUG) -MM $(DOMAINTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(DOMAINTEST_SED_OUTPUT_DIR_DEBUG)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(DEBUG mode\) $<  
	@$(CC) $(DOMAINTEST_CFLAGS_DEBUG) $(DOMAINTEST_INCLUDE_DIRS) $< -o $@

$(DOMAINTEST_OUTDIR_RELEASE)/%.c.o:%.c
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CC) $(DOMAINTEST_CFLAGS) -MM $(DOMAINTEST_CFLAGS) $(DOMAINTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(DOMAINTEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CC) $(DOMAINTEST_CFLAGS) $(DOMAINTEST_INCLUDE_DIRS) $< -o $@

$(DOMAINTEST_OUTDIR_DEBUG)/%.cpp.o:%.cpp
	@echo Calculating dependencies \(DEBUG mode\) $<
	@$(CPP) $(DOMAINTEST_CFLAGS_DEBUG) -MM $(DOMAINTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(DOMAINTEST_SED_OUTPUT_DIR_DEBUG)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(DEBUG mode\) $<
	@$(CPP) $(DOMAINTEST_CFLAGS_DEBUG) $(DOMAINTEST_INCLUDE_DIRS) $< -o $@

$(DOMAINTEST_OUTDIR_RELEASE)/%.cpp.o:%.cpp
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CPP) $(DOMAINTEST_CFLAGS) -MM $(DOMAINTEST_CFLAGS) $(DOMAINTEST_INCLUDE_DIRS) $< | sed "s/^.*:/$(DOMAINTEST_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	@$(CPP) $(DOMAINTEST_CFLAGS) $(DOMAINTEST_INCLUDE_DIRS) $< -o $@

.PHONY: all

all: Domain

clean:
	@rm -f $(OBJS)
	@rm -f $(DEPS)

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPS)
endif
