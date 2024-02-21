# Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

if(WIN32)
    add_definitions(
        -D_WIN32_WINNT=0x0601
        -D_CRT_SECURE_NO_WARNINGS
        )
endif()

set(DYNAMIC_TYPES_TEST_SOURCE
    DynamicTypesTests.cpp
    idl/BasicPubSubTypes.cxx
    idl/BasicTypeObjectSupport.cxx
    )

set(DYNAMIC_COMPLEX_TYPES_TEST_SOURCE
    DynamicComplexTypesTests.cpp
    idl/TestPubSubTypes.cxx
    idl/TestTypeObjectSupport.cxx
    )

set(DYNAMIC_TYPES_4_2_TEST_SOURCE
    DynamicTypes_4_2_Tests.cpp
    idl/new_features_4_2PubSubTypes.cxx
    idl/new_features_4_2TypeObjectSupport.cxx
    )

include_directories(mock/)

add_executable(DynamicTypesTests ${DYNAMIC_TYPES_TEST_SOURCE})
target_compile_definitions(DynamicTypesTests PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_link_libraries(DynamicTypesTests
    GTest::gtest
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    $<$<BOOL:${WIN32}>:ws2_32>
    $<$<BOOL:${LINK_SSL}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto>
    fastcdr
    fastrtps
    )
if(QNX)
    target_link_libraries(DynamicTypesTests socket)
endif()
gtest_discover_tests(DynamicTypesTests)

add_executable(DynamicComplexTypesTests ${DYNAMIC_COMPLEX_TYPES_TEST_SOURCE})
target_compile_definitions(DynamicComplexTypesTests PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )

target_link_libraries(DynamicComplexTypesTests
    GTest::gtest
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    $<$<BOOL:${WIN32}>:ws2_32>
    fastcdr
    fastrtps
    )
if(QNX)
    target_link_libraries(DynamicComplexTypesTests socket)
endif()
gtest_discover_tests(DynamicComplexTypesTests)


add_executable(DynamicTypes_4_2_Tests ${DYNAMIC_TYPES_4_2_TEST_SOURCE})
target_compile_definitions(DynamicTypes_4_2_Tests PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )

target_link_libraries(DynamicTypes_4_2_Tests
    GTest::gtest
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    $<$<BOOL:${WIN32}>:ws2_32>
    fastcdr
    fastrtps
    )
if(QNX)
    target_link_libraries(DynamicTypes_4_2_Tests socket)
endif()
gtest_discover_tests(DynamicTypes_4_2_Tests)

configure_file("types_profile.xml" "types_profile.xml" COPYONLY)
