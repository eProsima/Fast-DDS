# Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

project(RequestReplyExample VERSION 1 LANGUAGES CXX)

# Find requirements
if(NOT fastcdr_FOUND)
    find_package(fastcdr REQUIRED)
endif()

if(NOT fastrtps_FOUND)
    find_package(fastrtps 2.12 REQUIRED)
endif()

#Check C++11
include(CheckCXXCompilerFlag)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    check_cxx_compiler_flag(-std=c++11 SUPPORTS_CXX11)
    if(NOT SUPPORTS_CXX11)
        message(FATAL_ERROR "Compiler doesn't support C++11")
    endif()
endif()

message(STATUS "Configuring Request-Reply example...")
set(DDS_REQUEST_REPLY_EXAMPLE_COMMON_SOURCES
    Calculator.cxx
    Calculatorv1.cxx
    CalculatorPubSubTypes.cxx
    )
set( DDS_REQUEST_REPLY_EXAMPLE_CLIENT_SOURCES
    CalculatorClient.cpp
    )
set( DDS_REQUEST_REPLY_EXAMPLE_SERVER_SOURCES
    CalculatorServer.cpp
    )

add_executable(DDSCalculatorClient
    ${DDS_REQUEST_REPLY_EXAMPLE_COMMON_SOURCES}
    ${DDS_REQUEST_REPLY_EXAMPLE_CLIENT_SOURCES}
    )
target_compile_definitions(DDSCalculatorClient PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
)
target_link_libraries(DDSCalculatorClient fastrtps fastcdr)
install(TARGETS DDSCalculatorClient
    RUNTIME DESTINATION examples/cpp/dds/RequestReplyExample/${BIN_INSTALL_DIR})

add_executable(DDSCalculatorServer
    ${DDS_REQUEST_REPLY_EXAMPLE_COMMON_SOURCES}
    ${DDS_REQUEST_REPLY_EXAMPLE_SERVER_SOURCES}
    )
target_compile_definitions(DDSCalculatorServer PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
)
target_link_libraries(DDSCalculatorServer fastrtps fastcdr)
install(TARGETS DDSCalculatorServer
    RUNTIME DESTINATION examples/cpp/dds/RequestReplyExample/${BIN_INSTALL_DIR})
