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

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/types/idls/hello_world.idl
    ${CMAKE_CURRENT_BINARY_DIR}/types/idls/hello_world.idl
    COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/types/idls/numeric_array.idl
    ${CMAKE_CURRENT_BINARY_DIR}/types/idls/numeric_array.idl
    COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/types/idls/char_sequence.idl
    ${CMAKE_CURRENT_BINARY_DIR}/types/idls/char_sequence.idl
    COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/types/idls/basic_struct.idl
    ${CMAKE_CURRENT_BINARY_DIR}/types/idls/basic_struct.idl
    COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/types/idls/basic_array_struct.idl
    ${CMAKE_CURRENT_BINARY_DIR}/types/idls/basic_array_struct.idl
    COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/types/idls/float_bounded_sequence.idl
    ${CMAKE_CURRENT_BINARY_DIR}/types/idls/float_bounded_sequence.idl
    COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/types/idls/arrays_and_sequences.idl
    ${CMAKE_CURRENT_BINARY_DIR}/types/idls/arrays_and_sequences.idl
    COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/types/idls/complex_nested_arrays.idl
    ${CMAKE_CURRENT_BINARY_DIR}/types/idls/complex_nested_arrays.idl
    COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/types/idls/enum_struct.idl
    ${CMAKE_CURRENT_BINARY_DIR}/types/idls/enum_struct.idl
    COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/types/idls/union_struct.idl
    ${CMAKE_CURRENT_BINARY_DIR}/types/idls/union_struct.idl
    COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/types/idls/map_struct.idl
    ${CMAKE_CURRENT_BINARY_DIR}/types/idls/map_struct.idl
    COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/types/idls/bitset_struct.idl
    ${CMAKE_CURRENT_BINARY_DIR}/types/idls/bitset_struct.idl
    COPYONLY)

set(RESOURCEDYNTYPETOIDLTESTS_SOURCE
    DynamicTypeSerializationTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/xtypes/utils.cpp)

file(GLOB DATATYPE_SOURCES_CPP "types/type_objects/**/*.cpp")
file(GLOB DATATYPE_SOURCES_CXX "types/type_objects/**/*.cxx")

set(
    DATATYPE_SOURCES
        ${DATATYPE_SOURCES_CPP}
        ${DATATYPE_SOURCES_CXX}
    )

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

add_executable(DynamicTypeSerializationTests ${RESOURCEDYNTYPETOIDLTESTS_SOURCE} ${DATATYPE_SOURCES})
target_compile_definitions(DynamicTypeSerializationTests PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(DynamicTypeSerializationTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    )
target_link_libraries(DynamicTypeSerializationTests
    fastcdr
    fastdds
    GTest::gtest
    ${CMAKE_DL_LIBS})
gtest_discover_tests(DynamicTypeSerializationTests SOURCES ${TYPEOBJECTUTILSTESTS_SOURCE})
