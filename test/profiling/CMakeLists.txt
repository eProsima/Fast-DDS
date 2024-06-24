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

find_package(Threads REQUIRED)

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        BOOST_ASIO_STANDALONE
        ASIO_STANDALONE
        $<$<AND:$<BOOL:${WIN32}>,$<STREQUAL:"${CMAKE_SYSTEM_NAME}","WindowsStore">>:_WIN32_WINNT=0x0603>
        $<$<AND:$<BOOL:${WIN32}>,$<NOT:$<STREQUAL:"${CMAKE_SYSTEM_NAME}","WindowsStore">>>:_WIN32_WINNT=0x0601>
        $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
        $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
        )
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        BOOST_ASIO_STANDALONE
        ASIO_STANDALONE
        $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
        $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
        )
endif()

###############################################################################
# Binaries
###############################################################################
set(MEMORYTEST_SOURCE MemoryTestPublisher.cpp
    MemoryTestSubscriber.cpp
    MemoryTestTypes.cpp
    main_MemoryTest.cpp
    )
add_executable(MemoryTest ${MEMORYTEST_SOURCE})
target_include_directories(MemoryTest PRIVATE ${Asio_INCLUDE_DIR})
target_link_libraries(MemoryTest fastdds foonathan_memory fastdds::optionparser ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})

configure_file("cycles_tests.py" "cycles_tests.py")
configure_file("memory_tests.py" "memory_tests.py")
configure_file("memory_analysis.py" "memory_analysis.py")

find_package(Python3 COMPONENTS Interpreter REQUIRED)

if(Python3_Interpreter_FOUND)
    ###############################################################################
    # MemoryTest
    ###############################################################################
    find_program(VALGRIND_PROGRAM NAMES valgrind)

    # If valgrind is not found, set it to "valgrind" so that the test can be run with
    # the appropriate environment set at runtime.
    if (${VALGRIND_PROGRAM} STREQUAL "valgrind-NOTFOUND")
        set(VALGRIND_PROGRAM "valgrind")
    endif()

    add_test(NAME MemoryTest
        COMMAND ${Python3_EXECUTABLE} memory_tests.py)

    # Set test with label NoMemoryCheck
    set_property(TEST MemoryTest PROPERTY LABELS "NoMemoryCheck")

    if(WIN32)
        set_property(TEST MemoryTest PROPERTY ENVIRONMENT
            "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$ENV{PATH}")
    endif()
    set_property(TEST MemoryTest APPEND PROPERTY ENVIRONMENT
        "PROFILING_BINS=$<TARGET_FILE:MemoryTest>")
    set_property(TEST MemoryTest APPEND PROPERTY ENVIRONMENT
        "VALGRIND_BIN=${VALGRIND_PROGRAM}")
    if(SECURITY)
        set_property(TEST MemoryTest APPEND PROPERTY ENVIRONMENT
            "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs")
    endif()

    ###############################################################################
    # CyclesTest
    ###############################################################################
    add_test(NAME CyclesTest
        COMMAND ${Python3_EXECUTABLE} cycles_tests.py)

    # Set test with label NoMemoryCheck
    set_property(TEST CyclesTest PROPERTY LABELS "NoMemoryCheck")

    if(WIN32)
        set_property(TEST CyclesTest PROPERTY ENVIRONMENT
            "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$ENV{PATH}")
    endif()
    set_property(TEST CyclesTest APPEND PROPERTY ENVIRONMENT
        "PROFILING_BINS=$<TARGET_FILE:MemoryTest>")
    set_property(TEST CyclesTest APPEND PROPERTY ENVIRONMENT
        "VALGRIND_BIN=${VALGRIND_PROGRAM}")
    if(SECURITY)
        set_property(TEST CyclesTest APPEND PROPERTY ENVIRONMENT
            "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs")
    endif()
endif()

add_subdirectory(allocations)
