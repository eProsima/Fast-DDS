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

if(EPROSIMA_TEST_DNS_NOT_SET_UP)
    set(dns_filter "*ServerClientEnvironmentSetUpDNS*")
endif()

string(JOIN ":" SERVER_ATTRIBUTES_IGNORED_TESTS ${pkcs_filter} ${dns_filter})
if(NOT SERVER_ATTRIBUTES_IGNORED_TESTS STREQUAL "")
	message(STATUS "Ignoring tests '${SERVER_ATTRIBUTES_IGNORED_TESTS}'")
    string(PREPEND SERVER_ATTRIBUTES_IGNORED_TESTS "-")
endif()
message(STATUS "SERVER_ATTRIBUTES_IGNORED_TESTS set to '${SERVER_ATTRIBUTES_IGNORED_TESTS}'")

set(SERVER_ATTRIBUTES_TESTS_EXEC ServerAttributesTests)

set(SERVER_ATTRIBUTES_TESTS_SOURCE
    ServerAttributesTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/ServerAttributes.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/GuidPrefix_t.cpp
    )

add_executable(${SERVER_ATTRIBUTES_TESTS_EXEC} ${SERVER_ATTRIBUTES_TESTS_SOURCE})
target_include_directories( ${SERVER_ATTRIBUTES_TESTS_EXEC} PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    )

target_link_libraries(${SERVER_ATTRIBUTES_TESTS_EXEC}
    fastcdr
    GTest::gtest
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    $<$<BOOL:${WIN32}>:ws2_32>
    ${CMAKE_DL_LIBS})
fastdds_target_link_object_groups(${SERVER_ATTRIBUTES_TESTS_EXEC}
    fastdds::log
    fastdds::rtps::host-net
    fastdds::time
    )


gtest_discover_tests(${SERVER_ATTRIBUTES_TESTS_EXEC}
    TEST_PREFIX "ServerAttributesTests."
    TEST_FILTER ${SERVER_ATTRIBUTES_IGNORED_TESTS}
    )

unset(dns_filter)
