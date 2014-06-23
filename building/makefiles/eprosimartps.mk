EPROSIMARTPS_OUTDIR= $(OUTDIR)/eprosimartps
EPROSIMARTPS_OUTDIR_DEBUG = $(EPROSIMARTPS_OUTDIR)/debug
EPROSIMARTPS_OUTDIR_RELEASE = $(EPROSIMARTPS_OUTDIR)/release

# Get product version.
EPROSIMARTPSVERSION=-$(shell $(EPROSIMADIR)/scripts/common_pack_functions.sh printVersionFromCPP include/eprosimartps/eprosimartps_version.h)

EPROSIMARTPS_SED_OUTPUT_DIR_DEBUG= $(subst /,\\/,$(EPROSIMARTPS_OUTDIR_DEBUG))
EPROSIMARTPS_SED_OUTPUT_DIR_RELEASE= $(subst /,\\/,$(EPROSIMARTPS_OUTDIR_RELEASE))

EPROSIMARTPS_TARGET_DEBUG_FILE= libeprosimartpsd$(EPROSIMARTPSVERSION).so
EPROSIMARTPS_TARGET_DEBUG_Z_FILE= libeprosimartpsd$(EPROSIMARTPSVERSION).a
EPROSIMARTPS_TARGET_FILE= libeprosimartps$(EPROSIMARTPSVERSION).so
EPROSIMARTPS_TARGET_Z_FILE= libeprosimartps$(EPROSIMARTPSVERSION).a

EPROSIMARTPS_TARGET_DEBUG_FILE_LINK= libeprosimartpsd.so
EPROSIMARTPS_TARGET_DEBUG_Z_FILE_LINK= libeprosimartpsd.a
EPROSIMARTPS_TARGET_FILE_LINK= libeprosimartps.so
EPROSIMARTPS_TARGET_Z_FILE_LINK= libeprosimartps.a

EPROSIMARTPS_TARGET_DEBUG= $(BASEDIR)/lib/$(EPROSIMA_TARGET)/$(EPROSIMARTPS_TARGET_DEBUG_FILE)
EPROSIMARTPS_TARGET_DEBUG_Z= $(BASEDIR)/lib/$(EPROSIMA_TARGET)/$(EPROSIMARTPS_TARGET_DEBUG_Z_FILE)
EPROSIMARTPS_TARGET= $(BASEDIR)/lib/$(EPROSIMA_TARGET)/$(EPROSIMARTPS_TARGET_FILE)
EPROSIMARTPS_TARGET_Z= $(BASEDIR)/lib/$(EPROSIMA_TARGET)/$(EPROSIMARTPS_TARGET_Z_FILE)

EPROSIMARTPS_CFLAGS += $(CFLAGS) -std=c++0x 
EPROSIMARTPS_CFLAGS_DEBUG += $(CFLAGS_DEBUG) -std=c++0x  

EPROSIMARTPS_INCLUDE_DIRS= $(INCLUDE_DIRS) -I$(BASEDIR)/include \
		  -I$(EPROSIMADIR)/code

