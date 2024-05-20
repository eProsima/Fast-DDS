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

find_package(Python3 COMPONENTS Interpreter REQUIRED)

if(Python3_Interpreter_FOUND)

    # Set environment for tests
    set(TEST_ENVIRONMENT
        "PYTHON_VERSION=${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}")

    set(TESTS
        test_fastdds_installed
        test_fastdds_discovery
        test_ros_discovery
        test_fastdds_shm
    )

    foreach(TEST IN LISTS TESTS)
        add_test(
                NAME system.tools.fastdds.${TEST}
                COMMAND ${Python3_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/tests.py
                ${CMAKE_INSTALL_PREFIX}
                ${TEST}
            )

        # Set test properties
        set_property(
            TEST system.tools.fastdds.${TEST}
            PROPERTY LABELS "NoMemoryCheck"
        )

        # Set environment
        set_property(
            TEST system.tools.fastdds.${TEST}
            PROPERTY ENVIRONMENT ${TEST_ENVIRONMENT}
        )

    endforeach()

endif()
