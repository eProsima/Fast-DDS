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

# Configure TinyXML2 library path if installed in user library path
if(NOT (TINYXML2_FROM_SOURCE OR TINYXML2_FROM_THIRDPARTY))
    get_filename_component(TINYXML2_LIB_DIR ${TINYXML2_LIBRARY} DIRECTORY)
    set(TINYXML2_LIB_DIR_COMPOSE_VOLUME "- ${TINYXML2_LIB_DIR}:${CMAKE_INSTALL_PREFIX}/${DATA_INSTALL_DIR}/fastdds:ro")
    set(TINYXML2_LIB_DIR_COMPOSE_LD_LIBRARY_PATH ":${CMAKE_INSTALL_PREFIX}/${DATA_INSTALL_DIR}/fastdds")
endif()

# Linux configurations
if(UNIX AND NOT(APPLE) AND NOT(QNXNTO) AND NOT(ANDROID))
    # Find bash
    find_program(BASH_EXECUTABLE bash)
    if(NOT BASH_EXECUTABLE)
        message(FATAL_ERROR "bash not found")
    endif()

    set(SHELL_EXECUTABLE ${BASH_EXECUTABLE})
    set(DOCKER_IMAGE_NAME "ubuntu:22.04")

    set(PROJECT_BINARY_DIR_COMPOSE_VOLUME "${PROJECT_BINARY_DIR}:${PROJECT_BINARY_DIR}")
    set(fastcdr_LIB_DIR_COMPOSE_VOLUME "${fastcdr_LIB_DIR}:${fastcdr_LIB_DIR}")
    set(CMAKE_INSTALL_PREFIX_COMPOSE_VOLUME "${CMAKE_INSTALL_PREFIX}:${CMAKE_INSTALL_PREFIX}")

    set(PATH_ENVIRONMENT_VARIABLE_COMPOSE "LD_LIBRARY_PATH: ${PROJECT_BINARY_DIR}/src/cpp:${fastcdr_LIB_DIR}")
    if (TINYXML2_LIB_DIR_COMPOSE_LD_LIBRARY_PATH)
        set(PATH_ENVIRONMENT_VARIABLE_COMPOSE "${PATH_ENVIRONMENT_VARIABLE_COMPOSE}${TINYXML2_LIB_DIR_COMPOSE_LD_LIBRARY_PATH}")
    endif()
    set(EXAMPLE_PREFIX_DIR_COMPOSE "${PROJECT_BINARY_DIR}/examples/cpp")
    set(EXAMPLE_SUFFIX_DIR_COMPOSE "")
    set(FASTDDS_DEFAULT_PROFILES_FILE_PREFIX_COMPOSE "${PROJECT_BINARY_DIR}/examples/cpp")
    set(COMMAND_EXAMPLE_DIR_PREFIX_COMPOSE "$\${EXAMPLE_DIR}")
    set(COMMAND_CONCATENATE_COMPOSE "&")
    set(COMMAND_BACKGROUND_JOB_COMPOSE "")

    set(SUB_ADDITIONAL_ARGS_COMPOSE "$\${SUB_ADDITIONAL_ARGS_COMPOSE}")
    set(PUB_ADDITIONAL_ARGS_COMPOSE "$\${PUB_ADDITIONAL_ARGS_COMPOSE}")
    set(SPLIT_ARGS_COMPOSE "")

