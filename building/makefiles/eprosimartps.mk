EPROSIMARTPS_OUTDIR= $(OUTDIR)/eprosimartps
EPROSIMARTPS_OUTDIR_DEBUG = $(EPROSIMARTPS_OUTDIR)/debug
EPROSIMARTPS_OUTDIR_RELEASE = $(EPROSIMARTPS_OUTDIR)/release

EPROSIMADIR= $(BASEDIR)/thirdparty/dev-env


# Get product version.
EPROSIMARTPSVERSION=-$(shell $(BASEDIR)/thirdparty/dev-env/scripts/common_pack_functions.sh printVersionFromCPP include/eprosimartps/eprosimartps_version.h)

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

EPROSIMARTPS_CFLAGS += $(CFLAGS) -std=c++0x -O2 -Wno-unknown-pragmas 
EPROSIMARTPS_CFLAGS_DEBUG += $(CFLAGS_DEBUG) -std=c++0x  -Wno-unknown-pragmas -D__DEBUG


EPROSIMARTPS_INCLUDE_DIRS= $(INCLUDE_DIRS)  -I$(BASEDIR)/include \
		  -I$(BASEDIR)/thirdparty/eprosima-common-code


EPROSIMARTPS_SRC_CPPFILES= \
		  $(BASEDIR)/thirdparty/eprosima-common-code/eProsima_cpp/log/Log.cpp \
		  \
		  $(BASEDIR)/src/cpp/rtps/resources/ListenResourceImpl.cpp \
		  $(BASEDIR)/src/cpp/rtps/resources/ListenResource.cpp \
		  $(BASEDIR)/src/cpp/rtps/resources/ResourceSend.cpp \
		  $(BASEDIR)/src/cpp/rtps/resources/ResourceSendImpl.cpp \
		  $(BASEDIR)/src/cpp/rtps/resources/ResourceEvent.cpp \
		  $(BASEDIR)/src/cpp/rtps/resources/TimedEvent.cpp \
		  $(BASEDIR)/src/cpp/rtps/resources/TimedEventImpl.cpp \
		  \
		  $(BASEDIR)/src/cpp/rtps/writer/RTPSWriter.cpp \
		  $(BASEDIR)/src/cpp/rtps/writer/StatefulWriter.cpp \
		  $(BASEDIR)/src/cpp/rtps/writer/ReaderProxy.cpp \
		  $(BASEDIR)/src/cpp/rtps/writer/StatelessWriter.cpp \
		  $(BASEDIR)/src/cpp/rtps/writer/ReaderLocator.cpp \
		  $(BASEDIR)/src/cpp/rtps/writer/timedevent/PeriodicHeartbeat.cpp \
		  $(BASEDIR)/src/cpp/rtps/writer/timedevent/NackResponseDelay.cpp \
		  $(BASEDIR)/src/cpp/rtps/writer/timedevent/NackSupressionDuration.cpp \
		  $(BASEDIR)/src/cpp/rtps/writer/timedevent/UnsentChangesNotEmptyEvent.cpp \
		  \
		  $(BASEDIR)/src/cpp/rtps/Endpoint.cpp \
		  $(BASEDIR)/src/cpp/rtps/Participant.cpp \
		  $(BASEDIR)/src/cpp/rtps/ParticipantImpl.cpp \
		  \
	      $(BASEDIR)/src/cpp/utils/IPFinder.cpp \
		  $(BASEDIR)/src/cpp/utils/CDRMessagePool.cpp \
		  $(BASEDIR)/src/cpp/utils/eClock.cpp \
		  $(BASEDIR)/src/cpp/utils/StringMatching.cpp \
		  $(BASEDIR)/src/cpp/utils/TimedEvent.cpp \
		  $(BASEDIR)/src/cpp/utils/md5.cpp \
		  \
		  $(BASEDIR)/src/cpp/RTPSMessageCreator.cpp \
		  $(BASEDIR)/src/cpp/CacheChangePool.cpp \
		  \
		  $(BASEDIR)/src/cpp/history/History.cpp \
		  $(BASEDIR)/src/cpp/history/WriterHistory.cpp \
		  $(BASEDIR)/src/cpp/history/ReaderHistory.cpp \
		  \
		  $(BASEDIR)/src/cpp/writer/RTPSWriter.cpp \
		  $(BASEDIR)/src/cpp/writer/ReaderLocator.cpp \
		  $(BASEDIR)/src/cpp/writer/StatelessWriter.cpp \
		  $(BASEDIR)/src/cpp/writer/ReaderProxy.cpp \
		  $(BASEDIR)/src/cpp/writer/ReaderProxyData.cpp \
		  $(BASEDIR)/src/cpp/writer/StatefulWriter.cpp \
		  $(BASEDIR)/src/cpp/writer/RTPSMessageGroup.cpp \
		  $(BASEDIR)/src/cpp/writer/timedevent/PeriodicHeartbeat.cpp \
		  $(BASEDIR)/src/cpp/writer/timedevent/NackResponseDelay.cpp \
		  $(BASEDIR)/src/cpp/writer/timedevent/NackSupressionDuration.cpp \
		  $(BASEDIR)/src/cpp/writer/timedevent/UnsentChangesNotEmptyEvent.cpp \
		  \
		  $(BASEDIR)/src/cpp/reader/RTPSReader.cpp \
		  $(BASEDIR)/src/cpp/reader/StatelessReader.cpp \
		  $(BASEDIR)/src/cpp/reader/WriterProxy.cpp \
		  $(BASEDIR)/src/cpp/reader/WriterProxyData.cpp \
		  $(BASEDIR)/src/cpp/reader/StatefulReader.cpp \
		  $(BASEDIR)/src/cpp/reader/timedevent/HeartbeatResponseDelay.cpp \
		  $(BASEDIR)/src/cpp/reader/timedevent/WriterProxyLiveliness.cpp \
		  \
		  $(BASEDIR)/src/cpp/MessageReceiver.cpp \
		   $(BASEDIR)/src/cpp/ParticipantProxyData.cpp \
		  \
		  $(BASEDIR)/src/cpp/resources/ListenResource.cpp \
		  $(BASEDIR)/src/cpp/resources/ResourceSend.cpp \
		  $(BASEDIR)/src/cpp/resources/ResourceEvent.cpp \
		  \
		  $(BASEDIR)/src/cpp/pubsub/TopicDataType.cpp \
		  $(BASEDIR)/src/cpp/pubsub/Publisher.cpp \
		  $(BASEDIR)/src/cpp/pubsub/PublisherListener.cpp \
		  $(BASEDIR)/src/cpp/pubsub/Subscriber.cpp \
		  $(BASEDIR)/src/cpp/pubsub/SubscriberListener.cpp \
		  $(BASEDIR)/src/cpp/pubsub/RTPSDomain.cpp \
		  \
		  $(BASEDIR)/src/cpp/qos/ParameterList.cpp \
		  $(BASEDIR)/src/cpp/qos/ParameterTypes.cpp \
		  $(BASEDIR)/src/cpp/qos/QosList.cpp \
		  $(BASEDIR)/src/cpp/qos/QosPolicies.cpp \
		  $(BASEDIR)/src/cpp/qos/WriterQos.cpp \
		  $(BASEDIR)/src/cpp/qos/ReaderQos.cpp \
		  \
		  $(BASEDIR)/src/cpp/builtin/BuiltinProtocols.cpp \
		  \
		  $(BASEDIR)/src/cpp/builtin/discovery/participant/PDPSimple.cpp \
		  $(BASEDIR)/src/cpp/builtin/discovery/participant/PDPSimpleListener.cpp \
		  $(BASEDIR)/src/cpp/builtin/discovery/participant/PDPSimpleTopicDataType.cpp \
		  $(BASEDIR)/src/cpp/builtin/discovery/participant/timedevent/RemoteParticipantLeaseDuration.cpp \
		  $(BASEDIR)/src/cpp/builtin/discovery/participant/timedevent/ResendParticipantProxyDataPeriod.cpp \
		  \
		  $(BASEDIR)/src/cpp/builtin/discovery/endpoint/EDP.cpp \
		  $(BASEDIR)/src/cpp/builtin/discovery/endpoint/EDPSimple.cpp \
		  $(BASEDIR)/src/cpp/builtin/discovery/endpoint/EDPSimpleListeners.cpp \
		  $(BASEDIR)/src/cpp/builtin/discovery/endpoint/EDPSimpleTopicDataType.cpp \
		  \
		  $(BASEDIR)/src/cpp/builtin/discovery/endpoint/EDPStaticXML.cpp \
		  $(BASEDIR)/src/cpp/builtin/discovery/endpoint/EDPStatic.cpp \
		  \
		  $(BASEDIR)/src/cpp/builtin/liveliness/WLP.cpp \
		  $(BASEDIR)/src/cpp/builtin/liveliness/WLPListener.cpp \
		  $(BASEDIR)/src/cpp/builtin/liveliness/WLPTopicDataType.cpp \
		  $(BASEDIR)/src/cpp/builtin/liveliness/timedevent/WLivelinessPeriodicAssertion.cpp 


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
ifdef EPROSIMA_LIBRARY_PATH
	@mkdir -p $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)
