# Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

file(GLOB DDSSQLFILTER_SOURCES ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/DDSSQLFilter/*.cpp)

set(LISTENERTESTS_SOURCE ListenerTests.cpp
    ${DDSSQLFILTER_SOURCES}
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/Condition.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/ConditionNotifier.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/StatusCondition.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/StatusConditionImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/WaitSetImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Entity.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/policy/ParameterList.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/policy/QosPolicyUtils.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/DomainParticipant.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/DomainParticipantFactory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/DomainParticipantImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/qos/DomainParticipantFactoryQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/qos/DomainParticipantQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/DataWriter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/DataWriterHistory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/DataWriterImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/Publisher.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/PublisherImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/qos/DataWriterQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/qos/PublisherQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/qos/WriterQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/rpc/ReplierImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/rpc/RequesterImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/rpc/ServiceImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/rpc/ServiceTypeSupport.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/DataReader.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/DataReaderImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/history/DataReaderHistory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/qos/DataReaderQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/qos/ReaderQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/qos/SubscriberQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/ReadCondition.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/Subscriber.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/SubscriberImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/ContentFilteredTopic.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/ContentFilteredTopicImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/qos/TopicQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/Topic.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/TopicImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/TopicProxyFactory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/TypeSupport.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/utils/QosConverters.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/utils/TypePropagation.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/PropertyPolicy.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/ThreadSettings.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/PublicationBuiltinTopicData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/SubscriptionBuiltinTopicData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/ReaderProxyData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/WriterProxyData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/GuidPrefix_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/LocatorWithMask.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/FlowControllerConsts.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/CacheChangePool.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/TopicPayloadPool.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/TopicPayloadPoolRegistry.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/CDRMessage.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/network.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetmaskFilterKind.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterfaceWithFilter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/PortBasedTransportDescriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/LocatorSelectorSender.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/domain/DomainParticipant.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/publisher/qos/DataWriterQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/Host.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/TimedConditionVariable.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/UnitsParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLDynamicParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLElementParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLParserCommon.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLProfileManager.cpp
    )

if (FASTDDS_STATISTICS)

    set(statistics_sources
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/domain/DomainParticipantImpl.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/domain/DomainParticipantStatisticsListener.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/monitorservice_typesPubSubTypes.cxx
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/monitorservice_typesTypeObjectSupport.cxx
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/typesPubSubTypes.cxx
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/typesTypeObjectSupport.cxx
        )

    list(APPEND LISTENERTESTS_SOURCE ${statistics_sources})

endif()

# External sources
if(TINYXML2_SOURCE_DIR)
    list(APPEND LISTENERTESTS_SOURCE
        ${TINYXML2_SOURCE_DIR}/tinyxml2.cpp
        )
endif()

# Only include tinyxml2 headers manually if the associated config predates the use of CMake targets
if(TINYXML2_INCLUDE_DIR)
    include_directories(${TINYXML2_INCLUDE_DIR})
endif(TINYXML2_INCLUDE_DIR)

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

if(ANDROID)
    if (ANDROID_NATIVE_API_LEVEL LESS 24)
        list(APPEND LISTENERTESTS_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
    endif()
endif()

add_executable(ListenerTests ${LISTENERTESTS_SOURCE})
target_compile_definitions(ListenerTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    ASIO_DISABLE_VISIBILITY
    SQLITE_WIN32_GETVERSIONEX=0
    $<$<AND:$<BOOL:${ANDROID}>,$<NOT:$<BOOL:${HAVE_CXX14}>>,$<NOT:$<BOOL:${HAVE_CXX1Y}>>>:ASIO_DISABLE_STD_STRING_VIEW>
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    $<$<BOOL:${WIN32}>:_ENABLE_ATOMIC_ALIGNMENT_FIX>
    $<$<BOOL:${MSVC}>:NOMINMAX> # avoid conflic with std::min & std::max in visual studio
    )
target_include_directories(ListenerTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/DataSharingPayloadPool
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/Endpoint
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/NetworkFactory
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/ParticipantProxyData
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/PDP
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/PDPSimple
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/ReaderHistory
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/ResourceEvent
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSDomain
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSDomainImpl
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSParticipant
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSParticipantAttributes
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSParticipantImpl
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSReader
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSWriter
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/SharedMemTransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/StatefulReader
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/StatefulWriter
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/StatelessReader
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/StatelessWriter
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPTransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPv4TransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPv6TransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/TimedEvent
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/TypeLookupManager
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/UDPTransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/UDPv4TransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/UDPv6TransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/WLP
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/WriterHistory
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/XMLEndPointParser
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    )

target_link_libraries(ListenerTests
    fastcdr
    fastdds::log
    fastdds::xtypes::dynamic-types::impl
    fastdds::xtypes::type-representation
    foonathan_memory
    ${TINYXML2_LIBRARY}
    GTest::gmock
    ${CMAKE_DL_LIBS}
    ${THIRDPARTY_BOOST_LINK_LIBS}
    )
if(QNX)
    target_link_libraries(ListenerTests socket)
endif()
if(MSVC OR MSVC_IDE)
    target_link_libraries(ListenerTests iphlpapi Shlwapi ws2_32)
endif()
if (APPLE)
    target_link_libraries(ListenerTests "-framework CoreFoundation" "-framework IOKit")
endif()

gtest_discover_tests(ListenerTests)
