# Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

set(STRINGMATCHINGTESTS_SOURCE
    StringMatchingTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/Log.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/OStreamConsumer.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/StdoutConsumer.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/StdoutErrConsumer.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/LocatorWithMask.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/network.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetmaskFilterKind.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterfaceWithFilter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/StringMatching.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp)

set(FIXEDSIZESTRINGTESTS_SOURCE
    FixedSizeStringTests.cpp)

set(BITMAPRANGETESTS_SOURCE
    BitmapRangeTests.cpp)

set(RESOURCELIMITEDVECTORTESTS_SOURCE
    ResourceLimitedVectorTests.cpp)

set(LOCATORTESTS_SOURCE
    LocatorTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/Log.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/OStreamConsumer.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/StdoutConsumer.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/StdoutErrConsumer.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/LocatorWithMask.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/network.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetmaskFilterKind.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterfaceWithFilter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp)

set(FIXEDSIZEQUEUETESTS_SOURCE
    FixedSizeQueueTests.cpp)

set(SYSTEMINFOTESTS_SOURCE
    SystemInfoTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/Log.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/OStreamConsumer.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/StdoutConsumer.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/StdoutErrConsumer.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/LocatorWithMask.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/network.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetmaskFilterKind.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterfaceWithFilter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp)

include_directories(mock/)

add_executable(StringMatchingTests ${STRINGMATCHINGTESTS_SOURCE})
target_compile_definitions(StringMatchingTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(StringMatchingTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp)
target_link_libraries(StringMatchingTests GTest::gtest ${MOCKS})
if(MSVC OR MSVC_IDE)
    target_link_libraries(StringMatchingTests ${PRIVACY} iphlpapi Shlwapi
        )
endif()
gtest_discover_tests(StringMatchingTests)


add_executable(FixedSizeStringTests ${FIXEDSIZESTRINGTESTS_SOURCE})
target_compile_definitions(FixedSizeStringTests PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(FixedSizeStringTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include)
target_link_libraries(FixedSizeStringTests GTest::gtest ${MOCKS})
gtest_discover_tests(FixedSizeStringTests)


add_executable(BitmapRangeTests ${BITMAPRANGETESTS_SOURCE})
target_compile_definitions(BitmapRangeTests PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(BitmapRangeTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include)
target_link_libraries(BitmapRangeTests GTest::gtest ${MOCKS})
gtest_discover_tests(BitmapRangeTests)


add_executable(ResourceLimitedVectorTests ${RESOURCELIMITEDVECTORTESTS_SOURCE})
target_compile_definitions(ResourceLimitedVectorTests PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(ResourceLimitedVectorTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include)
target_link_libraries(ResourceLimitedVectorTests GTest::gtest ${MOCKS})
gtest_discover_tests(ResourceLimitedVectorTests)

add_executable(LocatorTests ${LOCATORTESTS_SOURCE})
target_compile_definitions(LocatorTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(LocatorTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/src/cpp)
target_link_libraries(LocatorTests GTest::gtest ${MOCKS})
if(QNX)
    target_link_libraries(LocatorTests socket)
endif()
if(EPROSIMA_TEST_DNS_NOT_SET_UP)
    message(STATUS "Ignoring 'LocatorDNSTests*'")
    set(IGNORE_COMMAND "-LocatorDNSTests*")
endif()
gtest_discover_tests(LocatorTests TEST_FILTER ${IGNORE_COMMAND})
unset(IGNORE_COMMAND)

add_executable(FixedSizeQueueTests ${FIXEDSIZEQUEUETESTS_SOURCE})
target_include_directories(FixedSizeQueueTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src/cpp ${PROJECT_BINARY_DIR}/include)
target_link_libraries(FixedSizeQueueTests GTest::gtest ${MOCKS})
gtest_discover_tests(FixedSizeQueueTests)

add_executable(SystemInfoTests ${SYSTEMINFOTESTS_SOURCE})
target_compile_definitions(SystemInfoTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(SystemInfoTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src/cpp ${PROJECT_BINARY_DIR}/include)
target_link_libraries(SystemInfoTests GTest::gtest)
gtest_discover_tests(SystemInfoTests)

add_executable(SharedMutexTests shared_mutex_tests.cpp)
target_compile_definitions(SharedMutexTests PUBLIC USE_THIRDPARTY_SHARED_MUTEX=1)
target_include_directories(SharedMutexTests PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(SharedMutexTests PUBLIC GTest::gtest)
gtest_discover_tests(SharedMutexTests)

###############################################################################
# Necessary files
###############################################################################
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/environment_test_file.json
    ${CMAKE_CURRENT_BINARY_DIR}/environment_test_file.json COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/empty_environment_test_file.json
    ${CMAKE_CURRENT_BINARY_DIR}/empty_environment_test_file.json COPYONLY)
