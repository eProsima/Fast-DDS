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
    add_definitions(
        -D_WIN32_WINNT=0x0601
        -D_CRT_SECURE_NO_WARNINGS
        -DNOMINMAX
        )
endif()

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

set(TCPTransportInterface_SOURCE
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/CDRMessage.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/RTPSMessageCreator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/tcp/RTCPMessageManager.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/tcp/TCPControlMessage.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPAcceptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPAcceptorBasic.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPChannelResource.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPChannelResourceBasic.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPTransportInterface.cpp
    )
if(TLS_FOUND)
    set(TCPTransportInterface_SOURCE
        ${TCPTransportInterface_SOURCE}
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPAcceptorSecure.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPChannelResourceSecure.cpp
        )
endif()

set(UDPV4TESTS_SOURCE
    UDPv4Tests.cpp
    mock/MockReceiverResource.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/PropertyPolicy.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/ThreadSettings.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/GuidPrefix_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/LocatorWithMask.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/CDRMessage.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/NetworkBuffer.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/NetworkFactory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/network.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/ChannelResource.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetmaskFilterKind.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterfaceWithFilter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/PortBasedTransportDescriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TransportInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/UDPChannelResource.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/UDPTransportInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/UDPv4Transport.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/Host.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
    ${TCPTransportInterface_SOURCE}
    )

set(ASIOHELPERSTESTS_SOURCE
    AsioHelpersTests.cpp
    mock/MockReceiverResource.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/PropertyPolicy.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/ThreadSettings.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/GuidPrefix_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/LocatorWithMask.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/CDRMessage.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/NetworkBuffer.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/NetworkFactory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/network.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/ChannelResource.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetmaskFilterKind.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterfaceWithFilter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/PortBasedTransportDescriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TransportInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/UDPChannelResource.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/UDPTransportInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/UDPv4Transport.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/Host.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
    ${TCPTransportInterface_SOURCE}
    )


set(UDPV6TESTS_SOURCE
    UDPv6Tests.cpp
    mock/MockReceiverResource.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/PropertyPolicy.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/ThreadSettings.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/GuidPrefix_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/LocatorWithMask.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/CDRMessage.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/NetworkBuffer.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/NetworkFactory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/network.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/ChannelResource.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetmaskFilterKind.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterfaceWithFilter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/PortBasedTransportDescriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TransportInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/UDPChannelResource.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/UDPTransportInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/UDPv4Transport.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/UDPv6Transport.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/Host.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
    ${TCPTransportInterface_SOURCE}
    )

set(TCPV4TESTS_SOURCE
    TCPv4Tests.cpp
    mock/MockReceiverResource.cpp
    mock/MockTCPChannelResource.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/policy/ParameterList.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/PropertyPolicy.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/ThreadSettings.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/GuidPrefix_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/LocatorWithMask.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/CDRMessage.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/RTPSMessageCreator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/NetworkBuffer.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/NetworkFactory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/network.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/ResourceEvent.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEvent.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEventImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/ChannelResource.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetmaskFilterKind.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterfaceWithFilter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/PortBasedTransportDescriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/tcp/RTCPMessageManager.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/tcp/TCPControlMessage.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPAcceptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPAcceptorBasic.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPChannelResource.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPChannelResourceBasic.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPTransportInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPv4Transport.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TransportInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/Host.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/TimedConditionVariable.cpp
    )

if(TLS_FOUND)
    set(TCPV4TESTS_SOURCE
        ${TCPV4TESTS_SOURCE}
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPAcceptorSecure.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPChannelResourceSecure.cpp
        )
endif()

set(TCPV6TESTS_SOURCE
    TCPv6Tests.cpp
    mock/MockReceiverResource.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/policy/ParameterList.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/PropertyPolicy.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/ThreadSettings.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/GuidPrefix_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/LocatorWithMask.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/CDRMessage.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/RTPSMessageCreator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/NetworkBuffer.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/NetworkFactory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/network.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/ResourceEvent.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEvent.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEventImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/ChannelResource.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetmaskFilterKind.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterfaceWithFilter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/PortBasedTransportDescriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/tcp/RTCPMessageManager.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/tcp/TCPControlMessage.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPAcceptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPAcceptorBasic.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPChannelResource.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPChannelResourceBasic.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPTransportInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPv6Transport.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TransportInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/Host.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/TimedConditionVariable.cpp
    )

