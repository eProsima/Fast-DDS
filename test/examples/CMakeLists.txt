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

# Find all docker compose yml files for testing
file(GLOB files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.yml)

# Configure the docker compose files
foreach(file ${files})
    message(STATUS "  Configuring example test: ${file}")
    configure_file(${file} ${CMAKE_CURRENT_BINARY_DIR}/${file} @ONLY)
endforeach()

# Copy pytest file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_examples.py ${CMAKE_CURRENT_BINARY_DIR}/test_examples.py @ONLY)

# Add test to ctest runs it
add_test(NAME example_tests COMMAND ${Python3_EXECUTABLE} -m pytest -vrP WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
