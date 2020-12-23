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

target_compile_definitions(ThroughputTest PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )

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
    intraprocess_best_effort
    intraprocess_reliable
    interprocess_best_effort_udp
    interprocess_reliable_udp
#    interprocess_best_effort_tcp
#    interprocess_reliable_tcp
    interprocess_best_effort_shm
    interprocess_reliable_shm
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
        # Set the interprocess flag
        if(${throughput_test_name} MATCHES "^interprocess")
            set(interproces_flag "--interprocess")
        else()
            set(interproces_flag "")
        endif()

        # Add the test
        add_test(
            NAME performance.throughput.${throughput_test_name}
            COMMAND ${PYTHON_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/throughput_tests.py
            --xml_file ${CMAKE_CURRENT_SOURCE_DIR}/xml/${throughput_test_name}.xml
            --recoveries_file ${CMAKE_CURRENT_SOURCE_DIR}/recoveries.csv
            --demands_file ${CMAKE_CURRENT_SOURCE_DIR}/payloads_demands.csv
            ${interproces_flag}
        )

        # Set test properties
        set_property(
            TEST performance.throughput.${throughput_test_name}
            PROPERTY LABELS "NoMemoryCheck"
        )
        set_property(
            TEST performance.throughput.${throughput_test_name}
            APPEND PROPERTY ENVIRONMENT "THROUGHPUT_TEST_BIN=$<TARGET_FILE:ThroughputTest>"
        )
        set_property(
            TEST performance.throughput.${throughput_test_name}
            APPEND PROPERTY ENVIRONMENT "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
        )

        # Add environment
        if(WIN32)
            set(WIN_PATH "$ENV{PATH}")
            get_target_property(LINK_LIBRARIES_ ${PROJECT_NAME} LINK_LIBRARIES)
            if(NOT "${LINK_LIBRARIES_}" STREQUAL "LINK_LIBRARIES_-NOTFOUND")
            list(APPEND LINK_LIBRARIES_ ${PROJECT_NAME})
                foreach(LIBRARY_LINKED ${LINK_LIBRARIES_})
                    if(TARGET ${LIBRARY_LINKED})
                        # Check if is a real target or a target interface
                        get_target_property(dependency_type ${LIBRARY_LINKED} TYPE)
                        if(NOT dependency_type STREQUAL "INTERFACE_LIBRARY")
                            set(WIN_PATH "$<TARGET_FILE_DIR:${LIBRARY_LINKED}>;${WIN_PATH}")
                        endif()
                        unset(dependency_type)
                    endif()
                endforeach()
            endif()
            string(REPLACE ";" "\\;" WIN_PATH "${WIN_PATH}")
            set_property(TEST performance.throughput.${throughput_test_name}  APPEND PROPERTY ENVIRONMENT "PATH=${WIN_PATH}")
        endif()

        if(SECURITY AND (${throughput_test_name} MATCHES "^interprocess"))
            # Add the secure version
            add_test(
                NAME performance.throughput.${throughput_test_name}_security
                COMMAND ${PYTHON_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/throughput_tests.py
                --xml_file ${CMAKE_CURRENT_SOURCE_DIR}/xml/${throughput_test_name}.xml
                --recoveries_file ${CMAKE_CURRENT_SOURCE_DIR}/recoveries.csv
                --demands_file ${CMAKE_CURRENT_SOURCE_DIR}/payloads_demands.csv
                --security
                ${interproces_flag}
            )

            # Set test properties
            set_property(
                TEST performance.throughput.${throughput_test_name}_security
                PROPERTY LABELS "NoMemoryCheck"
            )
            set_property(
                TEST performance.throughput.${throughput_test_name}_security
                APPEND PROPERTY ENVIRONMENT "THROUGHPUT_TEST_BIN=$<TARGET_FILE:ThroughputTest>"
            )
            set_property(
                TEST performance.throughput.${throughput_test_name}_security
                APPEND PROPERTY ENVIRONMENT "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
            )
            if(WIN32)
                set(WIN_PATH "$ENV{PATH}")
                set(WIN_PATH "$<TARGET_FILE_DIR:${PROJECT_NAME}>;$ENV{PATH}")
                string(REPLACE ";" "\\;" WIN_PATH "${WIN_PATH}")
                set_property(
                    TEST performance.throughput.${throughput_test_name}_security
                    APPEND PROPERTY ENVIRONMENT "PATH=${WIN_PATH}"
                )
            endif()

            set_property(
                TEST performance.throughput.${throughput_test_name}_security
                APPEND PROPERTY ENVIRONMENT "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs"
            )
        endif()
    endforeach(throughput_test_name)
endif()