EPROSIMARTPS_SRC_CPPFILES= \
 	      $(BASEDIR)/src/cpp/utils/IPFinder.cpp \
		  $(BASEDIR)/src/cpp/utils/RTPSLog.cpp \
		  $(BASEDIR)/src/cpp/utils/ObjectPool.cpp \
		  $(BASEDIR)/src/cpp/utils/eClock.cpp \
		  \
		  $(BASEDIR)/src/cpp/Endpoint.cpp \
		  $(BASEDIR)/src/cpp/HistoryCache.cpp \
		  $(BASEDIR)/src/cpp/RTPSMessageCreator.cpp \
		  $(BASEDIR)/src/cpp/CacheChangePool.cpp \
		  \
		  $(BASEDIR)/src/cpp/history/History.cpp \
		  $(BASEDIR)/src/cpp/history/WriterHistory.cpp \
		  \
		  $(BASEDIR)/src/cpp/writer/RTPSWriter.cpp \
		  $(BASEDIR)/src/cpp/writer/ReaderLocator.cpp \
		  $(BASEDIR)/src/cpp/writer/StatelessWriter.cpp \
		  $(BASEDIR)/src/cpp/writer/ReaderProxy.cpp \
		  $(BASEDIR)/src/cpp/writer/StatefulWriter.cpp \
		  $(BASEDIR)/src/cpp/writer/RTPSMessageGroup.cpp \
		  \
		  $(BASEDIR)/src/cpp/reader/RTPSReader.cpp \
		  $(BASEDIR)/src/cpp/reader/StatelessReader.cpp \
		  $(BASEDIR)/src/cpp/reader/WriterProxy.cpp \
		  $(BASEDIR)/src/cpp/reader/StatefulReader.cpp \
		  \
		  $(BASEDIR)/src/cpp/MessageReceiver.cpp \
		  $(BASEDIR)/src/cpp/Participant.cpp \
		  \
		  $(BASEDIR)/src/cpp/resources/ListenResource.cpp \
		  $(BASEDIR)/src/cpp/resources/ResourceSend.cpp \
		  $(BASEDIR)/src/cpp/resources/ResourceEvent.cpp \
		  \
		  $(BASEDIR)/src/cpp/dds/DDSTopicDataType.cpp \
		  $(BASEDIR)/src/cpp/dds/Publisher.cpp \
		  $(BASEDIR)/src/cpp/dds/PublisherListener.cpp \
		  $(BASEDIR)/src/cpp/dds/Subscriber.cpp \
		  $(BASEDIR)/src/cpp/dds/SubscriberListener.cpp \
		  $(BASEDIR)/src/cpp/dds/DomainParticipant \
		  \
		  $(BASEDIR)/src/cpp/qos/ParameterList.cpp \
		  $(BASEDIR)/src/cpp/qos/ParameterTypes.cpp \
		  $(BASEDIR)/src/cpp/qos/QosList.cpp \
		  $(BASEDIR)/src/cpp/qos/DDSQosPolicies.cpp \
		  \
		  $(BASEDIR)/src/cpp/utils/TimedEvent.cpp \
		  $(BASEDIR)/src/cpp/writer/timedevent/PeriodicHeartbeat.cpp \
		  $(BASEDIR)/src/cpp/writer/timedevent/NackResponseDelay.cpp \
		  $(BASEDIR)/src/cpp/writer/timedevent/NackSupressionDuration.cpp \
		  $(BASEDIR)/src/cpp/reader/timedevent/HeartbeatResponseDelay.cpp \
		  $(BASEDIR)/src/cpp/reader/timedevent/WriterProxyLiveliness.cpp \
		  $(BASEDIR)/src/cpp/discovery/timedevent/ResendDiscoveryDataPeriod.cpp \
		  \
		  $(BASEDIR)/src/cpp/discovery/ParticipantDiscoveryProtocol.cpp \
		  $(BASEDIR)/src/cpp/discovery/EndpointDiscoveryProtocol.cpp \
		  $(BASEDIR)/src/cpp/discovery/SimplePDP.cpp \
		  $(BASEDIR)/src/cpp/discovery/SPDPListener.cpp \
		  $(BASEDIR)/src/cpp/discovery/StaticEDP.cpp \
		  $(BASEDIR)/src/cpp/discovery/SimpleEDP.cpp \
		  $(BASEDIR)/src/cpp/discovery/SEDPListeners.cpp \
		  $(BASEDIR)/src/cpp/discovery/data/DiscoveredData.cpp \
		  \
		  $(BASEDIR)/src/cpp/qos/WriterQos.cpp \
		  $(BASEDIR)/src/cpp/qos/ReaderQos.cpp \
		  \
		  $(BASEDIR)/src/cpp/liveliness/WriterLiveliness.cpp \
		  $(BASEDIR)/src/cpp/liveliness/LivelinessPeriodicAssertion.cpp \
		  $(BASEDIR)/src/cpp/liveliness/WriterLivelinessListener.cpp \
		  $(BASEDIR)/src/cpp/discovery/timedevent/ParticipantLeaseDuration.cpp 

