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

find_package(PythonInterp 3 REQUIRED)

add_test(NAME memory_profiling
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/profiling.py)

set_property(TEST memory_profiling PROPERTY LABELS "NoMemoryCheck")
if(WIN32)
    set_property(TEST memory_profiling PROPERTY ENVIRONMENT
        "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;${BOOST_LIBRARYDIR}\\;$ENV{PATH}")
endif()
set_property(TEST memory_profiling APPEND PROPERTY ENVIRONMENT
    "PROFILING_BINS=$<TARGET_FILE:RTPSCommsProfiling>\\;$<TARGET_FILE:RTPSPubSubProfiling>")
set_property(TEST memory_profiling APPEND PROPERTY ENVIRONMENT
             "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
set_property(TEST memory_profiling APPEND PROPERTY ENVIRONMENT
             "VALGRIND_BIN=${CTEST_MEMORYCHECK_COMMAND}")
