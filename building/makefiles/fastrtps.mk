FASTRTPS_OUTDIR= $(OUTDIR)/fastrtps
FASTRTPS_OUTDIR_DEBUG = $(FASTRTPS_OUTDIR)/debug
FASTRTPS_OUTDIR_RELEASE = $(FASTRTPS_OUTDIR)/release

EPROSIMADIR= $(BASEDIR)/thirdparty/dev-env


# Get product version.
FASTRTPSVERSION=-$(shell $(BASEDIR)/thirdparty/dev-env/scripts/common_pack_functions.sh printVersionFromCPP include/fastrtps/fastrtps_version.h)

FASTRTPS_SED_OUTPUT_DIR_DEBUG= $(subst /,\\/,$(FASTRTPS_OUTDIR_DEBUG))
FASTRTPS_SED_OUTPUT_DIR_RELEASE= $(subst /,\\/,$(FASTRTPS_OUTDIR_RELEASE))

FASTRTPS_TARGET_DEBUG_FILE= libfastrtpsd$(FASTRTPSVERSION).so
FASTRTPS_TARGET_DEBUG_Z_FILE= libfastrtpsd$(FASTRTPSVERSION).a
FASTRTPS_TARGET_FILE= libfastrtps$(FASTRTPSVERSION).so
FASTRTPS_TARGET_Z_FILE= libfastrtps$(FASTRTPSVERSION).a

FASTRTPS_TARGET_DEBUG_FILE_LINK= libfastrtpsd.so
FASTRTPS_TARGET_DEBUG_Z_FILE_LINK= libfastrtpsd.a
FASTRTPS_TARGET_FILE_LINK= libfastrtps.so
FASTRTPS_TARGET_Z_FILE_LINK= libfastrtps.a

FASTRTPS_TARGET_DEBUG= $(BASEDIR)/lib/$(EPROSIMA_TARGET)/$(FASTRTPS_TARGET_DEBUG_FILE)
FASTRTPS_TARGET_DEBUG_Z= $(BASEDIR)/lib/$(EPROSIMA_TARGET)/$(FASTRTPS_TARGET_DEBUG_Z_FILE)
FASTRTPS_TARGET= $(BASEDIR)/lib/$(EPROSIMA_TARGET)/$(FASTRTPS_TARGET_FILE)
FASTRTPS_TARGET_Z= $(BASEDIR)/lib/$(EPROSIMA_TARGET)/$(FASTRTPS_TARGET_Z_FILE)

FASTRTPS_CFLAGS += $(CFLAGS) -std=c++0x -O2 -Wno-unknown-pragmas 
FASTRTPS_CFLAGS_DEBUG += $(CFLAGS_DEBUG) -std=c++0x  -Wno-unknown-pragmas -D__DEBUG


FASTRTPS_INCLUDE_DIRS= $(INCLUDE_DIRS)  -I$(BASEDIR)/include \
		  -I$(BASEDIR)/thirdparty/eprosima-common-code


