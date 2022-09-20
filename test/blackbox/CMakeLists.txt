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
macro(add_blackbox_gtest)
    # Parse arguments
    if("${ARGV0}" STREQUAL "NAME")
        set(uniValueArgs NAME COMMAND)
        unset(test)
        unset(command)
    else()
        set(test "${ARGV0}")
        set(command "${test}")
    endif()
    set(multiValueArgs SOURCES ENVIRONMENTS DEPENDENCIES LABELS IGNORE)
    cmake_parse_arguments(GTEST "" "${uniValueArgs}" "${multiValueArgs}" ${ARGN})

    if(GTEST_NAME)
        set(test ${GTEST_NAME})
        set(command ${GTEST_COMMAND})
    endif()

    # IGNORE keeps a filter expression for the test list:
    #   +   if GTEST_INDIVIDUAL is enforced the expressions are regular expression and the matching tests would be disabled
    #       using cmake add_test DISABLE property
    #   +   if no GTEST_INDIVIDUAL is enforce the filtering will be added to gtest command via --gtest_filter and it's
    #       own filtering syntax

    if(GTEST_INDIVIDUAL)
        if(WIN32)
            set(WIN_PATH "$ENV{PATH}")
            get_target_property(LINK_LIBRARIES_ ${command} LINK_LIBRARIES)
            if(NOT "${LINK_LIBRARIES_}" STREQUAL "LINK_LIBRARIES_-NOTFOUND")
                foreach(LIBRARY_LINKED ${LINK_LIBRARIES_})
                    if(TARGET ${LIBRARY_LINKED})
                        set(WIN_PATH "$<TARGET_FILE_DIR:${LIBRARY_LINKED}>;${WIN_PATH}")
                    endif()
                endforeach()
            endif()
            foreach(DEP ${GTEST_DEPENDENCIES})
                set(WIN_PATH "$<TARGET_FILE_DIR:${DEP}>;${WIN_PATH}")
            endforeach()
            string(REPLACE ";" "\\;" WIN_PATH "${WIN_PATH}")
        endif()

        foreach(GTEST_SOURCE_FILE ${GTEST_SOURCES})
            file(STRINGS ${GTEST_SOURCE_FILE} GTEST_TEST_NAMES REGEX "^TEST(\\(|_F)" )
            foreach(GTEST_TEST_NAME ${GTEST_TEST_NAMES})
                string(REGEX REPLACE ["\) \(,"] ";" GTEST_TEST_NAME ${GTEST_TEST_NAME})
                list(GET GTEST_TEST_NAME 1 GTEST_GROUP_NAME)
                list(GET GTEST_TEST_NAME 3 GTEST_TEST_NAME)
                add_test(NAME ${test}.${GTEST_GROUP_NAME}.${GTEST_TEST_NAME}
                    COMMAND ${command} --gtest_filter=${GTEST_GROUP_NAME}.${GTEST_TEST_NAME})

                # decide if disable
                unset(GTEST_USER_DISABLED)
                foreach(GTEST_USER_FILTER ${GTEST_IGNORE})
                    string(REGEX MATCH ${GTEST_USER_FILTER} GTEST_USER_DISABLED ${GTEST_TEST_NAME})
                    if(GTEST_USER_DISABLED)
                        break()
                    endif()
                endforeach()

                # Add environment
                if(WIN32)
                    set_property(TEST ${test}.${GTEST_GROUP_NAME}.${GTEST_TEST_NAME} APPEND PROPERTY ENVIRONMENT "PATH=${WIN_PATH}")
                endif()

                foreach(property ${GTEST_ENVIRONMENTS})
                    set_property(TEST ${test}.${GTEST_GROUP_NAME}.${GTEST_TEST_NAME} APPEND PROPERTY ENVIRONMENT "${property}")
                endforeach()

                # Add labels and enable
                set_tests_properties(${test}.${GTEST_GROUP_NAME}.${GTEST_TEST_NAME} PROPERTIES
                    LABELS "${GTEST_LABELS}"
                    DISABLED $<BOOL:${GTEST_USER_DISABLED}>)

            endforeach()
            file(STRINGS ${GTEST_SOURCE_FILE} GTEST_TEST_NAMES REGEX "^TEST_P" )
            foreach(GTEST_TEST_NAME ${GTEST_TEST_NAMES})
                string(REGEX REPLACE ["\) \(,"] ";" GTEST_TEST_NAME ${GTEST_TEST_NAME})
                list(GET GTEST_TEST_NAME 1 GTEST_GROUP_NAME)
                list(GET GTEST_TEST_NAME 3 GTEST_TEST_NAME)

                add_test(NAME ${test}.${GTEST_GROUP_NAME}.${GTEST_TEST_NAME}.Transport
                    COMMAND ${command} --gtest_filter=*/${GTEST_GROUP_NAME}.${GTEST_TEST_NAME}/Transport*)

                # decide if disable
                unset(GTEST_USER_DISABLED)
                foreach(GTEST_USER_FILTER ${GTEST_IGNORE})
                    string(REGEX MATCH ${GTEST_USER_FILTER} GTEST_USER_DISABLED ${GTEST_TEST_NAME})
                    if(GTEST_USER_DISABLED)
                        break()
                    endif()
                endforeach()

                # Add environment
                if(WIN32)
                    set_property(TEST ${test}.${GTEST_GROUP_NAME}.${GTEST_TEST_NAME}.Transport APPEND PROPERTY ENVIRONMENT "PATH=${WIN_PATH}")
                endif()

                foreach(property ${GTEST_ENVIRONMENTS})
                    set_property(TEST ${test}.${GTEST_GROUP_NAME}.${GTEST_TEST_NAME}.Transport APPEND PROPERTY ENVIRONMENT "${property}")
                endforeach()

                # Add labels and enable
                set_tests_properties(${test}.${GTEST_GROUP_NAME}.${GTEST_TEST_NAME}.Transport PROPERTIES
                    LABELS "${GTEST_LABELS}"
                    DISABLED $<BOOL:${GTEST_USER_DISABLED}>)

                add_test(NAME ${test}.${GTEST_GROUP_NAME}.${GTEST_TEST_NAME}.Intraprocess
                    COMMAND ${command} --gtest_filter=*/${GTEST_GROUP_NAME}.${GTEST_TEST_NAME}/Intraprocess*)

                # Add environment
                if(WIN32)
                    set_property(TEST ${test}.${GTEST_GROUP_NAME}.${GTEST_TEST_NAME}.Intraprocess APPEND PROPERTY ENVIRONMENT "PATH=${WIN_PATH}")
                endif()

                foreach(property ${GTEST_ENVIRONMENTS})
                    set_property(TEST ${test}.${GTEST_GROUP_NAME}.${GTEST_TEST_NAME}.Intraprocess APPEND PROPERTY ENVIRONMENT "${property}")
                endforeach()

                # Add labels and enable
                set_tests_properties(${test}.${GTEST_GROUP_NAME}.${GTEST_TEST_NAME}.Intraprocess PROPERTIES
                    LABELS "${GTEST_LABELS}"
                    DISABLED $<BOOL:${GTEST_USER_DISABLED}>)

                if(${test} MATCHES ".*_DDS_PIM$")
                    add_test(NAME ${test}.${GTEST_GROUP_NAME}.${GTEST_TEST_NAME}.Datasharing
                        COMMAND ${command} --gtest_filter=*/${GTEST_GROUP_NAME}.${GTEST_TEST_NAME}/Datasharing*)

                    # Add environment
                    if(WIN32)
                        set_property(TEST ${test}.${GTEST_GROUP_NAME}.${GTEST_TEST_NAME}.Datasharing APPEND PROPERTY ENVIRONMENT "PATH=${WIN_PATH}")
                    endif()

                    foreach(property ${GTEST_ENVIRONMENTS})
                        set_property(TEST ${test}.${GTEST_GROUP_NAME}.${GTEST_TEST_NAME}.Datasharing APPEND PROPERTY ENVIRONMENT "${property}")
                    endforeach()

                    # Add labels and enable
                    set_tests_properties(${test}.${GTEST_GROUP_NAME}.${GTEST_TEST_NAME}.Datasharing PROPERTIES
                        LABELS "${GTEST_LABELS}"
                        DISABLED $<BOOL:${GTEST_USER_DISABLED}>)

                endif()

            endforeach()
        endforeach()
    else()

        # add filtering statement if required
        if(GTEST_IGNORE)
            set(gtest_filter "--gtest_filter=${GTEST_IGNORE}")
        endif()

        add_test(NAME ${test} COMMAND ${command} ${gtest_filter})

        # Add environment
        if(WIN32)
            set(WIN_PATH "$ENV{PATH}")
            get_target_property(LINK_LIBRARIES_ ${test} LINK_LIBRARIES)
            if(NOT "${LINK_LIBRARIES_}" STREQUAL "LINK_LIBRARIES_-NOTFOUND")
                foreach(LIBRARY_LINKED ${LINK_LIBRARIES_})
                    if(TARGET ${LIBRARY_LINKED})
                        set(WIN_PATH "$<TARGET_FILE_DIR:${LIBRARY_LINKED}>;${WIN_PATH}")
                    endif()
                endforeach()
            endif()
            foreach(DEP ${GTEST_DEPENDENCIES})
                set(WIN_PATH "$<TARGET_FILE_DIR:${DEP}>;${WIN_PATH}")
            endforeach()
            string(REPLACE ";" "\\;" WIN_PATH "${WIN_PATH}")
            set_property(TEST ${test} APPEND PROPERTY ENVIRONMENT "PATH=${WIN_PATH}")
        endif()

        foreach(property ${GTEST_ENVIRONMENTS})
            set_property(TEST ${test} APPEND PROPERTY ENVIRONMENT "${property}")
        endforeach()

        # Add labels
        set_property(TEST ${test} PROPERTY LABELS "${GTEST_LABELS}")

        unset(gtest_filter)
    endif()
