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

# Macro to find cmake built thirdparty libraries.
#
# Arguments:
#   :package: The name of the packge to find. Used for find_package(${package})
#   :extra arguments: Arguments passed after ${package} are parsed with cmake_parse_arguments()
#
# Related CMake options:
#   :THIRDPARTY: Activate the use of internal thirdparties if external packges is not found [Defaults: OFF]
#   :THIRDPARTY_FORCE: Force the use of internal thirdparties [Defaults: OFF]. Set to ON if EPROSIMA_BUILD is set to ON.
#   :THIRDPARTY_UPDATE: Activate the auto update of internal thirdparties [Defaults: ON]
#   :THIRDPARTY_${package}: Activate the use of internal thirdparty ${package} [Defaults: OFF]
#
# The macro's procedure is as follows:
#   1. If the package has alredy been found do nothing
#   2. Try to find the package with find_package.
#         2.1. This step is not taken if THIRDPARTY_FORCE is set to ON.
#         2.2. This step is not taken for the case of windows installer. That is if EPROSIMA_INSTALLER is set to ON.
#   3. If the package is not found in 2), and at least one of THIRDPARTY, THIRDPARTY_FORCE, THIRDPARTY_${package} is set
#      to ON, use the thirdparty version.
#         3.1. If THIRDPARTY_UPDATE is set to ON, then update the corresponding git submodule.
#         3.2. Add the subdirectory /thirdparty/${package}.
#         3.3. Set ${package}_FOUND to TRUE.
#   4. If the package was not found anywhere, then print a WARNING or FATAL_ERROR message depending on whether the
#      package was required.
macro(eprosima_find_package package)
    # Only work if the package was not found previously
    if(NOT ${package}_FOUND)
        # Parse arguments.
        set(options REQUIRED)
        set(multiValueArgs OPTIONS)
        cmake_parse_arguments(FIND "${options}" "" "${multiValueArgs}" ${ARGN})

        # Define options
        option(THIRDPARTY "Activate the use of internal thirdparties if external packges is not found" OFF)
        option(THIRDPARTY_FORCE "Force the use of internal thirdparties" OFF)
        option(THIRDPARTY_${package} "Activate the use of internal thirdparty ${package}" OFF)
        option(THIRDPARTY_UPDATE "Activate the auto update of internal thirdparties" ON)

        # eProsima build sets thirdparty to ON
        if(EPROSIMA_BUILD)
            set(THIRDPARTY ON)
        endif()

        # 1. If THIRDPARTY_FORCE is set to ON, don't try to find the library outside thirdparty.
        # 2. For the case of Windows installer, we don't want to look for the package outside thirdparty. This way we
        #    use thirdparty, meaning we have more control over what is built.
        if(NOT THIRDPARTY_FORCE AND NOT EPROSIMA_INSTALLER)
            # Try to quetly find the package outside thridparty first.
            find_package(${package} QUIET)

            # Show message if package is found here.
            if(${package}_FOUND)
                # Cannot state where the package is. Asio sets Asio_DIR to Asio_DIR-NOTFOUND even when found.
                message(STATUS "Found ${package}")
            endif()
        endif()

        # Use thirdparty if THIRDPARTY_FORCE is set to ON, or if the package is not found elsewhere and at least one of
        # THIRDPARTY, THIRDPARTY_${package} is set to ON.
        if(THIRDPARTY_FORCE OR (NOT ${package}_FOUND AND (THIRDPARTY OR THIRDPARTY_${package})))
            if(THIRDPARTY_UPDATE)
                # Update submodule
                message(STATUS "Updating submodule thirdparty/${package}")
                execute_process(
                    COMMAND git submodule update --recursive --init "thirdparty/${package}"
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                    RESULT_VARIABLE EXECUTE_RESULT
                    )
                # A result different than 0 means that the submodule could not be updated.
                if(NOT EXECUTE_RESULT EQUAL 0)
                    message(WARNING "Cannot configure Git submodule ${package}")
                endif()
            endif()

            # Check that the package is correctly initialized by looking for its CMakeLists.txt file.
            set(SUBDIRECTORY_EXIST FALSE)
            if(EXISTS "${PROJECT_SOURCE_DIR}/thirdparty/${package}/CMakeLists.txt")
                set(SUBDIRECTORY_EXIST TRUE)
            endif()

            # If the subdirectory exists, we can add it so it's built.
            if(SUBDIRECTORY_EXIST)
                foreach(opt_ ${FIND_OPTIONS})
                    set(${opt_} ON)
                endforeach()
                add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/${package})
                set(${package}_FOUND TRUE)
                if(NOT IS_TOP_LEVEL)
                    set(${package}_FOUND TRUE PARENT_SCOPE)
                endif()
                message(STATUS "Found ${package}: ${PROJECT_SOURCE_DIR}/thirdparty/${package}")
            endif()
        endif()

        # If the package was found we have already show an status message.
        # if it was not found and it was required throw an fatal error.
        # if if was not found but it was not required show a warning.
        if((NOT ${package}_FOUND) AND ${FIND_REQUIRED})
            message(FATAL_ERROR "${package} package NOT found")
        elseif(NOT ${package}_FOUND)
            message(STATUS "${package} package NOT found")
        endif()
    endif()
