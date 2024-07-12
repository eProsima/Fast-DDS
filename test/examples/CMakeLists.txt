# Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
message(STATUS "Configuring examples tests")

# Find Python3
find_package(Python3 COMPONENTS Interpreter REQUIRED)

# Find docker
find_program(DOCKER_EXECUTABLE docker)
if(NOT DOCKER_EXECUTABLE)
    message(FATAL_ERROR "Docker not found")
endif()

set(FILE_EXTENSION "")
set(DOCKER_IMAGE_NAME "")
set(SHELL_EXECUTABLE "")
set(TINYXML2_LIB_DIR_COMPOSE_VOLUME "")
set(TINYXML2_LIB_DIR_COMPOSE_LD_LIBRARY_PATH "")

# Linux configurations
if(UNIX AND NOT(APPLE) AND NOT(QNXNTO) AND NOT(ANDROID))
    # Find bash
    find_program(BASH_EXECUTABLE bash)
    if(NOT BASH_EXECUTABLE)
        message(FATAL_ERROR "bash not found")
    endif()

    set(SHELL_EXECUTABLE ${BASH_EXECUTABLE})
    set(DOCKER_IMAGE_NAME "ubuntu:22.04")

# Windows configurations
elseif(WIN32)
    # Find pwsh
    find_program(PWSH_EXECUTABLE pwsh)
    if(NOT PWSH_EXECUTABLE)
        message(FATAL_ERROR "pwsh not found")
    endif()

    set(PWSH_EXECUTABLE ${BASH_EXECUTABLE})
    set(FILE_EXTENSION ".exe")

    # We don't know which docker image to use for Windows yet
    message(FATAL_ERROR "Windows not supported yet")

# Unsupported platform
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

# Configure TinyXML2 library path if installed in user library path
if(NOT (TINYXML2_FROM_SOURCE OR TINYXML2_FROM_THIRDPARTY))
    get_filename_component(TINYXML2_LIB_DIR ${TINYXML2_LIBRARY} DIRECTORY)
    set(TINYXML2_LIB_DIR_COMPOSE_VOLUME "- ${TINYXML2_LIB_DIR}:${CMAKE_INSTALL_PREFIX}/${DATA_INSTALL_DIR}/fastdds:ro")
    set(TINYXML2_LIB_DIR_COMPOSE_LD_LIBRARY_PATH ":${CMAKE_INSTALL_PREFIX}/${DATA_INSTALL_DIR}/fastdds")
endif()

# Find all pytest files for testing
file(GLOB examples_python_tests RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.py)

# Add security test only if security is enabled
if (NOT SECURITY)
    list(FILTER examples_python_tests EXCLUDE REGEX "security")
endif()

# Configure the pytest files, and add a test for each one
foreach(example_test ${examples_python_tests})
    get_filename_component(example_name ${example_test} NAME_WE)
    string(REPLACE "test_" "" example_name "${example_name}")
    message(STATUS "  Configuring example test: ${example_name}")
    configure_file(${example_test}
                   ${CMAKE_CURRENT_BINARY_DIR}/${example_name}/${example_test} @ONLY)
    configure_file(${example_name}.compose.yml
                   ${CMAKE_CURRENT_BINARY_DIR}/${example_name}/${example_name}.compose.yml @ONLY)
    add_test(NAME example_tests.${example_name}
             COMMAND ${Python3_EXECUTABLE} -m pytest -vrP
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${example_name})
endforeach()
