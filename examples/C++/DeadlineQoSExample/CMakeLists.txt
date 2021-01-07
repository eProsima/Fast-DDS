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

cmake_minimum_required(VERSION 2.8.12)

if(NOT CMAKE_VERSION VERSION_LESS 3.0)
    cmake_policy(SET CMP0048 NEW)
endif()

project(DeadlineQoSExample)

# Find requirements
if(NOT fastcdr_FOUND)
    find_package(fastcdr REQUIRED)
endif()

if(NOT foonathan_memory_FOUND)
    find_package(foonathan_memory REQUIRED)
endif()

if(NOT fastrtps_FOUND)
    find_package(fastrtps REQUIRED)
endif()

if(NOT Asio_FOUND)
    find_package(Asio QUIET)

    if(Asio_FOUND)
        message(STATUS "Found Asio")
    else()
        find_path(ASIO_INCLUDE_DIR NAMES asio.hpp)
        if(ASIO_INCLUDE_DIR)
            message(STATUS "Found Asio in ${ASIO_INCLUDE_DIR}")
        else()
            message(FATAL_ERROR "Asio not found!")
        endif()
    endif()
endif()

#Check C++11
include(CheckCXXCompilerFlag)
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG OR
        CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    check_cxx_compiler_flag(-std=c++11 SUPPORTS_CXX11)
    if(NOT SUPPORTS_CXX11)
        message(FATAL_ERROR "Compiler doesn't support C++11")
    endif()
endif()

message(STATUS "Configuring DeadlineQoS example...")
file(GLOB DEADLINEQOS_EXAMPLE_SOURCES "*.cxx")

add_definitions(
  -DBOOST_ASIO_STANDALONE
  -DASIO_STANDALONE
)

include_directories(${ASIO_INCLUDE_DIR})

add_executable(DeadlineQoSExample ${DEADLINEQOS_EXAMPLE_SOURCES})
target_compile_definitions(DeadlineQoSExample PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
)
target_link_libraries(DeadlineQoSExample fastrtps fastcdr foonathan_memory)
if(UNIX)
  target_link_libraries(DeadlineQoSExample pthread)
endif()

install(TARGETS DeadlineQoSExample
    RUNTIME DESTINATION examples/C++/DeadlineQoSExample/${BIN_INSTALL_DIR}
    )
