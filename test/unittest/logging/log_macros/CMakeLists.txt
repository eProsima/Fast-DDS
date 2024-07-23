# Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

set(LOG_COMMON_SOURCE
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/LocatorWithMask.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/network.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetmaskFilterKind.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterfaceWithFilter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
    )

# MACROS LOG TEST ALL
set(LOGMACROS_ALL_TEST_SOURCE LogMacrosAllActiveTests.cpp)

set(LOGMACROS_SOURCE
    ${LOG_COMMON_SOURCE}
    ${LOGMACROS_ALL_TEST_SOURCE})

add_executable(LogMacrosAllActiveTests ${LOGMACROS_SOURCE})
target_compile_definitions(LogMacrosAllActiveTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(LogMacrosAllActiveTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp)
target_link_libraries(LogMacrosAllActiveTests
    fastcdr
    fastdds::log
    GTest::gtest
    ${MOCKS}
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    )
gtest_discover_tests(LogMacrosAllActiveTests)


# MACROS LOG TEST NO INFO
set(LOGMACROS_NOINFO_TEST_SOURCE LogMacrosNoInfoTests.cpp)

set(LOGMACROS_SOURCE
    ${LOG_COMMON_SOURCE}
    ${LOGMACROS_NOINFO_TEST_SOURCE})

add_executable(LogMacrosNoInfoTests ${LOGMACROS_SOURCE})
target_compile_definitions(LogMacrosNoInfoTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(LogMacrosNoInfoTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp)
target_link_libraries(LogMacrosNoInfoTests
    fastcdr
    fastdds::log
    GTest::gtest
    ${MOCKS}
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    )
gtest_discover_tests(LogMacrosNoInfoTests)


# MACROS LOG TEST NO WARNING
set(LOGMACROS_NOWARNING_TEST_SOURCE LogMacrosNoWarningTests.cpp)

set(LOGMACROS_SOURCE
    ${LOG_COMMON_SOURCE}
    ${LOGMACROS_NOWARNING_TEST_SOURCE})

add_executable(LogMacrosNoWarningTests ${LOGMACROS_SOURCE})
target_compile_definitions(LogMacrosNoWarningTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(LogMacrosNoWarningTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp)
target_link_libraries(LogMacrosNoWarningTests
    fastcdr
    fastdds::log
    GTest::gtest
    ${MOCKS}
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    )
gtest_discover_tests(LogMacrosNoWarningTests)


# MACROS LOG TEST NO ERROR
set(LOGMACROS_NOERROR_TEST_SOURCE LogMacrosNoErrorTests.cpp)

set(LOGMACROS_SOURCE
    ${LOG_COMMON_SOURCE}
    ${LOGMACROS_NOERROR_TEST_SOURCE})

add_executable(LogMacrosNoErrorTests ${LOGMACROS_SOURCE})
target_compile_definitions(LogMacrosNoErrorTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(LogMacrosNoErrorTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp)
target_link_libraries(LogMacrosNoErrorTests
    fastcdr
    fastdds::log
    GTest::gtest
    ${MOCKS}
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    )
gtest_discover_tests(LogMacrosNoErrorTests)


# MACROS LOG TEST NO DEFAULT
set(LOGMACROS_DEFAULT_TEST_SOURCE LogMacrosDefaultTests.cpp)

set(LOGMACROS_SOURCE
    ${LOG_COMMON_SOURCE}
    ${LOGMACROS_DEFAULT_TEST_SOURCE})

add_executable(LogMacrosDefaultTests ${LOGMACROS_SOURCE})
target_compile_definitions(LogMacrosDefaultTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(LogMacrosDefaultTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp)
target_link_libraries(LogMacrosDefaultTests
    fastcdr
    fastdds::log
    GTest::gtest
    ${MOCKS}
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    )
gtest_discover_tests(LogMacrosDefaultTests)


# MACROS LOG TEST INTERNALDEBUG
# In case this test fails, it fails de compilation

set(LOGMACROS_INTERNALDEBUG_TEST_SOURCE LogMacrosInternalDebugOffTests.cpp)

set(LOGMACROS_SOURCE
    ${LOG_COMMON_SOURCE}
    ${LOGMACROS_INTERNALDEBUG_TEST_SOURCE})

add_executable(LogMacrosInternalDebugOffTests ${LOGMACROS_SOURCE})
target_compile_definitions(LogMacrosInternalDebugOffTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(LogMacrosInternalDebugOffTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp)
target_link_libraries(LogMacrosInternalDebugOffTests
    fastcdr
    fastdds::log
    GTest::gtest
    ${MOCKS}
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    )
gtest_discover_tests(LogMacrosInternalDebugOffTests)

# MACROS OLD LOG TEST ALL
set(LOGMACROS_ALL_TEST_SOURCE OldLogMacrosTests.cpp)

set(LOGMACROS_SOURCE
    ${LOG_COMMON_SOURCE}
    ${LOGMACROS_ALL_TEST_SOURCE})

add_executable(OldLogMacrosTests ${LOGMACROS_SOURCE})
target_compile_definitions(OldLogMacrosTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(OldLogMacrosTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp)
target_link_libraries(OldLogMacrosTests
    fastcdr
    fastdds::log
    GTest::gtest
    ${MOCKS}
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    )
gtest_discover_tests(OldLogMacrosTests)


# MACROS OLD DISABLE LOG TEST ALL
set(LOGMACROS_ALL_TEST_SOURCE OldLogMacrosDisableTests.cpp)

set(LOGMACROS_SOURCE
    ${LOG_COMMON_SOURCE}
    ${LOGMACROS_ALL_TEST_SOURCE})

add_executable(OldLogMacrosDisableTests ${LOGMACROS_SOURCE})
target_compile_definitions(OldLogMacrosDisableTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(OldLogMacrosDisableTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp)
target_link_libraries(OldLogMacrosDisableTests
    fastcdr
    fastdds::log
    GTest::gtest
    ${MOCKS}
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    )
gtest_discover_tests(OldLogMacrosDisableTests)
