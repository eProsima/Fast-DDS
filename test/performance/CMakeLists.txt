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

    add_subdirectory(latency)
    add_subdirectory(throughput)

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