FASTRTPS_SRC_CPPFILES= \
		  $(BASEDIR)/thirdparty/eprosima-common-code/eProsima_cpp/log/Log.cpp \
		  \
		  $(BASEDIR)/src/cpp/utils/eClock.cpp \
		  $(BASEDIR)/src/cpp/utils/IPFinder.cpp \
		  $(BASEDIR)/src/cpp/utils/md5.cpp \
		  $(BASEDIR)/src/cpp/utils/StringMatching.cpp \
		  \
		  $(BASEDIR)/src/cpp/rtps/resources/ListenResourceImpl.cpp \
		  $(BASEDIR)/src/cpp/rtps/resources/ListenResource.cpp \
		  $(BASEDIR)/src/cpp/rtps/resources/ResourceSend.cpp \
		  $(BASEDIR)/src/cpp/rtps/resources/ResourceSendImpl.cpp \
		  $(BASEDIR)/src/cpp/rtps/resources/ResourceEvent.cpp \
		  $(BASEDIR)/src/cpp/rtps/resources/TimedEvent.cpp \
		  $(BASEDIR)/src/cpp/rtps/resources/TimedEventImpl.cpp \
		  \
		  $(BASEDIR)/src/cpp/rtps/Endpoint.cpp \
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
		  $(BASEDIR)/src/cpp/rtps/history/CacheChangePool.cpp \
		  $(BASEDIR)/src/cpp/rtps/history/History.cpp \
		  $(BASEDIR)/src/cpp/rtps/history/WriterHistory.cpp \
		  \
		  $(BASEDIR)/src/cpp/rtps/reader/timedevent/HeartbeatResponseDelay.cpp \
		  $(BASEDIR)/src/cpp/rtps/reader/timedevent/WriterProxyLiveliness.cpp \
		  $(BASEDIR)/src/cpp/rtps/reader/WriterProxy.cpp \
		  $(BASEDIR)/src/cpp/rtps/reader/StatefulReader.cpp \
		  $(BASEDIR)/src/cpp/rtps/reader/StatelessReader.cpp \
		  $(BASEDIR)/src/cpp/rtps/reader/RTPSReader.cpp \
		  \
		  $(BASEDIR)/src/cpp/rtps/messages/CDRMessagePool.cpp \
		  $(BASEDIR)/src/cpp/rtps/messages/RTPSMessageCreator.cpp \
		  $(BASEDIR)/src/cpp/rtps/messages/RTPSMessageGroup.cpp \
		  $(BASEDIR)/src/cpp/rtps/messages/MessageReceiver.cpp \
		  \
		  $(BASEDIR)/src/cpp/rtps/participant/RTPSParticipant.cpp \
		  $(BASEDIR)/src/cpp/rtps/participant/RTPSParticipantImpl.cpp \
		  \
		  $(BASEDIR)/src/cpp/rtps/RTPSDomain.cpp \
		  \
		  $(BASEDIR)/src/cpp/Domain.cpp \
		  $(BASEDIR)/src/cpp/participant/Participant.cpp \
		  $(BASEDIR)/src/cpp/participant/ParticipantImpl.cpp \
		  \
		  $(BASEDIR)/src/cpp/publisher/Publisher.cpp \
		  $(BASEDIR)/src/cpp/publisher/PublisherImpl.cpp \
		  $(BASEDIR)/src/cpp/publisher/PublisherHistory.cpp \
		  \
		  $(BASEDIR)/src/cpp/subscriber/Subscriber.cpp \
		  $(BASEDIR)/src/cpp/subscriber/SubscriberImpl.cpp \
		  $(BASEDIR)/src/cpp/subscriber/SubscriberHistory.cpp \
		  \
		  $(BASEDIR)/src/cpp/qos/ParameterList.cpp \
		  $(BASEDIR)/src/cpp/qos/ParameterTypes.cpp \
		  $(BASEDIR)/src/cpp/qos/QosList.cpp \
		  $(BASEDIR)/src/cpp/qos/QosPolicies.cpp \
		  $(BASEDIR)/src/cpp/qos/WriterQos.cpp \
		  $(BASEDIR)/src/cpp/qos/ReaderQos.cpp \
		  \
		  $(BASEDIR)/src/cpp/rtps/builtin/BuiltinProtocols.cpp \
		  \
		  $(BASEDIR)/src/cpp/rtps/builtin/discovery/participant/PDPSimple.cpp \
		  $(BASEDIR)/src/cpp/rtps/builtin/discovery/participant/PDPSimpleListener.cpp \
		  $(BASEDIR)/src/cpp/rtps/builtin/discovery/participant/PDPSimpleTopicDataType.cpp \
		  $(BASEDIR)/src/cpp/rtps/builtin/discovery/participant/timedevent/RemoteParticipantLeaseDuration.cpp \
		  $(BASEDIR)/src/cpp/rtps/builtin/discovery/participant/timedevent/ResendParticipantProxyDataPeriod.cpp \
		  \
		  $(BASEDIR)/src/cpp/rtps/builtin/discovery/endpoint/EDP.cpp \
		  $(BASEDIR)/src/cpp/rtps/builtin/discovery/endpoint/EDPSimple.cpp \
		  $(BASEDIR)/src/cpp/rtps/builtin/discovery/endpoint/EDPSimpleListeners.cpp \
		  $(BASEDIR)/src/cpp/rtps/builtin/discovery/endpoint/EDPSimpleTopicDataType.cpp \
		  \
		  $(BASEDIR)/src/cpp/rtps/builtin/discovery/endpoint/EDPStaticXML.cpp \
		  $(BASEDIR)/src/cpp/rtps/builtin/discovery/endpoint/EDPStatic.cpp \
		  \
		  $(BASEDIR)/src/cpp/rtps/builtin/liveliness/WLP.cpp \
		  $(BASEDIR)/src/cpp/rtps/builtin/liveliness/WLPListener.cpp \
		  $(BASEDIR)/src/cpp/rtps/builtin/liveliness/WLPTopicDataType.cpp \
		  $(BASEDIR)/src/cpp/rtps/builtin/liveliness/timedevent/WLivelinessPeriodicAssertion.cpp \
		  \
		  $(BASEDIR)/src/cpp/rtps/builtin/data/ParticipantProxyData.cpp \
		  $(BASEDIR)/src/cpp/rtps/builtin/data/WriterProxyData.cpp \
		  $(BASEDIR)/src/cpp/rtps/builtin/data/ReaderProxyData.cpp 

