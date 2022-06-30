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

cmake_minimum_required(VERSION 3.16.3)

project(Configurability VERSION 1 LANGUAGES CXX)

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

#Check C++11
include(CheckCXXCompilerFlag)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    check_cxx_compiler_flag(-std=c++11 SUPPORTS_CXX11)
    check_cxx_compiler_flag(-std=c++11 SUPPORTS_CXX11)
    if(NOT SUPPORTS_CXX11)
        message(FATAL_ERROR "Compiler doesn't support C++11")
    endif()
endif()

message(STATUS "Configuring DDS Configurability example...")
file(GLOB DDS_CONFIGURABILITY_EXAMPLE_SOURCES "*.cxx")

add_executable(DDSConfigurabilityPublisher ${DDS_CONFIGURABILITY_EXAMPLE_SOURCES} ConfigurabilityPublisher.cpp)
target_compile_definitions(DDSConfigurabilityPublisher PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
)
target_link_libraries(DDSConfigurabilityPublisher fastrtps fastcdr foonathan_memory)
install(TARGETS DDSConfigurabilityPublisher
    RUNTIME DESTINATION examples/C++/DDS/Configurability/${BIN_INSTALL_DIR}
    )

add_executable(DDSConfigurabilitySubscriber ${DDS_CONFIGURABILITY_EXAMPLE_SOURCES} ConfigurabilitySubscriber.cpp)
target_compile_definitions(DDSConfigurabilitySubscriber PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
)
target_link_libraries(DDSConfigurabilitySubscriber fastrtps fastcdr foonathan_memory)
install(TARGETS DDSConfigurabilitySubscriber
    RUNTIME DESTINATION examples/C++/DDS/Configurability/${BIN_INSTALL_DIR}
    )