endif
$(EPROSIMARTPS_TARGET_DEBUG): $(EPROSIMARTPS_OBJS_DEBUG)
	$(LN) $(LDFLAGS) -shared -o $(EPROSIMARTPS_TARGET_DEBUG) $(LIBRARY_PATH) $(LIBS_DEBUG) $(EPROSIMARTPS_OBJS_DEBUG)
	$(LNK) -s -f $(EPROSIMARTPS_TARGET_DEBUG_FILE) $(BASEDIR)/lib/$(EPROSIMA_TARGET)/$(EPROSIMARTPS_TARGET_DEBUG_FILE_LINK)
ifdef EPROSIMA_LIBRARY_PATH
	$(CP) $(EPROSIMARTPS_TARGET_DEBUG) $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)
	$(LNK) -s -f $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)/$(EPROSIMARTPS_TARGET_DEBUG_FILE) $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)/$(EPROSIMARTPS_TARGET_DEBUG_FILE_LINK)
endif

$(EPROSIMARTPS_TARGET_DEBUG_Z): $(EPROSIMARTPS_OBJS_DEBUG)
	$(AR) -cru $(EPROSIMARTPS_TARGET_DEBUG_Z) $(EPROSIMARTPS_OBJS_DEBUG)
	$(LNK) -s -f $(EPROSIMARTPS_TARGET_DEBUG_Z_FILE) $(BASEDIR)/lib/$(EPROSIMA_TARGET)/$(EPROSIMARTPS_TARGET_DEBUG_Z_FILE_LINK)