endmacro()

option(RTPS_API_TESTS "Enable tests using RTPS API" ON)
option(FASTRTPS_API_TESTS "Enable tests using FastRTPS API" OFF)
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
    if(GTEST_INDIVIDUAL)
        set(pkcs_filter "[Pp][Kk][Cc][Ss]")
    else()
        set(pkcs_filter "-*pkcs*")
    endif() # GTEST_INDIVIDUAL
endif() # LibP11_FOUND

file(GLOB RTPS_BLACKBOXTESTS_TEST_SOURCE "common/RTPSBlackboxTests*.cpp")
set(RTPS_BLACKBOXTESTS_SOURCE ${RTPS_BLACKBOXTESTS_TEST_SOURCE}
    types/HelloWorld.cxx
    types/HelloWorldPubSubTypes.cxx
    types/KeyedHelloWorld.cxx
    types/KeyedHelloWorldPubSubTypes.cxx
    types/StringTest.cxx
    types/StringTestPubSubTypes.cxx
    types/Data64kb.cxx
    types/Data64kbPubSubTypes.cxx
    types/Data1mb.cxx
    types/Data1mbPubSubTypes.cxx
    types/KeyedData1mb.cxx
    types/KeyedData1mbPubSubTypes.cxx
    types/FixedSized.cxx
    types/FixedSizedPubSubTypes.cxx

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
    )
