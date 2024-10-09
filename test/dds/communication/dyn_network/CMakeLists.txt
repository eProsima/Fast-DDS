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
message(STATUS "Configuring dynamic network interfaces tests")

# Find docker
find_program(DOCKER_EXECUTABLE docker)
if(NOT DOCKER_EXECUTABLE)
    message(WARNING "Docker executable not found. dds.communication.dynamic_interfaces test won't be compiled.")
    return()
endif()

set(SHELL_EXECUTABLE "")
set(TINYXML2_LIB_DIR_COMPOSE_VOLUME "")
set(TINYXML2_LIB_DIR_COMPOSE_LD_LIBRARY_PATH "")

# Linux configurations
if(UNIX AND NOT(APPLE) AND NOT(QNXNTO) AND NOT(ANDROID))
    # Find bash
    find_program(BASH_EXECUTABLE bash)
    if(NOT BASH_EXECUTABLE)
        message(WARNING "bash executable not found. dds.communication.dynamic_interfaces test won't be compiled.")
        return()
    endif()

    set(SHELL_EXECUTABLE ${BASH_EXECUTABLE})

# Windows configurations
elseif(WIN32)
    # We don't know which docker image to use for Windows yet
    message(WARNING "Windows not supported yet. dds.communication.dynamic_interfaces test won't be compiled.")
    return()
# Unsupported platform
else()
    message(WARNING "Unsupported platform. dds.communication.dynamic_interfaces test won't be compiled.")
    return()
endif()

# Configure TinyXML2 library path if installed in user library path
if(NOT (TINYXML2_FROM_SOURCE OR TINYXML2_FROM_THIRDPARTY))
    get_filename_component(TINYXML2_LIB_DIR ${TINYXML2_LIBRARY} DIRECTORY)
    set(TINYXML2_LIB_DIR_COMPOSE_VOLUME "- ${TINYXML2_LIB_DIR}:${CMAKE_INSTALL_PREFIX}/${DATA_INSTALL_DIR}/fastdds:ro")
    set(TINYXML2_LIB_DIR_COMPOSE_LD_LIBRARY_PATH ":${CMAKE_INSTALL_PREFIX}/${DATA_INSTALL_DIR}/fastdds")
endif()

configure_file(Dockerfile
               ${CMAKE_CURRENT_BINARY_DIR}/Dockerfile @ONLY)
configure_file(dynamic_interfaces.compose.yml
               ${CMAKE_CURRENT_BINARY_DIR}/dynamic_interfaces.compose.yml @ONLY)
configure_file(launch_subscriber.bash
               ${CMAKE_CURRENT_BINARY_DIR}/launch_subscriber.bash @ONLY)
add_test(NAME dds.communication.dynamic_interfaces
         COMMAND ${DOCKER_EXECUTABLE} compose -f ${CMAKE_CURRENT_BINARY_DIR}/dynamic_interfaces.compose.yml up
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
