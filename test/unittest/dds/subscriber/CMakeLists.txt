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

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../profiles/test_xml_profile.xml
    ${CMAKE_CURRENT_BINARY_DIR}/test_xml_profile.xml
    COPYONLY)

set(SUBSCRIBERTESTS_SOURCE SubscriberTests.cpp)
set(DATAREADERTESTS_SOURCE
    DataReaderTests.cpp
    )
set(DATAREADERINSTANCETESTS_SOURCE DataReaderInstanceTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    )
set(DATAREADERHISTORYTESTS_SOURCE DataReaderHistoryTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/history/DataReaderHistory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/LocatorWithMask.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/History.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/ReaderHistory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/network.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetmaskFilterKind.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterfaceWithFilter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
    )

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

if(ANDROID)
    if (ANDROID_NATIVE_API_LEVEL LESS 24)
        list(APPEND SUBSCRIBERTESTS_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
        list(APPEND DATAREADERTESTS_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
        list(APPEND DATAREADERINSTANCETESTS_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
        list(APPEND DATAREADERHISTORYTESTS_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
    endif()
endif()

add_executable(SubscriberTests ${SUBSCRIBERTESTS_SOURCE})
target_compile_definitions(SubscriberTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(SubscriberTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/test/utils
    )
target_link_libraries(SubscriberTests fastdds fastcdr foonathan_memory
    GTest::gmock
    ${CMAKE_DL_LIBS})
gtest_discover_tests(SubscriberTests)

add_executable(DataReaderTests ${DATAREADERTESTS_SOURCE})
target_compile_definitions(DataReaderTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    ASIO_DISABLE_VISIBILITY
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(DataReaderTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    )
target_link_libraries(DataReaderTests fastdds fastcdr foonathan_memory
    GTest::gmock
    ${CMAKE_DL_LIBS})
gtest_discover_tests(DataReaderTests)

add_executable(DataReaderInstanceTests ${DATAREADERINSTANCETESTS_SOURCE})
target_compile_definitions(DataReaderInstanceTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(DataReaderInstanceTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    )
target_link_libraries(DataReaderInstanceTests foonathan_memory
    GTest::gmock
    ${CMAKE_DL_LIBS})
gtest_discover_tests(DataReaderInstanceTests)

add_executable(DataReaderHistoryTests ${DATAREADERHISTORYTESTS_SOURCE})
target_compile_definitions(DataReaderHistoryTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    $<$<BOOL:${MSVC}>:NOMINMAX> # avoid conflic with std::min & std::max in visual studio
    # acknowledge atomic layout issue on old versions of visual studio STL
    $<$<AND:$<BOOL:${MSVC}>,$<VERSION_LESS:${CMAKE_CXX_COMPILER_VERSION},19.29.30140.0>>:_ENABLE_ATOMIC_ALIGNMENT_FIX>
    )
target_include_directories(DataReaderHistoryTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSReader
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/StatelessReader
    ${PROJECT_SOURCE_DIR}/test/mock/dds/Topic
    ${PROJECT_SOURCE_DIR}/test/mock/dds/TypeSupport
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    )
target_link_libraries(DataReaderHistoryTests
    fastcdr
    fastdds::log
    foonathan_memory
    GTest::gmock
    ${CMAKE_DL_LIBS})
gtest_discover_tests(DataReaderHistoryTests)
