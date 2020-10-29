# Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
# Create tests                                                            #
###########################################################################

find_package(PythonInterp 3 REQUIRED)

if(PYTHONINTERP_FOUND)

    set(TESTS
        test_fast_discovery_closure
    )

    # windows auxiliary script to fork test execution
    set(PWS_LAUNCHER
        ${CMAKE_CURRENT_SOURCE_DIR}/launcher.ps1
    )

    # Calculate environment
    set(TEST_ENVIRONMENT 
        "PATH=$ENV{PATH};$<TARGET_FILE_DIR:fastcdr>;$<TARGET_FILE_DIR:fastrtps>")

    if(WIN32)

        if(TARGET tinyxml2 OR TARGET tinyxml2::tinyxml2)
            set(TEST_ENVIRONMENT 
                "${TEST_ENVIRONMENT};$<TARGET_FILE_DIR:${TINYXML2_LIBRARY}>")
        elseif(EXISTS TINYXML2_LIBRARY) 
            get_filename_component(
                TINYXML2_LIBRARY_DIR ${TINYXML2_LIBRARY} DIRECTORY)
            set(TEST_ENVIRONMENT 
                "${TEST_ENVIRONMENT};${TINYXML2_LIBRARY_DIR}")
            unset(TINYXML2_LIBRARY_DIR)
        endif()

        string(REPLACE ";" "\\;" TEST_ENVIRONMENT "${TEST_ENVIRONMENT}")

    endif(WIN32)

    # populate the tests
    foreach(TEST IN LISTS TESTS)

        # CTest has issues with signals on windows. We forked the test
        # execution using an auxiliary powershell script
        if(WIN32)
           add_test(
                    NAME system.tools.fastdds.${TEST}
                    COMMAND powershell "-File" ${PWS_LAUNCHER}
                            ${PYTHON_EXECUTABLE}
                            ${CMAKE_CURRENT_SOURCE_DIR}/tests.py
                            $<TARGET_FILE:fast-discovery-server>
                            ${TEST}
                )
        else()
            add_test(
                    NAME system.tools.fastdds.${TEST}
                    COMMAND ${PYTHON_EXECUTABLE}
                            ${CMAKE_CURRENT_SOURCE_DIR}/tests.py
                            $<TARGET_FILE:fast-discovery-server>
                            ${TEST}
                )
        endif()

        # Set test properties
        set_tests_properties(
            system.tools.fastdds.${TEST}
            PROPERTIES
                LABELS "NoMemoryCheck"
                ENVIRONMENT "${TEST_ENVIRONMENT}"
            )

    endforeach()

    unset(TEST_ENVIRONMENT)

endif()
