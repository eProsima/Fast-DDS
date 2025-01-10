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

###########################################################################
# Create tests                                                            #
###########################################################################

find_package(Python3 COMPONENTS Interpreter REQUIRED)

if(Python3_Interpreter_FOUND)

    set(TESTS
        test_fast_discovery_closure
        test_fast_discovery_udpv6_address
        test_fast_discovery_parse_XML_file_default_profile
        test_fast_discovery_parse_XML_file_URI_profile
        test_fast_discovery_prefix_override
        test_fast_discovery_locator_address_override
        test_fast_discovery_locator_override_same_address
        test_fast_discovery_locator_port_override
        test_fast_discovery_locator_override_same_port
        test_fast_discovery_backup
        test_fast_discovery_no_XML
        test_fast_discovery_incorrect_participant
        test_fast_discovery_no_prefix
        test_fast_discovery_several_server_ids
        test_fast_discovery_invalid_locator
        test_fast_discovery_non_existent_profile
    )

    if(SECURITY)
        set(TESTS
            ${TESTS}
            test_fast_discovery_security_disabled
            test_fast_discovery_security_enabled_xml_prefix
            test_fast_discovery_security_enabled_cli_prefix
        )

        configure_file(${PROJECT_SOURCE_DIR}/test/certs/maincacert.pem
            ${CMAKE_CURRENT_BINARY_DIR}/maincacert.pem COPYONLY)
        configure_file(${PROJECT_SOURCE_DIR}/test/certs/mainsubcert.pem
            ${CMAKE_CURRENT_BINARY_DIR}/mainsubcert.pem COPYONLY)
        configure_file(${PROJECT_SOURCE_DIR}/test/certs/mainsubkey.pem
            ${CMAKE_CURRENT_BINARY_DIR}/mainsubkey.pem COPYONLY)
        configure_file(${PROJECT_SOURCE_DIR}/test/certs/governance_helloworld_all_enable.smime
            ${CMAKE_CURRENT_BINARY_DIR}/governance_helloworld_all_enable.smime COPYONLY)
        configure_file(${PROJECT_SOURCE_DIR}/test/certs/permissions_helloworld.smime
            ${CMAKE_CURRENT_BINARY_DIR}/permissions_helloworld.smime COPYONLY)

        configure_file("test_xml_secure_discovery_server_profile.xml" "test_xml_secure_discovery_server_profile.xml" COPYONLY)
    endif()


    # windows auxiliary script to fork test execution
    set(PWS_LAUNCHER
        ${CMAKE_CURRENT_SOURCE_DIR}/launcher.ps1
    )

    # Calculate environment
    set(TEST_ENVIRONMENT
        "PATH=$ENV{PATH};$<TARGET_FILE_DIR:fastcdr>;$<TARGET_FILE_DIR:fastdds>")

    if(WIN32)

        if(TARGET tinyxml2 OR TARGET tinyxml2::tinyxml2)
            set(TEST_ENVIRONMENT
                "${TEST_ENVIRONMENT};$<TARGET_FILE_DIR:${TINYXML2_LIBRARY}>")
        elseif(EXISTS TINYXML2_LIBRARY)
            get_filename_component(
                TINYXML2_LIBRARY_DIR ${TINYXML2_LIBRARY} DIRECTORY)
            set(TEST_ENVIRONMENT
                "${TEST_ENVIRONMENT};${TINYXML2_LIBRARY_DIR}")
            unset(TINYXML2_LIBRARY_DIR)
        endif()

        string(REPLACE ";" "\\;" TEST_ENVIRONMENT "${TEST_ENVIRONMENT}")

    endif(WIN32)

    # populate the tests
    foreach(TEST IN LISTS TESTS)

        # CTest has issues with signals on windows. We forked the test
        # execution using an auxiliary powershell script
        if(WIN32)
           add_test(
                    NAME system.tools.fastdds.${TEST}
                    COMMAND powershell "-File" ${PWS_LAUNCHER}
                            ${Python3_EXECUTABLE}
                            ${CMAKE_CURRENT_SOURCE_DIR}/tests.py
                            $<TARGET_FILE:fast-discovery-server>
                            ${TEST}
                )
        else()
            add_test(
                    NAME system.tools.fastdds.${TEST}
                    COMMAND ${Python3_EXECUTABLE}
                            ${CMAKE_CURRENT_SOURCE_DIR}/tests.py
                            $<TARGET_FILE:fast-discovery-server>
                            ${TEST}
                )
        endif()

        # Set test properties
        set_tests_properties(
            system.tools.fastdds.${TEST}
            PROPERTIES
                LABELS "NoMemoryCheck"
                ENVIRONMENT "${TEST_ENVIRONMENT}"
            )

    endforeach()

    configure_file("test_xml_discovery_server_profile.xml" "test_xml_discovery_server_profile.xml" COPYONLY)
    configure_file("test_wrong_xml_discovery_server_profile.xml" "test_wrong_xml_discovery_server_profile.xml" COPYONLY)
    unset(TEST_ENVIRONMENT)

endif()

# C++ Unittests
set(CLI_MANAGER_TESTS_EXEC CliDiscoveryManagerTests)

set(CLI_MANAGER_TESTS_SOURCE
    CliDiscoveryManagerTests.cpp
    ${PROJECT_SOURCE_DIR}/tools/fds/CliDiscoveryManager.cpp
    )

add_executable(${CLI_MANAGER_TESTS_EXEC} ${CLI_MANAGER_TESTS_SOURCE})

target_compile_definitions(${CLI_MANAGER_TESTS_EXEC}
    PRIVATE FAST_SERVER_VERSION=\"${PROJECT_VERSION}\"
    PRIVATE FAST_SERVER_BINARY=\"$<TARGET_FILE_NAME:${PROJECT_NAME}>\" )

target_include_directories( ${CLI_MANAGER_TESTS_EXEC} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/tools/fds
    )

target_link_libraries(
    ${CLI_MANAGER_TESTS_EXEC}
    fastdds
    fastcdr
    fastdds::optionparser
    GTest::gtest
    ${CMAKE_DL_LIBS})

gtest_discover_tests(${CLI_MANAGER_TESTS_EXEC})
