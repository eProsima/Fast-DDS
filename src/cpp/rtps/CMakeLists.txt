# Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#{{{ Unit grouping basic RTPS source code without dependencies on other library sources

fastdds_add_unit(rtps::basic
    SOURCES
    ${PROJECT_SOURCE_DIR}/include/fastdds/rtps/attributes/EndpointSecurityAttributes.hpp
    ${PROJECT_SOURCE_DIR}/include/fastdds/rtps/attributes/PropertyPolicy.hpp
    ${PROJECT_SOURCE_DIR}/include/fastdds/rtps/attributes/ThreadSettings.hpp
    ${PROJECT_SOURCE_DIR}/include/fastdds/rtps/common/SerializedPayload.hpp
    ${PROJECT_SOURCE_DIR}/include/fastdds/rtps/flowcontrol/FlowControllerConsts.hpp
    ${PROJECT_SOURCE_DIR}/include/fastdds/rtps/transport/PortBasedTransportDescriptor.hpp
    ${PROJECT_SOURCE_DIR}/include/fastdds/rtps/transport/TransportInterface.hpp

    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/EndpointSecurityAttributes.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/PropertyPolicy.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/ThreadSettings.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/FlowControllerConsts.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/PortBasedTransportDescriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TransportInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Token.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/exceptions/Exception.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/StringMatching.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/TimedConditionVariable.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/UnitsParser.cpp
    $<$<BOOL:${SECURITY}>:${PROJECT_SOURCE_DIR}/src/cpp/rtps/security/exceptions/SecurityException.cpp>
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    )

#}}}

#{{{ Units grouping the timed events source code

# The event thread and the implementation of the timed events it runs.
fastdds_add_unit(rtps::resource-event
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/ResourceEvent.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/ResourceEvent.h
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEventImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEventImpl.h
    DEPENDS
    log
    time
    utils::network
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    )

# The timed event used by the entities, apart from the event thread so it can be mocked alone.
fastdds_add_unit(rtps::timed-event
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEvent.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEvent.h
    DEPENDS
    rtps::resource-event
    time
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    )

#}}}

#{{{ Unit grouping common RTPS source code: GUID prefixes and CDR messages

fastdds_add_unit(rtps::common
    SOURCES
    ${PROJECT_SOURCE_DIR}/include/fastdds/rtps/common/GuidPrefix_t.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/GuidPrefix_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/CDRMessage.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/CDRMessage.hpp
    DEPENDS
    log
    time
    utils::network
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    )

#}}}

#{{{ Unit grouping transport source code shared by the transport unit tests.

set(rtps_transport_tls_sources)
if(TLS_FOUND)
    set(rtps_transport_tls_sources
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPAcceptorSecure.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPChannelResourceSecure.cpp
        )
endif()

fastdds_add_unit(rtps::transport
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/RTPSMessageCreator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/NetworkBuffer.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/NetworkConfiguration.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/ChainingTransport.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/ChannelResource.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/MulticastTransportInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPAcceptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPAcceptorBasic.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPChannelResource.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPChannelResourceBasic.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/tcp/RTCPMessageManager.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/tcp/TCPControlMessage.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPTransportInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPv4Transport.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPv6Transport.cpp
    ${rtps_transport_tls_sources}
    DEPENDS
    log
    rtps::basic
    rtps::common
    time
    utils::network
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    $<$<BOOL:${TLS_FOUND}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto>
    $<$<BOOL:${QNX}>:socket>
    )

#}}}

#{{{ Unit grouping the UDP transport source code

fastdds_add_unit(rtps::udp-transport
    SOURCES
    ${PROJECT_SOURCE_DIR}/include/fastdds/rtps/transport/UDPTransportDescriptor.hpp
    ${PROJECT_SOURCE_DIR}/include/fastdds/rtps/transport/UDPv4TransportDescriptor.hpp
    ${PROJECT_SOURCE_DIR}/include/fastdds/rtps/transport/UDPv6TransportDescriptor.hpp

    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/UDPChannelResource.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/UDPChannelResource.h
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/UDPTransportInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/UDPTransportInterface.h
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/UDPv4Transport.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/UDPv4Transport.h
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/UDPv6Transport.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/UDPv6Transport.h
    DEPENDS
    log
    rtps::basic
    rtps::transport
    time
    utils::network
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    $<$<BOOL:${QNX}>:socket>
    )

