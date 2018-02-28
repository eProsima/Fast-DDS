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
    if(PERFORMANCE_TESTS)
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

        ###############################################################################
        # Binaries
        ###############################################################################
        set(LATENCYTEST_SOURCE LatencyTestPublisher.cpp
            LatencyTestSubscriber.cpp
            LatencyTestTypes.cpp
            main_LatencyTest.cpp
            )
        add_executable(LatencyTest ${LATENCYTEST_SOURCE})
        target_link_libraries(LatencyTest fastrtps ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})

        set(THROUGHPUTTEST_SOURCE ThroughputPublisher.cpp
            ThroughputSubscriber.cpp
            ThroughputTypes.cpp
            main_ThroughputTest.cpp
            )
        add_executable(ThroughputTest ${THROUGHPUTTEST_SOURCE})
        target_include_directories(ThroughputTest PRIVATE)
        target_link_libraries(ThroughputTest fastrtps ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})

        if(EPROSIMA_BUILD_TESTS)
            find_package(PythonInterp 3 REQUIRED)

            if(PYTHONINTERP_FOUND)
                ###############################################################################
                # LatencyTest
                ###############################################################################
                add_test(NAME LatencyTest
                    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/latency_tests.py)

                # Set test with label NoMemoryCheck
                set_property(TEST LatencyTest PROPERTY LABELS "NoMemoryCheck")

                if(WIN32)
                    set_property(TEST LatencyTest PROPERTY ENVIRONMENT
                        "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$ENV{PATH}")
                endif()
                set_property(TEST LatencyTest APPEND PROPERTY ENVIRONMENT
                    "LATENCY_TEST_BIN=$<TARGET_FILE:LatencyTest>")
                if(SECURITY)
                    set_property(TEST LatencyTest APPEND PROPERTY ENVIRONMENT
                        "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs")
                endif()

                ###############################################################################
                # ThroughputTest16
                ###############################################################################
                add_test(NAME ThroughputTest16
                    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/throughput_tests.py 16)

                # Set test with label NoMemoryCheck
                set_property(TEST ThroughputTest16 PROPERTY LABELS "NoMemoryCheck")

                if(WIN32)
                    set_property(TEST ThroughputTest16 PROPERTY ENVIRONMENT
                        "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$ENV{PATH}")
                endif()
                set_property(TEST ThroughputTest16 APPEND PROPERTY ENVIRONMENT
                    "THROUGHPUT_TEST_BIN=$<TARGET_FILE:ThroughputTest>")
                set_property(TEST ThroughputTest16 APPEND PROPERTY ENVIRONMENT
                    "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
                if(SECURITY)
                    set_property(TEST ThroughputTest16 APPEND PROPERTY ENVIRONMENT
                        "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs")
                endif()

                ###############################################################################
                # ThroughputTest32
                ###############################################################################
                add_test(NAME ThroughputTest32
                    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/throughput_tests.py 32)

                # Set test with label NoMemoryCheck
                set_property(TEST ThroughputTest32 PROPERTY LABELS "NoMemoryCheck")

                if(WIN32)
                    set_property(TEST ThroughputTest32 PROPERTY ENVIRONMENT
                        "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$ENV{PATH}")
                endif()
                set_property(TEST ThroughputTest32 APPEND PROPERTY ENVIRONMENT
                    "THROUGHPUT_TEST_BIN=$<TARGET_FILE:ThroughputTest>")
                set_property(TEST ThroughputTest32 APPEND PROPERTY ENVIRONMENT
                    "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
                if(SECURITY)
                    set_property(TEST ThroughputTest32 APPEND PROPERTY ENVIRONMENT
                        "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs")
                endif()

                ###############################################################################
                # ThroughputTest64
                ###############################################################################
                add_test(NAME ThroughputTest64
                    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/throughput_tests.py 64)

                # Set test with label NoMemoryCheck
                set_property(TEST ThroughputTest64 PROPERTY LABELS "NoMemoryCheck")

                if(WIN32)
                    set_property(TEST ThroughputTest64 PROPERTY ENVIRONMENT
                        "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$ENV{PATH}")
                endif()
                set_property(TEST ThroughputTest64 APPEND PROPERTY ENVIRONMENT
                    "THROUGHPUT_TEST_BIN=$<TARGET_FILE:ThroughputTest>")
                set_property(TEST ThroughputTest64 APPEND PROPERTY ENVIRONMENT
                    "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
                if(SECURITY)
                    set_property(TEST ThroughputTest64 APPEND PROPERTY ENVIRONMENT
                        "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs")
                endif()

                ###############################################################################
                # ThroughputTest128
                ###############################################################################
                add_test(NAME ThroughputTest128
                    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/throughput_tests.py 128)

                # Set test with label NoMemoryCheck
                set_property(TEST ThroughputTest128 PROPERTY LABELS "NoMemoryCheck")

                if(WIN32)
                    set_property(TEST ThroughputTest128 PROPERTY ENVIRONMENT
                        "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$ENV{PATH}")
                endif()
                set_property(TEST ThroughputTest128 APPEND PROPERTY ENVIRONMENT
                    "THROUGHPUT_TEST_BIN=$<TARGET_FILE:ThroughputTest>")
                set_property(TEST ThroughputTest128 APPEND PROPERTY ENVIRONMENT
                    "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
                if(SECURITY)
                    set_property(TEST ThroughputTest128 APPEND PROPERTY ENVIRONMENT
                        "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs")
                endif()

                ###############################################################################
                # ThroughputTest256
                ###############################################################################
                add_test(NAME ThroughputTest256
                    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/throughput_tests.py 256)

                # Set test with label NoMemoryCheck
                set_property(TEST ThroughputTest256 PROPERTY LABELS "NoMemoryCheck")

                if(WIN32)
                    set_property(TEST ThroughputTest256 PROPERTY ENVIRONMENT
                        "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$ENV{PATH}")
                endif()
                set_property(TEST ThroughputTest256 APPEND PROPERTY ENVIRONMENT
                    "THROUGHPUT_TEST_BIN=$<TARGET_FILE:ThroughputTest>")
                set_property(TEST ThroughputTest256 APPEND PROPERTY ENVIRONMENT
                    "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
                if(SECURITY)
                    set_property(TEST ThroughputTest256 APPEND PROPERTY ENVIRONMENT
                        "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs")
                endif()

                ###############################################################################
                # ThroughputTest512
                ###############################################################################
                add_test(NAME ThroughputTest512
                    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/throughput_tests.py 512)

                # Set test with label NoMemoryCheck
                set_property(TEST ThroughputTest512 PROPERTY LABELS "NoMemoryCheck")

                if(WIN32)
                    set_property(TEST ThroughputTest512 PROPERTY ENVIRONMENT
                        "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$ENV{PATH}")
                endif()
                set_property(TEST ThroughputTest512 APPEND PROPERTY ENVIRONMENT
                    "THROUGHPUT_TEST_BIN=$<TARGET_FILE:ThroughputTest>")
                set_property(TEST ThroughputTest512 APPEND PROPERTY ENVIRONMENT
                    "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
                if(SECURITY)
                    set_property(TEST ThroughputTest512 APPEND PROPERTY ENVIRONMENT
                        "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs")
                endif()

                ###############################################################################
                # ThroughputTest1024
                ###############################################################################
                add_test(NAME ThroughputTest1024
                    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/throughput_tests.py 1024)

                # Set test with label NoMemoryCheck
                set_property(TEST ThroughputTest1024 PROPERTY LABELS "NoMemoryCheck")

                if(WIN32)
                    set_property(TEST ThroughputTest1024 PROPERTY ENVIRONMENT
                        "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$ENV{PATH}")
                endif()
                set_property(TEST ThroughputTest1024 APPEND PROPERTY ENVIRONMENT
                    "THROUGHPUT_TEST_BIN=$<TARGET_FILE:ThroughputTest>")
                set_property(TEST ThroughputTest1024 APPEND PROPERTY ENVIRONMENT
                    "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
                if(SECURITY)
                    set_property(TEST ThroughputTest1024 APPEND PROPERTY ENVIRONMENT
                        "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs")
                endif()

                ###############################################################################
                # ThroughputTest2048
                ###############################################################################
                add_test(NAME ThroughputTest2048
                    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/throughput_tests.py 2048)

                # Set test with label NoMemoryCheck
                set_property(TEST ThroughputTest2048 PROPERTY LABELS "NoMemoryCheck")

                if(WIN32)
                    set_property(TEST ThroughputTest2048 PROPERTY ENVIRONMENT
                        "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$ENV{PATH}")
                endif()
                set_property(TEST ThroughputTest2048 APPEND PROPERTY ENVIRONMENT
                    "THROUGHPUT_TEST_BIN=$<TARGET_FILE:ThroughputTest>")
                set_property(TEST ThroughputTest2048 APPEND PROPERTY ENVIRONMENT
                    "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
                if(SECURITY)
                    set_property(TEST ThroughputTest2048 APPEND PROPERTY ENVIRONMENT
                        "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs")
                endif()

                ###############################################################################
                # ThroughputTest4096
                ###############################################################################
                add_test(NAME ThroughputTest4096
                    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/throughput_tests.py 4096)

                # Set test with label NoMemoryCheck
                set_property(TEST ThroughputTest4096 PROPERTY LABELS "NoMemoryCheck")

                if(WIN32)
                    set_property(TEST ThroughputTest4096 PROPERTY ENVIRONMENT
                        "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$ENV{PATH}")
                endif()
                set_property(TEST ThroughputTest4096 APPEND PROPERTY ENVIRONMENT
                    "THROUGHPUT_TEST_BIN=$<TARGET_FILE:ThroughputTest>")
                set_property(TEST ThroughputTest4096 APPEND PROPERTY ENVIRONMENT
                    "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
                if(SECURITY)
                    set_property(TEST ThroughputTest4096 APPEND PROPERTY ENVIRONMENT
                        "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs")
                endif()

                ###############################################################################
                # ThroughputTest8192
                ###############################################################################
                add_test(NAME ThroughputTest8192
                    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/throughput_tests.py 8192)

                # Set test with label NoMemoryCheck
                set_property(TEST ThroughputTest8192 PROPERTY LABELS "NoMemoryCheck")

                if(WIN32)
                    set_property(TEST ThroughputTest8192 PROPERTY ENVIRONMENT
                        "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$ENV{PATH}")
                endif()
                set_property(TEST ThroughputTest8192 APPEND PROPERTY ENVIRONMENT
                    "THROUGHPUT_TEST_BIN=$<TARGET_FILE:ThroughputTest>")
                set_property(TEST ThroughputTest8192 APPEND PROPERTY ENVIRONMENT
                    "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
                if(SECURITY)
                    set_property(TEST ThroughputTest8192 APPEND PROPERTY ENVIRONMENT
                        "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs")
                endif()
            endif()
        endif()
    endif()
endif()
