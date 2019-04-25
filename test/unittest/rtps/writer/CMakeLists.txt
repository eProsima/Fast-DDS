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

        set(WRITERPROXYTESTS_SOURCE ReaderProxyTests.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/ReaderProxy.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEvent.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEventImpl.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/log/Log.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/log/StdoutConsumer.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
            )

        if(WIN32)
            add_definitions(-D_WIN32_WINNT=0x0601)
        endif()

        include_directories(${ASIO_INCLUDE_DIR})

        add_executable(ReaderProxyTests ${WRITERPROXYTESTS_SOURCE})
        target_compile_definitions(ReaderProxyTests PRIVATE FASTRTPS_NO_LIB)
        target_include_directories(ReaderProxyTests PRIVATE
            ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/Endpoint
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSWriter
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/WriterHistory
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/StatefulWriter
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/HeartbeatResponseDelay
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/WriterProxyLiveliness
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/InitialAckNack
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/NackSupressionDuration
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include)
        target_link_libraries(ReaderProxyTests
            ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES}
            ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})
        add_gtest(ReaderProxyTests SOURCES ${WRITERPROXYTESTS_SOURCE})
    endif()
endif()
