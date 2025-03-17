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

set(BUILTIN_DATA_SERIALIZATION_TESTS_SOURCE BuiltinDataSerializationTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/policy/ParameterList.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/qos/WriterQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/qos/ReaderQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/xtypes/exception/Exception.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/xtypes/type_representation/dds_xtypes_typeobjectPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/ThreadSettings.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/ParticipantBuiltinTopicData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/ParticipantProxyData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/PublicationBuiltinTopicData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/SubscriptionBuiltinTopicData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/ReaderProxyData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/WriterProxyData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/GuidPrefix_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/LocatorWithMask.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/FlowControllerConsts.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/CDRMessage.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/network.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/ResourceEvent.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEvent.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEventImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetmaskFilterKind.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterfaceWithFilter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/Host.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/TimedConditionVariable.cpp
    )

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

if(ANDROID)
    if (ANDROID_NATIVE_API_LEVEL LESS 24)
        list(APPEND BUILTIN_DATA_SERIALIZATION_TESTS_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
    endif()
endif()


add_executable(BuiltinDataSerializationTests ${BUILTIN_DATA_SERIALIZATION_TESTS_SOURCE})
target_compile_definitions(BuiltinDataSerializationTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(BuiltinDataSerializationTests PRIVATE
    ${PROJECT_SOURCE_DIR}/test/mock/xtypes/TypeObjectRegistry
    ${PROJECT_SOURCE_DIR}/test/mock/xtypes/TypeObjectUtils
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/NetworkFactory
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSDomainImpl
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${Asio_INCLUDE_DIR}
    $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
    )
target_link_libraries(BuiltinDataSerializationTests
    fastdds::log
    foonathan_memory
    GTest::gmock
    ${CMAKE_DL_LIBS})
if(QNX)
    target_link_libraries(BuiltinDataSerializationTests socket)
endif()
if(MSVC OR MSVC_IDE)
    target_link_libraries(BuiltinDataSerializationTests ${PRIVACY} fastcdr iphlpapi Shlwapi ws2_32)
else()
    target_link_libraries(BuiltinDataSerializationTests ${PRIVACY} fastcdr)
endif()

if (APPLE)
    target_link_libraries(BuiltinDataSerializationTests ${PRIVACY} "-framework CoreFoundation" "-framework IOKit")
endif()

gtest_discover_tests(BuiltinDataSerializationTests)
