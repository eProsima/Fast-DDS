# Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

project(DDSDynamicHelloWorldExample VERSION 1 LANGUAGES CXX)

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
    check_cxx_compiler_flag(-std=c++11 SUPPORTS_CXX11)
    if(NOT SUPPORTS_CXX11)
        message(FATAL_ERROR "Compiler doesn't support C++11")
    endif()
endif()

message(STATUS "Configuring HelloWorld example...")
file(GLOB DDS_DYNAMIC_HELLOWORLD_EXAMPLE_SOURCES_CPP "*.cpp")

add_executable(DDSDynamicHelloWorldExample
    ${DDS_DYNAMIC_HELLOWORLD_EXAMPLE_SOURCES_CPP})

target_compile_definitions(DDSDynamicHelloWorldExample PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
)

target_link_libraries(DDSDynamicHelloWorldExample fastrtps fastcdr)
install(TARGETS DDSDynamicHelloWorldExample
    RUNTIME DESTINATION examples/cpp/dds/DynamicHelloWorldExample/${BIN_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/helloworld_example_type_profile.xml
    DESTINATION examples/cpp/dds/DynamicHelloWorldExample/${BIN_INSTALL_DIR})

file(COPY helloworld_example_type_profile.xml DESTINATION ${PROJECT_BINARY_DIR})
