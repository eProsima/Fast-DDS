# Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
    add_definitions(-DNOMINMAX)
endif()

###########################################################################
# WriterProxyTests
###########################################################################
set(WRITERPROXYTESTS_SOURCE WriterProxyTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/qos/WriterQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/PublicationBuiltinTopicData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/LocatorWithMask.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/network.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetmaskFilterKind.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterfaceWithFilter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/Host.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
    )

add_executable(WriterProxyTests ${WRITERPROXYTESTS_SOURCE})
target_compile_definitions(WriterProxyTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(WriterProxyTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/reader
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/Endpoint
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/external_locators
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSReader
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSWriter
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSParticipantImpl
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSDomainImpl
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/TimedEvent
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/StatefulReader
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/WriterProxyData
    ${PROJECT_SOURCE_DIR}/test/mock/dds/QosPolicies
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/ResourceEvent
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    )
target_link_libraries(WriterProxyTests
    fastcdr
    fastdds::log
    fastdds::xtypes::dynamic-types::impl
    fastdds::xtypes::type-representation
    foonathan_memory
    GTest::gmock
    ${CMAKE_DL_LIBS})
gtest_discover_tests(WriterProxyTests)

###########################################################################
# WriterProxyStopTest
###########################################################################
set(WRITERPROXYSTOPTEST_SOURCE WriterProxyStopTest.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/qos/WriterQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/PublicationBuiltinTopicData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/LocatorWithMask.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/network.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/ResourceEvent.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEvent.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEventImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetmaskFilterKind.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterfaceWithFilter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/Host.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/TimedConditionVariable.cpp
    )

add_executable(WriterProxyStopTest ${WRITERPROXYSTOPTEST_SOURCE})
target_compile_definitions(WriterProxyStopTest PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(WriterProxyStopTest PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/reader
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/Endpoint
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/external_locators
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSReader
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSWriter
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSParticipantImpl
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSDomainImpl
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/StatefulReader
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/WriterProxyData
    ${PROJECT_SOURCE_DIR}/test/mock/dds/QosPolicies
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    )
target_link_libraries(WriterProxyStopTest
    fastcdr
    fastdds::log
    fastdds::xtypes::dynamic-types::impl
    fastdds::xtypes::type-representation
    foonathan_memory
    GTest::gmock
    ${CMAKE_DL_LIBS})
gtest_discover_tests(WriterProxyStopTest)

###########################################################################
# WriterProxyAcknackTests
###########################################################################
set(WRITERPROXYACKNACKTESTS_SOURCE WriterProxyAcknackTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/qos/WriterQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/PublicationBuiltinTopicData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/LocatorWithMask.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/network.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEvent.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEventImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetmaskFilterKind.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterfaceWithFilter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/Host.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
    )

add_executable(WriterProxyAcknackTests ${WRITERPROXYACKNACKTESTS_SOURCE})
target_compile_definitions(WriterProxyAcknackTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(WriterProxyAcknackTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/reader
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/Endpoint
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/external_locators
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSReader
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSWriter
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSParticipantImpl
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSDomainImpl
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/StatefulReader
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/WriterProxyData
    ${PROJECT_SOURCE_DIR}/test/mock/dds/QosPolicies
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/ResourceEvent
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    )
target_link_libraries(WriterProxyAcknackTests
    fastcdr
    fastdds::log
    fastdds::xtypes::dynamic-types::impl
    fastdds::xtypes::type-representation
    foonathan_memory
    GTest::gmock
    ${CMAKE_DL_LIBS})
gtest_discover_tests(WriterProxyAcknackTests)

get_property(fastdds_sources_files TARGET ${PROJECT_NAME} PROPERTY SOURCES)
list(REMOVE_ITEM fastdds_sources_files "${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/DomainParticipantFactory.cpp")
get_property(fastdds_compile_definitions TARGET ${PROJECT_NAME} PROPERTY COMPILE_DEFINITIONS)
list(FILTER fastdds_compile_definitions EXCLUDE REGEX ".*FASTDDS_DYN_LINK.*")
get_property(fastdds_include_directories TARGET ${PROJECT_NAME} PROPERTY INCLUDE_DIRECTORIES)
get_property(fastdds_link_libraries TARGET ${PROJECT_NAME} PROPERTY LINK_LIBRARIES)

###########################################################################
# StatefulReaderTests
###########################################################################
set(STATEFUL_READER_TESTS_SOURCE StatefulReaderTests.cpp)

add_executable(StatefulReaderTests ${STATEFUL_READER_TESTS_SOURCE})

target_compile_definitions(StatefulReaderTests PRIVATE
    ${fastdds_compile_definitions}
)

target_sources(StatefulReaderTests
    PRIVATE
    ${fastdds_sources_files}
)

target_include_directories(StatefulReaderTests PRIVATE
    ${PROJECT_SOURCE_DIR}/test/mock/dds/DomainParticipantFactory
    ${fastdds_include_directories}
    )
target_link_libraries(StatefulReaderTests
    GTest::gmock
    ${fastdds_link_libraries}
    )
gtest_discover_tests(StatefulReaderTests)

###########################################################################
# StatelessReaderTests
###########################################################################
set(STATELESS_READER_TESTS_SOURCE StatelessReaderTests.cpp)

add_executable(StatelessReaderTests ${STATELESS_READER_TESTS_SOURCE})

target_compile_definitions(StatelessReaderTests PRIVATE
    ${fastdds_compile_definitions}
    )

target_sources(StatelessReaderTests
    PRIVATE
    ${fastdds_sources_files}
)

target_include_directories(StatelessReaderTests PRIVATE
    ${PROJECT_SOURCE_DIR}/test/mock/dds/DomainParticipantFactory
    ${fastdds_include_directories}
    )
target_link_libraries(StatelessReaderTests
    GTest::gmock
    ${fastdds_link_libraries}
    )
gtest_discover_tests(StatelessReaderTests)
