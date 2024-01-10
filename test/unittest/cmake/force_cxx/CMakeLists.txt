# Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

###############################################################################
# Auxiliary project for check_configuration.cmake module testing
###############################################################################
cmake_minimum_required(VERSION 3.20)

# setup
option(TEST_FALLBACK "Ignore CMake builtin instrospection and use our original fallback" OFF)

# Test preconditions
if(NOT FORCE_CXX)
    message(FATAL_ERROR "The FORCE_CXX variable must specify the C++ standard to follow")
endif()

if(NOT EPROSIMA_MODULE_PATH)
    message(FATAL_ERROR "check_configuration.cmake path must be specified")
endif()

list(APPEND CMAKE_MODULE_PATH "${EPROSIMA_MODULE_PATH}")

project(std_force_test VERSION 1.0.0 LANGUAGES CXX)

include(check_configuration)

if(TEST_FALLBACK)
    check_stdcxx(${FORCE_CXX} "run_fallback_test")
else()
    check_stdcxx(${FORCE_CXX})
endif()

add_executable(std_force_test fold.cpp)
