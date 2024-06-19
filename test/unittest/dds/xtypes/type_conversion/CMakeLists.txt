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

set(RESOURCEDYNTYPETOIDLTESTS_SOURCE
    DynTypeToIdlTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/xtypes/utils.cpp)

file(GLOB DATATYPE_SOURCES_CPP "types/type_objects/**/*.cpp")
file(GLOB DATATYPE_SOURCES_CXX "types/type_objects/**/*.cxx")
file(GLOB DATATYPE_SOURCES_IDL "types/idls/*.idl")

set(
    DATATYPE_SOURCES
        ${DATATYPE_SOURCES_CPP}
        ${DATATYPE_SOURCES_CXX}
        ${DATATYPE_SOURCES_IDL}
    )

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

add_executable(DynTypeToIdlTests ${RESOURCEDYNTYPETOIDLTESTS_SOURCE} ${DATATYPE_SOURCES})
target_compile_definitions(DynTypeToIdlTests PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(DynTypeToIdlTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    )
target_link_libraries(DynTypeToIdlTests
    fastcdr
    fastdds
    GTest::gtest
    ${CMAKE_DL_LIBS})
gtest_discover_tests(DynTypeToIdlTests SOURCES ${TYPEOBJECTUTILSTESTS_SOURCE})