endmacro()

# Macro to find all Fast DDS thirdparty libraries expect for Fast CDR (look at eprosima_find_package).
#
# Arguments:
#   :package: The name of the packge to find. Used for find_package(${package})
#   :thirdparty_name: The name of the package directory under thirdparty, i.e. thirdparty/${thirdparty_name}
#
# Related CMake options:
#   :THIRDPARTY: Activate the use of internal thirdparties if external packges is not found [Defaults: OFF]
#   :THIRDPARTY_FORCE: Force the use of internal thirdparties [Defaults: OFF]. Set to ON if EPROSIMA_BUILD is set to ON.
#   :THIRDPARTY_UPDATE: Activate the auto update of internal thirdparties [Defaults: ON]
#   :THIRDPARTY_${package}: Activate the use of internal thirdparty ${package} [Defaults: OFF]
#
# The macro's procedure is as follows:
#   1. Try to find the package with find_package.
#         1.1. This step is not taken if THIRDPARTY_FORCE is set to ON.
#         1.2. This step is not taken for the case of windows installer. That is if EPROSIMA_INSTALLER is set to ON and
#              at least one of MSVC, MSVC_IDE is set to ON at the same time.
#   2. If the package is not found in 1), and at least one of THIRDPARTY, THIRDPARTY_FORCE, THIRDPARTY_${package} is set
#      to ON, use the thirdparty version.
#         2.1. If THIRDPARTY_UPDATE is set to ON, then update the corresponding git submodule.
#         2.2. Append the thirdparty source directory to CMAKE_PREFIX_PATH.
#         2.3. Try to find the package again.
#   3. If the package was not found anywhere, then print an FATAL_ERROR message.
macro(eprosima_find_thirdparty package thirdparty_name)
    option(THIRDPARTY "Activate the use of internal thirdparties if external packges is not found" OFF)
    option(THIRDPARTY_FORCE "Force the use of internal thirdparties" OFF)
    option(THIRDPARTY_${package} "Activate the use of internal thirdparty ${package}" OFF)
    option(THIRDPARTY_UPDATE "Activate the auto update of internal thirdparties" ON)

    # eProsima build sets thirdparty to ON
    if(EPROSIMA_BUILD)
        set(THIRDPARTY ON)
    endif()

    # 1. If THIRDPARTY_FORCE is set to ON, don't try to find the library outside thirdparty.
    # 2. For the case of Windows installer, we don't want to look for the package outside thirdparty. This way we
    #    use thirdparty, meaning we have more control over what is built.
    if(NOT THIRDPARTY_FORCE AND (NOT (EPROSIMA_INSTALLER AND (MSVC OR MSVC_IDE))))
        # Try to quetly find the package outside thridparty first.
        find_package(${package} QUIET)

        # Show message if package is found here.
        if(${package}_FOUND)
            # Cannot state where the package is. Asio sets Asio_DIR to Asio_DIR-NOTFOUND even when found.
            message(STATUS "Found ${package}")
        endif()
    endif()

    # Use thirdparty if THIRDPARTY_FORCE is set to ON, or if the package is not found elsewhere and at least one of
    # THIRDPARTY, THIRDPARTY_${package} is set to ON.
    if(THIRDPARTY_FORCE OR (NOT ${package}_FOUND AND (THIRDPARTY OR THIRDPARTY_${package})))
        if(THIRDPARTY_UPDATE)
            # Update submodule
            message(STATUS "Updating submodule thirdparty/${thirdparty_name}")
            execute_process(
                COMMAND git submodule update --recursive --init "thirdparty/${thirdparty_name}"
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                RESULT_VARIABLE EXECUTE_RESULT
                )
            # A result different than 0 means that the submodule could not be updated.
            if(NOT EXECUTE_RESULT EQUAL 0)
                message(FATAL_ERROR "Cannot configure Git submodule ${package}")
            endif()
        endif()

        # Prepend CMAKE_PREFIX_PATH with the package thirdparty directory. The second path is needed for asio, since the
        # directory is "thirdparty/asio/asio"
        set(CMAKE_PREFIX_PATH ${PROJECT_SOURCE_DIR}/thirdparty/${thirdparty_name} ${CMAKE_PREFIX_PATH})
        set(CMAKE_PREFIX_PATH ${PROJECT_SOURCE_DIR}/thirdparty/${thirdparty_name}/${thirdparty_name} ${CMAKE_PREFIX_PATH})
        find_package(${package} REQUIRED)
    endif()

    # If the package was not found throw an error.
    if(NOT ${package}_FOUND)
        message(FATAL_ERROR "Cannot find package ${package}")
    endif()
endmacro()
