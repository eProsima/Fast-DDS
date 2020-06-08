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

cmake_minimum_required(VERSION 3.5)

# symlink creation requires install( CODE using generator expressions 
cmake_policy(SET CMP0087 NEW)

project(fast-discovery-server VERSION 1.0.0 LANGUAGES C CXX)

###############################################################################
# Load external dependencies
###############################################################################

if(NOT fastrtps_FOUND)
    find_package(fastrtps REQUIRED)
endif()

# Set C++11
include(CheckCXXCompilerFlag)
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG OR
        CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    check_cxx_compiler_flag(-std=c++11 SUPPORTS_CXX11)
    if(SUPPORTS_CXX11)
        add_compile_options(-std=c++11)
    else()
        message(FATAL_ERROR "Compiler doesn't support C++11")
    endif()
endif()

###############################################################################
# Compilation
###############################################################################

if (MSVC)

    # For isolated builds provide some IDE aids 
    if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

        # add fast headers to enable class view on visual studio IDE
        get_target_property(FAST_INCLUDE_DIR fastrtps INTERFACE_INCLUDE_DIRECTORIES)
        list(GET FAST_INCLUDE_DIR 0 FAST_INCLUDE_DIR)

        file(GLOB_RECURSE ALL_FAST_HEADERS
            "${FAST_INCLUDE_DIR}/**/*.h"
            "${FAST_INCLUDE_DIR}/**/*.hpp"
            "${FAST_INCLUDE_DIR}/**/*.hxx"
            )

        source_group("Fast-RTPS Headers" FILES ${ALL_FAST_HEADERS})

    endif()

    # add project headers to enable class view on visual studio IDE
    set(FAST_SERVER_HEADERS server.h optionparser.h) 

endif(MSVC)

add_executable(${PROJECT_NAME}
    server.cpp
    ${FAST_SERVER_HEADERS}
    ${ALL_FAST_HEADERS})

# we pass the version as a preprocessor definition
target_compile_definitions(${PROJECT_NAME} 
    PRIVATE FAST_SERVER_VERSION=\"${PROJECT_VERSION}\" 
    PRIVATE FAST_SERVER_BINARY=\"$<TARGET_FILE_BASE_NAME:${PROJECT_NAME}>\" )

target_link_libraries(${PROJECT_NAME} fastrtps fastcdr)

# Properties that change bin names depending on current config.
# This is convenient because on installation all bins share folder
set_target_properties(${PROJECT_NAME} PROPERTIES RELEASE_POSTFIX -${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX d-${PROJECT_VERSION})

###############################################################################
# Installation 
###############################################################################

# If not isolated integrate 
if(CMAKE_PROJECT_NAME STREQUAL "fastrtps" )
    set(BIN_INSTALL_DIR tools/fds/${BIN_INSTALL_DIR} CACHE PATH "Installation directory for binaries within Fast-DDS")
else()
    set(BIN_INSTALL_DIR bin/ CACHE PATH "Installation directory for binaries")
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${BIN_INSTALL_DIR} COMPONENT discovery)

# add symlink
if( WIN32 )
    # Use powershell to generate the link
    install( CODE "execute_process(COMMAND PowerShell -Command \"New-Item -ItemType SymbolicLink -Target $<TARGET_FILE_NAME:${PROJECT_NAME}> -Path ${PROJECT_NAME}$<$<CONFIG:Debug>:d>.exe\" WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/${BIN_INSTALL_DIR} )")
else()
    # Use ln to create the symbolic link. We remove the version from the file name but keep the debug suffix 
    install( CODE "execute_process(COMMAND ln -s $<TARGET_FILE_NAME:${PROJECT_NAME}> ${PROJECT_NAME}$<$<CONFIG:Debug>:d> WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/${BIN_INSTALL_DIR})")
endif()
