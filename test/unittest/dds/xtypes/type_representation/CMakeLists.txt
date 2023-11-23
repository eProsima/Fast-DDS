# Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

set(TYPEOBJECTUTILSTESTS_SOURCE TypeObjectUtilsTests.cpp)
set(TYPEOBJECTREGISTRYTESTS_SOURCE TypeObjectRegistryTests.cpp)

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

add_executable(TypeObjectUtilsTests ${TYPEOBJECTUTILSTESTS_SOURCE})
target_compile_definitions(TypeObjectUtilsTests PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(TypeObjectUtilsTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    )
target_link_libraries(TypeObjectUtilsTests
    fastcdr
    fastdds
    GTest::gtest
    ${CMAKE_DL_LIBS})
gtest_discover_tests(TypeObjectUtilsTests SOURCES ${TYPEOBJECTUTILSTESTS_SOURCE})

add_executable(TypeObjectRegistryTests ${TYPEOBJECTREGISTRYTESTS_SOURCE})
target_compile_definitions(TypeObjectRegistryTests PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(TypeObjectRegistryTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    )
target_link_libraries(TypeObjectRegistryTests
    fastcdr
    fastdds
    GTest::gtest
    ${CMAKE_DL_LIBS})
    gtest_discover_tests(TypeObjectRegistryTests SOURCES ${TYPEOBJECTREGISTRYTESTS_SOURCE})
