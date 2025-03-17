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

set(NETWORKFACTORYTESTS_SOURCE
    NetworkFactoryTests.cpp
    mock/MockTransport.cpp
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
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPv6Transport.cpp
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
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/TimedConditionVariable.cpp
    )

if(TLS_FOUND)
    set(NETWORKFACTORYTESTS_SOURCE
        ${NETWORKFACTORYTESTS_SOURCE}
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPChannelResourceSecure.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPAcceptorSecure.cpp
        )
endif()

if(ANDROID)
    if (ANDROID_NATIVE_API_LEVEL LESS 24)
        list(APPEND NETWORKFACTORYTESTS_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
    endif()
endif()

include_directories(mock/)

add_executable(NetworkFactoryTests ${NETWORKFACTORYTESTS_SOURCE})
target_compile_definitions(NetworkFactoryTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(NetworkFactoryTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/ParticipantProxyData
    ${PROJECT_SOURCE_DIR}/test/mock/dds/QosPolicies
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/MessageReceiver
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/ReceiverResource
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
    )
target_link_libraries(NetworkFactoryTests
    fastcdr
    fastdds::log
    foonathan_memory
    GTest::gtest ${MOCKS}
    $<$<BOOL:${TLS_FOUND}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto>
    ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})

if(QNX)
    target_link_libraries(NetworkFactoryTests socket)
endif()
add_executable(ExternalLocatorsTests
    ExternalLocatorsTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/qos/WriterQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/qos/ReaderQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/xtypes/exception/Exception.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/ThreadSettings.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/ParticipantBuiltinTopicData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/PublicationBuiltinTopicData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/SubscriptionBuiltinTopicData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/LocatorWithMask.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/FlowControllerConsts.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/external_locators.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/network.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetmaskFilterKind.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterfaceWithFilter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/Host.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/StringMatching.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
    )
target_compile_definitions(ExternalLocatorsTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(ExternalLocatorsTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/test/mock/xtypes/TypeObjectRegistry
    ${PROJECT_SOURCE_DIR}/test/mock/xtypes/TypeObjectUtils
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/ParticipantProxyData
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/ReaderProxyData
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSDomainImpl # Dependency of fastdds::xtypes::type_representation
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/WriterProxyData
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    )
target_link_libraries(ExternalLocatorsTests
    fastcdr
    fastdds::log
    fastdds::xtypes::dynamic-types::impl
    GTest::gmock
    ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})

if(QNX)
    target_link_libraries(ExternalLocatorsTests socket)
endif()
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
    target_link_libraries(NetworkFactoryTests IPHLPAPI shlwapi) # Later so mocks have precedence
    target_link_libraries(ExternalLocatorsTests IPHLPAPI shlwapi) # Later so mocks have precedence
endif()
if (APPLE)
    target_link_libraries(NetworkFactoryTests "-framework CoreFoundation" "-framework IOKit")
    target_link_libraries(ExternalLocatorsTests "-framework CoreFoundation" "-framework IOKit")
endif()

gtest_discover_tests(NetworkFactoryTests)
gtest_discover_tests(ExternalLocatorsTests)
