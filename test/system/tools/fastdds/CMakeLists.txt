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

find_package(PythonInterp 3 REQUIRED)

if(PYTHONINTERP_FOUND)

    set(TESTS
        test_fastdds_installed
        test_fastdds_discovery
        test_ros_discovery
        test_fastdds_shm
        test_fastdds_xml_validate
    )

    foreach(TEST IN LISTS TESTS)
        add_test(
                NAME system.tools.fastdds.${TEST}
                COMMAND ${PYTHON_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/tests.py
                ${CMAKE_INSTALL_PREFIX}
                ${TEST}
            )

        # Set test properties
        set_property(
            TEST system.tools.fastdds.${TEST}
            PROPERTY LABELS "NoMemoryCheck"
        )

    endforeach()

endif()

###############################################################################
# XML validation
###############################################################################
message(STATUS "Configuring CLI xml validation...")

# Copy the neccessary files over to the build directory
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../xmlvalidation/profileAll.xml
    ${CMAKE_CURRENT_BINARY_DIR}/xmldocuments/profileAll.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../xmlvalidation/profileDataReader.xml
    ${CMAKE_CURRENT_BINARY_DIR}/xmldocuments/profileDataReader.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../xmlvalidation/profileDataWriter.xml
    ${CMAKE_CURRENT_BINARY_DIR}/xmldocuments/profileDataWriter.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../xmlvalidation/profileDomainParticipant.xml
    ${CMAKE_CURRENT_BINARY_DIR}/xmldocuments/profileDomainParticipant.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../xmlvalidation/profileTransportDescriptor.xml
    ${CMAKE_CURRENT_BINARY_DIR}/xmldocuments/profileTransportDescriptor.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../xmlvalidation/profileLog.xml
    ${CMAKE_CURRENT_BINARY_DIR}/xmldocuments/profileLog.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../xmlvalidation/profileTypes.xml
    ${CMAKE_CURRENT_BINARY_DIR}/xmldocuments/profileTypes.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../xmlvalidation/XMLTesterExample.xml
    ${CMAKE_CURRENT_BINARY_DIR}/xmldocuments/XMLTesterExample.xml COPYONLY)


# Check XML profile validation against schema

if (MSVC)

    # WINDOWS
    set(env_fast_command "fastdds.bat")

else()

    # POSIX
    set(env_fast_command "fastdds")

endif()

# Main validator
add_test(NAME xml.validate COMMAND env_fast_command xml validate xmldocuments)

# Check the XML from examples (that follows the xsd schema, avoid static discovery xmls)
add_test(NAME xml.validate_examples_1 COMMAND env_fast_command xml validate ../../../../examples/cpp/dds/DynamicHelloWorldExample)
add_test(NAME xml.validate_examples_2 COMMAND env_fast_command xml validate ../../../../examples/cpp/dds/TypeLookupService)

# Check the XML from tests (that follows the xsd schema, avoid static discovery xmls or bad structured on purpose xmls)
#Blackbox tests
add_test(NAME xml.validate_tests_blackbox_1 COMMAND env_fast_command xml validate ../../../blackbox/discovery_participant_flags.xml)
add_test(NAME xml.validate_tests_blackbox_2 COMMAND env_fast_command xml validate ../../../blackbox/partitions.xml)
add_test(NAME xml.validate_tests_blackbox_3 COMMAND env_fast_command xml validate ../../../blackbox/persistence.xml)
add_test(NAME xml.validate_tests_blackbox_4 COMMAND env_fast_command xml validate ../../../blackbox/StatisticsDomainParticipant.xml)

# Unit tests
add_test(NAME xml.validate_tests_unittest_1 COMMAND env_fast_command xml validate ../../../unittest/dds)
add_test(NAME xml.validate_tests_unittest_2 COMMAND env_fast_command xml validate ../../../unittest/dynamic_types)
add_test(NAME xml.validate_tests_unittest_3 COMMAND env_fast_command xml validate ../../../unittest/xmlparser/log_inactive.xml)
add_test(NAME xml.validate_tests_unittest_4 COMMAND env_fast_command xml validate ../../../unittest/xmlparser/test_xml_profiles.xml)
add_test(NAME xml.validate_tests_unittest_5 COMMAND env_fast_command xml validate ../../../unittest/xmlparser/tls_config.xml)
add_test(NAME xml.validate_tests_unittest_6 COMMAND env_fast_command xml validate ../../../unittest/xmlparser/log_node_file_append.xml)
add_test(NAME xml.validate_tests_unittest_7 COMMAND env_fast_command xml validate ../../../unittest/xmlparser/UDP_transport_descriptors_config.xml)
add_test(NAME xml.validate_tests_unittest_8 COMMAND env_fast_command xml validate ../../../unittest/xmlparser/log_def_file.xml)
add_test(NAME xml.validate_tests_unittest_9 COMMAND env_fast_command xml validate ../../../unittest/xmlparser/log_stdouterr_two_thresholds.xml)
add_test(NAME xml.validate_tests_unittest_10 COMMAND env_fast_command xml validate ../../../unittest/xmlparser/test_xml_profiles_rooted.xml)
add_test(NAME xml.validate_tests_unittest_11 COMMAND env_fast_command xml validate ../../../unittest/xmlparser/test_xml_root_library_settings.xml)
add_test(NAME xml.validate_tests_unittest_12 COMMAND env_fast_command xml validate ../../../unittest/xmlparser/test_xml_duration.xml)
add_test(NAME xml.validate_tests_unittest_13 COMMAND env_fast_command xml validate ../../../unittest/xmlparser/SHM_transport_descriptors_config.xml)
add_test(NAME xml.validate_tests_unittest_14 COMMAND env_fast_command xml validate ../../../unittest/xmlparser/log_stdouterr.xml)
add_test(NAME xml.validate_tests_unittest_15 COMMAND env_fast_command xml validate ../../../unittest/xmlparser/test_xml_security_profiles.xml)

# System tests
add_test(NAME xml.validate_tests_system COMMAND env_fast_command xml validate ../.)

# Other tests
add_test(NAME xml.validate_tests_communication COMMAND env_fast_command xml validate ../../../communication)
add_test(NAME xml.validate_tests_dds COMMAND env_fast_command xml validate ../../../dds)

# Performance tests
if(PERFORMANCE_TESTS)
    add_test(NAME xml.validate_tests_performance COMMAND env_fast_command xml validate ../../../performance)
endif()

# Profiling tests
if(PROFILING_TESTS)
    add_test(NAME xml.validate_tests_profiling COMMAND env_fast_command xml validate ../../../profiling)
endif()






