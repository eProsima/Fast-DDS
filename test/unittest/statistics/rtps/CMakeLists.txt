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

set(STATISTICS_RTPS_MONITORSERVICETESTS_SOURCE MonitorServiceTests.cpp)

add_executable(MonitorServiceTests ${STATISTICS_RTPS_MONITORSERVICETESTS_SOURCE})

target_link_units(MonitorServiceTests UNITS statistics::types dds::participant-qos rtps::network-factory
    rtps::timed-event xtypes rtps::builtin-data rtps::parameter-list rtps::history rtps::proxy-data
    rtps::locator-selector-sender dds::writer-history statistics::monitor-service MOCKS BaseReader StatefulReader
    StatelessReader StatelessWriter BaseWriter StatefulWriter DomainParticipantFactory MessageReceiver RTPSDomainImpl
    ReaderHistory StatisticsBase TypeSupport WriterHistory xtypes::TypeObjectUtils XMLProfileManager)
target_include_directories(MonitorServiceTests PRIVATE
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

gtest_discover_tests(MonitorServiceTests)
