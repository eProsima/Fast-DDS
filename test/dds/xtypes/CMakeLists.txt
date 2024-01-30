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

# Standar tests
set(COMMON_SOURCE
    ${PROJECT_SOURCE_DIR}/test/dds/xtypes/idl/XtypesTestsType1PubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/dds/xtypes/idl/XtypesTestsType1TypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/test/dds/xtypes/idl/XtypesTestsType2PubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/dds/xtypes/idl/XtypesTestsType2TypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/test/dds/xtypes/idl/XtypesTestsType3PubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/dds/xtypes/idl/XtypesTestsType3TypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/test/dds/xtypes/idl/XtypesTestsTypeBigPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/dds/xtypes/idl/XtypesTestsTypeBigTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/test/dds/xtypes/idl/XtypesTestsTypeDepPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/dds/xtypes/idl/XtypesTestsTypeDepTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/test/dds/xtypes/idl/XtypesTestsTypeNoTypeObjectPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/xtypes/type_representation/TypeObjectUtils.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/RTPSDomain.cpp
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
target_link_libraries(DDSXtypesCommunication fastrtps fastcdr foonathan_memory ${CMAKE_DL_LIBS})

###############################################################################
# Necessary files
###############################################################################
# Standard tests
list(APPEND TEST_DEFINITIONS
    Case0
    Case1
    Case2
    Case3
    Case4
    Case5
    Case6
    Case7
    Case8
    Case9
    Case10
    )
    
# Python file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_build.py
    ${CMAKE_CURRENT_BINARY_DIR}/test_build.py COPYONLY)

# JSON files
foreach(TEST_FILE IN LISTS TEST_DEFINITIONS)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${TEST_FILE}.json
        ${CMAKE_CURRENT_BINARY_DIR}/${TEST_FILE}.json COPYONLY)
endforeach()

###############################################################################
# Tests specification
###############################################################################
if(PYTHONINTERP_FOUND)
    # Standard types test
    set(TEST_BUILDER ${BINARY_TEST_DIR}test_build.py)

    foreach(TEST_DEFINITION IN LISTS TEST_DEFINITIONS)
        set(TEST_NAME DDSXtypesCommunication_${TEST_DEFINITION})
        add_test(
            NAME ${TEST_NAME}
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/${TEST_BUILDER}
            ${TEST_DEFINITION}.json
            )

        if(WIN32)
            string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
            set_property(TEST ${TEST_NAME} APPEND PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
        endif()
    endforeach()
endif()