# Project sources are copied to the current directory
FASTRTPS_SRCS= $(FASTRTPS_SRC_CFILES) $(FASTRTPS_SRC_CPPFILES)

# Source directories
FASTRTPS_SOURCES_DIRS_AUX= $(foreach srcdir, $(dir $(FASTRTPS_SRCS)), $(srcdir))
FASTRTPS_SOURCES_DIRS= $(shell echo $(FASTRTPS_SOURCES_DIRS_AUX) | tr " " "\n" | sort | uniq | tr "\n" " ")

FASTRTPS_OBJS_DEBUG = $(foreach obj,$(notdir $(addsuffix .o, $(basename $(FASTRTPS_SRCS)))), $(FASTRTPS_OUTDIR_DEBUG)/$(obj))
FASTRTPS_DEPS_DEBUG = $(foreach dep,$(notdir $(addsuffix .d, $(basename $(FASTRTPS_SRCS)))), $(FASTRTPS_OUTDIR_DEBUG)/$(dep))
FASTRTPS_OBJS_RELEASE = $(foreach obj,$(notdir $(addsuffix .o, $(basename $(FASTRTPS_SRCS)))), $(FASTRTPS_OUTDIR_RELEASE)/$(obj))
FASTRTPS_DEPS_RELEASE = $(foreach dep,$(notdir $(addsuffix .d, $(basename $(FASTRTPS_SRCS)))), $(FASTRTPS_OUTDIR_RELEASE)/$(dep))

OBJS+= $(FASTRTPS_OBJS_DEBUG) $(FASTRTPS_OBJS_RELEASE)
DEPS+= $(FASTRTPS_DEPS_DEBUG) $(FASTRTPS_DEPS_RELEASE)

.PHONY: FASTRTPS checkFASTRTPSDirectories

FASTRTPS: checkFASTRTPSDirectories $(FASTRTPS_TARGET_DEBUG) $(FASTRTPS_TARGET_DEBUG_Z) $(FASTRTPS_TARGET) $(FASTRTPS_TARGET_Z)

checkFASTRTPSDirectories:
	@mkdir -p $(OUTDIR)
	@mkdir -p $(FASTRTPS_OUTDIR)
	@mkdir -p $(FASTRTPS_OUTDIR_DEBUG)
	@mkdir -p $(FASTRTPS_OUTDIR_RELEASE)
	@mkdir -p lib
	@mkdir -p lib/$(EPROSIMA_TARGET)
ifdef EPROSIMA_LIBRARY_PATH
	@mkdir -p $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)
endif
$(FASTRTPS_TARGET_DEBUG): $(FASTRTPS_OBJS_DEBUG)
	$(LN) $(LDFLAGS) -shared -o $(FASTRTPS_TARGET_DEBUG) $(LIBRARY_PATH) $(LIBS_DEBUG) $(FASTRTPS_OBJS_DEBUG)
	$(LNK) -s -f $(FASTRTPS_TARGET_DEBUG_FILE) $(BASEDIR)/lib/$(EPROSIMA_TARGET)/$(FASTRTPS_TARGET_DEBUG_FILE_LINK)
