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

# Recursively find all .idl files in the types directory
file(GLOB_RECURSE IDL_FILES ${CMAKE_CURRENT_SOURCE_DIR}/types/*.idl)

# Iterate over each .idl file found
foreach(IDL_FILE ${IDL_FILES})
    # Get the relative path of the .idl file
    file(RELATIVE_PATH RELATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${IDL_FILE})

    # Determine the destination directory
    set(DEST_DIR ${CMAKE_CURRENT_BINARY_DIR}/${RELATIVE_PATH})
    get_filename_component(DEST_DIR ${DEST_DIR} DIRECTORY)

    # Ensure the destination directory exists
    file(MAKE_DIRECTORY ${DEST_DIR})

    # Copy the .idl file to the destination directory
    configure_file(${IDL_FILE} ${DEST_DIR} COPYONLY)
endforeach()

set(RESOURCEDYNTYPETOIDLTESTS_SOURCE
    DynTypeIDLTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/UnitsParser.cpp)

file(GLOB DATATYPE_SOURCES "types/**/gen/*.cxx")

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

add_executable(DynTypeIDLTests ${RESOURCEDYNTYPETOIDLTESTS_SOURCE} ${DATATYPE_SOURCES})
target_compile_definitions(DynTypeIDLTests PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(DynTypeIDLTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    )
target_link_libraries(DynTypeIDLTests
    fastcdr
    fastdds
    GTest::gtest
    ${CMAKE_DL_LIBS})
gtest_discover_tests(DynTypeIDLTests SOURCES ${TYPEOBJECTUTILSTESTS_SOURCE})
