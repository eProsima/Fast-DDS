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
        THROUGHPUTTEST_SOURCE ThroughputPublisher.cpp
        ThroughputSubscriber.cpp
        ThroughputTypes.cpp
        main_ThroughputTest.cpp
    )
    add_executable(ThroughputTest ${THROUGHPUTTEST_SOURCE})

    target_include_directories(ThroughputTest PRIVATE)
    target_link_libraries(
        ThroughputTest
        fastrtps
        foonathan_memory
        ${CMAKE_THREAD_LIBS_INIT}
        ${CMAKE_DL_LIBS}
    )

    ###########################################################################
    # List Throughput tests                                                   #
    ###########################################################################
    set(
        THROUGHPUT_TEST_LIST
        interprocess_best_effort
        interprocess_reliable
    )

    ###########################################################################
    # List Recovery times                                                     #
    ###########################################################################
    set(
        RECOVERY_TIMES
        0
        20
        40
        60
        80
        100
    )

    ###########################################################################
    # Configure XML files                                                     #
    ###########################################################################
    foreach(throughput_test_name ${THROUGHPUT_TEST_LIST})
        configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/xml/${throughput_test_name}.xml
            ${CMAKE_CURRENT_BINARY_DIR}/xml/${throughput_test_name}.xml
            COPYONLY
        )
    endforeach(throughput_test_name)

    ###########################################################################
    # Create tests                                                            #
    ###########################################################################
    find_package(PythonInterp 3 REQUIRED)
    if(PYTHONINTERP_FOUND)
        # Loop over the test names
        foreach(throughput_test_name ${THROUGHPUT_TEST_LIST})
            foreach(recovery_time ${RECOVERY_TIMES})
                # Append recovery time to test name
                set(throughput_test_name_rec ${throughput_test_name}_${recovery_time})

                # Add the test
                add_test(
                    NAME performance.throughput.${throughput_test_name_rec}
                    COMMAND ${PYTHON_EXECUTABLE}
                    ${CMAKE_CURRENT_SOURCE_DIR}/throughput_tests.py
                    --xml_file ${CMAKE_CURRENT_SOURCE_DIR}/xml/${throughput_test_name}.xml
                    --recovery_time ${recovery_time}
                    --demands_file ${CMAKE_CURRENT_SOURCE_DIR}/payloads_demands.csv
                )

                # Set test properties
                set_property(
                    TEST performance.throughput.${throughput_test_name_rec}
                    PROPERTY LABELS "NoMemoryCheck"
                )
                set_property(
                    TEST performance.throughput.${throughput_test_name_rec}
                    APPEND PROPERTY ENVIRONMENT "THROUGHPUT_TEST_BIN=$<TARGET_FILE:ThroughputTest>"
                )
                set_property(
                    TEST performance.throughput.${throughput_test_name_rec}
                    APPEND PROPERTY ENVIRONMENT "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
                )
                if(WIN32)
                    set_property(
                        TEST performance.throughput.${throughput_test_name_rec}
                        PROPERTY ENVIRONMENT "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$ENV{PATH}"
                    )
                endif()

                if(SECURITY)
                    # Add the secure version
                    add_test(
                        NAME performance.throughput.${throughput_test_name_rec}_security
                        COMMAND ${PYTHON_EXECUTABLE}
                        ${CMAKE_CURRENT_SOURCE_DIR}/throughput_tests.py
                        --xml_file ${CMAKE_CURRENT_SOURCE_DIR}/xml/${throughput_test_name}.xml
                        --recovery_time ${recovery_time}
                        --demands_file ${CMAKE_CURRENT_SOURCE_DIR}/payloads_demands.csv
                        --security
                    )

                    # Set test properties
                    set_property(
                        TEST performance.throughput.${throughput_test_name_rec}_security
                        PROPERTY LABELS "NoMemoryCheck"
                    )
                    set_property(
                        TEST performance.throughput.${throughput_test_name_rec}_security
                        APPEND PROPERTY ENVIRONMENT "THROUGHPUT_TEST_BIN=$<TARGET_FILE:ThroughputTest>"
                    )
                    set_property(
                        TEST performance.throughput.${throughput_test_name_rec}_security
                        APPEND PROPERTY ENVIRONMENT "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
                    )
                    if(WIN32)
                        set_property(
                            TEST performance.throughput.${throughput_test_name_rec}_security
                            PROPERTY ENVIRONMENT "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$ENV{PATH}"
                        )
                    endif()

                    set_property(
                        TEST performance.throughput.${throughput_test_name_rec}_security
                        APPEND PROPERTY ENVIRONMENT "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs"
                    )
                endif()
            endforeach(recovery_time)
        endforeach(throughput_test_name)
    endif()
endif()
