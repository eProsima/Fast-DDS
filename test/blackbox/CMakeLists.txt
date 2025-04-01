# Copyright 2016, 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

option(RTPS_API_TESTS "Enable tests using RTPS API" ON)
option(FASTDDS_PIM_API_TESTS "Enable tests using FastDDS API" ON)

if(WIN32)
    add_definitions(
        -D_WIN32_WINNT=0x0601
        -D_CRT_SECURE_NO_WARNINGS
        )
endif()

###############################################################################
# Blackbox tests
###############################################################################

# OpenSSL on Windows requires hinting a config file on some specific tests.
# This tests require a very specific deployment (configuration setup & dependent libraries)
# that is only available on our CI, thus we enforced the use of EPROSIMA_BUILD to enable them.
if(OPENSSL_FOUND)
    # This feature is officially supported only for linux and windows OS.
    if(EPROSIMA_BUILD AND NOT APPLE)
        find_package(LibP11 REQUIRED)

        if(WIN32)
                get_filename_component(OPENSSL_DIR "${OPENSSL_INCLUDE_DIR}" DIRECTORY)
                set(OPENSSL_CONF "${OPENSSL_DIR}/bin/cnf/openssl.cnf")
                unset(OPENSSL_DIR)
        endif()
    else()
        find_package(LibP11)
    endif()
endif()

# Filter pksc11 related tests if library is not available
if(NOT LibP11_FOUND)
    set(pkcs_filter "*pkcs*")
endif() # LibP11_FOUND

string(JOIN ":" BLACKBOX_HIGH_LEVEL_IGNORED_TESTS ${pkcs_filter})
if(NOT BLACKBOX_HIGH_LEVEL_IGNORED_TESTS STREQUAL "")
	message(STATUS "Ignoring tests '${BLACKBOX_HIGH_LEVEL_IGNORED_TESTS}'")
    string(PREPEND BLACKBOX_HIGH_LEVEL_IGNORED_TESTS "-")
endif()
message(STATUS "BLACKBOX_HIGH_LEVEL_IGNORED_TESTS set to '${BLACKBOX_HIGH_LEVEL_IGNORED_TESTS}'")

file(GLOB RTPS_BLACKBOXTESTS_TEST_SOURCE "common/RTPSBlackboxTests*.cpp")
set(RTPS_BLACKBOXTESTS_SOURCE ${RTPS_BLACKBOXTESTS_TEST_SOURCE}
    types/Data1mbPubSubTypes.cxx
    types/Data1mbTypeObjectSupport.cxx
    types/Data100kbPubSubTypes.cxx
    types/Data100kbTypeObjectSupport.cxx
    types/Data64kbPubSubTypes.cxx
    types/Data64kbTypeObjectSupport.cxx
    types/FixedSizedPubSubTypes.cxx
    types/FixedSizedTypeObjectSupport.cxx
    types/HelloWorldPubSubTypes.cxx
    types/HelloWorldTypeObjectSupport.cxx
    types/KeyedData1mbPubSubTypes.cxx
    types/KeyedData1mbTypeObjectSupport.cxx
    types/KeyedHelloWorldPubSubTypes.cxx
    types/KeyedHelloWorldTypeObjectSupport.cxx
    types/core/core_typesPubSubTypes.cxx
    types/core/core_typesTypeObjectSupport.cxx
    types/StringTestPubSubTypes.cxx
    types/StringTestTypeObjectSupport.cxx
    types/UnboundedHelloWorldPubSubTypes.cxx
    types/UnboundedHelloWorldTypeObjectSupport.cxx

    utils/data_generators.cpp
    utils/lambda_functions.cpp
    utils/print_functions.cpp
    )
add_executable(BlackboxTests_RTPS ${RTPS_BLACKBOXTESTS_SOURCE})
target_compile_definitions(BlackboxTests_RTPS PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<NOT:$<BOOL:${IS_THIRDPARTY_BOOST_SUPPORTED}>>:FASTDDS_SHM_TRANSPORT_DISABLED> # Do not compile SHM Transport
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    $<$<BOOL:${WIN32}>:_ENABLE_ATOMIC_ALIGNMENT_FIX>
    )
target_include_directories(BlackboxTests_RTPS PRIVATE
    ${Asio_INCLUDE_DIR})
target_link_libraries(BlackboxTests_RTPS fastdds fastcdr foonathan_memory GTest::gtest)
gtest_discover_tests(BlackboxTests_RTPS
    TEST_PREFIX "BlackboxTests_RTPS."
    TEST_FILTER ${BLACKBOX_HIGH_LEVEL_IGNORED_TESTS}
    NO_PRETTY_VALUES
    )

