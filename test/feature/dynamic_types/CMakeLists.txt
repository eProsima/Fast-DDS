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

set(DYNAMIC_TYPES_DDS_TYPES_TEST_SOURCE
    DynamicTypesDDSTypesTest.cpp
    idl/dds-types-test/helpers/basic_inner_types.cxx
    idl/dds-types-test/helpers/basic_inner_typesPubSubTypes.cxx
    idl/dds-types-test/helpers/basic_inner_typesTypeObject.cxx
    idl/dds-types-test/aliases.cxx
    idl/dds-types-test/aliasesPubSubTypes.cxx
    idl/dds-types-test/aliasesTypeObject.cxx
    idl/dds-types-test/appendable.cxx
    idl/dds-types-test/appendablePubSubTypes.cxx
    idl/dds-types-test/appendableTypeObject.cxx
    idl/dds-types-test/arrays.cxx
    idl/dds-types-test/arraysPubSubTypes.cxx
    idl/dds-types-test/arraysTypeObject.cxx
    idl/dds-types-test/bitsets.cxx
    idl/dds-types-test/bitsetsPubSubTypes.cxx
    idl/dds-types-test/bitsetsTypeObject.cxx
    idl/dds-types-test/declarations.cxx
    idl/dds-types-test/declarationsPubSubTypes.cxx
    idl/dds-types-test/declarationsTypeObject.cxx
    idl/dds-types-test/enumerations.cxx
    idl/dds-types-test/enumerationsPubSubTypes.cxx
    idl/dds-types-test/enumerationsTypeObject.cxx
    idl/dds-types-test/final.cxx
    idl/dds-types-test/finalPubSubTypes.cxx
    idl/dds-types-test/finalTypeObject.cxx
    idl/dds-types-test/inheritance.cxx
    idl/dds-types-test/inheritancePubSubTypes.cxx
    idl/dds-types-test/inheritanceTypeObject.cxx
    idl/dds-types-test/maps.cxx
    idl/dds-types-test/mapsPubSubTypes.cxx
    idl/dds-types-test/mapsTypeObject.cxx
    idl/dds-types-test/primitives.cxx
    idl/dds-types-test/primitivesPubSubTypes.cxx
    idl/dds-types-test/primitivesTypeObject.cxx
    idl/dds-types-test/sequences.cxx
    idl/dds-types-test/sequencesPubSubTypes.cxx
    idl/dds-types-test/sequencesTypeObject.cxx
    idl/dds-types-test/strings.cxx
    idl/dds-types-test/stringsPubSubTypes.cxx
    idl/dds-types-test/stringsTypeObject.cxx
    idl/dds-types-test/structures.cxx
    idl/dds-types-test/structuresPubSubTypes.cxx
    idl/dds-types-test/structuresTypeObject.cxx
    idl/dds-types-test/unions.cxx
    idl/dds-types-test/unionsPubSubTypes.cxx
    idl/dds-types-test/unionsTypeObject.cxx
    )

add_executable(DynamicTypesTests DynamicTypesTests.cpp)
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
    fastdds
    )
if(QNX)
    target_link_libraries(DynamicTypesTests socket)
endif()
gtest_discover_tests(DynamicTypesTests)

add_executable(DynamicTypesDDSTypesTest ${DYNAMIC_TYPES_DDS_TYPES_TEST_SOURCE})
target_compile_definitions(DynamicTypesDDSTypesTest PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_link_libraries(DynamicTypesDDSTypesTest GTest::gtest
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    $<$<BOOL:${WIN32}>:ws2_32>
    fastcdr
    fastrtps
    )
add_gtest(DynamicTypesDDSTypesTest SOURCES ${DYNAMIC_TYPES_DDS_TYPES_TEST_SOURCE})

configure_file("types_profile.xml" "types_profile.xml" COPYONLY)
