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

find_package(PythonInterp 3)

add_definitions(
    -DBOOST_ASIO_STANDALONE
    -DASIO_STANDALONE
    )

include_directories(${Asio_INCLUDE_DIR})

###############################################################################
# Binaries
###############################################################################

file(GLOB COMMON_SOURCE
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/helpers/basic_inner_typesPubSubTypes.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/helpers/basic_inner_typesTypeObjectSupport.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/aliasesPubSubTypes.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/aliasesTypeObjectSupport.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/annotationsPubSubTypes.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/annotationsTypeObjectSupport.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/appendablePubSubTypes.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/appendableTypeObjectSupport.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/arraysPubSubTypes.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/arraysTypeObjectSupport.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/bitsetsPubSubTypes.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/bitsetsTypeObjectSupport.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/constantsPubSubTypes.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/constantsTypeObjectSupport.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/declarationsPubSubTypes.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/enumerationsPubSubTypes.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/enumerationsTypeObjectSupport.cxx
    # ${FASTDDS_BASIC_DIR}/test/dds-types-test/externalPubSubTypes.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/finalPubSubTypes.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/finalTypeObjectSupport.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/inheritancePubSubTypes.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/inheritanceTypeObjectSupport.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/keyPubSubTypes.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/keyTypeObjectSupport.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/mapsPubSubTypes.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/mapsTypeObjectSupport.cxx
    # ${FASTDDS_BASIC_DIR}/test/dds-types-test/member_idPubSubTypes.cxx
    # ${FASTDDS_BASIC_DIR}/test/dds-types-test/member_idTypeObjectSupport.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/mutablePubSubTypes.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/mutableTypeObjectSupport.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/optionalPubSubTypes.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/optionalTypeObjectSupport.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/primitivesPubSubTypes.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/primitivesTypeObjectSupport.cxx
    # ${FASTDDS_BASIC_DIR}/test/dds-types-test/relative_path_includePubSubTypes.cxx
    # ${FASTDDS_BASIC_DIR}/test/dds-types-test/relative_path_includeTypeObjectSupport.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/sequencesPubSubTypes.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/sequencesTypeObjectSupport.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/stringsPubSubTypes.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/stringsTypeObjectSupport.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/structuresPubSubTypes.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/structuresTypeObjectSupport.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/unionsPubSubTypes.cxx
    ${FASTDDS_BASIC_DIR}/test/dds-types-test/unionsTypeObjectSupport.cxx
    ${FASTDDS_BASIC_DIR}/test/dds/xtypes/BaseCasesIDLs/*.cxx
)

set(DDS_XTYPES_SOURCE ${COMMON_SOURCE}
    TypeLookupService_main.cpp
    TypeLookupServicePublisher.cpp
    TypeLookupServiceSubscriber.cpp
    )

if(WIN32)
    set_source_files_properties(${FASTDDS_BASIC_DIR}/test/dds-types-test/mapsPubSubTypes.cxx PROPERTIES COMPILE_OPTIONS /bigobj)
    set_source_files_properties(TypeLookupServicePublisher.cpp PROPERTIES COMPILE_OPTIONS /bigobj)
    set_source_files_properties(TypeLookupServiceSubscriber.cpp PROPERTIES COMPILE_OPTIONS /bigobj)
endif()

add_executable(DDSXtypesCommunication ${DDS_XTYPES_SOURCE})
target_compile_definitions(DDSXtypesCommunication PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(DDSXtypesCommunication PRIVATE
    ${FASTDDS_BASIC_DIR}/test/dds/xtypes
)

target_link_libraries(DDSXtypesCommunication
    fastcdr
    fastdds
    foonathan_memory
    ${CMAKE_DL_LIBS}
    )

###############################################################################
# Tests specification
###############################################################################
# Python script
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_build.py
    ${CMAKE_CURRENT_BINARY_DIR}/test_build.py COPYONLY)

# Get test definitions from the json files
file(GLOB TEST_DEFINITIONS
    "BaseCases/Case*"
    "TypesTestsCases/Cases*"
)

if(PYTHONINTERP_FOUND)
    # Standard types test
    set(TEST_BUILDER ${BINARY_TEST_DIR}test_build.py)

    foreach(TEST_DEFINITION IN LISTS TEST_DEFINITIONS)
        set(TEST_NAME DDSXtypesCommunication_${TEST_DEFINITION})
        add_test(
            NAME ${TEST_NAME}
            COMMAND
            ${PYTHON_EXECUTABLE}
            ${CMAKE_CURRENT_BINARY_DIR}/${TEST_BUILDER}
            --app $<TARGET_FILE:DDSXtypesCommunication>
            ${TEST_DEFINITION}
            )

        if(WIN32)
            string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
            set_property(TEST ${TEST_NAME} APPEND PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
        endif()
    endforeach()
endif()
