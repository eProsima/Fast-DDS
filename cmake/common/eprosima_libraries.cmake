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

macro(eprosima_find_package package)

    if(NOT ${package}_FOUND)

        # Parse arguments.
        set(options REQUIRED)
        set(multiValueArgs OPTIONS)
        cmake_parse_arguments(FIND "${options}" "" "${multiValueArgs}" ${ARGN})

        option(THIRDPARTY "Activate the use of internal thirdparties" OFF)
        option(THIRDPARTY_UPDATE "Activate the auto update of internal thirdparties" ON)

        if(EPROSIMA_BUILD)
            set(THIRDPARTY ON)
        endif()

        option(THIRDPARTY_${package} "Activate the use of internal thirdparty ${package}" OFF)

        if(NOT EPROSIMA_INSTALLER)
            find_package(${package} QUIET)
        endif()

        if(NOT ${package}_FOUND AND (THIRDPARTY OR THIRDPARTY_${package}))
            set(SUBDIRECTORY_EXIST TRUE)
            if(THIRDPARTY_UPDATE OR NOT EXISTS "${PROJECT_SOURCE_DIR}/thirdparty/${package}/CMakeLists.txt")
                message(STATUS "Updating submodule thirdparty/${package}")
                execute_process(
                    COMMAND git submodule update --recursive --init "thirdparty/${package}"
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                    RESULT_VARIABLE EXECUTE_RESULT
                    )
                if(NOT EXECUTE_RESULT EQUAL 0)
                    message(WARNING "Cannot configure Git submodule ${package}")
                    if(NOT EXISTS "${PROJECT_SOURCE_DIR}/thirdparty/${package}/CMakeLists.txt")
                        set(SUBDIRECTORY_EXIST FALSE)
                    endif()
                endif()
            endif()

            if(SUBDIRECTORY_EXIST)
                foreach(opt_ ${FIND_OPTIONS})
                    set(${opt_} ON)
                endforeach()
                add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/${package})
                set(${package}_FOUND TRUE)
                if(NOT IS_TOP_LEVEL)
                    set(${package}_FOUND TRUE PARENT_SCOPE)
                endif()
            endif()
        endif()

        if(${package}_FOUND)
            message(STATUS "${package} library found...")
        elseif(${FIND_REQUIRED})
            message(FATAL_ERROR "${package} library not found...")
        else()
            message(STATUS "${package} library not found...")
        endif()
    endif()
endmacro()

# TODO (Eduardo Ponz): Document macro
macro(eprosima_find_thirdparty package thirdparty_name)
    option(THIRDPARTY "Activate the use of internal thirdparties" OFF)
    option(THIRDPARTY_UPDATE "Activate the auto update of internal thirdparties" ON)
    option(THIRDPARTY_${package} "Activate the use of internal thirdparty ${package}" OFF)

    # eProsima build sets thirdparty to ON
    if(EPROSIMA_BUILD)
        set(THIRDPARTY ON)
    endif()

    # For the case of Windows installer, we don't want to look for the packge outside thirdparty. This way we use
    # thirdparty, meaning we have more control over what is built.
    if(NOT (EPROSIMA_INSTALLER AND (MSVC OR MSVC_IDE)))
        # Try to quetly find the package outside thridparty first.
        find_package(${package} REQUIRED QUIET)
    endif()

    # Only use thirdparty if the package is not found elsewhere
    if(NOT ${package}_FOUND AND (THIRDPARTY OR THIRDPARTY_${package}))
        if(THIRDPARTY_UPDATE)
            # Update submodule
            message(STATUS "Updating submodule thirdparty/${thirdparty_name}")
            execute_process(
                COMMAND git submodule update --recursive --init "thirdparty/${thirdparty_name}"
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                RESULT_VARIABLE EXECUTE_RESULT
                )
            # A result different than 0 means that the submodule could not be updated.
            if(EXECUTE_RESULT)
                message(FATAL_ERROR "Cannot configure Git submodule ${package}")
            endif()
        endif()

        # Add package thirdparty directory to CMAKE_PREFIX_PATH
        set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${PROJECT_SOURCE_DIR}/thirdparty/${thirdparty_name})
        set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${PROJECT_SOURCE_DIR}/thirdparty/${thirdparty_name}/${thirdparty_name})
        find_package(${package} REQUIRED)
    endif()

    # If the package was still not found, then we throw an error
    if(NOT ${package}_FOUND)
        message(ERROR "Cannot find package ${package}")
    endif()
endmacro()
