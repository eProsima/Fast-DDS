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
    set(COMM_EXAMPLE_DIR_COMPOSE "/docker_ws/build/fastdds/test/dds/communication")
else()
    # When using Docker-in-Docker, all libs and binaries are mounted from the host
    set(COMM_EXAMPLE_DIR_COMPOSE "${PROJECT_BINARY_DIR}/test/dds/communication")
endif()

# Script that executes the test in docker compose
configure_file(run_compose_test.bash
               ${CMAKE_CURRENT_BINARY_DIR}/run_compose_test.bash @ONLY)

# Dynamic Interfaces test
configure_file(dynamic_interfaces/net-tools.Dockerfile
               ${CMAKE_CURRENT_BINARY_DIR}/dynamic_interfaces/net-tools.Dockerfile @ONLY)
configure_file(dynamic_interfaces/dynamic_interfaces.compose.yml
               ${CMAKE_CURRENT_BINARY_DIR}/dynamic_interfaces/dynamic_interfaces.compose.yml @ONLY)
configure_file(dynamic_interfaces/launch_subscriber.bash
               ${CMAKE_CURRENT_BINARY_DIR}/dynamic_interfaces/launch_subscriber.bash @ONLY)

# The following test checks basic dynamic interfaces bring up and tear down with simple discovery
add_test(NAME feature.ip_mobility.interface_bringup
         COMMAND ${SHELL_EXECUTABLE}
            ${CMAKE_CURRENT_BINARY_DIR}/run_compose_test.bash
            ${CMAKE_CURRENT_BINARY_DIR}/dynamic_interfaces/dynamic_interfaces.compose.yml
            ${COMPOSE_PROJECT}_dyn
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

### TCP Keep Alive test

# Export vars into templates
if(USE_OUTSIDE_DOCKER)
    # When using Docker-outside-of-Docker, all libs and binaries must be delivered from the builder container
    set(CONF_EXAMPLE_DIR_COMPOSE "/docker_ws/build/fastdds/examples/cpp/configuration")
    set(TCP_KA_DIR_COMPOSE "/docker_ws/build/fastdds/test/feature/ip-mobility/system/tcp_keep_alive")
    set(PROFILES_PREFIX_FILE_COMPOSE "/docker_ws/build/fastdds/test/feature/ip-mobility/system/tcp_keep_alive")
else()
    # When using Docker-in-Docker, all libs and binaries are mounted from the host
    set(CONF_EXAMPLE_DIR_COMPOSE "${PROJECT_BINARY_DIR}/examples/cpp/configuration")
    set(TCP_KA_DIR_COMPOSE "${PROJECT_BINARY_DIR}/test/feature/ip-mobility/system/tcp_keep_alive")
    set(PROFILES_PREFIX_FILE_COMPOSE "${PROJECT_BINARY_DIR}/test/feature/ip-mobility/system/tcp_keep_alive")
endif()

set(TCP_KEEP_ALIVE_FILES
    net-ip-tools.Dockerfile
    change_local_ip_sim.sh
    change_wan_ip_sim.sh
    initial_peer_tcp_server.xml
    initial_peer_tcp.xml
    launch_receiver.bash
    launch_router_change_wan.bash
    launch_router_unplug.bash
    launch_router.bash
    launch_sender.bash
    launch_sender_change_loc.bash
    tcp_keep_alive.compose.yml
    unplug_sim.sh
)
set(TCP_KEEP_ALIVE_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/tcp_keep_alive)
foreach(f IN LISTS TCP_KEEP_ALIVE_FILES)
  configure_file(tcp_keep_alive/${f} ${TCP_KEEP_ALIVE_BIN_DIR}/${f} @ONLY)
endforeach()

# The following test check that TCP communication can survive to network unplugging
add_test(NAME feature.ip_mobility.tcp_keep_alive_unplug
    COMMAND ${SHELL_EXECUTABLE}
            ${CMAKE_CURRENT_BINARY_DIR}/run_compose_test.bash
            ${CMAKE_CURRENT_BINARY_DIR}/tcp_keep_alive/tcp_keep_alive.compose.yml
            ${COMPOSE_PROJECT}_unplug
            "unplug"  # Profile name
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# The following test check that TCP communication can survive to a change in the WAN IP address. Check tcp_keep_alive.compose.yml for details on topology
add_test(NAME feature.ip_mobility.tcp_keep_alive_change_wan_ip
    COMMAND ${SHELL_EXECUTABLE}
            ${CMAKE_CURRENT_BINARY_DIR}/run_compose_test.bash
            ${CMAKE_CURRENT_BINARY_DIR}/tcp_keep_alive/tcp_keep_alive.compose.yml
            ${COMPOSE_PROJECT}_wan
            "wan"  # Profile name
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# The following test check that TCP communication can survive to a change in the local IP address. Check tcp_keep_alive.compose.yml for details on topology
add_test(NAME feature.ip_mobility.tcp_keep_alive_change_local_ip
    COMMAND ${SHELL_EXECUTABLE}
            ${CMAKE_CURRENT_BINARY_DIR}/run_compose_test.bash
            ${CMAKE_CURRENT_BINARY_DIR}/tcp_keep_alive/tcp_keep_alive.compose.yml
            ${COMPOSE_PROJECT}_local
            "local"  # Profile name
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
