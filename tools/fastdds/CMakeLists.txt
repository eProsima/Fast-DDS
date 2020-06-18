# Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

project(fastdds-tool)

###############################################################################
# Installation
###############################################################################

install(DIRECTORY ${PROJECT_SOURCE_DIR}/
    DESTINATION ${CMAKE_INSTALL_PREFIX}/tools/fastdds
    USE_SOURCE_PERMISSIONS
    FILES_MATCHING 
        PATTERN "*.py"
        PATTERN "__pycache__" EXCLUDE
)

install(DIRECTORY DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

if (MSVC)

    # Windows
    install(FILES ${PROJECT_SOURCE_DIR}/fastdds.bat
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
    )

else()

    # POSIX
    set(fastdds_tool_target "../tools/fastdds/fastdds.py")
    set(fastdds_tool_link ${CMAKE_INSTALL_PREFIX}/bin/fastdds)
    install(CODE "execute_process(COMMAND ln -s -f \"${fastdds_tool_target}\" \"${fastdds_tool_link}\")")

endif()
