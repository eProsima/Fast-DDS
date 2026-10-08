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

#{{{ Interface library for grouping the DDS layer entities and QoS that compile alike in every consumer

add_library(fastdds-dds-core-full INTERFACE)

add_library(fastdds::dds::core-full ALIAS fastdds-dds-core-full)

target_compile_features(fastdds-dds-core-full INTERFACE cxx_std_11)

target_sources(fastdds-dds-core-full INTERFACE
    core/policy/QosPolicyUtils.cpp
    domain/qos/DomainParticipantFactoryQos.cpp
    domain/qos/DomainParticipantQos.cpp
    publisher/qos/DataWriterQos.cpp
    publisher/qos/PublisherQos.cpp
    rpc/ServiceTypeSupport.cpp
    subscriber/DataReader.cpp
    subscriber/qos/DataReaderQos.cpp
    subscriber/qos/SubscriberQos.cpp
    subscriber/ReadCondition.cpp
    subscriber/Subscriber.cpp
    topic/ContentFilteredTopic.cpp
    topic/ContentFilteredTopicImpl.cpp
    topic/qos/TopicQos.cpp
    topic/Topic.cpp
    topic/TopicProxyFactory.cpp
    topic/TypeSupport.cpp
    utils/QosConverters.cpp
    )

# Settings this group's code is sensitive to. See cmake/common/reusable_objects.cmake.
fastdds_declare_object_group(fastdds-dds-core-full
    MOCKS
        dds/QosPolicies
        dds/Topic
        dds/TypeSupport
        rtps/Log
        rtps/ReaderHistory
        rtps/RTPSParticipantAttributes
        rtps/UDPTransportDescriptor
        rtps/UDPv4TransportDescriptor
    DEFINES
        ENABLE_OLD_LOG_MACROS_
        FASTDDS_ENFORCE_LOG_INFO
        HAVE_LOG_NO_ERROR
        HAVE_LOG_NO_INFO
        HAVE_LOG_NO_WARNING
    )

#}}}

#{{{ Interface library for grouping the DDS layer implementation classes reached by the participant mocks

add_library(fastdds-dds-impl INTERFACE)

add_library(fastdds::dds::impl ALIAS fastdds-dds-impl)

target_compile_features(fastdds-dds-impl INTERFACE cxx_std_11)

target_sources(fastdds-dds-impl INTERFACE
    domain/DomainParticipant.cpp
    domain/DomainParticipantFactory.cpp
    subscriber/DataReaderImpl.cpp
    subscriber/SubscriberImpl.cpp
    topic/TopicImpl.cpp
    )

# Settings this group's code is sensitive to. See cmake/common/reusable_objects.cmake.
fastdds_declare_object_group(fastdds-dds-impl
    MOCKS
        dds/DomainParticipantFactory
        dds/DomainParticipantImpl
        dds/Publisher
        dds/QosPolicies
        dds/Topic
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
        rtps/RTPSParticipantImpl
        rtps/RTPSReader
        rtps/RTPSWriter
        rtps/SecurityManager
        rtps/SecurityPluginFactory
        rtps/SharedMemSegment
        rtps/SharedSecretHandle
        rtps/StatefulReader
        rtps/StatefulWriter
        rtps/StatelessWriter
        rtps/TimedEvent
        rtps/TypeLookupManager
        rtps/UDPTransportDescriptor
        rtps/UDPv4TransportDescriptor
        rtps/WriterHistory
        rtps/WriterProxyData
        rtps/XMLEndPointParser
        rtps/XMLProfileManager
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

#{{{ Interface library for grouping the type lookup service listeners and types

add_library(fastdds-type-lookup INTERFACE)

add_library(fastdds::type-lookup ALIAS fastdds-type-lookup)

target_compile_features(fastdds-type-lookup INTERFACE cxx_std_11)

target_sources(fastdds-type-lookup INTERFACE
    builtin/type_lookup_service/detail/rpc_typesPubSubTypes.cxx
    builtin/type_lookup_service/detail/TypeLookupTypesPubSubTypes.cxx
    builtin/type_lookup_service/TypeLookupReplyListener.cpp
    builtin/type_lookup_service/TypeLookupRequestListener.cpp
    )

# Settings this group's code is sensitive to. See cmake/common/reusable_objects.cmake.
fastdds_declare_object_group(fastdds-type-lookup
    MOCKS
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
        rtps/RTPSParticipantImpl
        rtps/RTPSReader
        rtps/RTPSWriter
        rtps/SecurityManager
        rtps/SecurityPluginFactory
        rtps/SharedSecretHandle
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
