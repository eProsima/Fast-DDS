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
# CMake modules tests
###############################################################################

set(build_dir "${fastdds_BINARY_DIR}/test/force_cxx")
set(module_dir "${fastdds_SOURCE_DIR}/cmake/common")
set(script "${CMAKE_CURRENT_LIST_DIR}/force_cxx/build.cmake")

# The C++ sources use fold expressions which were introduced in C++17 

# Tests that C++17 sources can be built enforcing C++17 through CMake
add_test(NAME cmake.force_standard.positive.builtin
        COMMAND ${CMAKE_COMMAND}
                -DFORCE_CXX=17 -DTEST_FALLBACK=OFF
                -DEPROSIMA_MODULE_PATH=${module_dir}
                -DBUILD_DIR=${build_dir}
                -P ${script}
        )

# Tests that C++17 sources can be built enforcing C++17 manually
add_test(NAME cmake.force_standard.positive.manual
        COMMAND ${CMAKE_COMMAND}
                -DFORCE_CXX=17 -DTEST_FALLBACK=ON
                -DEPROSIMA_MODULE_PATH=${module_dir}
                -DBUILD_DIR=${build_dir}
                -P ${script}
        )

# Tests that C++17 sources cannot be built enforcing C++14 through CMake
add_test(NAME cmake.force_standard.negative.builtin
        COMMAND ${CMAKE_COMMAND}
                -DFORCE_CXX=14 -DTEST_FALLBACK=OFF
                -DEPROSIMA_MODULE_PATH=${module_dir}
                -DBUILD_DIR=${build_dir}
                -P ${script}
        )

# Tests that C++17 sources cannot be built enforcing C++14 manually
add_test(NAME cmake.force_standard.negative.manual
        COMMAND ${CMAKE_COMMAND}
                -DFORCE_CXX=14 -DTEST_FALLBACK=ON
                -DEPROSIMA_MODULE_PATH=${module_dir}
                -DBUILD_DIR=${build_dir}
                -P ${script}
        )

# set the negative testing
set_tests_properties(
    cmake.force_standard.negative.builtin
    cmake.force_standard.negative.manual
    PROPERTIES WILL_FAIL TRUE)