#}}}

#{{{ Unit grouping the shared memory transport source code. Without Boost support only its descriptor is built.

set(SHM_TRANSPORT_SOURCES
    ${PROJECT_SOURCE_DIR}/include/fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.hpp

    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/shared_mem/SharedMemTransportDescriptor.cpp
    )
if(IS_THIRDPARTY_BOOST_OK)
    list(APPEND SHM_TRANSPORT_SOURCES
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/shared_mem/SharedMemTransport.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/shared_mem/SharedMemTransport.h
        )
endif()

fastdds_add_unit(rtps::shm-transport
    SOURCES
    ${SHM_TRANSPORT_SOURCES}
    DEPENDS
    log
    rtps::basic
    rtps::transport
    time
    utils::network
    utils::shared-memory
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    foonathan_memory
    ${THIRDPARTY_BOOST_LINK_LIBS}
    )

#}}}

#{{{ Unit with the RTPSParticipantAttributes, which set up the builtin transports.

fastdds_add_unit(rtps::participant-attributes
    SOURCES
    ${PROJECT_SOURCE_DIR}/include/fastdds/rtps/attributes/RTPSParticipantAttributes.hpp

    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/RTPSParticipantAttributes.cpp
    DEPENDS
    log
    rtps::shm-transport
    rtps::transport
    rtps::udp-transport
    time
    utils::network
    xmlparser::profile-manager
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    foonathan_memory
    )

#}}}

#{{{ Unit with the ServerAttributes, which parse the discovery server lists.

fastdds_add_unit(rtps::server-attributes
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/ServerAttributes.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/ServerAttributes.hpp
    DEPENDS
    log
    time
    utils::network
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    )

#}}}

#{{{ Unit with the MessageReceiver, which dispatches the received RTPS messages to the participant endpoints.
# A unit test not linking core replaces it with the MessageReceiver mock.

fastdds_add_unit(rtps::message-receiver
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/MessageReceiver.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/MessageReceiver.h
    DEPENDS
    core
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    foonathan_memory
    )

#}}}

#{{{ Unit grouping the NetworkFactory and the ReceiverResource it creates for the transports.

fastdds_add_unit(rtps::network-factory
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/NetworkFactory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/NetworkFactory.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/ReceiverResource.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/ReceiverResource.h
    DEPENDS
    log
    rtps::basic
    rtps::message-receiver
    rtps::transport
    rtps::udp-transport
    time
    utils::network
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    $<$<BOOL:${LINK_SSL}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto$<$<BOOL:${WIN32}>:$<SEMICOLON>crypt32.lib>>
    )

#}}}

#{{{ Unit grouping the proxy data source code, which holds the discovered information of remote participants and
# endpoints.

fastdds_add_unit(rtps::proxy-data
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/ParticipantProxyData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/ParticipantProxyData.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/ProxyDataFilters.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/ProxyHashTables.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/ReaderProxyData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/ReaderProxyData.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/WriterProxyData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/WriterProxyData.hpp
    DEPENDS
    log
    rtps::basic
    rtps::builtin-data
    rtps::common
    rtps::network-factory
    rtps::parameter-list
    rtps::timed-event
    time
    utils::network
    xtypes
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    foonathan_memory
    $<$<BOOL:${LINK_SSL}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto$<$<BOOL:${WIN32}>:$<SEMICOLON>crypt32.lib>>
    )

#}}}

#{{{ Unit with the external locators utilities, which add and filter the externally announced locators.

fastdds_add_unit(rtps::external-locators
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/external_locators.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/external_locators.hpp
    DEPENDS
    rtps::proxy-data
    time
    utils::network
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    foonathan_memory
    )

#}}}

