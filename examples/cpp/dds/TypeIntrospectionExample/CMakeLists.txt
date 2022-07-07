# Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
    cmake_policy(SET CMP0028 NEW)
endif()

project(TypeIntrospectionExample)

# Find requirements
if(NOT fastcdr_FOUND)
    find_package(fastcdr REQUIRED)
endif()

if(NOT fastrtps_FOUND)
    find_package(fastrtps REQUIRED)
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

# Get executable sources
message(STATUS "Configuring TypeIntrospection example...")

file(GLOB IMPLEMENTATION_SOURCES_CPP "*.cpp")
file(GLOB DATATYPEBASE_SOURCES_CPP "types/*.cpp")
# file(GLOB DATATYPE_HELLOWORLD_SOURCES_CPP "types/hello_world/**/*.cpp")
# file(GLOB DATATYPE_HELLOWORLD_SOURCES_CXX "types/hello_world/**/*.cxx")
# file(GLOB DATATYPE_ARRAY_SOURCES_CPP "types/array/**/*.cpp")
# file(GLOB DATATYPE_ARRAY_SOURCES_CXX "types/array/**/*.cxx")
# file(GLOB DATATYPE_STRUCT_SOURCES_CPP "types/struct/**/*.cpp")
# file(GLOB DATATYPE_STRUCT_SOURCES_CXX "types/struct/**/*.cxx")
file(GLOB DATATYPE_SOURCES_CPP "types/**/**/*.cpp")
file(GLOB DATATYPE_SOURCES_CXX "types/**/**/*.cxx")

set(
    EXAMPLE_SOURCES
        ${IMPLEMENTATION_SOURCES_CPP}
        ${DATATYPEBASE_SOURCES_CPP}
        ${DATATYPE_SOURCES_CPP}
        ${DATATYPE_SOURCES_CXX}
    )

# Create executable
add_executable(${PROJECT_NAME} ${EXAMPLE_SOURCES})
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
)
target_link_libraries(${PROJECT_NAME} fastrtps fastcdr fastdds::optionparser)
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION examples/C++/DDS/${PROJECT_NAME}/${BIN_INSTALL_DIR})


file(GLOB DATATYPE_XML "types/**/xml/*.xml")

# Copy required files
file(
    COPY
        ${DATATYPE_XML}
    DESTINATION
        ${PROJECT_BINARY_DIR}
    )
