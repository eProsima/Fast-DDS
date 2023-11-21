# Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

include(GoogleTest)

function(gtest_discover_tests TARGET)
    if (WIN32)
        add_custom_command(
            TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -DTARGET=${TARGET} -DCONFIG=$<CONFIG> -DRUNTIME_LIST=$<TARGET_RUNTIME_DLLS:${TARGET}> -P ${CMAKE_SOURCE_DIR}/cmake/testing/generate_google_test_win_wrapper.cmake
            COMMAND_EXPAND_LISTS
            VERBATIM
            )

        set(CMAKE_COMMAND "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_win_wrapper_$<CONFIG>.bat")
    endif()
    _gtest_discover_tests(${TARGET} ${ARGN})
endfunction()
