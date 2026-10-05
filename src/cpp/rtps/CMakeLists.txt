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

#{{{ Interface library for grouping the host, locator and network utilities

add_library(fastdds-rtps-host-net INTERFACE)

add_library(fastdds::rtps::host-net ALIAS fastdds-rtps-host-net)

target_compile_features(fastdds-rtps-host-net INTERFACE cxx_std_11)

target_sources(fastdds-rtps-host-net INTERFACE
    common/LocatorWithMask.cpp
    network/utils/network.cpp
    ${FASTDDS_DIR}/src/cpp/utils/Host.cpp
    ${FASTDDS_DIR}/src/cpp/utils/IPFinder.cpp
    ${FASTDDS_DIR}/src/cpp/utils/IPLocator.cpp
    ${FASTDDS_DIR}/src/cpp/utils/md5.cpp
    ${FASTDDS_DIR}/src/cpp/utils/SystemInfo.cpp
    )

# Settings this group's code is sensitive to. See cmake/common/reusable_objects.cmake.
fastdds_declare_object_group(fastdds-rtps-host-net
    MOCKS
        rtps/Log
    DEFINES
        ENABLE_OLD_LOG_MACROS_
        FASTDDS_ENFORCE_LOG_INFO
        HAVE_LOG_NO_ERROR
        HAVE_LOG_NO_INFO
        HAVE_LOG_NO_WARNING
    )

#}}}

#{{{ Interface library for grouping the RTPS core: discovery, endpoints and participant

add_library(fastdds-rtps-core-full INTERFACE)

add_library(fastdds::rtps::core-full ALIAS fastdds-rtps-core-full)

target_compile_features(fastdds-rtps-core-full INTERFACE cxx_std_11)

target_sources(fastdds-rtps-core-full INTERFACE
    builtin/BuiltinProtocols.cpp
    builtin/discovery/database/backup/SharedBackupFunctions.cpp
    builtin/discovery/database/DiscoveryDataBase.cpp
    builtin/discovery/database/DiscoveryParticipantInfo.cpp
    builtin/discovery/database/DiscoveryParticipantsAckStatus.cpp
    builtin/discovery/database/DiscoverySharedInfo.cpp
    builtin/discovery/endpoint/EDPClient.cpp
    builtin/discovery/endpoint/EDPServer.cpp
    builtin/discovery/endpoint/EDPServerListeners.cpp
    builtin/discovery/endpoint/EDPSimple.cpp
    builtin/discovery/endpoint/EDPSimpleListeners.cpp
    builtin/discovery/endpoint/EDPStatic.cpp
    builtin/discovery/participant/DirectMessageSender.cpp
    builtin/discovery/participant/PDPClient.cpp
    builtin/discovery/participant/PDPClientListener.cpp
    builtin/discovery/participant/PDPListener.cpp
    builtin/discovery/participant/PDPServer.cpp
    builtin/discovery/participant/PDPServerListener.cpp
    builtin/discovery/participant/PDPSimple.cpp
    builtin/discovery/participant/simple/PDPStatelessWriter.cpp
    builtin/discovery/participant/timedevent/DSClientEvent.cpp
    builtin/discovery/participant/timedevent/DServerEvent.cpp
    builtin/liveliness/WLP.cpp
    builtin/liveliness/WLPListener.cpp
    DataSharing/DataSharingListener.cpp
    DataSharing/DataSharingNotification.cpp
    domain/RTPSDomain.cpp
    domain/RTPSDomainExtras.cpp
    history/WriterHistory.cpp
    messages/MessageReceiver.cpp
    messages/RTPSGapBuilder.cpp
    messages/RTPSMessageGroup.cpp
    messages/SendBuffersManager.cpp
    network/ReceiverResource.cpp
    participant/RTPSParticipant.cpp
    participant/RTPSParticipantImpl.cpp
    reader/BaseReader.cpp
    reader/reader_utils.cpp
    reader/RTPSReader.cpp
    reader/StatefulPersistentReader.cpp
    reader/StatefulReader.cpp
    reader/StatelessPersistentReader.cpp
    reader/StatelessReader.cpp
    reader/WriterProxy.cpp
    writer/BaseWriter.cpp
    writer/PersistentWriter.cpp
    writer/ReaderLocator.cpp
    writer/RTPSWriter.cpp
    writer/StatefulPersistentWriter.cpp
    writer/StatefulWriter.cpp
    writer/StatelessPersistentWriter.cpp
    writer/StatelessWriter.cpp
    ${FASTDDS_DIR}/src/cpp/fastdds/builtin/type_lookup_service/TypeLookupManager.cpp
    )

