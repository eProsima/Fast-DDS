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

if(NOT ((MSVC OR MSVC_IDE) AND EPROSIMA_INSTALLER) AND fastcdr_FOUND)
    include(${PROJECT_SOURCE_DIR}/cmake/common/gtest.cmake)
    check_gtest()

    if(GTEST_FOUND)
        if(WIN32)
            add_definitions(
                -D_WIN32_WINNT=0x0601
                -D_CRT_SECURE_NO_WARNINGS
                )
        endif()

        add_definitions(
          -DBOOST_ASIO_STANDALONE
          -DASIO_STANDALONE
        )

        include_directories(${ASIO_INCLUDE_DIR})

        ###############################################################################
        # Unit tests
        ###############################################################################
        file(GLOB BLACKBOXTESTS_TEST_SOURCE "BlackboxTests*.cpp")
        set(BLACKBOXTESTS_SOURCE ${BLACKBOXTESTS_TEST_SOURCE}
            types/HelloWorld.cpp
            types/HelloWorldType.cpp
            types/KeyedHelloWorld.cpp
            types/KeyedHelloWorldType.cpp
            types/String.cpp
            types/StringType.cpp
            types/Data64kb.cpp
            types/Data64kbType.cpp
            types/Data1mb.cpp
            types/Data1mbType.cpp
            types/FixedSized.cpp
            types/FixedSizedType.cpp

            utils/data_generators.cpp
            utils/lambda_functions.cpp
            utils/print_functions.cpp

            ReqRepHelloWorldRequester.cpp
            ReqRepHelloWorldReplier.cpp
            TCPReqRepHelloWorldRequester.cpp
            TCPReqRepHelloWorldReplier.cpp
            )

        # Prepare static discovery xml file for blackbox tests.
        string(RANDOM LENGTH 4 ALPHABET 0123456789 TOPIC_RANDOM_NUMBER)
        math(EXPR TOPIC_RANDOM_NUMBER "${TOPIC_RANDOM_NUMBER} + 0") # Remove extra leading 0s.
        string(RANDOM LENGTH 4 ALPHABET 0123456789 W_UNICAST_PORT_RANDOM_NUMBER)
        math(EXPR W_UNICAST_PORT_RANDOM_NUMBER "${W_UNICAST_PORT_RANDOM_NUMBER} + 0") # Remove extra leading 0s.
        if(W_UNICAST_PORT_RANDOM_NUMBER LESS 1025)
            math(EXPR W_UNICAST_PORT_RANDOM_NUMBER "${W_UNICAST_PORT_RANDOM_NUMBER} + 1024") # Remove extra leading 0s.
        endif()
        math(EXPR R_UNICAST_PORT_RANDOM_NUMBER "${W_UNICAST_PORT_RANDOM_NUMBER} + 1")
        math(EXPR MULTICAST_PORT_RANDOM_NUMBER "${R_UNICAST_PORT_RANDOM_NUMBER} + 1")
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/PubSubWriter.xml.in
            ${CMAKE_CURRENT_BINARY_DIR}/PubSubWriter.xml)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/PubSubReader.xml.in
            ${CMAKE_CURRENT_BINARY_DIR}/PubSubReader.xml)

        add_executable(BlackboxTests ${BLACKBOXTESTS_SOURCE})
        target_compile_definitions(BlackboxTests PRIVATE)
        target_include_directories(BlackboxTests PRIVATE ${GTEST_INCLUDE_DIRS})
        target_link_libraries(BlackboxTests fastrtps fastcdr foonathan_memory ${GTEST_LIBRARIES})
        add_gtest(BlackboxTests SOURCES ${BLACKBOXTESTS_TEST_SOURCE}
            ENVIRONMENTS "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs"
            "TOPIC_RANDOM_NUMBER=${TOPIC_RANDOM_NUMBER}"
            "W_UNICAST_PORT_RANDOM_NUMBER=${W_UNICAST_PORT_RANDOM_NUMBER}"
            "R_UNICAST_PORT_RANDOM_NUMBER=${R_UNICAST_PORT_RANDOM_NUMBER}"
            "MULTICAST_PORT_RANDOM_NUMBER=${MULTICAST_PORT_RANDOM_NUMBER}"
            )
    endif()
endif()
