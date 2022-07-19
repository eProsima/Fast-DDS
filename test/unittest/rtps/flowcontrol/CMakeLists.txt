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
    if(GTest_FOUND)
        if(WIN32)
            add_definitions(-D_WIN32_WINNT=0x0601)
        endif()

        include_directories(${ASIO_INCLUDE_DIR})

        set(THROUGHPUTCONTROLLERTESTS_SOURCE
            ThroughputControllerTests.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/FlowController.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/ThroughputController.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/ThroughputControllerDescriptor.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp)

        add_executable(ThroughputControllerTests ${THROUGHPUTCONTROLLERTESTS_SOURCE})
        target_compile_definitions(ThroughputControllerTests PRIVATE FASTRTPS_NO_LIB
            BOOST_ASIO_STANDALONE
            ASIO_STANDALONE
            )
        target_include_directories(ThroughputControllerTests PRIVATE
            ${Asio_INCLUDE_DIR}    
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/Endpoint
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/AsyncWriterThread
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSParticipantImpl
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSWriter
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSReader
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
            ${PROJECT_SOURCE_DIR}/src/cpp
            )
        target_link_libraries(ThroughputControllerTests GTest::gmock)
        if(MSVC OR MSVC_IDE)
            target_link_libraries(ThroughputControllerTests ${PRIVACY}
                iphlpapi Shlwapi
                )
        endif()
        add_gtest(ThroughputControllerTests SOURCES ${THROUGHPUTCONTROLLERTESTS_SOURCE})
    endif()
endif()
