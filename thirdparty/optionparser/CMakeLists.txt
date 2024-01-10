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

cmake_minimum_required(VERSION 3.20)

#####################################################################################
#                             Interface Library
#####################################################################################

add_library(optionparser INTERFACE)

# introduce an alias to make even external and internal linking
add_library(fastdds::optionparser ALIAS optionparser)

target_include_directories(optionparser INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/fastdds/thirdparty/optionparser>
    )

###############################################################################
# Installation
###############################################################################

install(TARGETS optionparser
        EXPORT optionparser-targets
        COMPONENT libraries
        )

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DESTINATION ${INCLUDE_INSTALL_DIR}/fastdds/thirdparty
    COMPONENT libraries
    FILES_MATCHING
    PATTERN "*.h"
    PATTERN "*.hpp"
    )

if(INSTALLER_PLATFORM)
    set(INSTALL_DESTINATION_PATH ${DATA_INSTALL_DIR}/${CMAKE_PROJECT_NAME}-${INSTALLER_PLATFORM}/cmake)
else()
    set(INSTALL_DESTINATION_PATH ${DATA_INSTALL_DIR}/${CMAKE_PROJECT_NAME}/cmake)
endif()

# export library
install(EXPORT optionparser-targets
        DESTINATION ${INSTALL_DESTINATION_PATH}
        NAMESPACE fastdds::
        COMPONENT libraries
        )
