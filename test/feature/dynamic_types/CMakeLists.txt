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
    dds_types_tests/DynamicTypesAliasesDDSTypesTests.cpp
    dds_types_tests/DynamicTypesAnnotationsDDSTypesTests.cpp
    dds_types_tests/DynamicTypesAppendableDDSTypesTests.cpp
    dds_types_tests/DynamicTypesArraysDDSTypesTests.cpp
    dds_types_tests/DynamicTypesBitsetsDDSTypesTests.cpp
    dds_types_tests/DynamicTypesEnumerationsDDSTypesTests.cpp
    dds_types_tests/DynamicTypesFinalDDSTypesTests.cpp
    dds_types_tests/DynamicTypesInheritanceDDSTypesTests.cpp
    dds_types_tests/DynamicTypesKeyDDSTypesTests.cpp
    dds_types_tests/DynamicTypesMapsDDSTypesTests.cpp
    dds_types_tests/DynamicTypesMutableDDSTypesTests.cpp
    dds_types_tests/DynamicTypesPrimitivesDDSTypesTests.cpp
    dds_types_tests/DynamicTypesSequencesDDSTypesTests.cpp
    dds_types_tests/DynamicTypesStringsDDSTypesTests.cpp
    dds_types_tests/DynamicTypesStructuresDDSTypesTests.cpp
    dds_types_tests/DynamicTypesUnionsDDSTypesTests.cpp
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/aliasesPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/aliasesTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/annotationsPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/annotationsTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/appendablePubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/appendableTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/arraysPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/arraysTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/bitsetsPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/bitsetsTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/enumerationsPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/enumerationsTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/finalPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/finalTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/helpers/basic_inner_typesPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/helpers/basic_inner_typesTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/inheritancePubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/inheritanceTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/keyPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/keyTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/mapsPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/mapsTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/mutablePubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/mutableTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/primitivesPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/primitivesTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/sequencesPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/sequencesTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/stringsPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/stringsTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/structuresPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/structuresTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/unionsPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/unionsTypeObjectSupport.cxx
    # ${PROJECT_SOURCE_DIR}/test/dds-types-test/constantsPubSubTypes.cxx
    # ${PROJECT_SOURCE_DIR}/test/dds-types-test/constantsTypeObjectSupport.cxx
    # ${PROJECT_SOURCE_DIR}/test/dds-types-test/declarationsPubSubTypes.cxx
    # ${PROJECT_SOURCE_DIR}/test/dds-types-test/externalPubSubTypes.cxx
    # ${PROJECT_SOURCE_DIR}/test/dds-types-test/member_idPubSubTypes.cxx
    # ${PROJECT_SOURCE_DIR}/test/dds-types-test/member_idTypeObjectSupport.cxx
    # ${PROJECT_SOURCE_DIR}/test/dds-types-test/optionalPubSubTypes.cxx
    # ${PROJECT_SOURCE_DIR}/test/dds-types-test/optionalTypeObjectSupport.cxx
    # ${PROJECT_SOURCE_DIR}/test/dds-types-test/relative_path_includePubSubTypes.cxx
    # ${PROJECT_SOURCE_DIR}/test/dds-types-test/relative_path_includeTypeObjectSupport.cxx
    )

if(WIN32)
    set_source_files_properties(dds_types_tests/DynamicTypesMapsDDSTypesTests.cpp PROPERTIES COMPILE_OPTIONS /bigobj)
    set_source_files_properties(${PROJECT_SOURCE_DIR}/test/dds-types-test/mapsPubSubTypes.cxx PROPERTIES COMPILE_OPTIONS /bigobj)
endif()

add_executable(DynamicTypesTests DynamicTypesTests.cpp)
target_compile_definitions(DynamicTypesTests PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(DynamicTypesTests
    PRIVATE
    ${PROJECT_SOURCE_DIR}/test/utils/
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
    fastdds
    )
gtest_discover_tests(DynamicTypesDDSTypesTest)

configure_file("types_profile.xml" "types_profile.xml" COPYONLY)