#{{{ Unit with the RTPSDomain and the RTPSDomainImpl singleton, for units using the domain without depending on the rest
# of core. A unit test not linking core replaces it with the RTPSDomainImpl mock.

fastdds_add_unit(rtps::domain
    SOURCES
    ${PROJECT_SOURCE_DIR}/include/fastdds/rtps/RTPSDomain.hpp

    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/domain/RTPSDomain.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/domain/RTPSDomainExtras.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/domain/RTPSDomainImpl.hpp
    DEPENDS
    core
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    $<$<BOOL:${TINYXML2_INCLUDE_DIR}>:${TINYXML2_INCLUDE_DIR}>
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    ${TINYXML2_LIBRARY}
    foonathan_memory
    $<$<BOOL:${LINK_SSL}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto$<$<BOOL:${WIN32}>:$<SEMICOLON>crypt32.lib>>
    )

#}}}

#{{{ Unit grouping the base history, cache change pool, topic payload pool and its registry source code

fastdds_add_unit(rtps::history
    SOURCES
    ${PROJECT_SOURCE_DIR}/include/fastdds/rtps/history/History.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/CacheChangePool.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/CacheChangePool.h
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/History.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/TopicPayloadPool.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/TopicPayloadPool.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/TopicPayloadPoolRegistry.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/TopicPayloadPoolRegistry.hpp
    DEPENDS
    log
    rtps::basic
    time
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    )

#}}}

#{{{ Unit grouping the ParameterList source code

fastdds_add_unit(rtps::parameter-list
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/policy/ParameterList.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/policy/ParameterList.hpp
    DEPENDS
    rtps::basic # On Windows, every object emits the exported inline QoS policies, using SerializedPayload_t
    rtps::common
    time
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    )

#}}}

#{{{ Unit grouping the builtin topic data and the endpoint QoS source code

fastdds_add_unit(rtps::builtin-data
    SOURCES
    ${PROJECT_SOURCE_DIR}/include/fastdds/dds/publisher/qos/WriterQos.hpp
    ${PROJECT_SOURCE_DIR}/include/fastdds/dds/subscriber/qos/ReaderQos.hpp
    ${PROJECT_SOURCE_DIR}/include/fastdds/rtps/builtin/data/ParticipantBuiltinTopicData.hpp
    ${PROJECT_SOURCE_DIR}/include/fastdds/rtps/builtin/data/PublicationBuiltinTopicData.hpp
    ${PROJECT_SOURCE_DIR}/include/fastdds/rtps/builtin/data/SubscriptionBuiltinTopicData.hpp

    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/qos/WriterQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/qos/ReaderQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/utils/TypePropagation.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/utils/TypePropagation.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/ParticipantBuiltinTopicData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/PublicationBuiltinTopicData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/SubscriptionBuiltinTopicData.cpp
    DEPENDS
    log
    rtps::basic
    time
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    )

#}}}

#{{{ Unit grouping the security logging source code

if(SECURITY)
    fastdds_add_unit(rtps::security-logging
        SOURCES
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/security/logging/Logging.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/security/logging/Logging.h
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/security/logging/LoggingLevel.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/security/logging/LoggingLevel.h
        DEPENDS
        rtps::basic
        time
        LINK_LIBRARIES
        PUBLIC
        fastcdr
        )
endif()

#}}}

#{{{ Unit with the writer base classes: RTPSWriter, BaseWriter and the statistics part of the writers.
# A unit test not linking core replaces it with the BaseWriter mock.

# The statistics part of the writers is only built with FASTDDS_STATISTICS.
set(RTPS_WRITER_BASE_STATISTICS_SOURCES)
if(FASTDDS_STATISTICS)
    set(RTPS_WRITER_BASE_STATISTICS_SOURCES ${PROJECT_SOURCE_DIR}/src/cpp/statistics/rtps/writer/StatisticsWriterImpl.cpp)
endif()

fastdds_add_unit(rtps::writer-base
    SOURCES
    ${PROJECT_SOURCE_DIR}/include/fastdds/rtps/writer/RTPSWriter.hpp

    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/BaseWriter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/BaseWriter.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/RTPSWriter.cpp
    ${RTPS_WRITER_BASE_STATISTICS_SOURCES}
    DEPENDS
    core
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    foonathan_memory
    )