ifdef EPROSIMA_LIBRARY_PATH
	$(CP) $(EPROSIMARTPS_TARGET_DEBUG_Z) $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)
	$(LNK) -s -f $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)/$(EPROSIMARTPS_TARGET_DEBUG_Z_FILE) $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)/$(EPROSIMARTPS_TARGET_DEBUG_Z_FILE_LINK)
endif
$(EPROSIMARTPS_TARGET): $(EPROSIMARTPS_OBJS_RELEASE)
	$(LN) $(LDFLAGS) -shared -o $(EPROSIMARTPS_TARGET) $(LIBRARY_PATH) $(LIBS) $(EPROSIMARTPS_OBJS_RELEASE)
	$(LNK) -s -f $(EPROSIMARTPS_TARGET_FILE) $(BASEDIR)/lib/$(EPROSIMA_TARGET)/$(EPROSIMARTPS_TARGET_FILE_LINK)
ifdef EPROSIMA_LIBRARY_PATH
	$(CP) $(EPROSIMARTPS_TARGET) $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)
	$(LNK) -s -f $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)/$(EPROSIMARTPS_TARGET_FILE) $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)/$(EPROSIMARTPS_TARGET_FILE_LINK)
endif
$(EPROSIMARTPS_TARGET_Z): $(EPROSIMARTPS_OBJS_RELEASE)
	$(AR) -cru $(EPROSIMARTPS_TARGET_Z) $(EPROSIMARTPS_OBJS_RELEASE)
	$(LNK) -s -f $(EPROSIMARTPS_TARGET_Z_FILE) $(BASEDIR)/lib/$(EPROSIMA_TARGET)/$(EPROSIMARTPS_TARGET_Z_FILE_LINK)
ifdef EPROSIMA_LIBRARY_PATH
	$(CP) $(EPROSIMARTPS_TARGET_Z) $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)
	$(LNK) -s -f $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)/$(EPROSIMARTPS_TARGET_Z_FILE) $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)/$(EPROSIMARTPS_TARGET_Z_FILE_LINK)
endif
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


