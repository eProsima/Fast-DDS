# Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

set(TYPELOOKUPSERVICETESTS_SOURCE TypeLookupServiceTests.cpp
)

if(WIN32)
    set_source_files_properties(TypeLookupServiceTests.cpp PROPERTIES COMPILE_OPTIONS /bigobj)
endif()

add_executable(TypeLookupServiceTests ${TYPELOOKUPSERVICETESTS_SOURCE})
target_link_units(TypeLookupServiceTests UNITS ${STATISTICS_TYPES_UNIT} rtps::proxy-data rtps::history rtps::resource-event
    dds::participant-qos rtps::locator-selector-sender rtps::flow-control dds::type-lookup-listeners
    MOCKS BaseReader StatefulReader BaseWriter StatefulWriter MessageReceiver rtps::network-factory::NetworkFactory
    RTPSDomainImpl TimedEvent TypeLookupManager ReaderHistory WriterHistory xtypes::TypeObjectUtils
    DomainParticipantFactory RTPSMessageGroup RTPSParticipantImpl SecurityManager)
target_include_directories(TypeLookupServiceTests PUBLIC
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${Asio_INCLUDE_DIR}
    $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
    )

target_link_libraries(TypeLookupServiceTests
    fastcdr
    foonathan_memory
    GTest::gmock
    ${CMAKE_DL_LIBS}
    )
gtest_discover_tests(TypeLookupServiceTests)
