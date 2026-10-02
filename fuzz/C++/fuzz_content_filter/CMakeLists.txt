# Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

cmake_minimum_required(VERSION 3.22)

project(fuzz_content_filter VERSION 1 LANGUAGES CXX)

if(NOT fastcdr_FOUND)
    find_package(fastcdr 2 REQUIRED)
endif()

if(NOT foonathan_memory_FOUND)
    find_package(foonathan_memory REQUIRED)
endif()

if(NOT fastdds_FOUND)
    find_package(fastdds 3 REQUIRED)
endif()

message(STATUS "Configuring fuzz_content_filter...")
file(GLOB SOURCES_CXX "fuzz_*.cxx")

# The filter is resolved against a registered TypeObject, so the target needs a
# generated type. Rather than duplicate one, reuse the type the DDSSQLFilter
# unit test already maintains: it covers every primitive, strings, aliases,
# enums and a nested struct, which is what makes the field and value handling
# reachable.
set(CONTENT_FILTER_TYPE_DIR
    ${CMAKE_SOURCE_DIR}/test/unittest/dds/topic/DDSSQLFilter/data_types)
file(GLOB CONTENT_FILTER_TYPE_SOURCES "${CONTENT_FILTER_TYPE_DIR}/*.cxx")

add_executable(fuzz_content_filter ${SOURCES_CXX} ${CONTENT_FILTER_TYPE_SOURCES})

# DDSFilterFactory lives under src/cpp and is not in the public include tree;
# the data_types headers are included as "data_types/...", so their parent
# directory is what has to be on the search path.
target_include_directories(fuzz_content_filter PRIVATE
    ${CMAKE_SOURCE_DIR}/src/cpp
    ${CMAKE_SOURCE_DIR}/test/unittest/dds/topic/DDSSQLFilter
)
target_link_libraries(fuzz_content_filter fastdds fastcdr foonathan_memory $ENV{LIB_FUZZING_ENGINE})