#}}}

#{{{ Unit with the LocatorSelectorSender, which sends the messages of a writer to the selected locators.

fastdds_add_unit(rtps::locator-selector-sender
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/LocatorSelectorSender.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/LocatorSelectorSender.hpp
    DEPENDS
    rtps::writer-base
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    foonathan_memory
    )

#}}}

#{{{ Unit with the RTPSParticipantImpl, which owns the transports, the builtin protocols and the endpoints of a
# participant. A unit test not linking core replaces it with the RTPSParticipantImplAccessors mock, or with the
# RTPSParticipantImpl mock when it creates participants.

fastdds_add_unit(rtps::participant-impl
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/participant/RTPSParticipantImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/participant/RTPSParticipantImpl.hpp
    DEPENDS
    core
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    $<$<BOOL:${TINYXML2_INCLUDE_DIR}>:${TINYXML2_INCLUDE_DIR}>
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    ${TINYXML2_LIBRARY}
    foonathan_memory
    $<$<BOOL:${LINK_SSL}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto$<$<BOOL:${WIN32}>:$<SEMICOLON>crypt32.lib>>
    )

#}}}

#{{{ Unit with the RTPSMessageGroup, which serializes the submessages of an endpoint into RTPS messages and sends them.
# A unit test not linking core replaces it with the RTPSMessageGroup mock.

fastdds_add_unit(rtps::message-group
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/RTPSMessageGroup.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/RTPSMessageGroup.hpp
    DEPENDS
    core
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    foonathan_memory
    )

#}}}

#{{{ Unit grouping the flow controllers source code, which schedule the samples the writers send.

fastdds_add_unit(rtps::flow-control
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/FlowController.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/FlowControllerFactory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/FlowControllerFactory.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/FlowControllerImpl.hpp
    DEPENDS
    log
    rtps::basic
    rtps::message-group
    rtps::participant-impl
    time
    utils::network
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    foonathan_memory
    )

#}}}

#{{{ Unit with the reader base classes: RTPSReader, BaseReader and the statistics part of the readers.
# A unit test not linking core replaces it with the BaseReader mock.

# The statistics part of the readers is only built with FASTDDS_STATISTICS.
set(RTPS_READER_BASE_STATISTICS_SOURCES)
if(FASTDDS_STATISTICS)
    set(RTPS_READER_BASE_STATISTICS_SOURCES ${PROJECT_SOURCE_DIR}/src/cpp/statistics/rtps/reader/StatisticsReaderImpl.cpp)
endif()

fastdds_add_unit(rtps::reader-base
    SOURCES
    ${PROJECT_SOURCE_DIR}/include/fastdds/rtps/reader/RTPSReader.hpp

    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/reader/BaseReader.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/reader/BaseReader.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/reader/RTPSReader.cpp
    ${RTPS_READER_BASE_STATISTICS_SOURCES}
    DEPENDS
    core
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    foonathan_memory
    )

#}}}

#{{{ Unit with the ReaderHistory, the history of the RTPS readers.
# A unit test not linking core replaces it with the ReaderHistory mock.

fastdds_add_unit(rtps::reader-history
    SOURCES
    ${PROJECT_SOURCE_DIR}/include/fastdds/rtps/history/ReaderHistory.hpp

    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/ReaderHistory.cpp
    DEPENDS
    log
    rtps::history
    rtps::reader-base
    time
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    foonathan_memory
    )

#}}}

#{{{ Unit with the WriterHistory, the history of the RTPS writers.
# A unit test not linking core replaces it with the WriterHistory mock.

fastdds_add_unit(rtps::writer-history
    SOURCES
    ${PROJECT_SOURCE_DIR}/include/fastdds/rtps/history/WriterHistory.hpp

    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/WriterHistory.cpp
    DEPENDS
    log
    rtps::history
    rtps::writer-base
    time
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    foonathan_memory
    )

#}}}

