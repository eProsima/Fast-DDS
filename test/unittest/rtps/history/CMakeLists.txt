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

set(READERHISTORYTESTS_SOURCE ReaderHistoryTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/CacheChangePool.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/History.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/ReaderHistory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    )

set(BASICPOOLSTESTS_SOURCE BasicPoolsTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/CacheChangePool.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    )

set(CACHECHANGEPOOLTESTS_SOURCE CacheChangePoolTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/CacheChangePool.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    )

set(TOPICPAYLOADPOOLTESTS_SOURCE
    TopicPayloadPoolTests.cpp TopicPayloadPoolRegistryTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/TopicPayloadPool.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/TopicPayloadPoolRegistry.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    )

set(WRITERHISTORYTESTS_SOURCE WriterHistoryTests.cpp)

if(ANDROID)
    if (ANDROID_NATIVE_API_LEVEL LESS 24)
        list(APPEND BASICPOOLSTESTS_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
    endif()
endif()

add_executable(ReaderHistoryTests ${READERHISTORYTESTS_SOURCE})
target_include_directories(ReaderHistoryTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/Endpoint
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/ResourceEvent
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSReader
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/StatefulReader
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include)
target_link_libraries(ReaderHistoryTests
    fastcdr
    GTest::gmock
    ${CMAKE_DL_LIBS})
fastdds_target_link_object_groups(ReaderHistoryTests
    fastdds::log
    fastdds::rtps::host-net
    fastdds::rtps::net-interface
    fastdds::time
    )

gtest_discover_tests(ReaderHistoryTests)

add_executable(BasicPoolsTests ${BASICPOOLSTESTS_SOURCE})
target_include_directories(BasicPoolsTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include)
target_link_libraries(BasicPoolsTests
    fastcdr
    GTest::gtest
    ${CMAKE_DL_LIBS})
fastdds_target_link_object_groups(BasicPoolsTests
    fastdds::log
    fastdds::rtps::host-net
    fastdds::rtps::net-interface
    fastdds::time
    )

gtest_discover_tests(BasicPoolsTests)

add_executable(CacheChangePoolTests ${CACHECHANGEPOOLTESTS_SOURCE})
target_include_directories(CacheChangePoolTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include)
target_link_libraries(CacheChangePoolTests
    fastcdr
    GTest::gtest
    ${CMAKE_DL_LIBS})
fastdds_target_link_object_groups(CacheChangePoolTests
    fastdds::log
    fastdds::rtps::host-net
    fastdds::rtps::net-interface
    fastdds::time
    )

gtest_discover_tests(CacheChangePoolTests)

add_executable(TopicPayloadPoolTests ${TOPICPAYLOADPOOLTESTS_SOURCE})
target_include_directories(TopicPayloadPoolTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/TopicPayloadPoolProxy
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include)
target_link_libraries(TopicPayloadPoolTests
    fastcdr
    GTest::gtest
    ${CMAKE_DL_LIBS})
fastdds_target_link_object_groups(TopicPayloadPoolTests
    fastdds::log
    fastdds::rtps::host-net
    fastdds::rtps::net-interface
    fastdds::time
    )

gtest_discover_tests(TopicPayloadPoolTests)

add_executable(WriterHistoryTests ${WRITERHISTORYTESTS_SOURCE})
target_link_libraries(WriterHistoryTests 
    fastcdr 
    fastdds
    foonathan_memory
    GTest::gtest
    ${CMAKE_DL_LIBS})
gtest_discover_tests(WriterHistoryTests)
