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

if(WIN32)
    add_definitions(
        -D_WIN32_WINNT=0x0601
        -D_CRT_SECURE_NO_WARNINGS
        )
endif()

file(GLOB DYNAMIC_TYPES_SOURCE
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/string_convert.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/TopicDataType.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/DomainParticipantFactory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/xtypes/type_representation/*.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    )

# External sources
if(TINYXML2_SOURCE_DIR)
    list(APPEND DYNAMIC_TYPES_SOURCE
        ${TINYXML2_SOURCE_DIR}/tinyxml2.cpp
        )
endif()

if(ANDROID)
    if (ANDROID_NATIVE_API_LEVEL LESS 24)
        list(APPEND DYNAMIC_TYPES_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
    endif()
endif()

set(DYNAMIC_TYPES_TEST_SOURCE
    DynamicTypesTests.cpp
    idl/BasicPubSubTypes.cxx
    idl/BasicTypeObjectSupport.cxx
    ${DYNAMIC_TYPES_SOURCE}
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/qos/WriterQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/qos/ReaderQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/ThreadSettings.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/FlowControllerConsts.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/ThroughputControllerDescriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/PortBasedTransportDescriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/xmlparser/XMLDynamicParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/xmlparser/XMLElementParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/xmlparser/XMLParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/xmlparser/XMLParserCommon.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/xmlparser/XMLProfileManager.cpp
    )

set(DYNAMIC_COMPLEX_TYPES_TEST_SOURCE
    DynamicComplexTypesTests.cpp
    idl/TestPubSubTypes.cxx
    idl/TestTypeObjectSupport.cxx
    ${DYNAMIC_TYPES_SOURCE}
    )

set(DYNAMIC_TYPES_4_2_TEST_SOURCE
    DynamicTypes_4_2_Tests.cpp
    idl/new_features_4_2PubSubTypes.cxx
    idl/new_features_4_2TypeObjectSupport.cxx
    ${DYNAMIC_TYPES_SOURCE}
    )

include_directories(mock/)

add_executable(DynamicTypesTests ${DYNAMIC_TYPES_TEST_SOURCE})
target_compile_definitions(DynamicTypesTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(DynamicTypesTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/SharedMemTransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPTransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPv4TransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPv6TransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/UDPTransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/UDPv4TransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/UDPv6TransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSParticipantAttributes
    $<$<BOOL:${TINYXML2_INCLUDE_DIR}>:${TINYXML2_INCLUDE_DIR}>
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
    )
target_link_libraries(DynamicTypesTests GTest::gtest
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    $<$<BOOL:${WIN32}>:ws2_32>
    ${TINYXML2_LIBRARY}
    fastcdr
    )
if(QNX)
    target_link_libraries(DynamicTypesTests socket)
endif()
gtest_discover_tests(DynamicTypesTests)


add_executable(DynamicComplexTypesTests ${DYNAMIC_COMPLEX_TYPES_TEST_SOURCE})
target_compile_definitions(DynamicComplexTypesTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(DynamicComplexTypesTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>)
target_link_libraries(DynamicComplexTypesTests GTest::gtest
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    $<$<BOOL:${WIN32}>:ws2_32>
    ${TINYXML2_LIBRARY}
    fastcdr
    )
if(QNX)
    target_link_libraries(DynamicComplexTypesTests socket)
endif()
gtest_discover_tests(DynamicComplexTypesTests)


add_executable(DynamicTypes_4_2_Tests ${DYNAMIC_TYPES_4_2_TEST_SOURCE})
target_compile_definitions(DynamicTypes_4_2_Tests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(DynamicTypes_4_2_Tests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp)
    target_link_libraries(DynamicTypes_4_2_Tests GTest::gtest
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    $<$<BOOL:${WIN32}>:ws2_32>
    ${TINYXML2_LIBRARY}
    fastcdr
    $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
    )
if(QNX)
    target_link_libraries(DynamicTypes_4_2_Tests socket)
endif()
gtest_discover_tests(DynamicTypes_4_2_Tests)

configure_file("types_profile.xml" "types_profile.xml" COPYONLY)
