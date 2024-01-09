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
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/builtin/type_lookup_service/detail/*.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/builtin/type_lookup_service/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Entity.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/policy/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/qos/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/qos/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/history/DataReaderHistory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/qos/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/DDSSQLFilter/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/qos/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/utils/QosConverters.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/xtypes/exception/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/xtypes/type_representation/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/xtypes/type_representation/*.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/discovery/database/backup/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/discovery/database/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/discovery/endpoint/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/discovery/participant/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/discovery/participant/timedevent/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/liveliness/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/DataSharing/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/participant/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/persistence/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/persistence/sqlite3.c
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/reader/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/xmlparser/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/publisher/qos/DataWriterQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/statistics/rtps/reader/StatisticsReaderImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/statistics/rtps/StatisticsBase.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/statistics/rtps/writer/StatisticsWriterImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/*.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/*.cpp
    )

if (FASTDDS_STATISTICS)
    file(GLOB statistics_sources
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/domain/*.cpp
        )
    list(APPEND DYNAMIC_TYPES_SOURCE ${statistics_sources})
endif()

if(TLS_FOUND)
    list(APPEND DYNAMIC_TYPES_SOURCE
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPChannelResourceSecure.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPAcceptorSecure.cpp
        )
endif()

if(SECURITY)
    file(GLOB security_sources
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/exceptions/Exception.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Token.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/security/exceptions/SecurityException.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/security/common/SharedSecretHandle.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/security/logging/Logging.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/security/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/discovery/participant/DS/PDPSecurityInitiatorListener.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/security/accesscontrol/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/security/artifact_providers/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/security/authentication/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/security/cryptography/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/security/logging/*.cpp
        )
    list(APPEND DYNAMIC_TYPES_SOURCE ${security_sources})
endif()

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

file(GLOB DYNAMIC_TYPES_NON_MOCK_SOURCE
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/ChannelResource.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/PortBasedTransportDescriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/shared_mem/SharedMemTransportDescriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/tcp/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPAcceptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPAcceptorBasic.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPChannelResource.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPChannelResourceBasic.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPTransportInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPv4Transport.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPv6Transport.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/UDPChannelResource.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/UDPTransportInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/UDPv4Transport.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/UDPv6Transport.cpp
    )

if (IS_THIRDPARTY_BOOST_OK)
    list(APPEND DYNAMIC_TYPES_NON_MOCK_SOURCE
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/shared_mem/test_SharedMemTransport.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/shared_mem/SharedMemTransport.cpp
        )
endif()

set(DYNAMIC_TYPES_TEST_SOURCE
    DynamicTypesTests.cpp
    idl/BasicPubSubTypes.cxx
    idl/BasicTypeObjectSupport.cxx
    ${DYNAMIC_TYPES_SOURCE}
    ${DYNAMIC_TYPES_NON_MOCK_SOURCE}
    )

set(DYNAMIC_COMPLEX_TYPES_TEST_SOURCE
    DynamicComplexTypesTests.cpp
    idl/TestPubSubTypes.cxx
    idl/TestTypeObjectSupport.cxx
    ${DYNAMIC_TYPES_SOURCE}
    ${DYNAMIC_TYPES_NON_MOCK_SOURCE}
    )

set(DYNAMIC_TYPES_4_2_TEST_SOURCE
    DynamicTypes_4_2_Tests.cpp
    idl/new_features_4_2PubSubTypes.cxx
    idl/new_features_4_2TypeObjectSupport.cxx
    ${DYNAMIC_TYPES_SOURCE}
    ${DYNAMIC_TYPES_NON_MOCK_SOURCE}
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
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSParticipantAttributes
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    ${Asio_INCLUDE_DIR}
    $<$<BOOL:${TINYXML2_INCLUDE_DIR}>:${TINYXML2_INCLUDE_DIR}>
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    $<$<BOOL:${OPENSSL_INCLUDE_DIR}>:${OPENSSL_INCLUDE_DIR}>
    $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
    )
target_link_libraries(DynamicTypesTests GTest::gtest
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    $<$<BOOL:${WIN32}>:ws2_32>
    ${TINYXML2_LIBRARY}
    ${THIRDPARTY_BOOST_LINK_LIBS}
    $<$<BOOL:${LINK_SSL}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto>
    fastcdr
    foonathan_memory
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
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
    $<$<BOOL:${OPENSSL_INCLUDE_DIR}>:${OPENSSL_INCLUDE_DIR}>
    )

target_link_libraries(DynamicComplexTypesTests GTest::gtest
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    $<$<BOOL:${WIN32}>:ws2_32>
    ${TINYXML2_LIBRARY}
    ${THIRDPARTY_BOOST_LINK_LIBS}
    fastcdr
    foonathan_memory
    $<$<BOOL:${LINK_SSL}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto>
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
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    $<$<BOOL:${OPENSSL_INCLUDE_DIR}>:${OPENSSL_INCLUDE_DIR}>
    )

target_link_libraries(DynamicTypes_4_2_Tests GTest::gtest
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    $<$<BOOL:${WIN32}>:ws2_32>
    ${TINYXML2_LIBRARY}
    ${THIRDPARTY_BOOST_LINK_LIBS}
    fastcdr
    foonathan_memory
    $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
    $<$<BOOL:${LINK_SSL}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto>
    )
if(QNX)
    target_link_libraries(DynamicTypes_4_2_Tests socket)
endif()
gtest_discover_tests(DynamicTypes_4_2_Tests)

configure_file("types_profile.xml" "types_profile.xml" COPYONLY)
