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
    ${CMAKE_CURRENT_SOURCE_DIR}/types/alias_struct/alias_struct.idl
    ${CMAKE_CURRENT_BINARY_DIR}/types/alias_struct/alias_struct.idl
    COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/types/array_struct/array_struct.idl
    ${CMAKE_CURRENT_BINARY_DIR}/types/array_struct/array_struct.idl
    COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/types/bitmask_struct/bitmask_struct.idl
    ${CMAKE_CURRENT_BINARY_DIR}/types/bitmask_struct/bitmask_struct.idl
    COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/types/bitset_struct/bitset_struct.idl
    ${CMAKE_CURRENT_BINARY_DIR}/types/bitset_struct/bitset_struct.idl
    COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/types/enum_struct/enum_struct.idl
    ${CMAKE_CURRENT_BINARY_DIR}/types/enum_struct/enum_struct.idl
    COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/types/extensibility_struct/extensibility_struct.idl
    ${CMAKE_CURRENT_BINARY_DIR}/types/extensibility_struct/extensibility_struct.idl
    COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/types/key_struct/key_struct.idl
    ${CMAKE_CURRENT_BINARY_DIR}/types/key_struct/key_struct.idl
    COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/types/map_struct/map_struct.idl
    ${CMAKE_CURRENT_BINARY_DIR}/types/map_struct/map_struct.idl
    COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/types/sequence_struct/sequence_struct.idl
    ${CMAKE_CURRENT_BINARY_DIR}/types/sequence_struct/sequence_struct.idl
    COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/types/string_struct/string_struct.idl
    ${CMAKE_CURRENT_BINARY_DIR}/types/string_struct/string_struct.idl
    COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/types/struct_struct/struct_struct.idl
    ${CMAKE_CURRENT_BINARY_DIR}/types/struct_struct/struct_struct.idl
    COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/types/union_struct/union_struct.idl
    ${CMAKE_CURRENT_BINARY_DIR}/types/union_struct/union_struct.idl
    COPYONLY)

set(RESOURCEDYNTYPETOIDLTESTS_SOURCE
    DynTypeIDLTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/xtypes/utils.cpp)

file(GLOB DATATYPE_SOURCES "types/**/gen/*.cxx")

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

add_executable(DynTypeIDLTests ${RESOURCEDYNTYPETOIDLTESTS_SOURCE} ${DATATYPE_SOURCES})
target_compile_definitions(DynTypeIDLTests PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(DynTypeIDLTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    )
target_link_libraries(DynTypeIDLTests
    fastcdr
    fastdds
    GTest::gtest
    ${CMAKE_DL_LIBS})
gtest_discover_tests(DynTypeIDLTests SOURCES ${TYPEOBJECTUTILSTESTS_SOURCE})
