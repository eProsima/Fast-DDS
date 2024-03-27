# Copyright 2018-2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

cmake_minimum_required(VERSION 3.20)

project(fastdds_allocation_test VERSION 1 LANGUAGES CXX)

# Default to C++11
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Find requirements (Version 1.4 is required, but their CMake does not set the version correctly)
find_package(osrf_testing_tools_cpp)

if (osrf_testing_tools_cpp_FOUND)
    message(STATUS "Configuring AllocationTest...")
    file(GLOB ALLOCTEST_EXAMPLE_SOURCES_CXX "*.cxx")
    file(GLOB ALLOCTEST_EXAMPLE_SOURCES_CPP "*.cpp")

    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test_xml_profile.xml
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
        )
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test.sh
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
        )
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test_with_memory_tool.sh
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
        )

    add_executable(AllocationTest ${ALLOCTEST_EXAMPLE_SOURCES_CXX} ${ALLOCTEST_EXAMPLE_SOURCES_CPP})
    target_link_libraries(AllocationTest fastdds fastcdr foonathan_memory osrf_testing_tools_cpp::memory_tools)
    install(TARGETS AllocationTest
        RUNTIME DESTINATION test/profiling/allocations/${BIN_INSTALL_DIR})
else(osrf_testing_tools_cpp_FOUND)
    message(STATUS "osrf_testing_tools_cpp not found, skipping AllocationTest.")
endif(osrf_testing_tools_cpp_FOUND)
