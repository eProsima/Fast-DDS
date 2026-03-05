# Copyright (C) 2025, Proyectos y Sistemas de Mantenimiento SL (eProsima)
#
# This program is commercial software licensed under the terms of the
# eProsima Software License Agreement Rev 03 (the "License")
#
# You may obtain a copy of the License at
# https://www.eprosima.com/licenses/LICENSE-REV03

# Find docker
find_program(DOCKER_EXECUTABLE docker)
if(NOT DOCKER_EXECUTABLE)
    message(WARNING "Docker executable not found. feature.ip_mobility test won't be compiled.")
    return()
endif()

set(SHELL_EXECUTABLE "")
# Linux configurations
if(UNIX AND NOT(APPLE) AND NOT(QNXNTO) AND NOT(ANDROID))
    # Find bash
    find_program(BASH_EXECUTABLE bash)
    if(NOT BASH_EXECUTABLE)
        message(WARNING "bash executable not found. feature.ip_mobility test won't be compiled.")
        return()
    endif()

    set(SHELL_EXECUTABLE ${BASH_EXECUTABLE})

# Windows configurations
elseif(WIN32)
    # We don't know which docker image to use for Windows
    message(WARNING "Windows not supported. feature.ip_mobility test won't be compiled.")
    return()
# Unsupported platform
else()
    message(WARNING "Unsupported platform. feature.ip_mobility test won't be compiled.")
    return()
endif()

# Make compose project name unique per build dir
string(SHA1 _di_hash "${CMAKE_CURRENT_BINARY_DIR}")
string(SUBSTRING "${_di_hash}" 0 8 COMPOSE_PROJECT)

# Export vars into templates
if(USE_OUTSIDE_DOCKER)
    # When using Docker-outside-of-Docker, all libs and binaries must be delivered from the builder container
    set(EXAMPLE_DIR_COMPOSE "/docker_ws/build/fastdds/test/dds/communication")
else()
    # When using Docker-in-Docker, all libs and binaries are mounted from the host
    set(EXAMPLE_DIR_COMPOSE "${PROJECT_BINARY_DIR}/test/dds/communication")
endif()

# Running script that executes the test in docker compose
configure_file(run_compose_test.bash
               ${CMAKE_CURRENT_BINARY_DIR}/run_compose_test.bash @ONLY)
# Test specific configuration files
configure_file(net-tools.Dockerfile
               ${CMAKE_CURRENT_BINARY_DIR}/net-tools.Dockerfile @ONLY)
configure_file(dynamic_interfaces.compose.yml
               ${CMAKE_CURRENT_BINARY_DIR}/dynamic_interfaces.compose.yml @ONLY)
configure_file(launch_subscriber.bash
               ${CMAKE_CURRENT_BINARY_DIR}/launch_subscriber.bash @ONLY)

add_test(NAME feature.ip_mobility.interface_bringup
         COMMAND ${SHELL_EXECUTABLE}
            ${CMAKE_CURRENT_BINARY_DIR}/run_compose_test.bash
            ${CMAKE_CURRENT_BINARY_DIR}/dynamic_interfaces.compose.yml
            ${COMPOSE_PROJECT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
