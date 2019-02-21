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

        set(WRITERPROXYTESTS_SOURCE WriterProxyTests.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/reader/WriterProxy.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/log/Log.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/log/StdoutConsumer.cpp
            )

        if(WIN32)
            add_definitions(-D_WIN32_WINNT=0x0601)
        endif()

        add_executable(WriterProxyTests ${WRITERPROXYTESTS_SOURCE})
        target_compile_definitions(WriterProxyTests PRIVATE FASTRTPS_NO_LIB)
        target_include_directories(WriterProxyTests PRIVATE
            ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/Endpoint
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSReader
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/StatefulReader
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/HeartbeatResponseDelay
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/WriterProxyLiveliness
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/InitialAckNack
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include)
        target_link_libraries(WriterProxyTests foonathan_memory
            ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES}
            ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})
        add_gtest(WriterProxyTests SOURCES ${WRITERPROXYTESTS_SOURCE})
    endif()
endif()
