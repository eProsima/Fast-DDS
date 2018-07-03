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

macro(set_sources)
    set(${PROJECT_NAME}_SOURCES
        ${${PROJECT_NAME}_SOURCES}
        ${ARGN}
        )

    set_property(GLOBAL PROPERTY ${PROJECT_NAME}_SOURCES_PROPERTY ${${PROJECT_NAME}_SOURCES})
endmacro()

macro(set_public_headers_directory abs_directory rel_directory)
    # Parse arguments.
    set(args_to_install_)
    set(options INSTALL)
    set(oneValueArgs DESTINATION COMPONENT)
    cmake_parse_arguments(FIND_PH "${options}" "${oneValueArgs}" "" ${ARGN})

    if(FIND_PH_DESTINATION)
        list(APPEND args_to_install_ "DESTINATION" "${FIND_PH_DESTINATION}")
    endif()
    if(FIND_PH_COMPONENT)
        list(APPEND args_to_install_ "COMPONENT" "${FIND_PH_COMPONENT}")
    endif()

    install(DIRECTORY ${abs_directory}/${rel_directory}
        ${args_to_install_}
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
        PATTERN "*.ipp"
        )

    if(FIND_PH_INSTALL)
        get_property(${PROJECT_NAME}_PUBLIC_HEADERS_DIRECTORIES GLOBAL PROPERTY ${PROJECT_NAME}_PUBLIC_HEADERS_DIRECTORIES_PROPERTY)
        set(${PROJECT_NAME}_PUBLIC_HEADERS_DIRECTORIES ${${PROJECT_NAME}_PUBLIC_HEADERS_DIRECTORIES} ${abs_directory})
        set_property(GLOBAL PROPERTY ${PROJECT_NAME}_PUBLIC_HEADERS_DIRECTORIES_PROPERTY ${${PROJECT_NAME}_PUBLIC_HEADERS_DIRECTORIES})
    endif()

endmacro()

macro(set_public_header abs_directory rel_directory file)
    set(args_to_install_)
    set(options INSTALL)
    set(oneValueArgs DESTINATION COMPONENT)
    cmake_parse_arguments(FIND_PH "${options}" "${oneValueArgs}" "" ${ARGN})

    if(FIND_PH_DESTINATION)
        list(APPEND args_to_install_ "DESTINATION" "${FIND_PH_DESTINATION}")
    endif()
    if(FIND_PH_COMPONENT)
        list(APPEND args_to_install_ "COMPONENT" "${FIND_PH_COMPONENT}")
    endif()

    install(FILES ${abs_directory}/${rel_directory}/${file}
        ${args_to_install_}
        )

    if(FIND_PH_INSTALL)
        get_property(${PROJECT_NAME}_PUBLIC_HEADERS_FILES GLOBAL PROPERTY ${PROJECT_NAME}_PUBLIC_HEADERS_FILES_PROPERTY)
        set(${PROJECT_NAME}_PUBLIC_HEADERS_FILES ${${PROJECT_NAME}_PUBLIC_HEADERS_FILES} ${rel_directory}/${file})
        set_property(GLOBAL PROPERTY ${PROJECT_NAME}_PUBLIC_HEADERS_FILES_PROPERTY ${${PROJECT_NAME}_PUBLIC_HEADERS_FILES})
    endif()
endmacro()