file(GLOB BLACKBOXTESTS_TEST_SOURCE "common/BlackboxTests*.cpp")
set(BLACKBOXTESTS_SOURCE ${BLACKBOXTESTS_TEST_SOURCE}
    types/Data1mbPubSubTypes.cxx
    types/Data1mbTypeObjectSupport.cxx
    types/Data100kbPubSubTypes.cxx
    types/Data100kbTypeObjectSupport.cxx
    types/Data64kbPubSubTypes.cxx
    types/Data64kbTypeObjectSupport.cxx
    types/FixedSizedPubSubTypes.cxx
    types/FixedSizedTypeObjectSupport.cxx
    types/HelloWorldPubSubTypes.cxx
    types/HelloWorldTypeObjectSupport.cxx
    types/KeyedData1mbPubSubTypes.cxx
    types/KeyedData1mbTypeObjectSupport.cxx
    types/KeyedHelloWorldPubSubTypes.cxx
    types/KeyedHelloWorldTypeObjectSupport.cxx
    types/core/core_typesPubSubTypes.cxx
    types/core/core_typesTypeObjectSupport.cxx
    types/StringTestPubSubTypes.cxx
    types/StringTestTypeObjectSupport.cxx
    types/TestIncludeRegression3361TypeObjectSupport.cxx
    types/TestRegression3361PubSubTypes.cxx
    types/TestRegression3361TypeObjectSupport.cxx
    types/UnboundedHelloWorldPubSubTypes.cxx
    types/UnboundedHelloWorldTypeObjectSupport.cxx

    utils/data_generators.cpp
    utils/lambda_functions.cpp
    utils/print_functions.cpp

    common/DatagramInjectionTransport.cpp
    api/dds-pim/TCPReqRepHelloWorldRequester.cpp
    api/dds-pim/TCPReqRepHelloWorldReplier.cpp
    api/dds-pim/TCPReqRepHelloWorldService.cpp
    )

file(GLOB DDS_BLACKBOXTESTS_TEST_SOURCE "common/DDSBlackboxTests*.cpp")
set(DDS_BLACKBOXTESTS_SOURCE
    ${DDS_BLACKBOXTESTS_TEST_SOURCE}
    ${BLACKBOXTESTS_SOURCE}
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/CDRMessage.cpp
    )

# Prepare static discovery xml file for blackbox tests.
string(RANDOM LENGTH 4 ALPHABET 0123456789 TOPIC_RANDOM_NUMBER)
math(EXPR TOPIC_RANDOM_NUMBER "${TOPIC_RANDOM_NUMBER} + 0") # Remove extra leading 0s.
string(RANDOM LENGTH 4 ALPHABET 0123456789 W_UNICAST_PORT_RANDOM_NUMBER)
math(EXPR W_UNICAST_PORT_RANDOM_NUMBER "${W_UNICAST_PORT_RANDOM_NUMBER} + 0") # Remove extra leading 0s.
if(W_UNICAST_PORT_RANDOM_NUMBER LESS 1025)
    math(EXPR W_UNICAST_PORT_RANDOM_NUMBER "${W_UNICAST_PORT_RANDOM_NUMBER} + 1024") # Remove extra leading 0s.
endif()
math(EXPR R_UNICAST_PORT_RANDOM_NUMBER "${W_UNICAST_PORT_RANDOM_NUMBER} + 1")
math(EXPR MULTICAST_PORT_RANDOM_NUMBER "${R_UNICAST_PORT_RANDOM_NUMBER} + 1")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/PubSubWriter_static_disc.xml.in
    ${CMAKE_CURRENT_BINARY_DIR}/PubSubWriter_static_disc.xml)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/PubSubReader_static_disc.xml.in
    ${CMAKE_CURRENT_BINARY_DIR}/PubSubReader_static_disc.xml)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/PubSubWriterPersistence_static_disc.xml.in
    ${CMAKE_CURRENT_BINARY_DIR}/PubSubWriterPersistence_static_disc.xml)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/PubSubReaderPersistence_static_disc.xml.in
    ${CMAKE_CURRENT_BINARY_DIR}/PubSubReaderPersistence_static_disc.xml)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/discovery_participant_flags_profile.xml
    ${CMAKE_CURRENT_BINARY_DIR}/discovery_participant_flags_profile.xml)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/StatisticsDomainParticipant_profile.xml
    ${CMAKE_CURRENT_BINARY_DIR}/StatisticsDomainParticipant_profile.xml)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/MonitorServiceDomainParticipant_profile.xml
    ${CMAKE_CURRENT_BINARY_DIR}/MonitorServiceDomainParticipant_profile.xml)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/MonitorServiceConnectionList_profile.xml
    ${CMAKE_CURRENT_BINARY_DIR}/MonitorServiceConnectionList_profile.xml)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/utils/check_guid.py
    ${CMAKE_CURRENT_BINARY_DIR}/check_guid.py)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/partitions_profile.xml
    ${CMAKE_CURRENT_BINARY_DIR}/partitions_profile.xml)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/builtin_transports_profile.xml
    ${CMAKE_CURRENT_BINARY_DIR}/builtin_transports_profile.xml)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/auth_handshake_props_profile.xml
    ${CMAKE_CURRENT_BINARY_DIR}/auth_handshake_props_profile.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/discovery_participants_initial_peers_profile.xml
    ${CMAKE_CURRENT_BINARY_DIR}/discovery_participants_initial_peers_profile.xml)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/discovery_participants_client_server_profile.xml
    ${CMAKE_CURRENT_BINARY_DIR}/discovery_participants_client_server_profile.xml)
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/datagrams" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

