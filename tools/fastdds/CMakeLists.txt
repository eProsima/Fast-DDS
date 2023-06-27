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

# empty LANGUAGES defaults to C in Ubuntu and breaks the build
project(fastdds-tool LANGUAGES CXX)

# copy fastdds.py in build dir
if(EPROSIMA_BUILD)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/fastdds.py
        ${CMAKE_CURRENT_BINARY_DIR}/fastdds.py COPYONLY)

    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/discovery
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shm
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/xml_ci
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

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
        ${PROJECT_SOURCE_DIR}/ros-discovery.bat
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
    )

else()

    # POSIX
    install(FILES ${PROJECT_SOURCE_DIR}/fastdds
        ${PROJECT_SOURCE_DIR}/ros-discovery
        PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
    )

endif()
