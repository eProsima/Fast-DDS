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

set(RESOURCEIDLTOJSONTESTS_SOURCE
    idl_to_json_Tests.cpp
    ${PROJECT_SOURCE_DIR}/test/unittest/dds/xtypes/dyndata_idl/types/types.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/xtypes/utils.cpp)

file(GLOB DATATYPEBASE_SOURCES_CPP "types/*.cpp")
file(GLOB DATATYPE_SOURCES_CPP "types/**/**/*.cpp")
file(GLOB DATATYPE_SOURCES_CXX "types/**/**/*.cxx")

set(
    EXAMPLE_SOURCES
        ${IMPLEMENTATION_SOURCES_CPP}
        ${DATATYPEBASE_SOURCES_CPP}
        ${DATATYPE_SOURCES_CPP}
        ${DATATYPE_SOURCES_CXX}
    )

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

add_executable(idl_to_json_Tests ${RESOURCEIDLTOJSONTESTS_SOURCE} ${EXAMPLE_SOURCES})
target_compile_definitions(idl_to_json_Tests PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(idl_to_json_Tests PRIVATE
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include)
target_link_libraries(idl_to_json_Tests
    fastdds
    fastcdr
    GTest::gtest
    )
gtest_discover_tests(idl_to_json_Tests)
