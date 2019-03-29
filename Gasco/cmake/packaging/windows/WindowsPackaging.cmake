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

set(PROJECT_NAME_LARGE "Fast RTPS")
set(MSVC_ARCH x64Win64VS2017)

if(${CPACK_GENERATOR} STREQUAL "NSIS" )
    cmake_policy(SET CMP0012 NEW)

    if(OFF)
        # Define name of the NSIS file
        set(CPACK_PACKAGE_FILE_NAME eProsima_FastRTPS-1.7.1-Windows)
    else()
        # Define name of the NSIS file
        set(CPACK_PACKAGE_FILE_NAME eProsima_FastRTPS-1.7.1-${MSVC_ARCH})
    endif()

    set(CPACK_NSIS_DISPLAY_NAME "Fast RTPS 1.7.1")
    set(CPACK_NSIS_PACKAGE_NAME "eProsima Fast RTPS 1.7.1")
    set(CPACK_NSIS_URL_INFO_ABOUT "www.eprosima.com")
    set(CPACK_NSIS_CONTACT "support@eprosima.com")
    set(CPACK_NSIS_MUI_ICON "C:/Users/MiguelBarro/Documents/Fast-RTPS\\utils\\images\\icon\\eprosima_icon.ico")
    set(CPACK_PACKAGE_ICON "C:/Users/MiguelBarro/Documents/Fast-RTPS\\utils\\images\\icon\\eprosima_icon.bmp")
    set(CPACK_NSIS_INSTALL_ROOT "$VARPROGRAMFILES\\eProsima")
    set(CPACK_RESOURCE_FILE_README "C:/Users/MiguelBarro/Documents/Fast-RTPS/Gasco/README.html")
    
    # Define cmake script to copy images files and prepare auxiliary NSIS scripts.
    set(CPACK_INSTALL_SCRIPT "C:/Users/MiguelBarro/Documents/Fast-RTPS/Gasco/cmake/packaging/windows/NSISPackaging.cmake")
endif()

