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

#{{{ Interface library for grouping the statistics topic types

add_library(fastdds-statistics-types INTERFACE)

add_library(fastdds::statistics::types ALIAS fastdds-statistics-types)

target_compile_features(fastdds-statistics-types INTERFACE cxx_std_11)

# Same condition under which the consumer suites compile these sources.
if(FASTDDS_STATISTICS)
    target_sources(fastdds-statistics-types INTERFACE
        types/monitorservice_typesPubSubTypes.cxx
        types/monitorservice_typesTypeObjectSupport.cxx
        types/typesPubSubTypes.cxx
        types/typesTypeObjectSupport.cxx
        )
endif()

# Settings this group's code is sensitive to. See cmake/common/reusable_objects.cmake.
fastdds_declare_object_group(fastdds-statistics-types
    MOCKS
        dds/DomainParticipantFactory
        dds/QosPolicies
        dds/TypeSupport
        rtps/BuiltinProtocols
        rtps/EDP
        rtps/Endpoint
        rtps/Log
        rtps/MessageReceiver
        rtps/NetworkFactory
        rtps/ParticipantProxyData
        rtps/PDP
        rtps/ReaderHistory
        rtps/ReaderLocator
        rtps/ReaderProxyData
        rtps/ReceiverResource
        rtps/ResourceEvent
        rtps/RTPSDomain
        rtps/RTPSDomainImpl
        rtps/RTPSMessageGroup
        rtps/RTPSParticipant
        rtps/RTPSParticipantAttributes
        rtps/RTPSReader
        rtps/RTPSWriter
        rtps/StatefulReader
        rtps/StatefulWriter
        rtps/StatelessWriter
        rtps/TypeLookupManager
        rtps/UDPTransportDescriptor
        rtps/UDPv4TransportDescriptor
        rtps/WriterHistory
        rtps/WriterProxyData
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

#{{{ Interface library for grouping the statistics monitor service

add_library(fastdds-statistics-monitor-service INTERFACE)

add_library(fastdds::statistics::monitor-service ALIAS fastdds-statistics-monitor-service)

target_compile_features(fastdds-statistics-monitor-service INTERFACE cxx_std_11)

# Same condition under which the consumer suites compile these sources.
if(FASTDDS_STATISTICS)
    target_sources(fastdds-statistics-monitor-service INTERFACE
        rtps/monitor-service/MonitorService.cpp
        rtps/monitor-service/MonitorServiceListener.cpp
        )
endif()

# Settings this group's code is sensitive to. See cmake/common/reusable_objects.cmake.
fastdds_declare_object_group(fastdds-statistics-monitor-service
    MOCKS
        dds/DataWriterHistory
        dds/QosPolicies
        dds/TypeSupport
        rtps/BuiltinProtocols
        rtps/EDP
        rtps/Endpoint
        rtps/Log
        rtps/MessageReceiver
        rtps/NetworkFactory
        rtps/ParticipantProxyData
        rtps/PDP
        rtps/ReaderHistory
        rtps/ReaderLocator
        rtps/ReaderProxyData
        rtps/ReceiverResource
        rtps/ResourceEvent
        rtps/RTPSDomain
        rtps/RTPSDomainImpl
        rtps/RTPSMessageGroup
        rtps/RTPSParticipant
        rtps/RTPSParticipantAttributes
        rtps/RTPSReader
        rtps/RTPSWriter
        rtps/StatefulReader
        rtps/StatefulWriter
        rtps/StatelessWriter
        rtps/TimedEvent
        rtps/TypeLookupManager
        rtps/UDPTransportDescriptor
        rtps/UDPv4TransportDescriptor
        rtps/WriterHistory
        rtps/WriterProxyData
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
