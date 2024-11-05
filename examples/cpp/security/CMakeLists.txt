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

cmake_minimum_required(VERSION 3.20)

project(fastdds_security_example VERSION 1 LANGUAGES CXX)

# Find requirements
if(NOT fastcdr_FOUND)
    find_package(fastcdr 2 REQUIRED)
endif()

if(NOT fastdds_FOUND)
    find_package(fastdds 3 REQUIRED)
endif()

#Check C++11
include(CheckCXXCompilerFlag)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    check_cxx_compiler_flag(-std=c++11 SUPPORTS_CXX11)
    if(NOT SUPPORTS_CXX11)
        message(FATAL_ERROR "Compiler doesn't support C++11")
    endif()
endif()

# Set CMAKE_BUILD_TYPE to Release by default.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

message(STATUS "Configuring secure hello world example...")

file(GLOB SECURE_HELLO_WORLD_SOURCES_CXX "*.cxx")
file(GLOB SECURE_HELLO_WORLD_SOURCES_CPP "*.cpp")

add_executable(security ${SECURE_HELLO_WORLD_SOURCES_CXX} ${SECURE_HELLO_WORLD_SOURCES_CPP})

target_compile_definitions(security PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    $<$<BOOL:${SHM_TRANSPORT_DEFAULT}>:SHM_TRANSPORT_BUILTIN> # Enable SHM as built-in transport
)

target_link_libraries(security fastdds fastcdr)

set(FASTDDS_SECURITY_EXAMPLE_BIN_INSTALL_DIR ${DATA_INSTALL_DIR}/fastdds/examples/cpp/security/${BIN_INSTALL_DIR})

# Install executable and certs
install(TARGETS security RUNTIME DESTINATION ${FASTDDS_SECURITY_EXAMPLE_BIN_INSTALL_DIR})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/certs DESTINATION ${FASTDDS_SECURITY_EXAMPLE_BIN_INSTALL_DIR})

# Copy the XML files over to the build directory
file(GLOB XML_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.xml)

# For each xml file detected
foreach(XML_FILE_COMPLETE_PATH ${XML_FILES})
    # Obtain the file name
    get_filename_component(XML_FILE ${XML_FILE_COMPLETE_PATH} NAME_WE)

    # Copy the file from src to build folders
    configure_file(
        ${XML_FILE_COMPLETE_PATH}                   # from full src path
        ${CMAKE_CURRENT_BINARY_DIR}/${XML_FILE}.xml # to relative build path
        COPYONLY)

    # Install XML config files
    install(FILES ${XML_FILE_COMPLETE_PATH} DESTINATION ${FASTDDS_SECURITY_EXAMPLE_BIN_INSTALL_DIR})
endforeach()

# Copy certs to build directory
add_custom_command(TARGET security POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${PROJECT_SOURCE_DIR}/certs
            ${PROJECT_BINARY_DIR}/certs)