if(TLS_FOUND)
    set(TCPV6TESTS_SOURCE
        ${TCPV6TESTS_SOURCE}
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPAcceptorSecure.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPChannelResourceSecure.cpp
        )
endif()

set(SHAREDMEMTESTS_SOURCE
    SharedMemTests.cpp
    mock/MockReceiverResource.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/PropertyPolicy.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/ThreadSettings.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/GuidPrefix_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/LocatorWithMask.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/NetworkBuffer.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/NetworkFactory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/network.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/ChannelResource.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetmaskFilterKind.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterfaceWithFilter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/PortBasedTransportDescriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/shared_mem/SharedMemTransport.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/shared_mem/SharedMemTransportDescriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TransportInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/Host.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
    ${TCPTransportInterface_SOURCE}
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
target_compile_definitions(UDPv4Tests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(UDPv4Tests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/MessageReceiver
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/ReceiverResource
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
    )
target_link_libraries(UDPv4Tests
    fastcdr
    fastdds::log
    GTest::gtest
    ${MOCKS}
    $<$<BOOL:${TLS_FOUND}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto>)
if(QNX)
    target_link_libraries(UDPv4Tests socket)
endif()
if(MSVC OR MSVC_IDE)
    target_link_libraries(UDPv4Tests ${PRIVACY} iphlpapi Shlwapi )
endif()
if (APPLE)
    target_link_libraries(UDPv4Tests ${PRIVACY} "-framework CoreFoundation" "-framework IOKit")
endif()
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
    target_compile_definitions(UDPv6Tests PRIVATE
        BOOST_ASIO_STANDALONE
        ASIO_STANDALONE
        $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
        $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
        )
    target_include_directories(UDPv6Tests PRIVATE
        ${Asio_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/test/mock/rtps/MessageReceiver
        ${PROJECT_SOURCE_DIR}/test/mock/rtps/ReceiverResource
        ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
        ${PROJECT_SOURCE_DIR}/src/cpp
        )
    target_link_libraries(UDPv6Tests
        fastcdr
        fastdds::log
        GTest::gtest
        ${MOCKS}
        $<$<BOOL:${TLS_FOUND}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto>)
    if(QNX)
        target_link_libraries(UDPv6Tests socket)
    endif()
    if(MSVC OR MSVC_IDE)
        target_link_libraries(UDPv6Tests ${PRIVACY} iphlpapi Shlwapi )
    endif()
    if (APPLE)
        target_link_libraries(UDPv6Tests ${PRIVACY} "-framework CoreFoundation" "-framework IOKit")
    endif()
    gtest_discover_tests(UDPv6Tests)

    ##########################
    # TCPv6 tests
    ##########################
    add_executable(TCPv6Tests ${TCPV6TESTS_SOURCE})
    target_compile_definitions(TCPv6Tests PRIVATE
        BOOST_ASIO_STANDALONE
        ASIO_STANDALONE
        $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
        $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
        )
    target_include_directories(TCPv6Tests PRIVATE
        ${Asio_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/test/mock/rtps/ParticipantProxyData
        ${PROJECT_SOURCE_DIR}/test/mock/dds/QosPolicies
        ${PROJECT_SOURCE_DIR}/test/mock/rtps/MessageReceiver
        ${PROJECT_SOURCE_DIR}/test/mock/rtps/ReceiverResource
        ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
        ${PROJECT_SOURCE_DIR}/src/cpp
        $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
        )
    target_link_libraries(TCPv6Tests
        fastcdr
        fastdds::log
        GTest::gtest
        ${MOCKS}
        $<$<BOOL:${TLS_FOUND}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto>)
    if(QNX)
        target_link_libraries(TCPv6Tests socket)
    endif()
    if(MSVC OR MSVC_IDE)
        target_link_libraries(TCPv6Tests ${PRIVACY} fastcdr iphlpapi Shlwapi)
    elseif(APPLE)
        target_link_libraries(TCPv6Tests ${PRIVACY} fastcdr "-framework CoreFoundation" "-framework IOKit")
    else()
        target_link_libraries(TCPv6Tests ${PRIVACY} fastcdr)
    endif()
    gtest_discover_tests(TCPv6Tests)
endif()

##########################
# TCPv4 tests
##########################
add_executable(TCPv4Tests ${TCPV4TESTS_SOURCE})
target_compile_definitions(TCPv4Tests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(TCPv4Tests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/ParticipantProxyData
    ${PROJECT_SOURCE_DIR}/test/mock/dds/QosPolicies
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/MessageReceiver
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/ReceiverResource
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
    )
target_link_libraries(TCPv4Tests
    fastcdr
    fastdds::log
    GTest::gtest
    ${MOCKS}
    $<$<BOOL:${TLS_FOUND}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto>)
if(QNX)
    target_link_libraries(TCPv4Tests socket)
endif()
if(MSVC OR MSVC_IDE)
    target_link_libraries(TCPv4Tests ${PRIVACY} fastcdr iphlpapi Shlwapi)
elseif(APPLE)
    target_link_libraries(TCPv4Tests ${PRIVACY} fastcdr "-framework CoreFoundation" "-framework IOKit")
else()
    target_link_libraries(TCPv4Tests ${PRIVACY} fastcdr)
endif()
gtest_discover_tests(TCPv4Tests)

##########################
# SHM tests
##########################
if(IS_THIRDPARTY_BOOST_OK)
    add_executable(SharedMemTests ${SHAREDMEMTESTS_SOURCE})

    target_compile_definitions(SharedMemTests PRIVATE
        BOOST_ASIO_STANDALONE
        ASIO_STANDALONE
        $<$<BOOL:${WIN32}>:_ENABLE_ATOMIC_ALIGNMENT_FIX>
        $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
        $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
        )

    target_include_directories(SharedMemTests PRIVATE
        ${Asio_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/test/mock/rtps/MessageReceiver
        ${PROJECT_SOURCE_DIR}/test/mock/rtps/ReceiverResource
        ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
        ${PROJECT_SOURCE_DIR}/src/cpp
        ${THIRDPARTY_BOOST_INCLUDE_DIR}
        $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
        )
    target_link_libraries(SharedMemTests
        foonathan_memory
        fastcdr
        fastdds::log
        GTest::gtest
        ${MOCKS}
        $<$<BOOL:${TLS_FOUND}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto>
        ${THIRDPARTY_BOOST_LINK_LIBS}
        eProsima_atomic
        )
    if(QNX)
        target_link_libraries(SharedMemTests socket)
    endif()
    if(MSVC OR MSVC_IDE)
        target_link_libraries(SharedMemTests ${PRIVACY} iphlpapi Shlwapi )
    elseif(APPLE)
        target_link_libraries(SharedMemTests ${PRIVACY} "-framework CoreFoundation" "-framework IOKit")
    else()
        target_link_libraries(SharedMemTests ${PRIVACY} )
    endif()
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
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/ThreadSettings.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/PortBasedTransportDescriptor.cpp)

set(PORTBASED_TRANSPORTDESCRIPTOR_TESTS_TARGET PortBasedTransportDescriptorTests)

add_executable(${PORTBASED_TRANSPORTDESCRIPTOR_TESTS_TARGET}
    ${PORTBASED_TRANSPORTDESCRIPTOR_TESTS_SOURCES})

target_compile_definitions(${PORTBASED_TRANSPORTDESCRIPTOR_TESTS_TARGET}
    PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG>)

target_include_directories(${PORTBASED_TRANSPORTDESCRIPTOR_TESTS_TARGET}
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include)

target_link_libraries(${PORTBASED_TRANSPORTDESCRIPTOR_TESTS_TARGET}
    GTest::gtest
    ${MOCKS})

gtest_discover_tests(${PORTBASED_TRANSPORTDESCRIPTOR_TESTS_TARGET})

#####################################
# AsioHelpers tests
#####################################
add_executable(AsioHelpersTests ${ASIOHELPERSTESTS_SOURCE})
target_compile_definitions(AsioHelpersTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(AsioHelpersTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/MessageReceiver
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/ReceiverResource
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
    )
target_link_libraries(AsioHelpersTests
    fastcdr
    fastdds::log
    GTest::gtest
    ${MOCKS}
    $<$<BOOL:${TLS_FOUND}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto>)
if(QNX)
    target_link_libraries(AsioHelpersTests socket)
endif()
if(MSVC OR MSVC_IDE)
    target_link_libraries(AsioHelpersTests ${PRIVACY} iphlpapi Shlwapi )
endif()
if (APPLE)
    target_link_libraries(AsioHelpersTests ${PRIVACY} "-framework CoreFoundation" "-framework IOKit")
endif()
gtest_discover_tests(AsioHelpersTests)