target_include_directories(BlackboxTests_RTPS PRIVATE
    ${Asio_INCLUDE_DIR})
target_link_libraries(BlackboxTests_RTPS fastrtps fastcdr foonathan_memory GTest::gtest)
add_blackbox_gtest(BlackboxTests_RTPS SOURCES ${RTPS_BLACKBOXTESTS_TEST_SOURCE} IGNORE ${pkcs_filter})

file(GLOB BLACKBOXTESTS_TEST_SOURCE "common/BlackboxTests*.cpp")
set(BLACKBOXTESTS_SOURCE ${BLACKBOXTESTS_TEST_SOURCE}
    types/HelloWorld.cxx
    types/HelloWorldPubSubTypes.cxx
    types/HelloWorldTypeObject.cxx
    types/KeyedHelloWorld.cxx
    types/KeyedHelloWorldPubSubTypes.cxx
    types/StringTest.cxx
    types/StringTestPubSubTypes.cxx
    types/Data64kb.cxx
    types/Data64kbPubSubTypes.cxx
    types/Data1mb.cxx
    types/Data1mbPubSubTypes.cxx
    types/KeyedData1mb.cxx
    types/KeyedData1mbPubSubTypes.cxx
    types/FixedSized.cxx
    types/FixedSizedPubSubTypes.cxx

    utils/data_generators.cpp
    utils/lambda_functions.cpp
    utils/print_functions.cpp

    common/TCPReqRepHelloWorldRequester.cpp
    common/TCPReqRepHelloWorldReplier.cpp
    )

