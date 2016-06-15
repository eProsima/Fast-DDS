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

get_filename_component(FILE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../" ABSOLUTE)

if(MSVC12)
    if(CMAKE_CL_64)
        include("${FILE_PREFIX_DIR}/x64Win64VS2013/fastrtps/cmake/fastrtpsConfig.cmake")
    else()
        include("${FILE_PREFIX_DIR}/i86Win32VS2013/fastrtps/cmake/fastrtpsConfig.cmake")
    endif()
elseif(MSVC14)
    if(CMAKE_CL_64)
        include("${FILE_PREFIX_DIR}/x64Win64VS2015/fastrtps/cmake/fastrtpsConfig.cmake")
    else()
        include("${FILE_PREFIX_DIR}/i86Win32VS2015/fastrtps/cmake/fastrtpsConfig.cmake")
    endif()
else()
    message(FATAL_ERROR "Not supported version of Visual Studio")
endif()