#{{{ Unit with the StatefulWriter, the reliable writer that keeps track of each matched reader.
# A unit test not linking core replaces it with the StatefulWriter mock.

fastdds_add_unit(rtps::stateful-writer
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/StatefulWriter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/StatefulWriter.hpp
    DEPENDS
    core
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    foonathan_memory
    $<$<BOOL:${LINK_SSL}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto$<$<BOOL:${WIN32}>:$<SEMICOLON>crypt32.lib>>
    )

#}}}

#{{{ Unit with the StatelessWriter, the writer that sends to its reader locators without tracking each reader.
# A unit test not linking core replaces it with the StatelessWriter mock.

fastdds_add_unit(rtps::stateless-writer
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/StatelessWriter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/StatelessWriter.hpp
    DEPENDS
    core
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    foonathan_memory
    $<$<BOOL:${LINK_SSL}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto$<$<BOOL:${WIN32}>:$<SEMICOLON>crypt32.lib>>
    )

#}}}

#{{{ Unit with the ReaderLocator, which sends the messages of a writer to a remote reader.
# A unit test not linking core replaces it with the ReaderLocator mock.

fastdds_add_unit(rtps::reader-locator
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/ReaderLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/ReaderLocator.hpp
    DEPENDS
    core
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    foonathan_memory
    $<$<BOOL:${LINK_SSL}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto$<$<BOOL:${WIN32}>:$<SEMICOLON>crypt32.lib>>
    )

#}}}

#{{{ Unit with the RTPSGapBuilder, which groups the irrelevant sequence numbers of a writer into GAP submessages.
# A unit test not linking core replaces it with the RTPSGapBuilder mock.

fastdds_add_unit(rtps::gap-builder
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/RTPSGapBuilder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/RTPSGapBuilder.hpp
    DEPENDS
    core
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    foonathan_memory
    )

#}}}

#{{{ Unit with the ReaderProxy, which holds the state of a reader matched with a StatefulWriter.

fastdds_add_unit(rtps::reader-proxy
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/ReaderProxy.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/ReaderProxy.hpp
    DEPENDS
    log
    rtps::gap-builder
    rtps::history
    rtps::reader-locator
    rtps::stateful-writer
    rtps::timed-event
    rtps::writer-base
    time
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    foonathan_memory
    )

#}}}

#{{{ Unit with the LivelinessManager, which tracks the liveliness of the writers.

fastdds_add_unit(rtps::liveliness-manager
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/LivelinessData.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/LivelinessManager.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/LivelinessManager.hpp
    DEPENDS
    log
    rtps::timed-event
    time
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    )

#}}}

#{{{ Unit with the StatefulReader, the reliable reader that keeps track of each matched writer.
# A unit test not linking core replaces it with the StatefulReader mock.

fastdds_add_unit(rtps::stateful-reader
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/reader/StatefulReader.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/reader/StatefulReader.hpp
    DEPENDS
    core
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    foonathan_memory
    $<$<BOOL:${LINK_SSL}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto$<$<BOOL:${WIN32}>:$<SEMICOLON>crypt32.lib>>
    )

#}}}

#{{{ Unit with the StatelessReader, the reader that accepts data without tracking each writer.
# A unit test not linking core replaces it with the StatelessReader mock.

fastdds_add_unit(rtps::stateless-reader
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/reader/StatelessReader.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/reader/StatelessReader.hpp
    DEPENDS
    core
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    foonathan_memory
    $<$<BOOL:${LINK_SSL}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto$<$<BOOL:${WIN32}>:$<SEMICOLON>crypt32.lib>>
    )

#}}}

#{{{ Unit with the WLP, the builtin writer liveliness protocol of a participant.
# A unit test not linking core replaces it with the WLP mock.

fastdds_add_unit(rtps::wlp
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/liveliness/WLP.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/liveliness/WLP.hpp
    DEPENDS
    core
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    foonathan_memory
    )

#}}}

#{{{ Unit with the SecurityManager, which drives the security plugins of a participant.
# A unit test not linking core replaces it with the SecurityManager mock.

