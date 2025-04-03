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

    # Set environment for tests
    set(TEST_ENVIRONMENT
        "PYTHON_VERSION=${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}")

    set(TESTS
        test_fastdds_installed
        test_fastdds_version
        test_fastdds_discovery
        test_fastdds_discovery_run
        test_fastdds_discovery_help
        test_fastdds_discovery_examples
        test_ros_discovery
        test_fastdds_shm
        test_fastdds_shm_force
        test_fastdds_xml_validate
    )

    foreach(TEST IN LISTS TESTS)
        add_test(
                NAME system.tools.fastdds.${TEST}
                COMMAND ${Python3_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/tests.py
                ${CMAKE_INSTALL_PREFIX}
                ${TEST}
            )

        # Set test properties
        set_property(
            TEST system.tools.fastdds.${TEST}
            PROPERTY LABELS "NoMemoryCheck"
        )

        # Set environment
        set_property(
            TEST system.tools.fastdds.${TEST}
            PROPERTY ENVIRONMENT ${TEST_ENVIRONMENT}
        )

    endforeach()

    if (INSTALL_TOOLS AND NOT WIN32)
        set(DISCOVERY_PARSER_TESTS
            TestDiscoveryParser.test_parser_shutdown_when_on
            TestDiscoveryParser.test_parser_shutdown_when_off
            TestDiscoveryParser.test_parser_auto_fails_ip
            TestDiscoveryParser.test_parser_auto_fails_domain
            TestDiscoveryParser.test_parser_auto_fails_extra_arg
            TestDiscoveryParser.test_parser_auto
            TestDiscoveryParser.test_parser_auto_reverse
            TestDiscoveryParser.test_parser_auto_domain_env
            TestDiscoveryParser.test_parser_start_fails_ip
            TestDiscoveryParser.test_parser_start_fails_domain
            TestDiscoveryParser.test_parser_start_fails_extra_arg
            TestDiscoveryParser.test_parser_start
            TestDiscoveryParser.test_parser_start_domain_env
            TestDiscoveryParser.test_parser_stop_when_off
            TestDiscoveryParser.test_parser_stop_when_on
            TestDiscoveryParser.test_parser_stop_whith_unknown_args
            TestDiscoveryParser.test_parser_list_when_off
            TestDiscoveryParser.test_parser_list_when_on
            TestDiscoveryParser.test_parser_add_fails_domain
            TestDiscoveryParser.test_parser_add_fails_ip
            TestDiscoveryParser.test_parser_add_when_off
            TestDiscoveryParser.test_parser_add_when_on
            TestDiscoveryParser.test_parser_set_fails_domain
            TestDiscoveryParser.test_parser_set_fails_ip
            TestDiscoveryParser.test_parser_set_when_off
            TestDiscoveryParser.test_parser_set_when_on
        )

        foreach(TEST ${DISCOVERY_PARSER_TESTS})
            add_test(
                NAME system.tools.fastdds.${TEST}
                COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_discovery_parser.py ${TEST}
                )
            # Set environment
            set(TEST_ENVIRONMENT
                "PYTHON_VERSION=${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}"
                "TOOL_PATH=${CMAKE_INSTALL_PREFIX}"
            )
            set_property(
                TEST system.tools.fastdds.${TEST}
                PROPERTY ENVIRONMENT ${TEST_ENVIRONMENT}
            )
        endforeach()
    endif()

    ###############################################################################
    # XML GENERAL validation
    ###############################################################################
    message(STATUS "Configuring CLI xml validation...")

    # Configure command based on the OS running
    if (MSVC)
        set(env_fast_command "fastdds.bat") # WINDOWS
    else()
        set(env_fast_command "fastdds")     # POSIX
    endif()

    # Copy the examples validation files over to the build directory
    file(GLOB_RECURSE XML_EXAMPLE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../../../examples/*_profile.xml)
    # for each xml file detected
    foreach(XML_EXAMPLE_FILE_COMPLETE_PATH ${XML_EXAMPLE_FILES})
        # obtain the file name
        get_filename_component(XML_EXAMPLE_FILE ${XML_EXAMPLE_FILE_COMPLETE_PATH} NAME_WE)
        # copy the file from src to build folders
        configure_file(
            ${XML_EXAMPLE_FILE_COMPLETE_PATH}                                          # from full src path
            ${CMAKE_CURRENT_BINARY_DIR}/xmldocuments/examples/${XML_EXAMPLE_FILE}.xml  # to relative build path
            COPYONLY)
    endforeach()

    # Copy the blackbox tests validation files over to the build directory
    file(GLOB_RECURSE XML_BLACKBOX_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../../blackbox/*_profile.xml)
    # for each xml file detected
    foreach(XML_BLACKBOX_FILE_COMPLETE_PATH ${XML_BLACKBOX_FILES})
        # obtain the file name
        get_filename_component(XML_BLACKBOX_FILE ${XML_BLACKBOX_FILE_COMPLETE_PATH} NAME_WE)
        # copy the file from src to build folders
        configure_file(
            ${XML_BLACKBOX_FILE_COMPLETE_PATH}                                               # from full src path
            ${CMAKE_CURRENT_BINARY_DIR}/xmldocuments/test/blackbox/${XML_BLACKBOX_FILE}.xml  # to relative build path
            COPYONLY)
    endforeach()

    # Copy the unit tests validation files over to the build directory
    file(GLOB_RECURSE XML_UNITTEST_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../../unittest/*_profile.xml)
    # for each xml file detected
    foreach(XML_UNITTEST_FILE_COMPLETE_PATH ${XML_UNITTEST_FILES})
        # obtain the file name
        get_filename_component(XML_UNITTEST_FILE ${XML_UNITTEST_FILE_COMPLETE_PATH} NAME_WE)
        # copy the file from src to build folders
        configure_file(
            ${XML_UNITTEST_FILE_COMPLETE_PATH}                                              # from full src path
            ${CMAKE_CURRENT_BINARY_DIR}/xmldocuments/test/unittest/${XML_UNITTEST_FILE}.xml # to relative build path
            COPYONLY)
    endforeach()

    # Copy the system tests validation files over to the build directory
    file(GLOB_RECURSE XML_SYSTEM_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../*_profile.xml)
    # for each xml file detected
    foreach(XML_SYSTEM_FILE_COMPLETE_PATH ${XML_SYSTEM_FILES})
        # obtain the file name
        get_filename_component(XML_SYSTEM_FILE ${XML_SYSTEM_FILE_COMPLETE_PATH} NAME_WE)
        # copy the file from src to build folders
        configure_file(
            ${XML_SYSTEM_FILE_COMPLETE_PATH}                                            # from full src path
            ${CMAKE_CURRENT_BINARY_DIR}/xmldocuments/test/system/${XML_SYSTEM_FILE}.xml # to relative build path
            COPYONLY)
    endforeach()

    # Copy the comunication tests validation files over to the build directory
    file(GLOB_RECURSE XML_COMMUNICATION_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../../communication/*_profile.xml)
    # for each xml file detected
    foreach(XML_COMMUNICATION_FILE_COMPLETE_PATH ${XML_COMMUNICATION_FILES})
        # obtain the file name
        get_filename_component(XML_COMMUNICATION_FILE ${XML_COMMUNICATION_FILE_COMPLETE_PATH} NAME_WE)
        # copy the file from src to build folders
        configure_file(
            ${XML_COMMUNICATION_FILE_COMPLETE_PATH}                                                   # from full src path
            ${CMAKE_CURRENT_BINARY_DIR}/xmldocuments/test/communication/${XML_COMMUNICATION_FILE}.xml # to relative build path
            COPYONLY)
    endforeach()

    # Copy the dds tests validation files over to the build directory
    file(GLOB_RECURSE XML_DDS_TEST_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../../dds/*_profile.xml)
    # for each xml file detected
    foreach(XML_DDS_TEST_FILE_COMPLETE_PATH ${XML_DDS_TEST_FILES})
        # obtain the file name
        get_filename_component(XML_DDS_TEST_FILE ${XML_DDS_TEST_FILE_COMPLETE_PATH} NAME_WE)
        # copy the file from src to build folders
        configure_file(
            ${XML_DDS_TEST_FILE_COMPLETE_PATH}                                         # from full src path
            ${CMAKE_CURRENT_BINARY_DIR}/xmldocuments/test/dds/${XML_DDS_TEST_FILE}.xml # to relative build path
            COPYONLY)
    endforeach()

    if(PERFORMANCE_TESTS)
        # Copy the performance tests validation files over to the build directory
        file(GLOB_RECURSE XML_PERFORMANCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../../performance/*_profile.xml)
        # for each xml file detected
        foreach(XML_PERFORMANCE_FILE_COMPLETE_PATH ${XML_PERFORMANCE_FILES})
            # obtain the file name
            get_filename_component(XML_PERFORMANCE_FILE ${XML_PERFORMANCE_FILE_COMPLETE_PATH} NAME_WE)
            # copy the file from src to build folders
            configure_file(
                ${XML_PERFORMANCE_FILE_COMPLETE_PATH}                                                 # from full src path
                ${CMAKE_CURRENT_BINARY_DIR}/xmldocuments/test/performance/${XML_PERFORMANCE_FILE}.xml # to relative build path
                COPYONLY)
        endforeach()
    endif()

    if(PROFILING_TESTS)
        # Copy the profiling tests validation files over to the build directory
        file(GLOB_RECURSE XML_PROFILING_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../../profiling/*_profile.xml)
        # for each xml file detected
        foreach(XML_PROFILING_FILE_COMPLETE_PATH ${XML_PROFILING_FILES})
            # obtain the file name
            get_filename_component(XML_PROFILING_FILE ${XML_PROFILING_FILE_COMPLETE_PATH} NAME_WE)
            # copy the file from src to build folders
            configure_file(
                ${XML_PROFILING_FILE_COMPLETE_PATH}                                               # from full src path
                ${CMAKE_CURRENT_BINARY_DIR}/xmldocuments/test/profiling/${XML_PROFILING_FILE}.xml # to relative build path
                COPYONLY)
        endforeach()
    endif()

    # Define test
    add_test(NAME xml.validate COMMAND ${env_fast_command} xml validate xmldocuments)

    set_property(
        TEST xml.validate
        PROPERTY ENVIRONMENT ${TEST_ENVIRONMENT}
    )

    ###############################################################################
    # XML Static discovery validation
    ###############################################################################

    # Copy the examples validation files over to the build directory
    file(GLOB_RECURSE STATIC_DISC_XML_EXAMPLE ${CMAKE_CURRENT_SOURCE_DIR}/../../../../examples/*_static_disc.xml)

    # for each xml file detected in examples folder
    foreach(STATIC_DISC_XML_EXAMPLE_COMPLETE_PATH ${STATIC_DISC_XML_EXAMPLE})
        # obtain the file name
        get_filename_component(STATIC_DISC_XML_EXAMPLE ${STATIC_DISC_XML_EXAMPLE_COMPLETE_PATH} NAME_WE)
        # copy the file from src to build folders
        configure_file(
            ${STATIC_DISC_XML_EXAMPLE_COMPLETE_PATH}                                                 # from full src path
            ${CMAKE_CURRENT_BINARY_DIR}/xml_static_disc_docs/examples/${STATIC_DISC_XML_EXAMPLE}.xml # to relative build path
            COPYONLY)
    endforeach()

    # Copy the test validation files over to the build directory
    file(GLOB_RECURSE STATIC_DISC_XML_TEST ${CMAKE_CURRENT_SOURCE_DIR}/../*_static_disc.xml)

    # for each xml file detected in test folder
    foreach(STATIC_DISC_XML_TEST_COMPLETE_PATH ${STATIC_DISC_XML_TEST})
        # obtain the file name
        get_filename_component(STATIC_DISC_XML_TEST ${STATIC_DISC_XML_TEST_COMPLETE_PATH} NAME_WE)
        # copy the file from src to build folders
        configure_file(
            ${STATIC_DISC_XML_TEST_COMPLETE_PATH}                                             # from full src path
            ${CMAKE_CURRENT_BINARY_DIR}/xml_static_disc_docs/test/system/${STATIC_DISC_XML_TEST}.xml # to relative build path
            COPYONLY)
    endforeach()

    # Copy the sampled test files over to the build directory
    file(GLOB_RECURSE STATIC_DISC_XML_SAMPLED_TEST ${CMAKE_CURRENT_SOURCE_DIR}/../../../blackbox/*_static_disc.xml.in)

    # set the expected CMake variables for the samples
    set(TOPIC_RANDOM_NUMBER "123")
    set(W_UNICAST_PORT_RANDOM_NUMBER "456")
    set(R_UNICAST_PORT_RANDOM_NUMBER "654")
    set(MULTICAST_PORT_RANDOM_NUMBER "789")

    # for each xml file detected in examples folder
    foreach(STATIC_DISC_XML_SAMPLED_TEST_COMPLETE_PATH ${STATIC_DISC_XML_SAMPLED_TEST})
        # obtain the file name
        get_filename_component(STATIC_DISC_XML_SAMPLED_TEST ${STATIC_DISC_XML_SAMPLED_TEST_COMPLETE_PATH} NAME_WE)
        # copy the file from src to build folders
        configure_file(
            ${STATIC_DISC_XML_SAMPLED_TEST_COMPLETE_PATH}                                                 # from full src path
            ${CMAKE_CURRENT_BINARY_DIR}/xml_static_disc_docs/test/blackbox/${STATIC_DISC_XML_SAMPLED_TEST}.xml # to relative build path
            )
    endforeach()

    # Define test
    add_test(NAME xml.static_disc_validate COMMAND ${env_fast_command} xml validate xml_static_disc_docs -x ${CMAKE_SOURCE_DIR}/resources/xsd/fastdds_static_discovery.xsd)

    set_property(
            TEST xml.static_disc_validate
            PROPERTY ENVIRONMENT ${TEST_ENVIRONMENT}
    )

endif()
