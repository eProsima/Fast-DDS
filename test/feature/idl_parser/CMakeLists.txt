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

if(WIN32)
    add_definitions(
        -D_WIN32_WINNT=0x0601
        -D_CRT_SECURE_NO_WARNINGS
    )
endif()

add_executable(IdlParserTests IdlParserTests.cpp)
target_compile_definitions(IdlParserTests PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
)
target_include_directories(IdlParserTests
    PRIVATE
    ${PROJECT_SOURCE_DIR}/test/utils/
)
target_link_libraries(IdlParserTests
    GTest::gtest
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    $<$<BOOL:${WIN32}>:ws2_32>
    fastcdr
    fastdds
)
gtest_discover_tests(IdlParserTests)

if(EXISTS "${PROJECT_SOURCE_DIR}/thirdparty/dds-types-test/IDL")
    message(STATUS "Copying IDL directory from ${PROJECT_SOURCE_DIR}/thirdparty/dds-types-test/IDL for idl_parser testing")
    add_custom_command(
        TARGET IdlParserTests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/thirdparty/dds-types-test/IDL $<TARGET_FILE_DIR:IdlParserTests>/IDL
    )
else()
    message(WARNING "The folder thirdparty/dds-types-test/IDL does not exist. The git submodule might not have been initialized.")
endif()