# Same conditions under which the consumer suites compile these sources.
if(FASTDDS_STATISTICS)
    target_sources(fastdds-rtps-core-full INTERFACE
        ${FASTDDS_DIR}/src/cpp/statistics/rtps/StatisticsBase.cpp
        ${FASTDDS_DIR}/src/cpp/statistics/rtps/reader/StatisticsReaderImpl.cpp
        ${FASTDDS_DIR}/src/cpp/statistics/rtps/writer/StatisticsWriterImpl.cpp
        )
endif()

if(IS_THIRDPARTY_BOOST_OK)
    target_sources(fastdds-rtps-core-full INTERFACE
        transport/shared_mem/test_SharedMemTransport.cpp
        )
endif()

if(SECURITY)
    target_sources(fastdds-rtps-core-full INTERFACE
        builtin/discovery/participant/DS/PDPSecurityInitiatorListener.cpp
        security/SecurityPluginFactory.cpp
        )
endif()

# Settings this group's code is sensitive to. See cmake/common/reusable_objects.cmake.
fastdds_declare_object_group(fastdds-rtps-core-full
    MOCKS
        dds/QosPolicies
        dds/TypeSupport
        rtps/BuiltinProtocols
        rtps/DataSharingPayloadPool
        rtps/EDP
        rtps/Endpoint
        rtps/Log
        rtps/MessageReceiver
        rtps/NetworkFactory
        rtps/ParticipantProxyData
        rtps/PDP
        rtps/PDPSimple
        rtps/ReaderHistory
        rtps/ReaderLocator
        rtps/ReaderProxyData
        rtps/ReceiverResource
        rtps/ResourceEvent
        rtps/RTPSDomain
        rtps/RTPSDomainImpl
        rtps/RTPSGapBuilder
        rtps/RTPSMessageGroup
        rtps/RTPSParticipant
        rtps/RTPSParticipantAttributes
        rtps/RTPSParticipantImpl
        rtps/RTPSReader
        rtps/RTPSWriter
        rtps/SecurityManager
        rtps/SecurityPluginFactory
        rtps/SharedMemSegment
        rtps/SharedMemTransportDescriptor
        rtps/SharedSecretHandle
        rtps/StatefulReader
        rtps/StatefulWriter
        rtps/StatelessReader
        rtps/StatelessWriter
        rtps/TCPTransportDescriptor
        rtps/TCPv4TransportDescriptor
        rtps/TCPv6TransportDescriptor
        rtps/TimedEvent
        rtps/TypeLookupManager
        rtps/UDPTransportDescriptor
        rtps/UDPv4TransportDescriptor
        rtps/UDPv6TransportDescriptor
        rtps/WLP
        rtps/WriterHistory
        rtps/WriterProxyData
        rtps/XMLEndPointParser
        rtps/XMLProfileManager
        rtps/external_locators
        xtypes/TypeObjectRegistry
        xtypes/TypeObjectUtils
    DEFINES
        ENABLE_OLD_LOG_MACROS_
        FASTDDS_ENFORCE_LOG_INFO
        HAVE_LOG_NO_ERROR
        HAVE_LOG_NO_INFO
        HAVE_LOG_NO_WARNING
    )

#}}}
