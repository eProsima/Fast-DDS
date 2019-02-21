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
    target_link_libraries(LatencyTest fastrtps foonathan_memory ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})

    set(THROUGHPUTTEST_SOURCE ThroughputPublisher.cpp
        ThroughputSubscriber.cpp
        ThroughputTypes.cpp
        main_ThroughputTest.cpp
        )
    add_executable(ThroughputTest ${THROUGHPUTTEST_SOURCE})
    target_include_directories(ThroughputTest PRIVATE)
    target_link_libraries(ThroughputTest fastrtps foonathan_memory ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})

    if(WIN32)
        if (EXISTS $ENV{GSTREAMER_1_0_ROOT_X86_64})
            if (EXISTS "$ENV{GSTREAMER_1_0_ROOT_X86_64}/include/gstreamer-1.0/gst/gstversion.h")
                file (READ "$ENV{GSTREAMER_1_0_ROOT_X86_64}/include/gstreamer-1.0/gst/gstversion.h" GSTREAMER_VERSION_CONTENTS)
                STRING(REGEX MATCH "#define +GST_VERSION_MAJOR +\\(([0-9]+)\\)" _dummy "${GSTREAMER_VERSION_CONTENTS}")
                SET(GSTREAMER_VERSION_MAJOR "${CMAKE_MATCH_1}")

                STRING(REGEX MATCH "#define +GST_VERSION_MINOR +\\(([0-9]+)\\)" _dummy "${GSTREAMER_VERSION_CONTENTS}")
                SET(GSTREAMER_VERSION_MINOR "${CMAKE_MATCH_1}")

                STRING(REGEX MATCH "#define +GST_VERSION_MICRO +\\(([0-9]+)\\)" _dummy "${GSTREAMER_VERSION_CONTENTS}")
                SET(GSTREAMER_VERSION_MICRO "${CMAKE_MATCH_1}")

                SET(GSTREAMER_VERSION "${GSTREAMER_VERSION_MAJOR}.${GSTREAMER_VERSION_MINOR}.${GSTREAMER_VERSION_MICRO}")
            endif ()
        endif ()

        if (GSTREAMER_VERSION)
            if ("${GSTREAMER_VERSION}" VERSION_LESS "1.4")
                message(STATUS "Windows: GStreamer version less than the minimum required. ${GSTREAMER_VERSION}")
            else()
                SET(GST_FOUND TRUE)
                message(STATUS "Windows: GStreamer FOUND. Version: ${GSTREAMER_VERSION}")
            endif ()
        endif()
    else()
        find_package(PkgConfig)
        pkg_check_modules(GST gstreamer-1.0>=1.4)
    endif()

    if(GST_FOUND)
        set(VIDEOTEST_SOURCE
            VideoTestPublisher.cpp
            VideoTestSubscriber.cpp
            VideoTestTypes.cpp
            main_VideoTest.cpp
            )
        add_executable(VideoTest ${VIDEOTEST_SOURCE})

        target_compile_options(VideoTest PUBLIC ${GST_CFLAGS})

        if(NOT WIN32)
            target_include_directories(VideoTest PUBLIC
                ${GST_INCLUDE_DIRS}
                )
        else()
            target_include_directories(VideoTest PUBLIC
                $ENV{GSTREAMER_1_0_ROOT_X86_64}/include
                $ENV{GSTREAMER_1_0_ROOT_X86_64}/include/glib-2.0
                $ENV{GSTREAMER_1_0_ROOT_X86_64}/include/gstreamer-1.0
                $ENV{GSTREAMER_1_0_ROOT_X86_64}/lib/glib-2.0/include)
        endif()

        if (WIN32)
            target_link_libraries(VideoTest fastrtps foonathan_memory
                ${CMAKE_THREAD_LIBS_INIT}
                ${CMAKE_DL_LIBS}
                ${GST_LIBRARIES}
                $ENV{GSTREAMER_1_0_ROOT_X86_64}/lib/gstapp-1.0.lib
                $ENV{GSTREAMER_1_0_ROOT_X86_64}/lib/gstbase-1.0.lib
                $ENV{GSTREAMER_1_0_ROOT_X86_64}/lib/gstreamer-1.0.lib
                $ENV{GSTREAMER_1_0_ROOT_X86_64}/lib/gobject-2.0.lib
                $ENV{GSTREAMER_1_0_ROOT_X86_64}/lib/glib-2.0.lib)
        else()
            target_link_libraries(VideoTest fastrtps foonathan_memory
                ${CMAKE_THREAD_LIBS_INIT}
                ${CMAKE_DL_LIBS}
                ${GST_LIBRARIES}
                gstapp-1.0
                )
        endif()
    else()
        message(STATUS "GStreamer libraries not found")
    endif()

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

        if(GST_FOUND)
            ###############################################################################
            # VideoTest
            ###############################################################################
            add_test(NAME VideoTest
                COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/video_tests.py)

            # Set test with label NoMemoryCheck
            set_property(TEST VideoTest PROPERTY LABELS "NoMemoryCheck")

            if(WIN32)
                set_property(TEST VideoTest PROPERTY ENVIRONMENT
                    "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$ENV{PATH}")
            endif()
            set_property(TEST VideoTest APPEND PROPERTY ENVIRONMENT
                "VIDEO_TEST_BIN=$<TARGET_FILE:VideoTest>")
            if(SECURITY)
                set_property(TEST VideoTest APPEND PROPERTY ENVIRONMENT
                    "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs")
            endif()
        endif()

    endif()
endif()
