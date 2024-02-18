# Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/*.cxx
    ${PROJECT_SOURCE_DIR}/test/dds-types-test/helpers/*.cxx
    ${PROJECT_SOURCE_DIR}/test/dds/xtypes/BaseCasesIDLs/*.cxx
)

set(DDS_XTYPES_SOURCE ${COMMON_SOURCE}
    TypeLookupService_main.cpp
    TypeLookupServicePublisher.cpp
    TypeLookupServiceSubscriber.cpp
    )

add_executable(DDSXtypesCommunication ${DDS_XTYPES_SOURCE})
target_compile_definitions(DDSXtypesCommunication PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(DDSXtypesCommunication PRIVATE
    ${PROJECT_SOURCE_DIR}/test/dds/xtypes
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
    "TypesTestsCases/Case*"
)

if(PYTHONINTERP_FOUND)
    # Standard types test
    set(TEST_BUILDER ${BINARY_TEST_DIR}test_build.py)

    foreach(TEST_DEFINITION IN LISTS TEST_DEFINITIONS)
        set(TEST_NAME DDSXtypesCommunication_${TEST_DEFINITION})
        add_test(
            NAME ${TEST_NAME}
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/${TEST_BUILDER}
            ${TEST_DEFINITION}
            )

        if(WIN32)
            string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
            set_property(TEST ${TEST_NAME} APPEND PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
        endif()
    endforeach()
endif()
