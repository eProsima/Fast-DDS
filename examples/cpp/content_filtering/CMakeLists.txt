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

project(fastdds_content_filtering_example VERSION 1 LANGUAGES CXX)

# Find requirements
if(NOT fastcdr_FOUND)
    find_package(fastcdr 2 REQUIRED)
endif()

if(NOT fastdds_FOUND)
    find_package(fastdds 3 REQUIRED)
endif()

message(STATUS "Configuring content filtering example...")

file(GLOB HELLO_WORLD_SOURCES_CXX "*.cxx")
file(GLOB HELLO_WORLD_SOURCES_CPP "*.cpp")

add_executable(content_filtering ${HELLO_WORLD_SOURCES_CXX} ${HELLO_WORLD_SOURCES_CPP})
target_compile_definitions(content_filtering PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
)
target_compile_features(content_filtering PRIVATE cxx_std_11)
target_link_libraries(content_filtering fastdds fastcdr fastdds::optionparser)
install(TARGETS content_filtering
    RUNTIME DESTINATION ${DATA_INSTALL_DIR}/fastdds/examples/cpp/content_filtering/${BIN_INSTALL_DIR})
