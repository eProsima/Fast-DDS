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

set(FLOWCONTROLLERFACTORYTESTS_SOURCE
    FlowControllerFactoryTests.cpp
    )

add_executable(FlowControllerFactoryTests ${FLOWCONTROLLERFACTORYTESTS_SOURCE})
target_include_directories(FlowControllerFactoryTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    )
target_link_units(FlowControllerFactoryTests UNITS rtps::basic time log rtps::locator-selector-sender
    rtps::flow-control rtps::history ${STATISTICS_TYPES_UNIT} rtps::timed-event rtps::builtin-data rtps::proxy-data
    dds::participant-qos MOCKS DomainParticipantFactory BaseReader BaseWriter MessageReceiver
    rtps::network-factory::NetworkFactory RTPSMessageGroup RTPSParticipantImpl RTPSDomainImpl SecurityManager
    xtypes::TypeObjectUtils ReaderHistory WriterHistory)
target_link_libraries(FlowControllerFactoryTests
    fastcdr
    GTest::gmock
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    )
gtest_discover_tests(FlowControllerFactoryTests)

set(FLOWCONTROLLERPUBLISHMODESTESTS_SOURCE
    FlowControllerPublishModesOnPureSyncTests.cpp
    FlowControllerPublishModesOnSyncTests.cpp
    FlowControllerPublishModesOnAsyncTests.cpp
    FlowControllerPublishModesOnLimitedAsyncTests.cpp
    FlowControllerPublishModesTests.cpp
    )

add_executable(FlowControllerPublishModesTests ${FLOWCONTROLLERPUBLISHMODESTESTS_SOURCE})
target_include_directories(FlowControllerPublishModesTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    )
target_link_units(FlowControllerPublishModesTests UNITS rtps::basic time log rtps::locator-selector-sender rtps::history
    rtps::flow-control ${STATISTICS_TYPES_UNIT} rtps::timed-event rtps::builtin-data rtps::proxy-data
    dds::participant-qos MOCKS DomainParticipantFactory BaseReader BaseWriter MessageReceiver
    rtps::network-factory::NetworkFactory RTPSMessageGroup RTPSParticipantImpl RTPSDomainImpl SecurityManager
    xtypes::TypeObjectUtils ReaderHistory WriterHistory)
target_link_libraries(FlowControllerPublishModesTests
    fastcdr
    GTest::gmock
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    )
gtest_discover_tests(FlowControllerPublishModesTests)

set(FLOWCONTROLLERSCHEDULERSTESTS_SOURCE
    FlowControllerSchedulersTests.cpp
    )

add_executable(FlowControllerSchedulersTests ${FLOWCONTROLLERSCHEDULERSTESTS_SOURCE})
target_include_directories(FlowControllerSchedulersTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    )
target_link_units(FlowControllerSchedulersTests UNITS rtps::basic time log rtps::locator-selector-sender rtps::history
    rtps::flow-control ${STATISTICS_TYPES_UNIT} rtps::timed-event rtps::builtin-data rtps::proxy-data
    dds::participant-qos MOCKS DomainParticipantFactory BaseReader BaseWriter MessageReceiver
    rtps::network-factory::NetworkFactory RTPSMessageGroup RTPSParticipantImpl RTPSDomainImpl SecurityManager
    xtypes::TypeObjectUtils ReaderHistory WriterHistory)
target_link_libraries(FlowControllerSchedulersTests
    fastcdr
    GTest::gmock
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    )
gtest_discover_tests(FlowControllerSchedulersTests)
