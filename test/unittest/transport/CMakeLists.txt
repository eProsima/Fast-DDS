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

# Copy certs
if(TLS_FOUND)
    configure_file(${PROJECT_SOURCE_DIR}/test/certs/maincacert.pem
        ${CMAKE_CURRENT_BINARY_DIR}/maincacert.pem COPYONLY)
    configure_file(${PROJECT_SOURCE_DIR}/test/certs/mainpubcert.pem
        ${CMAKE_CURRENT_BINARY_DIR}/mainpubcert.pem COPYONLY)
    configure_file(${PROJECT_SOURCE_DIR}/test/certs/mainpubkey.pem
        ${CMAKE_CURRENT_BINARY_DIR}/mainpubkey.pem COPYONLY)
    configure_file(${PROJECT_SOURCE_DIR}/test/certs/mainsubcert.pem
        ${CMAKE_CURRENT_BINARY_DIR}/mainsubcert.pem COPYONLY)
    configure_file(${PROJECT_SOURCE_DIR}/test/certs/mainsubkey.pem
        ${CMAKE_CURRENT_BINARY_DIR}/mainsubkey.pem COPYONLY)
    configure_file(${PROJECT_SOURCE_DIR}/test/unittest/transport/certs/ca.crt
        ${CMAKE_CURRENT_BINARY_DIR}/ca.crt COPYONLY)
    configure_file(${PROJECT_SOURCE_DIR}/test/unittest/transport/certs/fastdds.crt
        ${CMAKE_CURRENT_BINARY_DIR}/fastdds.crt COPYONLY)
    configure_file(${PROJECT_SOURCE_DIR}/test/unittest/transport/certs/fastdds.key
        ${CMAKE_CURRENT_BINARY_DIR}/fastdds.key COPYONLY)
    configure_file(${PROJECT_SOURCE_DIR}/test/unittest/transport/certs/dh_params.pem
        ${CMAKE_CURRENT_BINARY_DIR}/dh_params.pem COPYONLY)
    #configure_file(${PROJECT_SOURCE_DIR}/test/certs/governance_helloworld_all_enable.smime
    #    ${CMAKE_CURRENT_BINARY_DIR}/governance_helloworld_all_enable.smime COPYONLY)
    #configure_file(${PROJECT_SOURCE_DIR}/test/certs/permissions_helloworld.smime
    #    ${CMAKE_CURRENT_BINARY_DIR}/permissions_helloworld.smime COPYONLY)
endif()

set(UDPV4TESTS_SOURCE
    UDPv4Tests.cpp
    mock/MockReceiverResource.cpp
    )

set(ASIOHELPERSTESTS_SOURCE
    AsioHelpersTests.cpp
    mock/MockReceiverResource.cpp
    )


set(UDPV6TESTS_SOURCE
    UDPv6Tests.cpp
    mock/MockReceiverResource.cpp
    )

set(TCPV4TESTS_SOURCE
    TCPv4Tests.cpp
    mock/MockReceiverResource.cpp
    mock/MockTCPChannelResource.cpp
    )

set(TCPV6TESTS_SOURCE
    TCPv6Tests.cpp
    mock/MockReceiverResource.cpp
    )

set(SHAREDMEMTESTS_SOURCE
    SharedMemTests.cpp
    mock/MockReceiverResource.cpp
    )

