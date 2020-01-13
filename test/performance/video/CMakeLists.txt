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
# Find GStreamer                                                          #
###########################################################################
if(WIN32)
    if (EXISTS $ENV{GSTREAMER_1_0_ROOT_X86_64})
        if (EXISTS "$ENV{GSTREAMER_1_0_ROOT_X86_64}/include/gstreamer-1.0/gst/gstversion.h")
            # Get the version file
            file (
                READ "$ENV{GSTREAMER_1_0_ROOT_X86_64}/include/gstreamer-1.0/gst/gstversion.h"
                GSTREAMER_VERSION_CONTENTS
            )

            # Find the version major
            STRING(
                REGEX MATCH "#define +GST_VERSION_MAJOR +\\(([0-9]+)\\)"
                _dummy
                "${GSTREAMER_VERSION_CONTENTS}"
            )
            SET(GSTREAMER_VERSION_MAJOR "${CMAKE_MATCH_1}")

            # Find the version minor
            STRING(
                REGEX MATCH "#define +GST_VERSION_MINOR +\\(([0-9]+)\\)"
                _dummy
                "${GSTREAMER_VERSION_CONTENTS}"
            )
            SET(GSTREAMER_VERSION_MINOR "${CMAKE_MATCH_1}")

            # Find the version micro
            STRING(REGEX MATCH "#define +GST_VERSION_MICRO +\\(([0-9]+)\\)"
                _dummy "${GSTREAMER_VERSION_CONTENTS}"
            )
            SET(GSTREAMER_VERSION_MICRO "${CMAKE_MATCH_1}")

            # Construct the version
            SET(GSTREAMER_VERSION "${GSTREAMER_VERSION_MAJOR}.${GSTREAMER_VERSION_MINOR}.${GSTREAMER_VERSION_MICRO}")
        endif ()
    endif ()

    # Check the version
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
    ###########################################################################
    # Create and link executable                                              #
    ###########################################################################
    set(
        VIDEOTEST_SOURCE
        VideoTestPublisher.cpp
        VideoTestSubscriber.cpp
        VideoTestTypes.cpp
        main_VideoTest.cpp
    )
    add_executable(VideoTest ${VIDEOTEST_SOURCE})
    target_compile_options(VideoTest PUBLIC ${GST_CFLAGS})

    if(NOT WIN32)
        target_include_directories(
            VideoTest
            PUBLIC
            ${GST_INCLUDE_DIRS}
        )
    else()
        target_include_directories(
            VideoTest
            PUBLIC
            $ENV{GSTREAMER_1_0_ROOT_X86_64}/include
            $ENV{GSTREAMER_1_0_ROOT_X86_64}/include/glib-2.0
            $ENV{GSTREAMER_1_0_ROOT_X86_64}/include/gstreamer-1.0
            $ENV{GSTREAMER_1_0_ROOT_X86_64}/lib/glib-2.0/include
        )
    endif()

    if (WIN32)
        target_link_libraries(
            VideoTest
            fastrtps
            foonathan_memory
            ${CMAKE_THREAD_LIBS_INIT}
            ${CMAKE_DL_LIBS}
            ${GST_LIBRARIES}
            $ENV{GSTREAMER_1_0_ROOT_X86_64}/lib/gstapp-1.0.lib
            $ENV{GSTREAMER_1_0_ROOT_X86_64}/lib/gstbase-1.0.lib
            $ENV{GSTREAMER_1_0_ROOT_X86_64}/lib/gstreamer-1.0.lib
            $ENV{GSTREAMER_1_0_ROOT_X86_64}/lib/gobject-2.0.lib
            $ENV{GSTREAMER_1_0_ROOT_X86_64}/lib/glib-2.0.lib
        )
    else()
        target_link_libraries(
            VideoTest
            fastrtps
            foonathan_memory
            ${CMAKE_THREAD_LIBS_INIT}
            ${CMAKE_DL_LIBS}
            ${GST_LIBRARIES}
            gstapp-1.0
        )
    endif()

    ###########################################################################
    # List Video tests                                                        #
    ###########################################################################
    set(
        VIDEO_TEST_LIST
        interprocess_best_effort
        interprocess_reliable
        interprocess_best_effort_tcp
        interprocess_reliable_tcp
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
        if(GST_FOUND)
            # Loop over the test names
            foreach(video_test_name ${VIDEO_TEST_LIST})
                # Add the test
                add_test(
                    NAME performance.video.${video_test_name}
                    COMMAND ${PYTHON_EXECUTABLE}
                    ${CMAKE_CURRENT_SOURCE_DIR}/video_tests.py
                    --xml_file ${CMAKE_CURRENT_SOURCE_DIR}/xml/${video_test_name}.xml
                )

                # Set test properties
                set_property(
                    TEST performance.video.${video_test_name}
                    PROPERTY LABELS "NoMemoryCheck"
                )
                set_property(
                    TEST performance.video.${video_test_name}
                    APPEND PROPERTY ENVIRONMENT "VIDEO_TEST_BIN=$<TARGET_FILE:VideoTest>"
                )
                if(WIN32)
                    set(WIN_PATH "$ENV{PATH}")
                    set(WIN_PATH "$<TARGET_FILE_DIR:${PROJECT_NAME}>;$ENV{PATH}")
                    string(REPLACE ";" "\\;" WIN_PATH "${WIN_PATH}")
                    set_property(
                        TEST performance.video.${video_test_name}
                        APPEND PROPERTY ENVIRONMENT "PATH=${WIN_PATH}"
                    )
                endif()

                # If there is security, add a secure test as well
                if(SECURITY)
                    # Add the secure verison
                    add_test(
                        NAME performance.video.${video_test_name}_security
                        COMMAND ${PYTHON_EXECUTABLE}
                        ${CMAKE_CURRENT_SOURCE_DIR}/video_tests.py
                        --xml_file ${CMAKE_CURRENT_SOURCE_DIR}/xml/${video_test_name}.xml
                    )

                    # Set test properties
                    set_property(
                        TEST performance.video.${video_test_name}_security
                        APPEND PROPERTY ENVIRONMENT "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs"
                    )
                    set_property(
                        TEST performance.video.${video_test_name}_security
                        APPEND PROPERTY ENVIRONMENT "VIDEO_TEST_BIN=$<TARGET_FILE:VideoTest>"
                    )
                    if(WIN32)
                        set(WIN_PATH "$ENV{PATH}")
                        set(WIN_PATH "$<TARGET_FILE_DIR:${PROJECT_NAME}>;$ENV{PATH}")
                        string(REPLACE ";" "\\;" WIN_PATH "${WIN_PATH}")
                        set_property(
                            TEST performance.video.${video_test_name}_security
                            APPEND PROPERTY ENVIRONMENT "PATH=${WIN_PATH}"
                        )
                    endif()
                endif()
            endforeach(video_test_name)
        endif()
    endif()
else()
    message(STATUS "GStreamer libraries not found. Could not compile VideoTests")
endif()
