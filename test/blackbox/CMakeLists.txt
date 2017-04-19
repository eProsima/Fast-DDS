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
    include(${PROJECT_SOURCE_DIR}/cmake/dev/gtest.cmake)
    check_gtest()

    if(GTEST_FOUND)
        if(WIN32)
            add_definitions(-D_WIN32_WINNT=0x0601)
        endif()

        add_definitions(
          -DBOOST_ASIO_STANDALONE
          -DASIO_STANDALONE
        )

        include_directories(${ASIO_INCLUDE_DIR})

        ###############################################################################
        # Unit tests
        ###############################################################################
        set(BLACKBOXTESTS_SOURCE BlackboxTests.cpp
            types/HelloWorld.cpp
            types/HelloWorldType.cpp
            types/String.cpp
            types/StringType.cpp
            types/Data64kb.cpp
            types/Data64kbType.cpp
            types/Data1mb.cpp
            types/Data1mbType.cpp
            ReqRepHelloWorldRequester.cpp
            ReqRepHelloWorldReplier.cpp
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

        add_executable(BlackboxTests_PreallocMem ${BLACKBOXTESTS_SOURCE})
        add_blackbox_gtest(BlackboxTests_PreallocMem PreallocMem ${BLACKBOXTESTS_SOURCE}
            ENVIRONMENT "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs"
            ENVIRONMENT "TOPIC_RANDOM_NUMBER=${TOPIC_RANDOM_NUMBER}"
            ENVIRONMENT "W_UNICAST_PORT_RANDOM_NUMBER=${W_UNICAST_PORT_RANDOM_NUMBER}"
            ENVIRONMENT "R_UNICAST_PORT_RANDOM_NUMBER=${R_UNICAST_PORT_RANDOM_NUMBER}"
            ENVIRONMENT "MULTICAST_PORT_RANDOM_NUMBER=${MULTICAST_PORT_RANDOM_NUMBER}"
            )
        target_compile_definitions(BlackboxTests_PreallocMem PRIVATE
            PREALLOCATED_MEMORY_MODE_TEST)
        target_include_directories(BlackboxTests_PreallocMem PRIVATE ${GTEST_INCLUDE_DIRS})
        target_link_libraries(BlackboxTests_PreallocMem fastrtps fastcdr ${GTEST_LIBRARIES})

        add_executable(BlackboxTests_ReallocMem ${BLACKBOXTESTS_SOURCE})
        add_blackbox_gtest(BlackboxTests_ReallocMem ReallocMem ${BLACKBOXTESTS_SOURCE}
            ENVIRONMENT "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs"
            ENVIRONMENT "TOPIC_RANDOM_NUMBER=${TOPIC_RANDOM_NUMBER}"
            ENVIRONMENT "W_UNICAST_PORT_RANDOM_NUMBER=${W_UNICAST_PORT_RANDOM_NUMBER}"
            ENVIRONMENT "R_UNICAST_PORT_RANDOM_NUMBER=${R_UNICAST_PORT_RANDOM_NUMBER}"
            ENVIRONMENT "MULTICAST_PORT_RANDOM_NUMBER=${MULTICAST_PORT_RANDOM_NUMBER}"
            )
        target_compile_definitions(BlackboxTests_ReallocMem PRIVATE
            PREALLOCATED_WITH_REALLOC_MEMORY_MODE_TEST)
        target_include_directories(BlackboxTests_ReallocMem PRIVATE ${GTEST_INCLUDE_DIRS})
        target_link_libraries(BlackboxTests_ReallocMem fastrtps fastcdr ${GTEST_LIBRARIES})

        add_executable(BlackboxTests_DynMem ${BLACKBOXTESTS_SOURCE})
        add_blackbox_gtest(BlackboxTests_DynMem DynMem ${BLACKBOXTESTS_SOURCE}
            ENVIRONMENT "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs"
            ENVIRONMENT "TOPIC_RANDOM_NUMBER=${TOPIC_RANDOM_NUMBER}"
            ENVIRONMENT "W_UNICAST_PORT_RANDOM_NUMBER=${W_UNICAST_PORT_RANDOM_NUMBER}"
            ENVIRONMENT "R_UNICAST_PORT_RANDOM_NUMBER=${R_UNICAST_PORT_RANDOM_NUMBER}"
            ENVIRONMENT "MULTICAST_PORT_RANDOM_NUMBER=${MULTICAST_PORT_RANDOM_NUMBER}"
            )
        target_compile_definitions(BlackboxTests_DynMem PRIVATE
            DYNAMIC_RESERVE_MEMORY_MODE_TEST)
        target_include_directories(BlackboxTests_DynMem PRIVATE ${GTEST_INCLUDE_DIRS})
        target_link_libraries(BlackboxTests_DynMem fastrtps fastcdr ${GTEST_LIBRARIES})
    endif()
endif()