#		  
#		  $(BASEDIR)/src/cpp/dds/ParameterList.cpp \
#		  $(BASEDIR)/src/cpp/CDRMessage.cpp \
#		  $(BASEDIR)/src/cpp/exceptions/Exception.cpp \
#		  $(BASEDIR)/src/cpp/exceptions/NotEnoughMemoryException.cpp \
#		  $(BASEDIR)/src/cpp/exceptions/BadParamException.cpp

# Project sources are copied to the current directory
EPROSIMARTPS_SRCS= $(EPROSIMARTPS_SRC_CFILES) $(EPROSIMARTPS_SRC_CPPFILES)

# Source directories
EPROSIMARTPS_SOURCES_DIRS_AUX= $(foreach srcdir, $(dir $(EPROSIMARTPS_SRCS)), $(srcdir))
EPROSIMARTPS_SOURCES_DIRS= $(shell echo $(EPROSIMARTPS_SOURCES_DIRS_AUX) | tr " " "\n" | sort | uniq | tr "\n" " ")

EPROSIMARTPS_OBJS_DEBUG = $(foreach obj,$(notdir $(addsuffix .o, $(basename $(EPROSIMARTPS_SRCS)))), $(EPROSIMARTPS_OUTDIR_DEBUG)/$(obj))
EPROSIMARTPS_DEPS_DEBUG = $(foreach dep,$(notdir $(addsuffix .d, $(basename $(EPROSIMARTPS_SRCS)))), $(EPROSIMARTPS_OUTDIR_DEBUG)/$(dep))
EPROSIMARTPS_OBJS_RELEASE = $(foreach obj,$(notdir $(addsuffix .o, $(basename $(EPROSIMARTPS_SRCS)))), $(EPROSIMARTPS_OUTDIR_RELEASE)/$(obj))
EPROSIMARTPS_DEPS_RELEASE = $(foreach dep,$(notdir $(addsuffix .d, $(basename $(EPROSIMARTPS_SRCS)))), $(EPROSIMARTPS_OUTDIR_RELEASE)/$(dep))

OBJS+= $(EPROSIMARTPS_OBJS_DEBUG) $(EPROSIMARTPS_OBJS_RELEASE)
DEPS+= $(EPROSIMARTPS_DEPS_DEBUG) $(EPROSIMARTPS_DEPS_RELEASE)

.PHONY: eprosimartps checkEPROSIMARTPSDirectories

eprosimartps: checkEPROSIMARTPSDirectories $(EPROSIMARTPS_TARGET_DEBUG) $(EPROSIMARTPS_TARGET_DEBUG_Z) $(EPROSIMARTPS_TARGET) $(EPROSIMARTPS_TARGET_Z)

checkEPROSIMARTPSDirectories:
	@mkdir -p $(OUTDIR)
	@mkdir -p $(EPROSIMARTPS_OUTDIR)
	@mkdir -p $(EPROSIMARTPS_OUTDIR_DEBUG)
	@mkdir -p $(EPROSIMARTPS_OUTDIR_RELEASE)
	@mkdir -p lib
	@mkdir -p lib/$(EPROSIMA_TARGET)
	@mkdir -p $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)

$(EPROSIMARTPS_TARGET_DEBUG): $(EPROSIMARTPS_OBJS_DEBUG)
	$(LN) $(LDFLAGS) -shared -o $(EPROSIMARTPS_TARGET_DEBUG) $(LIBRARY_PATH) $(LIBS_DEBUG) $(EPROSIMARTPS_OBJS_DEBUG)
	$(LNK) -s -f $(EPROSIMARTPS_TARGET_DEBUG_FILE) $(BASEDIR)/lib/$(EPROSIMA_TARGET)/$(EPROSIMARTPS_TARGET_DEBUG_FILE_LINK)
	$(CP) $(EPROSIMARTPS_TARGET_DEBUG) $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)
	$(LNK) -s -f $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)/$(EPROSIMARTPS_TARGET_DEBUG_FILE) $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)/$(EPROSIMARTPS_TARGET_DEBUG_FILE_LINK)