if(ANDROID)
    if (ANDROID_NATIVE_API_LEVEL LESS 24)
        list(APPEND TCPV6TESTS_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
        list(APPEND TCPV4TESTS_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
        list(APPEND UDPV6TESTS_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
        list(APPEND UDPV4TESTS_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
        list(APPEND SHAREDMEMTESTS_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
    endif()
endif()

include_directories(mock/)

##########################
# UDPv4 tests
##########################
add_executable(UDPv4Tests ${UDPV4TESTS_SOURCE})
target_include_directories(UDPv4Tests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
    )
target_link_units(UDPv4Tests UNITS rtps::network-factory rtps::udp-transport MOCKS MessageReceiver)
target_link_libraries(UDPv4Tests
    fastcdr
    GTest::gtest
    ${MOCKS}
    $<$<BOOL:${TLS_FOUND}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto>
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    $<$<BOOL:${QNX}>:socket>
)
gtest_discover_tests(UDPv4Tests)

##########################
# IPv6 tests
##########################
option(DISABLE_UDPV6_TESTS "Disable UDPv6 tests because fails in some systems" OFF)

if(NOT DISABLE_UDPV6_TESTS)
    ##########################
    # UDPv6 tests
    ##########################
    add_executable(UDPv6Tests ${UDPV6TESTS_SOURCE})
    target_include_directories(UDPv6Tests PRIVATE
        ${Asio_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
        ${PROJECT_SOURCE_DIR}/src/cpp
        )
    target_link_units(UDPv6Tests UNITS rtps::network-factory rtps::udp-transport MOCKS MessageReceiver)
    target_link_libraries(UDPv6Tests
        fastcdr
        GTest::gtest
        ${MOCKS}
        $<$<BOOL:${TLS_FOUND}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto>
        $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
        $<$<BOOL:${QNX}>:socket>
    )
    gtest_discover_tests(UDPv6Tests)

    ##########################
    # TCPv6 tests
    ##########################
    add_executable(TCPv6Tests ${TCPV6TESTS_SOURCE})
    target_include_directories(TCPv6Tests PRIVATE
        ${Asio_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
        ${PROJECT_SOURCE_DIR}/src/cpp
        $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
        )
    target_link_units(TCPv6Tests UNITS rtps::network-factory rtps::timed-event rtps::parameter-list MOCKS MessageReceiver)
    target_link_libraries(TCPv6Tests
        fastcdr
        GTest::gtest
        ${MOCKS}
        $<$<BOOL:${TLS_FOUND}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto>
        $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
        $<$<BOOL:${QNX}>:socket>
    )
    gtest_discover_tests(TCPv6Tests)
endif()

##########################
# TCPv4 tests
##########################
add_executable(TCPv4Tests ${TCPV4TESTS_SOURCE})
target_include_directories(TCPv4Tests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
    )
target_link_units(TCPv4Tests UNITS rtps::network-factory rtps::timed-event rtps::parameter-list MOCKS MessageReceiver)
target_link_libraries(TCPv4Tests
    fastcdr
    GTest::gtest
    ${MOCKS}
    $<$<BOOL:${TLS_FOUND}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto>
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    $<$<BOOL:${QNX}>:socket>
)
gtest_discover_tests(TCPv4Tests)

##########################
# SHM tests
##########################
if(IS_THIRDPARTY_BOOST_OK)
    add_executable(SharedMemTests ${SHAREDMEMTESTS_SOURCE})

    target_include_directories(SharedMemTests PRIVATE
        ${Asio_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
        ${PROJECT_SOURCE_DIR}/src/cpp
        ${THIRDPARTY_BOOST_INCLUDE_DIR}
        $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
        )
    target_link_units(SharedMemTests UNITS rtps::network-factory utils::shared-memory rtps::shm-transport MOCKS
    MessageReceiver)
    target_link_libraries(SharedMemTests
        foonathan_memory
        fastcdr
        GTest::gtest
        ${MOCKS}
        $<$<BOOL:${TLS_FOUND}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto>
        $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
        $<$<BOOL:${QNX}>:socket>
        ${THIRDPARTY_BOOST_LINK_LIBS}
        eProsima_atomic
        )
    gtest_discover_tests(SharedMemTests)
endif()

# Add 'xfail' label to flaky tests
set_property(DIRECTORY APPEND PROPERTY
    TEST_INCLUDE_FILES ${CMAKE_CURRENT_LIST_DIR}/xfail_tests.cmake
    )

#####################################
# PortBasedTransportDescriptor tests
#####################################
set(PORTBASED_TRANSPORTDESCRIPTOR_TESTS_SOURCES
    PortBasedTransportDescriptorTests.cpp
    )

set(PORTBASED_TRANSPORTDESCRIPTOR_TESTS_TARGET PortBasedTransportDescriptorTests)

add_executable(${PORTBASED_TRANSPORTDESCRIPTOR_TESTS_TARGET}
    ${PORTBASED_TRANSPORTDESCRIPTOR_TESTS_SOURCES})

target_include_directories(${PORTBASED_TRANSPORTDESCRIPTOR_TESTS_TARGET}
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include)

target_link_units(${PORTBASED_TRANSPORTDESCRIPTOR_TESTS_TARGET} UNITS rtps::basic)
target_link_libraries(${PORTBASED_TRANSPORTDESCRIPTOR_TESTS_TARGET}
    GTest::gtest
    ${MOCKS})

gtest_discover_tests(${PORTBASED_TRANSPORTDESCRIPTOR_TESTS_TARGET})

#####################################
# AsioHelpers tests
#####################################
add_executable(AsioHelpersTests ${ASIOHELPERSTESTS_SOURCE})
target_include_directories(AsioHelpersTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
    )
target_link_units(AsioHelpersTests UNITS rtps::network-factory rtps::udp-transport MOCKS MessageReceiver)
target_link_libraries(AsioHelpersTests
    fastcdr
    GTest::gtest
    ${MOCKS}
    $<$<BOOL:${TLS_FOUND}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto>
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    $<$<BOOL:${QNX}>:socket>
)
gtest_discover_tests(AsioHelpersTests)