file(GLOB DDS_BLACKBOXTESTS_TEST_SOURCE "common/DDSBlackboxTests*.cpp")
set(DDS_BLACKBOXTESTS_SOURCE
    ${DDS_BLACKBOXTESTS_TEST_SOURCE}
    ${BLACKBOXTESTS_SOURCE}
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/discovery/participant/ServerAttributes.cpp
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
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/PubSubWriter.xml.in
    ${CMAKE_CURRENT_BINARY_DIR}/PubSubWriter.xml)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/PubSubReader.xml.in
    ${CMAKE_CURRENT_BINARY_DIR}/PubSubReader.xml)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/PubSubWriterPersistence.xml.in
    ${CMAKE_CURRENT_BINARY_DIR}/PubSubWriterPersistence.xml)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/PubSubReaderPersistence.xml.in
    ${CMAKE_CURRENT_BINARY_DIR}/PubSubReaderPersistence.xml)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/discovery_participant_flags.xml
    ${CMAKE_CURRENT_BINARY_DIR}/discovery_participant_flags.xml)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/persistence.xml
    ${CMAKE_CURRENT_BINARY_DIR}/persistence.xml)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/StatisticsDomainParticipant.xml
    ${CMAKE_CURRENT_BINARY_DIR}/StatisticsDomainParticipant.xml)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/utils/check_guid.py
    ${CMAKE_CURRENT_BINARY_DIR}/check_guid.py)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/partitions.xml
    ${CMAKE_CURRENT_BINARY_DIR}/partitions.xml)

if(FASTRTPS_API_TESTS)
    set(BLACKBOXTESTS_FASTRTPS_SOURCE
        ${BLACKBOXTESTS_SOURCE}
        api/fastrtps_deprecated/ReqRepHelloWorldRequester.cpp
        api/fastrtps_deprecated/ReqRepHelloWorldReplier.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/discovery/participant/ServerAttributes.cpp
        )

    add_executable(BlackboxTests_FastRTPS ${BLACKBOXTESTS_FASTRTPS_SOURCE})
    target_compile_definitions(BlackboxTests_FastRTPS PRIVATE
        BOOST_ASIO_STANDALONE
        ASIO_STANDALONE
        $<$<NOT:$<BOOL:${IS_THIRDPARTY_BOOST_SUPPORTED}>>:FASTDDS_SHM_TRANSPORT_DISABLED> # Do not compile SHM Transport
        $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
        $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
        )
    target_include_directories(BlackboxTests_FastRTPS PRIVATE
        ${Asio_INCLUDE_DIR}
        api/fastrtps_deprecated)
    target_link_libraries(BlackboxTests_FastRTPS
        fastrtps
        fastcdr
        foonathan_memory
        GTest::gtest
        $<$<BOOL:${LibP11_FOUND}>:eProsima_p11>  # $<TARGET_NAME_IF_EXISTS:eProsima_p11>
        )
       
    add_blackbox_gtest(BlackboxTests_FastRTPS SOURCES ${BLACKBOXTESTS_TEST_SOURCE}
        ENVIRONMENTS "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs"
        "TOPIC_RANDOM_NUMBER=${TOPIC_RANDOM_NUMBER}"
        "W_UNICAST_PORT_RANDOM_NUMBER=${W_UNICAST_PORT_RANDOM_NUMBER}"
        "R_UNICAST_PORT_RANDOM_NUMBER=${R_UNICAST_PORT_RANDOM_NUMBER}"
        "MULTICAST_PORT_RANDOM_NUMBER=${MULTICAST_PORT_RANDOM_NUMBER}"
        $<$<BOOL:${OPENSSL_CONF}>:OPENSSL_CONF=${OPENSSL_CONF}>
        IGNORE ${pkcs_filter}
        )
