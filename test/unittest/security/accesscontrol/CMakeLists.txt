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

set(COMMON_SOURCES_ACCESS_CONTROL_TEST_SOURCE
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/policy/ParameterList.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/EndpointSecurityAttributes.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/PropertyPolicy.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/PublicationBuiltinTopicData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/SubscriptionBuiltinTopicData.cpp
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

if(TINYXML2_SOURCE_DIR)
    list(APPEND COMMON_SOURCES_ACCESS_CONTROL_TEST_SOURCE
        ${TINYXML2_SOURCE_DIR}/tinyxml2.cpp
        )
endif()

# Only include tinyxml2 headers manually if the associated config predates the use of CMake targets
if(TINYXML2_INCLUDE_DIR)
    include_directories(${TINYXML2_INCLUDE_DIR})
endif(TINYXML2_INCLUDE_DIR)

add_executable(AccessControlTests ${COMMON_SOURCES_ACCESS_CONTROL_TEST_SOURCE}
    ${PROJECT_SOURCE_DIR}/src/cpp/security/authentication/PKIDH.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/security/authentication/PKIIdentityHandle.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/security/authentication/PKIHandshakeHandle.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/security/accesscontrol/AccessPermissionsHandle.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/security/accesscontrol/CommonParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/security/accesscontrol/GovernanceParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/security/accesscontrol/Permissions.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/security/accesscontrol/DistinguishedName.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/security/accesscontrol/PermissionsParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/security/artifact_providers/FileProvider.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/security/artifact_providers/Pkcs11Provider.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/StringMatching.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/qos/WriterQos.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AccessControlTests.cpp)

target_compile_definitions(AccessControlTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )

target_include_directories(AccessControlTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/NetworkFactory/
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/PDPSimple
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/EDP
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/ParticipantProxyData
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/WriterProxyData/
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/ReaderProxyData/
    ${PROJECT_SOURCE_DIR}/test/mock/dds/QosPolicies
    ${OPENSSL_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    )

target_link_libraries(AccessControlTests
    GTest::gmock
    ${OPENSSL_LIBRARIES}
    fastcdr
    fastdds::log
    foonathan_memory
    $<$<BOOL:${WIN32}>:ws2_32>
    ${TINYXML2_LIBRARY}
    $<$<BOOL:${LibP11_FOUND}>:eProsima_p11>  # $<TARGET_NAME_IF_EXISTS:eProsima_p11>
    )
if(QNX)
    target_link_libraries(AccessControlTests socket)
endif()
if(MSVC OR MSVC_IDE)
    target_link_libraries(AccessControlTests ${PRIVACY} iphlpapi Shlwapi)
endif()


gtest_discover_tests(AccessControlTests
    PROPERTIES ENVIRONMENT "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs")

####################################################################################################
####################################################################################################
# DistinguishedNameTests
set(
    DISTINGUISHEDNAME_TEST_NAME
    "DistinguishedNameTests"
)

add_executable(
    ${DISTINGUISHEDNAME_TEST_NAME}

    # src files
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/LocatorWithMask.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/network.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetmaskFilterKind.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterfaceWithFilter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/security/accesscontrol/DistinguishedName.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    # Log related files
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp

    # test files
    ${CMAKE_CURRENT_SOURCE_DIR}/DistinguishedNameTests.cpp
)

target_compile_definitions(
    ${DISTINGUISHEDNAME_TEST_NAME}
    PRIVATE
        BOOST_ASIO_STANDALONE
        ASIO_STANDALONE
        $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
        $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
)

target_include_directories(
    ${DISTINGUISHEDNAME_TEST_NAME}
    PRIVATE
        ${Asio_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_BINARY_DIR}/include
        ${PROJECT_SOURCE_DIR}/src/cpp
)

target_link_libraries(${DISTINGUISHEDNAME_TEST_NAME}
    fastcdr
    fastdds::log
    GTest::gtest
)

gtest_discover_tests( ${DISTINGUISHEDNAME_TEST_NAME})
