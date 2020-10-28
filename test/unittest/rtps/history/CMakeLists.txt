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

if(NOT ((MSVC OR MSVC_IDE) AND EPROSIMA_INSTALLER))
    include(${PROJECT_SOURCE_DIR}/cmake/common/gtest.cmake)
    check_gtest()
    check_gmock()

    if(GTEST_FOUND AND GMOCK_FOUND)
        find_package(Threads REQUIRED)

        set(READERHISTORYTESTS_SOURCE ReaderHistoryTests.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/ReaderHistory.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/History.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/CacheChangePool.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/Log.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/OStreamConsumer.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/StdoutConsumer.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/StdoutErrConsumer.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp)

        set(HISTORYTESTS_SOURCE HistoryTests.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/History.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/CacheChangePool.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/Log.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/OStreamConsumer.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/StdoutConsumer.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/StdoutErrConsumer.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp)

        set(BASICPOOLSTESTS_SOURCE BasicPoolsTests.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/CacheChangePool.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/Log.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/OStreamConsumer.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/StdoutConsumer.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/StdoutErrConsumer.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp)

        set(CACHECHANGEPOOLTESTS_SOURCE CacheChangePoolTests.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/CacheChangePool.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/Log.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/OStreamConsumer.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/StdoutConsumer.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/StdoutErrConsumer.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp)

        if(WIN32)
            add_definitions(-D_WIN32_WINNT=0x0601)
        endif()

        add_executable(ReaderHistoryTests ${READERHISTORYTESTS_SOURCE})
        target_compile_definitions(ReaderHistoryTests PRIVATE FASTRTPS_NO_LIB)
        target_include_directories(ReaderHistoryTests PRIVATE
            ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/Endpoint
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSReader
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/StatefulReader
            ${PROJECT_SOURCE_DIR}/src/cpp
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include)
        target_link_libraries(ReaderHistoryTests
            ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES}
            ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})
        add_gtest(ReaderHistoryTests SOURCES ${READERHISTORYTESTS_SOURCE})

        add_executable(HistoryTests ${HISTORYTESTS_SOURCE})
        target_compile_definitions(HistoryTests PRIVATE FASTRTPS_NO_LIB)
        target_include_directories(HistoryTests PRIVATE
            ${GTEST_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/src/cpp
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include)
        target_link_libraries(HistoryTests
            ${GTEST_LIBRARIES}
            ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})
        add_gtest(HistoryTests SOURCES ${HISTORYTESTS_SOURCE})

        add_executable(BasicPoolsTests ${BASICPOOLSTESTS_SOURCE})
        target_compile_definitions(BasicPoolsTests PRIVATE FASTRTPS_NO_LIB)
        target_include_directories(BasicPoolsTests PRIVATE
            ${GTEST_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/src/cpp
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include)
        target_link_libraries(BasicPoolsTests
            ${GTEST_LIBRARIES}
            ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})
        add_gtest(BasicPoolsTests SOURCES ${BASICPOOLSTESTS_SOURCE})

        add_executable(CacheChangePoolTests ${CACHECHANGEPOOLTESTS_SOURCE})
        target_compile_definitions(CacheChangePoolTests PRIVATE FASTRTPS_NO_LIB)
        target_include_directories(CacheChangePoolTests PRIVATE
            ${GTEST_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/src/cpp
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include)
        target_link_libraries(CacheChangePoolTests
            ${GTEST_LIBRARIES}
            ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})
        add_gtest(CacheChangePoolTests SOURCES ${CACHECHANGEPOOLTESTS_SOURCE})

    endif()
endif()