$(EPROSIMARTPS_TARGET_DEBUG_Z): $(EPROSIMARTPS_OBJS_DEBUG)
	$(AR) -cru $(EPROSIMARTPS_TARGET_DEBUG_Z) $(EPROSIMARTPS_OBJS_DEBUG)
	$(LNK) -s -f $(EPROSIMARTPS_TARGET_DEBUG_Z_FILE) $(BASEDIR)/lib/$(EPROSIMA_TARGET)/$(EPROSIMARTPS_TARGET_DEBUG_Z_FILE_LINK)
	$(CP) $(EPROSIMARTPS_TARGET_DEBUG_Z) $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)
	$(LNK) -s -f $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)/$(EPROSIMARTPS_TARGET_DEBUG_Z_FILE) $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)/$(EPROSIMARTPS_TARGET_DEBUG_Z_FILE_LINK)

$(EPROSIMARTPS_TARGET): $(EPROSIMARTPS_OBJS_RELEASE)
	$(LN) $(LDFLAGS) -shared -o $(EPROSIMARTPS_TARGET) $(LIBRARY_PATH) $(LIBS) $(EPROSIMARTPS_OBJS_RELEASE)
	$(LNK) -s -f $(EPROSIMARTPS_TARGET_FILE) $(BASEDIR)/lib/$(EPROSIMA_TARGET)/$(EPROSIMARTPS_TARGET_FILE_LINK)
	$(CP) $(EPROSIMARTPS_TARGET) $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)
	$(LNK) -s -f $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)/$(EPROSIMARTPS_TARGET_FILE) $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)/$(EPROSIMARTPS_TARGET_FILE_LINK)

$(EPROSIMARTPS_TARGET_Z): $(EPROSIMARTPS_OBJS_RELEASE)
	$(AR) -cru $(EPROSIMARTPS_TARGET_Z) $(EPROSIMARTPS_OBJS_RELEASE)
	$(LNK) -s -f $(EPROSIMARTPS_TARGET_Z_FILE) $(BASEDIR)/lib/$(EPROSIMA_TARGET)/$(EPROSIMARTPS_TARGET_Z_FILE_LINK)
	$(CP) $(EPROSIMARTPS_TARGET_Z) $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)
	$(LNK) -s -f $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)/$(EPROSIMARTPS_TARGET_Z_FILE) $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)/$(EPROSIMARTPS_TARGET_Z_FILE_LINK)

vpath %.cpp $(EPROSIMARTPS_SOURCES_DIRS)

$(EPROSIMARTPS_OUTDIR_DEBUG)/%.o:%.cpp
	@echo Calculating dependencies \(DEBUG mode\) $<
	@$(CPP) $(EPROSIMARTPS_CFLAGS_DEBUG) -MM $(EPROSIMARTPS_INCLUDE_DIRS) $< | sed "s/^.*:/$(EPROSIMARTPS_SED_OUTPUT_DIR_DEBUG)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(DEBUG mode\) $<  
	$(CPP) $(EPROSIMARTPS_CFLAGS_DEBUG) $(EPROSIMARTPS_INCLUDE_DIRS) $< -o $@

$(EPROSIMARTPS_OUTDIR_RELEASE)/%.o:%.cpp
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CPP) $(EPROSIMARTPS_CFLAGS) -MM $(EPROSIMARTPS_INCLUDE_DIRS) $< | sed "s/^.*:/$(EPROSIMARTPS_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	$(CPP) $(EPROSIMARTPS_CFLAGS) $(EPROSIMARTPS_INCLUDE_DIRS) $< -o $@

