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
    set(args_to_install_)
    set(next_)
    set(install_ FALSE)

    foreach(arg ${ARGN})
        if(next_)
            if("${next_}" STREQUAL "DESTINATION")
                list(APPEND args_to_install_ "DESTINATION" "${arg}")
            elseif("${next_}" STREQUAL "COMPONENT")
                list(APPEND args_to_install_ "COMPONENT" "${arg}")
            else()
                message(FATAL_ERROR "set_public_headers_directory: assert error")
            endif()
            set(next_)
        elseif("${arg}" STREQUAL "DESTINATION")
            set(next_ "${arg}")
        elseif("${arg}" STREQUAL "COMPONENT")
            set(next_ "${arg}")
        elseif("${arg}" STREQUAL "INSTALL")
            set(install_ TRUE)
        else()
            message(FATAL_ERROR "set_public_headers_directory: Unknown argmuent ${arg}")
        endif()
    endforeach()

    install(DIRECTORY ${abs_directory}/${rel_directory}
        ${args_to_install_}
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
        PATTERN "*.ipp"
        )

    if(install_)
        get_property(${PROJECT_NAME}_PUBLIC_HEADERS_DIRECTORIES GLOBAL PROPERTY ${PROJECT_NAME}_PUBLIC_HEADERS_DIRECTORIES_PROPERTY)
        set(${PROJECT_NAME}_PUBLIC_HEADERS_DIRECTORIES ${${PROJECT_NAME}_PUBLIC_HEADERS_DIRECTORIES} ${abs_directory})
        set_property(GLOBAL PROPERTY ${PROJECT_NAME}_PUBLIC_HEADERS_DIRECTORIES_PROPERTY ${${PROJECT_NAME}_PUBLIC_HEADERS_DIRECTORIES})
    endif()

endmacro()

macro(set_public_header abs_directory rel_directory file)
    set(args_to_install_)
    set(next_)
    set(install_ FALSE)

    foreach(arg ${ARGN})
        if(next_)
            if("${next_}" STREQUAL "DESTINATION")
                list(APPEND args_to_install_ "DESTINATION" "${arg}")
            elseif("${next_}" STREQUAL "COMPONENT")
                list(APPEND args_to_install_ "COMPONENT" "${arg}")
            else()
                message(FATAL_ERROR "set_public_headers_directory: assert error")
            endif()
            set(next_)
        elseif("${arg}" STREQUAL "DESTINATION")
            set(next_ "${arg}")
        elseif("${arg}" STREQUAL "COMPONENT")
            set(next_ "${arg}")
        elseif("${arg}" STREQUAL "INSTALL")
            set(install_ TRUE)
        else()
            message(FATAL_ERROR "set_public_headers_directory: Unknown argmuent ${arg}")
        endif()
    endforeach()

    install(FILES ${abs_directory}/${rel_directory}/${file}
        ${args_to_install_}
        )

    if(install_)
        get_property(${PROJECT_NAME}_PUBLIC_HEADERS_FILES GLOBAL PROPERTY ${PROJECT_NAME}_PUBLIC_HEADERS_FILES_PROPERTY)
        set(${PROJECT_NAME}_PUBLIC_HEADERS_FILES ${${PROJECT_NAME}_PUBLIC_HEADERS_FILES} ${rel_directory}/${file})
        set_property(GLOBAL PROPERTY ${PROJECT_NAME}_PUBLIC_HEADERS_FILES_PROPERTY ${${PROJECT_NAME}_PUBLIC_HEADERS_FILES})
    endif()
endmacro()
