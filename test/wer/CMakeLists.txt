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

cmake_minimum_required(VERSION 3.16.3)
cmake_policy(VERSION 3.16.3...3.20.3)

find_package(GTest CONFIG REQUIRED)

# The purpose of this library is provide Windows Error Reporting support to the
# testing framework. In order to do so:
#
# - gtest must not catch all exceptions (C++/SEH like segmentation fault).
#   This can be achieved modifying the flag testing::GTEST_FLAG(catch_exceptions)
#   which is like using the ctest cli option --gtest_catch_exceptions=0.
#
# - CTest prevents OS error handling in tests child processes. This can be
#   workaround by calling the windows API SetErrorMode(0)
#
# - CTest timeout mechanism uses windows API TerminateProcess() to wipe out a
#   test tree. Thus it cannot be relied on. The OS has no chance of using WER.
#   Instead a watchdog thread can be used to launch a termination exception that
#   activates WER.

add_library(wer INTERFACE)

# introduce an alias to make even external and internal linking
add_library(fastdds::wer ALIAS wer)

target_include_directories(wer INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# If the dependent test doesn't use gtest this is harmless
target_link_libraries(wer INTERFACE GTest::gtest)

# Specify the timeout time in seconds
target_compile_definitions(wer INTERFACE WER_ENABLED WER_TIMEOUT_TIME=${WER_TIMEOUT_TIME})
