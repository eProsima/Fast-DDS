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

macro(compile_example example example_directory)
    if(NOT ((EPROSIMA_INSTALLER OR EPROSIMA_INSTALLER_MINION) AND (MSVC OR MSVC_IDE)))

        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/config/fastrtpsConfig.cmake
            "include(\"${PROJECT_BINARY_DIR}/cmake/config/fastrtpsTargets.cmake\")\n"
            )

        # Separate CMAKE_PREFIX_PATH
        string(REPLACE ";" "|" CMAKE_PREFIX_PATH_ "${CMAKE_PREFIX_PATH}")
        set(${example}_CMAKE_ARGS
            "-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}"
            "-DCMAKE_PREFIX_PATH:PATH=${CMAKE_CURRENT_BINARY_DIR}/config|${CMAKE_PREFIX_PATH_}"
            "-DBIN_INSTALL_DIR:PATH=${BIN_INSTALL_DIR}")
        if(UNIX)
            list(APPEND ${example}_CMAKE_ARGS
                "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
                )
        endif()
        list(APPEND ${example}_CMAKE_ARGS LIST_SEPARATOR "|")

        include(ExternalProject)

        ExternalProject_Add(${example}
            DEPENDS ${PROJECT_NAME}
            CMAKE_GENERATOR "${CMAKE_GENERATOR}"
            CMAKE_ARGS
            ${${example}_CMAKE_ARGS}
            SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${example_directory}"
            BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/${example_directory}"
            INSTALL_COMMAND ""
            )
    endif()
endmacro()
