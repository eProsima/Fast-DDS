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

set(COMMON_SOURCES_AUTH_PLUGIN_TEST_SOURCE
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/policy/ParameterList.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/EndpointSecurityAttributes.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/PropertyPolicy.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/ParticipantBuiltinTopicData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/ParticipantProxyData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/GuidPrefix_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/LocatorWithMask.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Token.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/exceptions/Exception.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/CDRMessage.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/network.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/ResourceEvent.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEvent.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEventImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/security/common/SharedSecretHandle.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/security/exceptions/SecurityException.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/security/logging/Logging.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/security/logging/LoggingLevel.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetmaskFilterKind.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterfaceWithFilter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/TimedConditionVariable.cpp
    )

add_executable(BuiltinPKIDH ${COMMON_SOURCES_AUTH_PLUGIN_TEST_SOURCE}
    ${PROJECT_SOURCE_DIR}/src/cpp/security/authentication/PKIDH.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/security/authentication/PKIIdentityHandle.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/security/authentication/PKIHandshakeHandle.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/security/artifact_providers/FileProvider.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/security/artifact_providers/Pkcs11Provider.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/Host.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BuiltinPKIDHTests.cpp)
target_compile_definitions(BuiltinPKIDH PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(BuiltinPKIDH PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/NetworkFactory/
    ${OPENSSL_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    )
target_link_libraries(BuiltinPKIDH
    GTest::gtest
    ${OPENSSL_LIBRARIES}
    fastcdr
    fastdds::log
    foonathan_memory
    $<$<BOOL:${WIN32}>:ws2_32>
    $<$<BOOL:${LibP11_FOUND}>:eProsima_p11>  # $<TARGET_NAME_IF_EXISTS:eProsima_p11>
    )
if(QNX)
    target_link_libraries(BuiltinPKIDH socket)
endif()
if (APPLE)
    target_link_libraries(BuiltinPKIDH "-framework CoreFoundation" "-framework IOKit")
endif()
gtest_discover_tests(BuiltinPKIDH
    PROPERTIES ENVIRONMENT "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs")
