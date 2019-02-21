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

macro(add_blackbox_gtest test memorymode)
    # Parse arguments
    set(multiValueArgs SOURCES ENVIRONMENTS)
    cmake_parse_arguments(GTEST "" "" "${multiValueArgs}" ${ARGN})

    if(GTEST_INDIVIDUAL)
        if(WIN32)
            set(WIN_PATH "$ENV{PATH}")
            get_target_property(LINK_LIBRARIES_ ${test} LINK_LIBRARIES)
            if(NOT "${LINK_LIBRARIES_}" STREQUAL "LINK_LIBRARIES_-NOTFOUND")
                foreach(LIBRARY_LINKED ${LINK_LIBRARIES_})
                    if(TARGET ${LIBRARY_LINKED})
                        set(WIN_PATH "$<TARGET_FILE_DIR:${LIBRARY_LINKED}>;${WIN_PATH}")
                    endif()
                endforeach()
            endif()
            string(REPLACE ";" "\\;" WIN_PATH "${WIN_PATH}")
        endif()

        foreach(GTEST_SOURCE_FILE ${GTEST_SOURCES})
            file(STRINGS ${GTEST_SOURCE_FILE} GTEST_NAMES REGEX ^BLACKBOXTEST)
            foreach(GTEST_NAME ${GTEST_NAMES})
                string(REGEX REPLACE ["\) \(,"] ";" GTEST_NAME ${GTEST_NAME})
                list(GET GTEST_NAME 1 GTEST_GROUP_NAME)
                list(GET GTEST_NAME 3 GTEST_NAME)
                add_test(NAME ${GTEST_GROUP_NAME}_${memorymode}.${GTEST_NAME}
                    COMMAND ${test}
                    --gtest_filter=${GTEST_GROUP_NAME}_${memorymode}.${GTEST_NAME})

                # Add environment
                if(WIN32)
                    set_tests_properties(${GTEST_GROUP_NAME}_${memorymode}.${GTEST_NAME} PROPERTIES ENVIRONMENT "PATH=${WIN_PATH}")
                endif()

                foreach(property ${GTEST_ENVIRONMENTS})
                    set_property(TEST ${GTEST_GROUP_NAME}_${memorymode}.${GTEST_NAME} APPEND PROPERTY ENVIRONMENT "${property}")
                endforeach()
            endforeach()
        endforeach()
    else()
        add_test(NAME ${test} COMMAND ${test})

        # Add environment
        if(WIN32)
            set(WIN_PATH "$ENV{PATH}")
            get_target_property(LINK_LIBRARIES_ ${test} LINK_LIBRARIES)
            if(NOT "${LINK_LIBRARIES_}" STREQUAL "LINK_LIBRARIES_-NOTFOUND")
                foreach(LIBRARY_LINKED ${LINK_LIBRARIES_})
                    if(TARGET ${LIBRARY_LINKED})
                        set(WIN_PATH "$<TARGET_FILE_DIR:${LIBRARY_LINKED}>;${WIN_PATH}")
                    endif()
                endforeach()
            endif()
            string(REPLACE ";" "\\;" WIN_PATH "${WIN_PATH}")
            set_tests_properties(${test} PROPERTIES ENVIRONMENT "PATH=${WIN_PATH}")
        endif()

        foreach(property ${GTEST_ENVIRONMENTS})
            set_property(TEST ${test} APPEND PROPERTY ENVIRONMENT "${property}")
        endforeach()
    endif()
endmacro()

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

        add_executable(BlackboxTests_PreallocMem ${BLACKBOXTESTS_SOURCE})
        target_compile_definitions(BlackboxTests_PreallocMem PRIVATE
            PREALLOCATED_MEMORY_MODE_TEST)
        target_include_directories(BlackboxTests_PreallocMem PRIVATE ${GTEST_INCLUDE_DIRS})
        target_link_libraries(BlackboxTests_PreallocMem fastrtps fastcdr foonathan_memory ${GTEST_LIBRARIES})
        add_blackbox_gtest(BlackboxTests_PreallocMem PreallocMem SOURCES ${BLACKBOXTESTS_TEST_SOURCE}
            ENVIRONMENTS "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs"
            "TOPIC_RANDOM_NUMBER=${TOPIC_RANDOM_NUMBER}"
            "W_UNICAST_PORT_RANDOM_NUMBER=${W_UNICAST_PORT_RANDOM_NUMBER}"
            "R_UNICAST_PORT_RANDOM_NUMBER=${R_UNICAST_PORT_RANDOM_NUMBER}"
            "MULTICAST_PORT_RANDOM_NUMBER=${MULTICAST_PORT_RANDOM_NUMBER}"
            )

        add_executable(BlackboxTests_ReallocMem ${BLACKBOXTESTS_SOURCE})
        target_compile_definitions(BlackboxTests_ReallocMem PRIVATE
            PREALLOCATED_WITH_REALLOC_MEMORY_MODE_TEST)
        target_include_directories(BlackboxTests_ReallocMem PRIVATE ${GTEST_INCLUDE_DIRS})
        target_link_libraries(BlackboxTests_ReallocMem fastrtps fastcdr foonathan_memory ${GTEST_LIBRARIES})
        add_blackbox_gtest(BlackboxTests_ReallocMem ReallocMem SOURCES ${BLACKBOXTESTS_TEST_SOURCE}
            ENVIRONMENTS "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs"
            "TOPIC_RANDOM_NUMBER=${TOPIC_RANDOM_NUMBER}"
            "W_UNICAST_PORT_RANDOM_NUMBER=${W_UNICAST_PORT_RANDOM_NUMBER}"
            "R_UNICAST_PORT_RANDOM_NUMBER=${R_UNICAST_PORT_RANDOM_NUMBER}"
            "MULTICAST_PORT_RANDOM_NUMBER=${MULTICAST_PORT_RANDOM_NUMBER}"
            )

        add_executable(BlackboxTests_DynMem ${BLACKBOXTESTS_SOURCE})
        target_compile_definitions(BlackboxTests_DynMem PRIVATE
            DYNAMIC_RESERVE_MEMORY_MODE_TEST)
        target_include_directories(BlackboxTests_DynMem PRIVATE ${GTEST_INCLUDE_DIRS})
        target_link_libraries(BlackboxTests_DynMem fastrtps fastcdr foonathan_memory ${GTEST_LIBRARIES})
        add_blackbox_gtest(BlackboxTests_DynMem DynMem SOURCES ${BLACKBOXTESTS_TEST_SOURCE}
            ENVIRONMENTS "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs"
            "TOPIC_RANDOM_NUMBER=${TOPIC_RANDOM_NUMBER}"
            "W_UNICAST_PORT_RANDOM_NUMBER=${W_UNICAST_PORT_RANDOM_NUMBER}"
            "R_UNICAST_PORT_RANDOM_NUMBER=${R_UNICAST_PORT_RANDOM_NUMBER}"
            "MULTICAST_PORT_RANDOM_NUMBER=${MULTICAST_PORT_RANDOM_NUMBER}"
            )
    endif()
endif()
