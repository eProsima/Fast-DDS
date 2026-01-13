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

message(STATUS "Configuring docker based communication tests")

option(USE_OUTSIDE_DOCKER "Run docker against a host daemon (Docker-outside-of-Docker)." OFF)
set(DOCKER_HOST_IP "" CACHE STRING "Docker host IP to use.")

# Find docker
find_program(DOCKER_EXECUTABLE docker)
if(NOT DOCKER_EXECUTABLE)
    message(WARNING "Docker executable not found. Docker based communication tests won't be compiled.")
    return()
endif()

set(SHELL_EXECUTABLE "")

# Linux configurations
if(UNIX AND NOT(APPLE) AND NOT(QNXNTO) AND NOT(ANDROID))
    # Find bash
    find_program(BASH_EXECUTABLE bash)
    if(NOT BASH_EXECUTABLE)
        message(WARNING "bash executable not found. Docker based communication tests won't be compiled.")
        return()
    endif()

    set(SHELL_EXECUTABLE ${BASH_EXECUTABLE})

# Windows configurations
elseif(WIN32)
    # We don't know which docker image to use for Windows yet
    message(WARNING "Windows not supported yet. Docker based communication tests won't be compiled.")
    return()
# Unsupported platform
else()
    message(WARNING "Unsupported platform. Docker based communication tests won't be compiled.")
    return()
endif()

set(_docker_host "tcp://172.17.0.1:2375")  # Default docker host for Linux
if(DOCKER_HOST_IP)
    set(_docker_host "${DOCKER_HOST_IP}")
endif()

# Make compose project name unique per build dir
string(SHA1 _di_hash "${CMAKE_CURRENT_BINARY_DIR}")
string(SUBSTRING "${_di_hash}" 0 8 COMPOSE_PROJECT)

# Export vars into templates
if(USE_OUTSIDE_DOCKER)
    # When using Docker-outside-of-Docker, all libs and binaries must be delivered from the builder container
    set(DOCKER_HOST_RESOLVED_IP "${_docker_host}")
    set(PROJECT_BINARY_VOLUME "- /github/docker-testing/build/fastdds:/docker_ws/build/fastdds")
    set(PROJECT_BINARY_DIR_HOST "/docker_ws/build/fastdds")
    set(fastcdr_LIB_DIR_VOLUME "- /github/docker-testing/install/fastcdr/lib:/docker_ws/install/fastcdr/lib")
    set(CMAKE_INSTALL_PREFIX_VOLUME "- /github/docker-testing/install/fastdds:/docker_ws/install/fastdds")
    set(LD_LIBRARY_PATH_COMPOSE "/docker_ws/build/fastdds/src/cpp:/docker_ws/install/fastcdr/lib:/docker_ws/install/fastdds/lib")
    set(EXAMPLE_DIR_COMPOSE "/docker_ws/build/fastdds/test/dds/communication")
else()
    # When using Docker-in-Docker, all libs and binaries are mounted from the host
    set(DOCKER_HOST_RESOLVED_IP "")
    set(PROJECT_BINARY_VOLUME "- ${PROJECT_BINARY_DIR}:${PROJECT_BINARY_DIR}")
    set(PROJECT_BINARY_DIR_HOST "${PROJECT_BINARY_DIR}")
    set(fastcdr_LIB_DIR_VOLUME "- ${fastcdr_LIB_DIR}:${fastcdr_LIB_DIR}")
    set(CMAKE_INSTALL_PREFIX_VOLUME "- ${CMAKE_INSTALL_PREFIX}:${CMAKE_INSTALL_PREFIX}")
    set(LD_LIBRARY_PATH_COMPOSE "${PROJECT_BINARY_DIR}/src/cpp:${fastcdr_LIB_DIR}:${CMAKE_INSTALL_PREFIX}/lib")
    set(EXAMPLE_DIR_COMPOSE "${PROJECT_BINARY_DIR}/test/dds/communication")
endif()

# Running script that executes the test in docker compose
configure_file(run_compose_test.bash
               ${CMAKE_CURRENT_BINARY_DIR}/run_compose_test.bash @ONLY)

configure_file(cli.Dockerfile
               ${CMAKE_CURRENT_BINARY_DIR}/cli.Dockerfile @ONLY)
configure_file(ds_init_no_interfaces.compose.yml
         ${CMAKE_CURRENT_BINARY_DIR}/ds_init_no_interfaces.compose.yml @ONLY)
configure_file(launch_ds_no_interfaces.bash
         ${CMAKE_CURRENT_BINARY_DIR}/launch_ds_no_interfaces.bash @ONLY)
add_test(NAME dds.communication.ds_init_with_no_interfaces
    COMMAND ${SHELL_EXECUTABLE}
            ${CMAKE_CURRENT_BINARY_DIR}/run_compose_test.bash
            ${CMAKE_CURRENT_BINARY_DIR}/ds_init_no_interfaces.compose.yml
            ${COMPOSE_PROJECT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
