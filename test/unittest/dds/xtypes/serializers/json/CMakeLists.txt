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
    ${CMAKE_CURRENT_SOURCE_DIR}/types/comprehensive_type/json/ComprehensiveType_Unfilled_EPROSIMA.json
    ${CMAKE_CURRENT_BINARY_DIR}/types/comprehensive_type/json/ComprehensiveType_Unfilled_EPROSIMA.json
    COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/types/comprehensive_type/json/ComprehensiveType_Filled_EPROSIMA_1.json
    ${CMAKE_CURRENT_BINARY_DIR}/types/comprehensive_type/json/ComprehensiveType_Filled_EPROSIMA_1.json
    COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/types/comprehensive_type/json/ComprehensiveType_Filled_EPROSIMA_2.json
    ${CMAKE_CURRENT_BINARY_DIR}/types/comprehensive_type/json/ComprehensiveType_Filled_EPROSIMA_2.json
    COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/types/comprehensive_type/json/ComprehensiveType_Filled_EPROSIMA_3.json
    ${CMAKE_CURRENT_BINARY_DIR}/types/comprehensive_type/json/ComprehensiveType_Filled_EPROSIMA_3.json
    COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/types/comprehensive_type/json/ComprehensiveType_Unfilled_OMG.json
    ${CMAKE_CURRENT_BINARY_DIR}/types/comprehensive_type/json/ComprehensiveType_Unfilled_OMG.json
    COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/types/comprehensive_type/json/ComprehensiveType_Filled_OMG_1.json
    ${CMAKE_CURRENT_BINARY_DIR}/types/comprehensive_type/json/ComprehensiveType_Filled_OMG_1.json
    COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/types/comprehensive_type/json/ComprehensiveType_Filled_OMG_2.json
    ${CMAKE_CURRENT_BINARY_DIR}/types/comprehensive_type/json/ComprehensiveType_Filled_OMG_2.json
    COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/types/comprehensive_type/json/ComprehensiveType_Filled_OMG_3.json
    ${CMAKE_CURRENT_BINARY_DIR}/types/comprehensive_type/json/ComprehensiveType_Filled_OMG_3.json
    COPYONLY)

set(SERIALIZE_JSON_SOURCE
    DynDataJSONTests.cpp)

file(GLOB DATATYPE_IMPLEMENTATION_CPP "types/**/implementation/*.cpp")
file(GLOB DATATYPE_SOURCES_CXX "types/**/gen/*.cxx")

set(
    EXAMPLE_SOURCES
        ${DATATYPE_IMPLEMENTATION_CPP}
        ${DATATYPE_SOURCES_CXX}
    )

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

add_executable(DynDataJSONTests ${SERIALIZE_JSON_SOURCE} ${EXAMPLE_SOURCES})
target_compile_definitions(DynDataJSONTests PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(DynDataJSONTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include)
target_link_libraries(DynDataJSONTests
    fastdds
    fastcdr
    GTest::gtest
    )
gtest_discover_tests(DynDataJSONTests)
