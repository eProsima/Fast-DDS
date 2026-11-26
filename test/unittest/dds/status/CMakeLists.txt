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

set(LISTENERTESTS_SOURCE ListenerTests.cpp)

# Only include tinyxml2 headers manually if the associated config predates the use of CMake targets
if(TINYXML2_INCLUDE_DIR)
    include_directories(${TINYXML2_INCLUDE_DIR})
endif(TINYXML2_INCLUDE_DIR)

if(ANDROID)
    if (ANDROID_NATIVE_API_LEVEL LESS 24)
        list(APPEND LISTENERTESTS_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
    endif()
endif()

add_executable(ListenerTests ${LISTENERTESTS_SOURCE})
target_link_units(ListenerTests UNITS dds::participant-qos xmlparser::parser dds::sql-filter rtps::proxy-data
    rtps::history rtps::resource-event xmlparser::common dds::condition rtps::locator-selector-sender
    rtps::flow-control dds::writer-history dds::reader-history dds::entities ${STATISTICS_TYPES_UNIT} MOCKS BaseReader
    StatefulReader StatelessReader StatelessWriter BaseWriter StatefulWriter DataSharingPayloadPool MessageReceiver
    rtps::network-factory::NetworkFactory RTPSMessageGroup RTPSDomainImpl RTPSParticipantImpl SecurityManager
    TimedEvent WLP ReaderHistory WriterHistory XMLEndpointParser xtypes::TypeObjectUtils)
target_include_directories(ListenerTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    )

target_link_libraries(ListenerTests
    fastcdr
    foonathan_memory
    ${TINYXML2_LIBRARY}
    GTest::gmock
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    $<$<BOOL:${WIN32}>:ws2_32>
    $<$<BOOL:${QNX}>:socket>
    ${CMAKE_DL_LIBS}
    ${THIRDPARTY_BOOST_LINK_LIBS}
    )

gtest_discover_tests(ListenerTests)