if(FASTDDS_PIM_API_TESTS)
    set(BLACKBOXTESTS_FASTDDS_PIM_SOURCE
        ${DDS_BLACKBOXTESTS_SOURCE}
        api/dds-pim/ReqRepHelloWorldRequester.cpp
        api/dds-pim/ReqRepHelloWorldReplier.cpp
        api/dds-pim/ReqRepHelloWorldService.cpp
        )

    if (FASTDDS_STATISTICS)

        set(statistics_sources
            ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/monitorservice_typesPubSubTypes.cxx
            ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/monitorservice_typesTypeObjectSupport.cxx
            ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/typesPubSubTypes.cxx
            ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/typesTypeObjectSupport.cxx
            )

        list(APPEND BLACKBOXTESTS_FASTDDS_PIM_SOURCE ${statistics_sources})

    endif()

    add_executable(BlackboxTests_DDS_PIM ${BLACKBOXTESTS_FASTDDS_PIM_SOURCE})
    target_compile_definitions(BlackboxTests_DDS_PIM PRIVATE
        BOOST_ASIO_STANDALONE
        ASIO_STANDALONE
        $<$<NOT:$<BOOL:${IS_THIRDPARTY_BOOST_SUPPORTED}>>:FASTDDS_SHM_TRANSPORT_DISABLED> # Do not compile SHM Transport
        $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
        $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
        $<$<BOOL:${WIN32}>:_ENABLE_ATOMIC_ALIGNMENT_FIX>
        )
    target_include_directories(BlackboxTests_DDS_PIM PRIVATE
        ${Asio_INCLUDE_DIR}
        api/dds-pim)
    target_link_libraries(BlackboxTests_DDS_PIM
        fastdds
        fastcdr
        foonathan_memory
        GTest::gtest
        $<$<BOOL:${LibP11_FOUND}>:eProsima_p11>  # $<TARGET_NAME_IF_EXISTS:eProsima_p11>
        )
    gtest_discover_tests(BlackboxTests_DDS_PIM
        PROPERTIES
            ENVIRONMENT "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs"
            ENVIRONMENT "TOPIC_RANDOM_NUMBER=${TOPIC_RANDOM_NUMBER}"
            ENVIRONMENT "W_UNICAST_PORT_RANDOM_NUMBER=${W_UNICAST_PORT_RANDOM_NUMBER}"
            ENVIRONMENT "R_UNICAST_PORT_RANDOM_NUMBER=${R_UNICAST_PORT_RANDOM_NUMBER}"
            ENVIRONMENT "MULTICAST_PORT_RANDOM_NUMBER=${MULTICAST_PORT_RANDOM_NUMBER}"
            ENVIRONMENT $<IF:$<BOOL:${OPENSSL_CONF}>,OPENSSL_CONF=${OPENSSL_CONF},>
        TEST_PREFIX "BlackboxTests_DDS_PIM."
        TEST_FILTER ${BLACKBOX_HIGH_LEVEL_IGNORED_TESTS}
        NO_PRETTY_VALUES
        )
endif(FASTDDS_PIM_API_TESTS)

# Add 'xfail' label to flaky tests
set_property(DIRECTORY APPEND PROPERTY
    TEST_INCLUDE_FILES ${CMAKE_CURRENT_LIST_DIR}/xfail_tests.cmake
    )

# Necessary files
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/environment_file.json
    ${CMAKE_CURRENT_BINARY_DIR}/environment_file.json COPYONLY)

unset(pkcs_filter)
