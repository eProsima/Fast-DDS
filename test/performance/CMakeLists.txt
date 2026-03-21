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
    if("${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore")
        add_definitions(-D_WIN32_WINNT=0x0603)
    else()
        add_definitions(-D_WIN32_WINNT=0x0601)
    endif()
endif()

option(VIDEO_TESTS "Activate the building and execution of performance tests" OFF)
add_subdirectory(latency)
add_subdirectory(throughput)
if(VIDEO_TESTS)
# // TODO(jlbueno): migrate to Fast DDS API
#    add_subdirectory(video)
endif()