ifdef EPROSIMA_LIBRARY_PATH
	$(CP) $(FASTRTPS_TARGET_DEBUG) $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)
	$(LNK) -s -f $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)/$(FASTRTPS_TARGET_DEBUG_FILE) $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)/$(FASTRTPS_TARGET_DEBUG_FILE_LINK)
endif

$(FASTRTPS_TARGET_DEBUG_Z): $(FASTRTPS_OBJS_DEBUG)
	$(AR) -cru $(FASTRTPS_TARGET_DEBUG_Z) $(FASTRTPS_OBJS_DEBUG)
	$(LNK) -s -f $(FASTRTPS_TARGET_DEBUG_Z_FILE) $(BASEDIR)/lib/$(EPROSIMA_TARGET)/$(FASTRTPS_TARGET_DEBUG_Z_FILE_LINK)
ifdef EPROSIMA_LIBRARY_PATH
	$(CP) $(FASTRTPS_TARGET_DEBUG_Z) $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)
	$(LNK) -s -f $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)/$(FASTRTPS_TARGET_DEBUG_Z_FILE) $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)/$(FASTRTPS_TARGET_DEBUG_Z_FILE_LINK)
endif
$(FASTRTPS_TARGET): $(FASTRTPS_OBJS_RELEASE)
	$(LN) $(LDFLAGS) -shared -o $(FASTRTPS_TARGET) $(LIBRARY_PATH) $(LIBS) $(FASTRTPS_OBJS_RELEASE)
	$(LNK) -s -f $(FASTRTPS_TARGET_FILE) $(BASEDIR)/lib/$(EPROSIMA_TARGET)/$(FASTRTPS_TARGET_FILE_LINK)
ifdef EPROSIMA_LIBRARY_PATH
	$(CP) $(FASTRTPS_TARGET) $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)
	$(LNK) -s -f $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)/$(FASTRTPS_TARGET_FILE) $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)/$(FASTRTPS_TARGET_FILE_LINK)
endif
$(FASTRTPS_TARGET_Z): $(FASTRTPS_OBJS_RELEASE)
	$(AR) -cru $(FASTRTPS_TARGET_Z) $(FASTRTPS_OBJS_RELEASE)
	$(LNK) -s -f $(FASTRTPS_TARGET_Z_FILE) $(BASEDIR)/lib/$(EPROSIMA_TARGET)/$(FASTRTPS_TARGET_Z_FILE_LINK)
ifdef EPROSIMA_LIBRARY_PATH
	$(CP) $(FASTRTPS_TARGET_Z) $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)
	$(LNK) -s -f $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)/$(FASTRTPS_TARGET_Z_FILE) $(EPROSIMA_LIBRARY_PATH)/proyectos/$(EPROSIMA_TARGET)/$(FASTRTPS_TARGET_Z_FILE_LINK)
endif
vpath %.cpp $(FASTRTPS_SOURCES_DIRS)

$(FASTRTPS_OUTDIR_DEBUG)/%.o:%.cpp
	@echo Calculating dependencies \(DEBUG mode\) $<
	@$(CPP) $(FASTRTPS_CFLAGS_DEBUG) -MM $(FASTRTPS_INCLUDE_DIRS) $< | sed "s/^.*:/$(FASTRTPS_SED_OUTPUT_DIR_DEBUG)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(DEBUG mode\) $<  
	$(CPP) $(FASTRTPS_CFLAGS_DEBUG) $(FASTRTPS_INCLUDE_DIRS) $< -o $@

$(FASTRTPS_OUTDIR_RELEASE)/%.o:%.cpp
	@echo Calculating dependencies \(RELEASE mode\) $<
	@$(CPP) $(FASTRTPS_CFLAGS) -MM $(FASTRTPS_INCLUDE_DIRS) $< | sed "s/^.*:/$(FASTRTPS_SED_OUTPUT_DIR_RELEASE)\/&/g" > $(@:%.o=%.d)
	@echo Compiling \(RELEASE mode\) $<
	$(CPP) $(FASTRTPS_CFLAGS) $(FASTRTPS_INCLUDE_DIRS) $< -o $@


