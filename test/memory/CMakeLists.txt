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
    if(MEMORY_TESTS)
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
        set(MEMORYTEST_SOURCE MemoryTestPublisher.cpp
            MemoryTestSubscriber.cpp
            MemoryTestTypes.cpp
            main_MemoryTest.cpp
            )
        add_executable(MemoryTest ${MEMORYTEST_SOURCE})
        target_link_libraries(MemoryTest fastrtps ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})

        if(EPROSIMA_BUILD_TESTS)
            find_package(PythonInterp 3 REQUIRED)

            if(PYTHONINTERP_FOUND)
                ###############################################################################
                # MemoryTest
                ###############################################################################
                add_test(NAME MemoryTest
                    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/memory_tests.py)

                # Set test with label NoMemoryCheck
                set_property(TEST MemoryTest PROPERTY LABELS "NoMemoryCheck")

                if(WIN32)
                    set_property(TEST MemoryTest PROPERTY ENVIRONMENT
                        "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$ENV{PATH}")
                endif()
                set_property(TEST MemoryTest APPEND PROPERTY ENVIRONMENT
                    "MEMORY_TEST_BIN=$<TARGET_FILE:MemoryTest>")
                set_property(TEST MemoryTest APPEND PROPERTY ENVIRONMENT
                    "VALGRIND_BIN=valgrind")
                if(SECURITY)
                    set_property(TEST MemoryTest APPEND PROPERTY ENVIRONMENT
                        "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs")
                endif()

            endif()
        endif()
    endif()
endif()