# Windows configurations
elseif(WIN32)
    # Find pwsh
    find_program(PWSH_EXECUTABLE pwsh)
    if(NOT PWSH_EXECUTABLE)
        message(FATAL_ERROR "pwsh not found")
    endif()

    set(PWSH_VERSION_DOCKER "latest")
    set(SHELL_EXECUTABLE "C:/\"Program Files\"/PowerShell/${PWSH_VERSION_DOCKER}/pwsh.exe")

    set(FILE_EXTENSION ".exe")
    set(DOCKER_IMAGE_NAME "windows-testing")

    cmake_path(GET fastcdr_LIB_DIR PARENT_PATH fastcdr_INSTALL_DIR)

    # Ensure drives inside docker are mounted in C: drive
    string(REGEX REPLACE ".:" "C:" CMAKE_INSTALL_PREFIX_C ${CMAKE_INSTALL_PREFIX})
    string(REGEX REPLACE ".:" "C:" PROJECT_BINARY_DIR_C ${PROJECT_BINARY_DIR})
    string(REGEX REPLACE ".:" "C:" fastcdr_INSTALL_DIR_C ${fastcdr_INSTALL_DIR})

    set(PROJECT_BINARY_DIR_COMPOSE_VOLUME "${PROJECT_BINARY_DIR}:${PROJECT_BINARY_DIR_C}")
    set(fastcdr_LIB_DIR_COMPOSE_VOLUME "${fastcdr_INSTALL_DIR}/bin:${fastcdr_INSTALL_DIR_C}/bin")
    set(CMAKE_INSTALL_PREFIX_COMPOSE_VOLUME "${CMAKE_INSTALL_PREFIX}/bin:${CMAKE_INSTALL_PREFIX_C}/bin")

    set(PATH_ENVIRONMENT_VARIABLE_COMPOSE "PATH: C:/Program Files/OpenSSL-Win64;${CMAKE_INSTALL_PREFIX_C}/bin;${fastcdr_INSTALL_DIR_C}/bin;C:/Windows/System32;C:/Windows/System32/downlevel;")
    set(EXAMPLE_PREFIX_DIR_COMPOSE "${PROJECT_BINARY_DIR_C}/examples/cpp")
    if (CMAKE_BUILD_TYPE)
        set(EXAMPLE_SUFFIX_DIR_COMPOSE "${CMAKE_BUILD_TYPE}")
    else()
        set(EXAMPLE_SUFFIX_DIR_COMPOSE "Release")
    endif()
    set(FASTDDS_DEFAULT_PROFILES_FILE_PREFIX_COMPOSE "${PROJECT_BINARY_DIR_C}/examples/cpp")
    set(COMMAND_EXAMPLE_DIR_PREFIX_COMPOSE "& $\$Env:EXAMPLE_DIR")
    set(COMMAND_CONCATENATE_COMPOSE "&\" \"")
    set(COMMAND_BACKGROUND_JOB_COMPOSE "; Receive-Job 1 -Wait")

    set(SUB_ADDITIONAL_ARGS_COMPOSE "$\$Env:SUB_ADDITIONAL_ARGS_COMPOSE")
    set(PUB_ADDITIONAL_ARGS_COMPOSE "$\$Env:PUB_ADDITIONAL_ARGS_COMPOSE")
    set(SPLIT_ARGS_COMPOSE ".split(' ')")

    set(WIN_DOCKERFILE ${CMAKE_CURRENT_LIST_DIR}/windows/Dockerfile)
    # Generate image for testing
    add_custom_target(
        windows_docker_image_testing_generation
        ALL
        # Launch the docker build command using the build context.
        COMMAND ${DOCKER_EXECUTABLE} build
            --tag ${DOCKER_IMAGE_NAME}
            --file ${WIN_DOCKERFILE}
            ${CMAKE_CURRENT_LIST_DIR}
        COMMENT "Creating windows image for testing..." VERBATIM
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
# Unsupported platform
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

if(WIN32)
    # Temporarily, test hello world
    file(GLOB examples_python_tests RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/test_custom_payload_pool.py
        ${CMAKE_CURRENT_SOURCE_DIR}/test_hello_world.py
        ${CMAKE_CURRENT_SOURCE_DIR}/test_rtps.py
        ${CMAKE_CURRENT_SOURCE_DIR}/test_flow_control.py
        ${CMAKE_CURRENT_SOURCE_DIR}/test_content_filter.py)
else()
    # Find all pytest files for testing
    file(GLOB examples_python_tests RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.py)
endif()

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
