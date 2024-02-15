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
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/aliasesPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/aliasesTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/helpers/basic_inner_typesPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/helpers/basic_inner_typesTypeObjectSupport.cxx
    # idl/dds-types-test/annotationsPubSubTypes.cxx
    # idl/dds-types-test/annotationsTypeObjectSupport.cxx
    # idl/dds-types-test/appendablePubSubTypes.cxx
    # idl/dds-types-test/appendableTypeObjectSupport.cxx
    # idl/dds-types-test/arraysPubSubTypes.cxx
    # idl/dds-types-test/arraysTypeObjectSupport.cxx
    # idl/dds-types-test/bitsetsPubSubTypes.cxx
    # idl/dds-types-test/bitsetsTypeObjectSupport.cxx
    # idl/dds-types-test/constantsPubSubTypes.cxx
    # idl/dds-types-test/constantsTypeObjectSupport.cxx
    # idl/dds-types-test/declarationsPubSubTypes.cxx
    # idl/dds-types-test/enumerationsPubSubTypes.cxx
    # idl/dds-types-test/enumerationsTypeObjectSupport.cxx
    # idl/dds-types-test/externalPubSubTypes.cxx
    # idl/dds-types-test/finalPubSubTypes.cxx
    # idl/dds-types-test/finalTypeObjectSupport.cxx
    # idl/dds-types-test/inheritancePubSubTypes.cxx
    # idl/dds-types-test/inheritanceTypeObjectSupport.cxx
    # idl/dds-types-test/keyPubSubTypes.cxx
    # idl/dds-types-test/keyTypeObjectSupport.cxx
    # idl/dds-types-test/mapsPubSubTypes.cxx
    # idl/dds-types-test/mapsTypeObjectSupport.cxx
    # idl/dds-types-test/member_idPubSubTypes.cxx
    # idl/dds-types-test/member_idTypeObjectSupport.cxx
    # idl/dds-types-test/mutablePubSubTypes.cxx
    # idl/dds-types-test/mutableTypeObjectSupport.cxx
    # idl/dds-types-test/optionalPubSubTypes.cxx
    # idl/dds-types-test/optionalTypeObjectSupport.cxx
    # idl/dds-types-test/primitivesPubSubTypes.cxx
    # idl/dds-types-test/primitivesTypeObjectSupport.cxx
    # idl/dds-types-test/relative_path_includePubSubTypes.cxx
    # idl/dds-types-test/relative_path_includeTypeObjectSupport.cxx
    # idl/dds-types-test/sequencesPubSubTypes.cxx
    # idl/dds-types-test/sequencesTypeObjectSupport.cxx
    # idl/dds-types-test/stringsPubSubTypes.cxx
    # idl/dds-types-test/stringsTypeObjectSupport.cxx
    # idl/dds-types-test/structuresPubSubTypes.cxx
    # idl/dds-types-test/structuresTypeObjectSupport.cxx
    # idl/dds-types-test/unionsPubSubTypes.cxx
    # idl/dds-types-test/unionsTypeObjectSupport.cxx
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
gtest_discover_tests(DynamicTypesDDSTypesTest)

configure_file("types_profile.xml" "types_profile.xml" COPYONLY)
