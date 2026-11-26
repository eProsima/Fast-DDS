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

# ReaderProxy
set(WRITERPROXYTESTS_SOURCE ReaderProxyTests.cpp)

add_executable(ReaderProxyTests ${WRITERPROXYTESTS_SOURCE})
target_include_directories(ReaderProxyTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    )
target_link_units(ReaderProxyTests UNITS rtps::proxy-data rtps::builtin-data rtps::history
    rtps::locator-selector-sender rtps::reader-proxy rtps::flow-control rtps::resource-event ${STATISTICS_TYPES_UNIT}
    dds::participant-qos MOCKS DomainParticipantFactory BaseReader BaseWriter StatefulWriter
    StatelessWriter MessageReceiver rtps::network-factory::NetworkFactory RTPSDomainImpl ReaderLocator RTPSGapBuilder
    RTPSMessageGroup TimedEvent WriterHistory ReaderHistory RTPSParticipantImpl SecurityManager xtypes::TypeObjectUtils)
target_link_libraries(ReaderProxyTests
    fastcdr
    foonathan_memory
    GTest::gmock
    ${CMAKE_DL_LIBS}
    ${THIRDPARTY_BOOST_LINK_LIBS})
gtest_discover_tests(ReaderProxyTests)

set(LIVELINESSMANAGERTESTS_SOURCE LivelinessManagerTests.cpp)

add_executable(LivelinessManagerTests ${LIVELINESSMANAGERTESTS_SOURCE})
target_include_directories(LivelinessManagerTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    )
target_link_units(LivelinessManagerTests UNITS rtps::basic rtps::liveliness-manager)
target_link_libraries(LivelinessManagerTests
    fastcdr
    GTest::gmock
    )
gtest_discover_tests(LivelinessManagerTests)

if(NOT QNX)
    set(RTPSWRITERTESTS_SOURCE RTPSWriterTests.cpp)

    add_executable(RTPSWriterTests ${RTPSWRITERTESTS_SOURCE})
    target_include_directories(RTPSWriterTests PRIVATE
        ${Asio_INCLUDE_DIR})
    target_link_libraries(RTPSWriterTests fastcdr fastdds foonathan_memory
        GTest::gmock
        ${CMAKE_DL_LIBS})
    gtest_discover_tests(RTPSWriterTests)
endif()
