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

#{{{ Unit grouping the QoS source code of the DDS entities and the DomainParticipantFactory, with the QoS converters.

fastdds_add_unit(dds::participant-qos
    SOURCES
    ${PROJECT_SOURCE_DIR}/include/fastdds/dds/domain/qos/DomainParticipantFactoryQos.hpp
    ${PROJECT_SOURCE_DIR}/include/fastdds/dds/domain/qos/DomainParticipantQos.hpp

    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/policy/QosPolicyUtils.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/qos/DomainParticipantFactoryQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/qos/DomainParticipantQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/qos/DataWriterQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/qos/PublisherQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/qos/DataReaderQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/qos/SubscriberQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/qos/TopicQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/utils/QosConverters.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/utils/QosConverters.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/publisher/qos/DataWriterQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/subscriber/qos/DataReaderQos.cpp
    DEPENDS
    log
    rtps::basic
    rtps::builtin-data
    rtps::participant-attributes
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

#{{{ Unit with the DomainParticipantFactory, the entry point creating the DomainParticipants.
# A unit test not linking core replaces it with the DomainParticipantFactory mock.

fastdds_add_unit(dds::participant-factory
    SOURCES
    ${PROJECT_SOURCE_DIR}/include/fastdds/dds/domain/DomainParticipantFactory.hpp

    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/DomainParticipantFactory.cpp
    DEPENDS
    dds::entities
    dds::participant-qos
    log
    rtps::basic
    rtps::domain
    rtps::history
    time
    utils::network
    xmlparser::endpoint-parser
    xmlparser::profile-manager
    xtypes
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    $<$<BOOL:${TINYXML2_INCLUDE_DIR}>:${TINYXML2_INCLUDE_DIR}>
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    ${TINYXML2_LIBRARY}
    foonathan_memory
    $<$<BOOL:${LINK_SSL}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto$<$<BOOL:${WIN32}>:$<SEMICOLON>crypt32.lib>>
    )

#}}}

#{{{ Unit grouping the DDS entities source code: the DomainParticipant, publishers, subscribers, topics and the RPC
# entities. Their QoS are in dds::participant-qos. DataWriter.cpp has its own unit, so the DataWriter mock can replace
# it alone.

# The statistics DomainParticipantImpl, created by the DomainParticipantFactory, and the statistics units it uses are
# only built with FASTDDS_STATISTICS.
set(DDS_ENTITIES_STATISTICS_SOURCES)
set(DDS_ENTITIES_STATISTICS_UNITS)
if(FASTDDS_STATISTICS)
    set(DDS_ENTITIES_STATISTICS_SOURCES
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/domain/DomainParticipantImpl.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/domain/DomainParticipantImpl.hpp
        )
    set(DDS_ENTITIES_STATISTICS_UNITS
        statistics::participant-listener
        statistics::types
        )
endif()

fastdds_add_unit(dds::entities
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/DomainParticipant.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/DomainParticipantImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/DataWriterImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/Publisher.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/PublisherImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/rpc/RPCTypeObjectSupport.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/rpc/ServiceTypeSupport.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/DataReader.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/DataReaderImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/ReadCondition.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/Subscriber.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/SubscriberImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/ContentFilteredTopic.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/ContentFilteredTopicImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/Topic.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/TopicImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/TopicProxyFactory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/domain/DomainParticipant.cpp
    ${DDS_ENTITIES_STATISTICS_SOURCES}
    DEPENDS
    dds::condition
    dds::data-writer
    dds::participant-factory
    dds::participant-qos
    dds::reader-history
    dds::sql-filter
    dds::type-support
    dds::writer-history
    log
    rtps::basic
    rtps::builtin-data
    rtps::common
    rtps::data-sharing-pool
    rtps::domain
    rtps::history
    rtps::participant
    rtps::participant-impl
    rtps::reader-base
    rtps::stateful-writer
    rtps::timed-event
    rtps::wlp
    rtps::writer-base
    rtps::writer-history
    ${DDS_ENTITIES_STATISTICS_UNITS}
    time
    utils::network
    xmlparser::common
    xmlparser::profile-manager
    xtypes
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    $<$<BOOL:${TINYXML2_INCLUDE_DIR}>:${TINYXML2_INCLUDE_DIR}>
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    ${TINYXML2_LIBRARY}
    foonathan_memory
    $<$<BOOL:${LINK_SSL}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto$<$<BOOL:${WIN32}>:$<SEMICOLON>crypt32.lib>>
    )

#}}}
