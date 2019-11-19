# Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
    find_package(Threads REQUIRED)

    if(WIN32)
        if("${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore")
            add_definitions(-D_WIN32_WINNT=0x0603)
        else()
            add_definitions(-D_WIN32_WINNT=0x0601)
        endif()
    endif()

    add_definitions(
      -DBOOST_ASIO_STANDALONE
      -DASIO_STANDALONE
    )

    include_directories(${ASIO_INCLUDE_DIR})

    ###########################################################################
    # Create and link executable                                              #
    ###########################################################################
    set(
        LATENCYTEST_SOURCE LatencyTestPublisher.cpp
        LatencyTestSubscriber.cpp
        LatencyTestTypes.cpp
        main_LatencyTest.cpp
    )
    add_executable(LatencyTest ${LATENCYTEST_SOURCE})

    target_link_libraries(
        LatencyTest
        fastrtps
        foonathan_memory
        ${CMAKE_THREAD_LIBS_INIT}
        ${CMAKE_DL_LIBS}
    )

    ###########################################################################
    # List Latency tests                                                      #
    ###########################################################################
    set(
        LATENCY_TEST_LIST
        intraprocess_best_effort
        intraprocess_reliable
        interprocess_best_effort
        interprocess_reliable
    )

    ###########################################################################
    # Configure XML files                                                     #
    ###########################################################################
    foreach(latency_test_name ${LATENCY_TEST_LIST})
        configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/xml/${latency_test_name}.xml
            ${CMAKE_CURRENT_BINARY_DIR}/xml/${latency_test_name}.xml
            COPYONLY
        )
    endforeach(latency_test_name)

    ###########################################################################
    # Create tests                                                            #
    ###########################################################################
    find_package(PythonInterp 3 REQUIRED)
    if(PYTHONINTERP_FOUND)
        # Loop over the test names
        foreach(latency_test_name ${LATENCY_TEST_LIST})
            # Set the interprocess flag
            if(${latency_test_name} MATCHES "^interprocess")
                set(interproces_flag "--interprocess")
            else()
                set(interproces_flag "")
            endif()

            # Add the test
            add_test(
                NAME performance.latency.${latency_test_name}
                COMMAND ${PYTHON_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/latency_tests.py
                ${LATENCY_TEST_BIN}
                --xml_file ${CMAKE_CURRENT_SOURCE_DIR}/xml/${latency_test_name}.xml
                ${interproces_flag}
            )

            # Set test properties
            set_property(
                TEST performance.latency.${latency_test_name}
                PROPERTY LABELS "NoMemoryCheck"
            )
            set_property(
                TEST performance.latency.${latency_test_name}
                APPEND PROPERTY ENVIRONMENT "LATENCY_TEST_BIN=$<TARGET_FILE:LatencyTest>"
            )
            if(WIN32)
                set_property(
                    TEST performance.latency.${latency_test_name}
                    PROPERTY ENVIRONMENT "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$ENV{PATH}"
            )
            endif()

            # If there is security, add a secure test as well
            if(SECURITY)
                # Add the secure verison
                add_test(
                    NAME performance.latency.${latency_test_name}_security
                    COMMAND ${PYTHON_EXECUTABLE}
                    ${CMAKE_CURRENT_SOURCE_DIR}/latency_tests.py
                    ${LATENCY_TEST_BIN}
                    --xml_file ${CMAKE_CURRENT_SOURCE_DIR}/xml/${latency_test_name}.xml
                    --security
                    ${interproces_flag}
                )

                # Set test properties
                set_property(
                    TEST performance.latency.${latency_test_name}_security
                    PROPERTY LABELS "NoMemoryCheck"
                )
                set_property(
                    TEST performance.latency.${latency_test_name}_security
                    APPEND PROPERTY ENVIRONMENT "LATENCY_TEST_BIN=$<TARGET_FILE:LatencyTest>"
                )
                set_property(
                    TEST performance.latency.${latency_test_name}_security
                    APPEND PROPERTY ENVIRONMENT "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs"
                )
                if(WIN32)
                    set_property(
                        TEST performance.latency.${latency_test_name}_security
                        PROPERTY ENVIRONMENT "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$ENV{PATH}"
                )
                endif()
            endif()
        endforeach(latency_test_name)
    endif()
endif()
