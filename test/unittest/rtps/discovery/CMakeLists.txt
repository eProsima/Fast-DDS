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

#EDP TESTS
set(EDPTESTS_SOURCE EdpTests.cpp)

add_executable(EdpTests ${EDPTESTS_SOURCE})
target_link_units(EdpTests UNITS ${STATISTICS_TYPES_UNIT} rtps::external-locators rtps::history rtps::resource-event
    rtps::proxy-data dds::participant-qos rtps::locator-selector-sender rtps::flow-control rtps::edp
    MOCKS DomainParticipantFactory BaseReader StatefulReader BaseWriter StatefulWriter MessageReceiver
    rtps::network-factory::NetworkFactory PDP RTPSDomainImpl RTPSParticipantImpl SecurityManager TimedEvent
    ReaderHistory WriterHistory RTPSMessageGroup)
target_include_directories(EdpTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    ${Asio_INCLUDE_DIR}
    )
target_link_libraries(EdpTests
    fastcdr
    foonathan_memory
    GTest::gmock
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    $<$<BOOL:${WIN32}>:ws2_32>
    $<$<BOOL:${QNX}>:socket>
    ${CMAKE_DL_LIBS})

gtest_discover_tests(EdpTests)

#PDP TESTS

set(PDPTESTS_SOURCE PDPTests.cpp)

add_executable(PDPTests ${PDPTESTS_SOURCE})

target_include_directories(PDPTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${Asio_INCLUDE_DIR}
    )

target_link_units(PDPTests UNITS ${STATISTICS_TYPES_UNIT} rtps::network-factory rtps::builtin-data rtps::parameter-list
    rtps::history rtps::resource-event dds::participant-qos rtps::proxy-data rtps::external-locators
    rtps::locator-selector-sender rtps::flow-control dds::type-lookup-types rtps::pdp MOCKS DomainParticipantFactory
    BaseReader StatefulReader StatelessReader StatelessWriter BaseWriter StatefulWriter MessageReceiver
    BuiltinProtocols EDP RTPSParticipantImpl SecurityManager TimedEvent TypeLookupManager WLP ReaderHistory
    WriterHistory RTPSMessageGroup RTPSDomainImpl xtypes::TypeObjectUtils)
target_link_libraries(PDPTests
    fastcdr
    foonathan_memory
    GTest::gmock
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    $<$<BOOL:${WIN32}>:ws2_32>
    $<$<BOOL:${QNX}>:socket>
    ${CMAKE_DL_LIBS}
    $<$<BOOL:${LINK_SSL}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto>)

gtest_discover_tests(PDPTests)

#TOPIC INTEREST PAYLOAD TESTS
