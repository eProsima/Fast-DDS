# Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

set(STATISTICS_RTPS_TESTS_SOURCE
    RTPSStatisticsTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/CDRMessage.cpp
    )

add_executable(RTPSStatisticsTests ${STATISTICS_RTPS_TESTS_SOURCE})

target_include_directories(RTPSStatisticsTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${Asio_INCLUDE_DIR}
    )

target_link_libraries(RTPSStatisticsTests
    fastdds
    fastcdr
    GTest::gtest
    GTest::gmock
    $<$<BOOL:${QNX}>:socket>
)
gtest_discover_tests(RTPSStatisticsTests)

set(TCPTransportInterface_SOURCE
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/PortBasedTransportDescriptor.cpp
    )

set(STATISTICS_RTPS_MONITORSERVICETESTS_SOURCE
    MonitorServiceTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/policy/ParameterList.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/qos/WriterQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/qos/ReaderQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/PropertyPolicy.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/ThreadSettings.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/PublicationBuiltinTopicData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/WriterProxyData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/GuidPrefix_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/FlowControllerConsts.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/CacheChangePool.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/TopicPayloadPool.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/TopicPayloadPoolRegistry.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/CDRMessage.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/ResourceEvent.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEvent.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEventImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/LocatorSelectorSender.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/statistics/rtps/monitor-service/MonitorService.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/statistics/rtps/monitor-service/MonitorServiceListener.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/monitorservice_typesPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/monitorservice_typesTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/typesPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/typesTypeObjectSupport.cxx
    ${TCPTransportInterface_SOURCE}
    )

add_executable(MonitorServiceTests ${STATISTICS_RTPS_MONITORSERVICETESTS_SOURCE})

target_include_directories(MonitorServiceTests PRIVATE
    mock/Publisher
    mock/StatisticsBase
    ${PROJECT_SOURCE_DIR}/test/mock/dds/DomainParticipantFactory
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/BuiltinProtocols
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/EDP
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/Endpoint
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/external_locators
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/ExternalLocatorsProcessor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/ParticipantProxyData
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/ReaderHistory
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/ReceiverResource
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSDomainImpl
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSParticipantImpl
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSReader
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSWriter
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/SecurityManager
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/StatefulReader
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/StatefulWriter
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/StatelessReader
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/StatelessWriter
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/TypeLookupManager
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/WLP
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/WriterHistory
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    ${Asio_INCLUDE_DIR}
    )

target_link_libraries(MonitorServiceTests
    fastcdr
    GTest::gtest
    GTest::gmock
    $<$<BOOL:${TLS_FOUND}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto>
    $<$<BOOL:${QNX}>:socket>
    )
fastdds_target_link_object_groups(MonitorServiceTests
    fastdds::log
    fastdds::xtypes::dynamic-types::impl
    fastdds::xtypes::type-representation
    fastdds::rtps::host-net
    fastdds::rtps::transport-core
    fastdds::rtps::network-factory
    fastdds::rtps::transport-tcp
    fastdds::rtps::transport-tcpv6
    fastdds::rtps::net-interface
    fastdds::time
    )


gtest_discover_tests(MonitorServiceTests)