endif(FASTRTPS_API_TESTS)

if(FASTDDS_PIM_API_TESTS)
    set(BLACKBOXTESTS_FASTDDS_PIM_SOURCE
        ${DDS_BLACKBOXTESTS_SOURCE}
        api/dds-pim/ReqRepHelloWorldRequester.cpp
        api/dds-pim/ReqRepHelloWorldReplier.cpp
        )

    if (FASTDDS_STATISTICS)

        set(statistics_sources
            ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/types.cxx
            ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/typesPubSubTypes.cxx
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
        )
    target_include_directories(BlackboxTests_DDS_PIM PRIVATE
        ${Asio_INCLUDE_DIR}
        api/dds-pim)
    target_link_libraries(BlackboxTests_DDS_PIM
        fastrtps
        fastcdr
        foonathan_memory
        GTest::gtest
        $<$<BOOL:${LibP11_FOUND}>:eProsima_p11>  # $<TARGET_NAME_IF_EXISTS:eProsima_p11>
        )
    add_blackbox_gtest(BlackboxTests_DDS_PIM SOURCES ${DDS_BLACKBOXTESTS_SOURCE}
        ENVIRONMENTS "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs"
        "TOPIC_RANDOM_NUMBER=${TOPIC_RANDOM_NUMBER}"
        "W_UNICAST_PORT_RANDOM_NUMBER=${W_UNICAST_PORT_RANDOM_NUMBER}"
        "R_UNICAST_PORT_RANDOM_NUMBER=${R_UNICAST_PORT_RANDOM_NUMBER}"
        "MULTICAST_PORT_RANDOM_NUMBER=${MULTICAST_PORT_RANDOM_NUMBER}"
        $<$<BOOL:${OPENSSL_CONF}>:OPENSSL_CONF=${OPENSSL_CONF}>
        IGNORE ${pkcs_filter}
        )
endif(FASTDDS_PIM_API_TESTS)

# Add 'xfail' label to flaky tests
set(BLACKBOX_XFAIL_LIST XFAIL_RTPS)
if(FASTRTPS_API_TESTS)
    set(BLACKBOX_XFAIL_LIST ${BLACKBOX_XFAIL_LIST} XFAIL_FASTRTPS)
endif()
if(FASTDDS_PIM_API_TESTS)
    set(BLACKBOX_XFAIL_LIST ${BLACKBOX_XFAIL_LIST} XFAIL_DDS_PIM)
endif()

foreach(BLACKBOX_XFAIL_TEST ${BLACKBOX_XFAIL_LIST})
    add_xfail_label(${CMAKE_CURRENT_SOURCE_DIR}/${BLACKBOX_XFAIL_TEST}.list)
    if(SECURITY)
        add_xfail_label(${CMAKE_CURRENT_SOURCE_DIR}/${BLACKBOX_XFAIL_TEST}_SECURITY.list)
    endif()
endforeach()

# Necessary files
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/environment_file.json
    ${CMAKE_CURRENT_BINARY_DIR}/environment_file.json COPYONLY)
