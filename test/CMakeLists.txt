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

# Require C++ 14 for testing as both GTest v1.14.0 and ROS 2 Jazzy require it anyways
set(FORCE_CXX "14")
check_stdcxx(${FORCE_CXX})

option(PERFORMANCE_TESTS "Activate the building and execution of performance tests" OFF)
option(SYSTEM_TESTS "Activate the building and execution of system tests" OFF)
option(PROFILING_TESTS "Activate the building and execution of profiling tests" OFF)
option(EPROSIMA_BUILD_TESTS "Activate the building and execution unit tests and integral tests" OFF)
option(FASTDDS_EXAMPLE_TESTS "Activate the building and execution of Fast DDS examples tests" OFF)

if(EPROSIMA_BUILD)
    set(EPROSIMA_BUILD_TESTS ON)
    set(SYSTEM_TESTS ON)
endif()

configure_file(CTestCustom.cmake.in ${PROJECT_BINARY_DIR}/CTestCustom.cmake)

file(TO_CMAKE_PATH "${PROJECT_SOURCE_DIR}/valgrind.supp" MEMORYCHECK_SUPPRESSIONS_FILE_TMP)
set(MEMORYCHECK_SUPPRESSIONS_FILE ${MEMORYCHECK_SUPPRESSIONS_FILE_TMP} CACHE FILEPATH "Valgrind suppresions file")

# Check if /etc/hosts file has been modified to add dummy host test subjects
if(WIN32)
    execute_process(COMMAND powershell -C Resolve-DNSName -Name www.acme.com.test -CacheOnly
        RESULT_VARIABLE EPROSIMA_TEST_DNS_NOT_SET_UP OUTPUT_QUIET ERROR_QUIET)
elseif(APPLE)
	execute_process(
        COMMAND dscacheutil -q host -a name www.acme.com.test
        COMMAND grep www.acme.com.test
        RESULT_VARIABLE EPROSIMA_TEST_DNS_NOT_SET_UP OUTPUT_QUIET ERROR_QUIET)
else()
    execute_process(COMMAND getent hosts www.acme.com.test
        RESULT_VARIABLE EPROSIMA_TEST_DNS_NOT_SET_UP OUTPUT_QUIET ERROR_QUIET)
endif()
message(STATUS "EPROSIMA_TEST_DNS_NOT_SET_UP: '${EPROSIMA_TEST_DNS_NOT_SET_UP}'")

###############################################################################
# Testing
###############################################################################
if(EPROSIMA_BUILD_TESTS)

    find_package(GTest CONFIG REQUIRED)
    include(${PROJECT_SOURCE_DIR}/cmake/testing/GoogleTest.cmake)

    add_subdirectory(blackbox)
    add_subdirectory(feature)
    add_subdirectory(unittest)
    add_subdirectory(dds/communication)
    add_subdirectory(dds/communication/security)
    add_subdirectory(dds/discovery)
    add_subdirectory(dds/xtypes)

    if(UNIX AND NOT APPLE AND STRICT_REALTIME)
        add_subdirectory(realtime)
    endif()
endif()

###############################################################################
# Example tests
###############################################################################
if (COMPILE_EXAMPLES AND FASTDDS_EXAMPLE_TESTS)
    add_subdirectory(examples)
endif()

###############################################################################
# Profiling tests using valgrind
###############################################################################
if(NOT ((MSVC OR MSVC_IDE)) AND PROFILING_TESTS)
    add_subdirectory(profiling)
endif()

###############################################################################
# Performance tests
###############################################################################
if(PERFORMANCE_TESTS)
    add_subdirectory(performance)
endif()

###############################################################################
# System tests
###############################################################################
if(SYSTEM_TESTS)
    find_package(GTest CONFIG REQUIRED)
    include(${PROJECT_SOURCE_DIR}/cmake/testing/GoogleTest.cmake)

    add_subdirectory(system/tools/fastdds)
    add_subdirectory(system/tools/fds)
endif()

if(EPROSIMA_BUILD_TESTS AND QNX)
    install(DIRECTORY ${PROJECT_BINARY_DIR}/test/
      DESTINATION bin/Fast-DDS_test
      PATTERN "*.cmake" EXCLUDE
      PATTERN "*.d" EXCLUDE
      PATTERN "*.dir" EXCLUDE
      PATTERN "*.internal" EXCLUDE
      PATTERN "*.make" EXCLUDE
      PATTERN "*.marks" EXCLUDE
      PATTERN "*.o" EXCLUDE
      PATTERN "*.ts" EXCLUDE
      PATTERN "*.txt" EXCLUDE
      PATTERN "CMakeFiles" EXCLUDE
      PATTERN "Makefile" EXCLUDE
      PATTERN "cmake" EXCLUDE
      )
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/test/certs
      DESTINATION bin/Fast-DDS_test
      )
endif()

if (ANDROID)
    option(INSTALL_ANDROID_TESTS "Install Built Tests to Device" OFF)
    if (INSTALL_ANDROID_TESTS)
        install(CODE    "set(ANDROID \"${ANDROID}\")
                        set(ANDROID_TESTING_ROOT \"${ANDROID_TESTING_ROOT}\")
                        set(CMAKE_BINARY_DIR \"${CMAKE_BINARY_DIR}\")
                        message(STATUS \"Android testing ROOT is '${ANDROID_TESTING_ROOT}'\")
                        if(ANDROID_TESTING_ROOT)
                            file(GLOB_RECURSE CTEST_GENERATED_FILES RELATIVE \"${CMAKE_BINARY_DIR}\" \"CTestTestfile.cmake\")
                            foreach(CTEST_FILE \${CTEST_GENERATED_FILES})
                                file(READ \${CTEST_FILE} contents)
                                message(STATUS \"Processing file \${CTEST_FILE}\")
                                string(REGEX REPLACE \"${CMAKE_BINARY_DIR}\" \"${ANDROID_TESTING_ROOT}\" contents \${contents})
                                file(REMOVE \${CTEST_FILE})
                                file(WRITE  \${CTEST_FILE} \${contents})
                            endforeach()
                        endif()
                        execute_process(COMMAND adb push \"${CMAKE_BINARY_DIR}/test\" \"${ANDROID_TESTING_ROOT}\") ")
    endif()
endif()
