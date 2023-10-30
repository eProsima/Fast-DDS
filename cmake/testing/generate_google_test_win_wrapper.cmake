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

if (NOT DEFINED TARGET)
    message(FATAL_ERROR "This scrips needs TARGET variable set")
endif()
if (NOT DEFINED CONFIG)
    message(FATAL_ERROR "This scrips needs CONFIG variable set")
endif()
if (NOT DEFINED RUNTIME_LIST)
    message(FATAL_ERROR "This scrips needs RUNTIME_LIST variable set")
endif()

set(_path "")

foreach(_runtime_dll IN LISTS RUNTIME_LIST)
    cmake_path(GET _runtime_dll PARENT_PATH _runtime_dll_path)
    cmake_path(NATIVE_PATH _runtime_dll_path _runtime_dll_path_native)
    list(APPEND _path "${_runtime_dll_path_native}")
endforeach()

list(REMOVE_DUPLICATES _path)

cmake_path(NATIVE_PATH CMAKE_COMMAND _cmake_command)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_win_wrapper_${CONFIG}.bat" "
@ECHO OFF
set \"PATH=${_path};%PATH%\"
\"${_cmake_command}\" %*
")
