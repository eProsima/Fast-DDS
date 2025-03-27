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

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/thirdparty/dds-types-test/IDL")
    # Update submodule
    message(STATUS "Updating submodule thirdparty/dds-types-test")
    execute_process(
        COMMAND git submodule update --recursive --init "thirdparty/dds-types-test"
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        RESULT_VARIABLE UPDATE_SUBMODULE_RESULT
        )
    # A result different than 0 means that the submodule could not be updated.
    if(NOT UPDATE_SUBMODULE_RESULT EQUAL 0)
        message(FATAL_ERROR "Cannot configure Git submodule thirdparty/dds-types-test")
    endif()
endif()

if(WIN32)
    add_definitions(
        -D_WIN32_WINNT=0x0601
        -D_CRT_SECURE_NO_WARNINGS
    )

    find_program(PREPROCESSOR_EXEC cl.exe)
    if(PREPROCESSOR_EXEC)
        message(STATUS "Found preprocessor: ${PREPROCESSOR_EXEC}")
        # Add quotes to handle spaces in the path
        set(PREPROCESSOR_EXEC "\\\"${PREPROCESSOR_EXEC}\\\"")
    else()
        message(FATAL_ERROR "Preprocessor not found. Some of IdlParserTests might fail.")
    endif()
endif()

configure_file(IdlParserTests.hpp.in config/IdlParserTests.hpp @ONLY)
add_executable(IdlParserTests IdlParserTests.cpp)
target_compile_definitions(IdlParserTests PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
)
target_include_directories(IdlParserTests
    PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/config
    ${PROJECT_SOURCE_DIR}/test/utils/
)
target_link_libraries(IdlParserTests
    GTest::gtest
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    $<$<BOOL:${WIN32}>:ws2_32>
    fastcdr
    fastdds
)
gtest_discover_tests(IdlParserTests PROPERTIES ${IDLPARSERTESTS_ENV_PROPERTIES})

message(STATUS "Copying IDL directory from ${PROJECT_SOURCE_DIR}/thirdparty/dds-types-test/IDL for idl_parser testing")
add_custom_command(
    TARGET IdlParserTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/thirdparty/dds-types-test/IDL ${CMAKE_CURRENT_BINARY_DIR}/IDL
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/test/feature/idl_parser/no_path_included.idl ${CMAKE_CURRENT_BINARY_DIR}/IDL/
)