set(RTPS_SECURITY_MANAGER_UNIT)
if(SECURITY)
    fastdds_add_unit(rtps::security-manager
        SOURCES
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/security/SecurityManager.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/security/SecurityManager.h
        DEPENDS
        log
        rtps::basic
        rtps::common
        rtps::edp
        rtps::history
        rtps::participant-impl
        rtps::pdp
        rtps::proxy-data
        rtps::reader-history
        rtps::security-logging
        rtps::stateful-writer
        rtps::timed-event
        rtps::writer-history
        time
        INCLUDE_DIRECTORIES
        ${Asio_INCLUDE_DIR}
        ${THIRDPARTY_BOOST_INCLUDE_DIR}
        LINK_LIBRARIES
        PUBLIC
        fastcdr
        foonathan_memory
        OpenSSL::SSL
        OpenSSL::Crypto
        $<$<BOOL:${WIN32}>:crypt32.lib>
        )
    set(RTPS_SECURITY_MANAGER_UNIT rtps::security-manager)
endif()

#}}}

#{{{ Unit with the PDP, the base of the participant discovery protocols.
# A unit test not linking core replaces it with the PDP mock.

fastdds_add_unit(rtps::pdp
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/discovery/participant/PDP.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/discovery/participant/PDP.h
    DEPENDS
    dds::type-lookup-manager
    log
    rtps::basic
    rtps::edp
    rtps::external-locators
    rtps::participant-impl
    rtps::proxy-data
    ${RTPS_SECURITY_MANAGER_UNIT}
    rtps::timed-event
    rtps::wlp
    rtps::writer-history
    time
    utils::network
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    foonathan_memory
    )

#}}}

#{{{ Unit with the EDP, the base of the endpoint discovery protocols.
# A unit test not linking core replaces it with the EDP mock.

fastdds_add_unit(rtps::edp
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/discovery/endpoint/EDP.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/discovery/endpoint/EDP.h
    DEPENDS
    log
    rtps::basic
    rtps::external-locators
    rtps::participant-impl
    rtps::pdp
    rtps::proxy-data
    rtps::reader-base
    ${RTPS_SECURITY_MANAGER_UNIT}
    rtps::writer-base
    time
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    foonathan_memory
    $<$<BOOL:${LINK_SSL}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto$<$<BOOL:${WIN32}>:$<SEMICOLON>crypt32.lib>>
    )

#}}}

#{{{ Unit grouping the persistence services source code, with the SQLite3 one only when SQLITE3_SUPPORT is enabled.

set(RTPS_PERSISTENCE_SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/persistence/PersistenceFactory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/persistence/PersistenceService.h
    )
if(SQLITE3_SUPPORT)
    list(APPEND RTPS_PERSISTENCE_SOURCES
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/persistence/sqlite3.c
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/persistence/sqlite3.h
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/persistence/SQLite3PersistenceService.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/persistence/SQLite3PersistenceService.h
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/persistence/SQLite3PersistenceServiceStatements.h
        )
endif()

fastdds_add_unit(rtps::persistence
    SOURCES
    ${RTPS_PERSISTENCE_SOURCES}
    DEPENDS
    log
    rtps::basic
    rtps::writer-history
    time
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    foonathan_memory
    )

#}}}

#{{{ Unit with the RTPSParticipant, which forwards to its RTPSParticipantImpl.
# A unit test not linking core replaces it with the RTPSParticipant mock.

fastdds_add_unit(rtps::participant
    SOURCES
    ${PROJECT_SOURCE_DIR}/include/fastdds/rtps/participant/RTPSParticipant.hpp

    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/participant/RTPSParticipant.cpp
    DEPENDS
    core
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    foonathan_memory
    )

#}}}

#{{{ Unit with the DataSharingPayloadPool, the base of the shared memory payload pools of the data sharing writers and
# readers. A unit test not linking core replaces it with the DataSharingPayloadPool mock.

fastdds_add_unit(rtps::data-sharing-pool
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/DataSharing/DataSharingPayloadPool.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/DataSharing/DataSharingPayloadPool.hpp
    DEPENDS
    core
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    foonathan_memory
    )

#}}}
