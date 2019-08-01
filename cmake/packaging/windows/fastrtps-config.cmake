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

if(MSVC_VERSION EQUAL 1900)
    if(CMAKE_CL_64)
        include("${CMAKE_CURRENT_LIST_DIR}/../share/fastrtps-x64Win64VS2015/cmake/fastrtps-config.cmake")
    else()
        include("${CMAKE_CURRENT_LIST_DIR}/../share/fastrtps-i86Win32VS2015/cmake/fastrtps-config.cmake")
    endif()
elseif(MSVC_VERSION GREATER 1900)
    if(CMAKE_CL_64)
        include("${CMAKE_CURRENT_LIST_DIR}/../share/fastrtps-x64Win64VS2017/cmake/fastrtps-config.cmake")
    else()
        include("${CMAKE_CURRENT_LIST_DIR}/../share/fastrtps-i86Win32VS2017/cmake/fastrtps-config.cmake")
    endif()
else()
    message(FATAL_ERROR "Not supported version of Visual Studio")
endif()
