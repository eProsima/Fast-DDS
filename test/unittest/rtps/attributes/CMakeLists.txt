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
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/ServerAttributes.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/GuidPrefix_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/LocatorWithMask.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/network.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/Host.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
    )

add_executable(${SERVER_ATTRIBUTES_TESTS_EXEC} ${SERVER_ATTRIBUTES_TESTS_SOURCE})
target_compile_definitions(${SERVER_ATTRIBUTES_TESTS_EXEC} PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories( ${SERVER_ATTRIBUTES_TESTS_EXEC} PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    )

target_link_libraries(
    ${SERVER_ATTRIBUTES_TESTS_EXEC}
    GTest::gtest
    ${CMAKE_DL_LIBS})

if(MSVC OR MSVC_IDE)
    target_link_libraries(${SERVER_ATTRIBUTES_TESTS_EXEC} ${PRIVACY} fastcdr iphlpapi Shlwapi ws2_32 fastdds::log)
else()
    target_link_libraries(${SERVER_ATTRIBUTES_TESTS_EXEC} ${PRIVACY} fastcdr fastdds::log)
endif()

if (APPLE)
    target_link_libraries(${SERVER_ATTRIBUTES_TESTS_EXEC} ${PRIVACY} "-framework CoreFoundation" "-framework IOKit")
endif()

gtest_discover_tests(${SERVER_ATTRIBUTES_TESTS_EXEC}
    TEST_PREFIX "ServerAttributesTests."
    TEST_FILTER ${SERVER_ATTRIBUTES_IGNORED_TESTS}
    )

unset(dns_filter)
