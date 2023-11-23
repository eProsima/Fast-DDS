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

set(THREAD_SETTINGS_TESTS_EXEC ThreadSettingsTests)

set(THREAD_SETTINGS_TESTS_SOURCE
    ThreadSettingsTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/ThreadSettings.cpp)

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

add_executable(${THREAD_SETTINGS_TESTS_EXEC} ${THREAD_SETTINGS_TESTS_SOURCE})

target_include_directories(
    ${THREAD_SETTINGS_TESTS_EXEC}
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include)

target_link_libraries(
    ${THREAD_SETTINGS_TESTS_EXEC}
    GTest::gtest
    ${CMAKE_DL_LIBS})

gtest_discover_tests(${THREAD_SETTINGS_TESTS_EXEC})
