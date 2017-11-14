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

macro(find_eprosima_package package)
    if(NOT (EPROSIMA_INSTALLER AND (MSVC OR MSVC_IDE)))

        option(THIRDPARTY_${package} "Activate the use of internal thirdparty ${package}" OFF)

        if(THIRDPARTY OR THIRDPARTY_${package})
            set(EPROSIMA_PACKAGE_EXTERNAL_DIR "" CACHE PATH "External directory to compile eprosima libraries")
            set(${package}ExternalDir ${PROJECT_BINARY_DIR}/external/${package})

            if("${EPROSIMA_PACKAGE_EXTERNAL_DIR}" STREQUAL "")
                set(EPROSIMA_PACKAGE_EXTERNAL_DIR ${PROJECT_BINARY_DIR}/external)
            else()
                set(${package}ExternalDir ${EPROSIMA_PACKAGE_EXTERNAL_DIR}/${package})
            endif()

            if(MINION)
                set(CMAKE_INSTALL_PREFIX_ "${CMAKE_INSTALL_PREFIX}")
            else()
                set(CMAKE_INSTALL_PREFIX_ "${PROJECT_BINARY_DIR}/external/install")
            endif()

            if(NOT EXISTS "${${package}ExternalDir}/install.txt")
                set(ANDROID_BUILD_OPTIONS "")
                if(ANDROID)
                    set(ANDROID_BUILD_OPTIONS "-DANDROID_ABI=${ANDROID_ABI}"
                        "-DANDROID_NDK=${ANDROID_NDK}"
                        "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
                        "-DANDROID_NATIVE_API_LEVEL=${ANDROID_NATIVE_API_LEVEL}"
                        "-DANDROID_TOOLCHAIN=${ANDROID_TOOLCHAIN}"
                        "-DANDROID_PLATFORM=${ANDROID_PLATFORM}"               
                        "-DANDROID_STL=${ANDROID_STL}"
                        "-DANDROID_PIE=${ANDROID_PIE}"
                        "-DANDROID_CPP_FEATURES=${ANDROID_CPP_FEATURES}"
                        "-DANDROID_ALLOW_UNDEFINED_SYMBOLS=${ANDROID_ALLOW_UNDEFINED_SYMBOLS}"
                        "-DANDROID_ARM_MODE=${ANDROID_ARM_MODE}"
                        "-DANDROID_ARM_NEON=${ANDROID_ARM_NEON}"
                        "-DANDROID_DISABLE_NO_EXECUTE=${ANDROID_DISABLE_NO_EXECUTE}"
                        "-DANDROID_DISABLE_RELRO=${ANDROID_DISABLE_RELRO}"
                        "-DANDROID_DISABLE_FORMAT_STRING_CHECKS=${ANDROID_DISABLE_FORMAT_STRING_CHECKS}"
                        "-DANDROID_FUNCTION_LEVEL_LINKING=${ANDROID_FUNCTION_LEVEL_LINKING}"
                        )
                endif()

                # Separate CMAKE_PREFIX_PATH
                string(REPLACE ";" "|" CMAKE_PREFIX_PATH_ "${CMAKE_PREFIX_PATH}")
                string(REPLACE " " "\\ " CMAKE_C_FLAGS_ "${CMAKE_C_FLAGS}")
                string(REPLACE " " "\\ " CMAKE_CXX_FLAGS_ "${CMAKE_CXX_FLAGS}")

                set(${package}_CMAKE_ARGS
                    "\${SOURCE_DIR_}"
                    "\${GENERATOR_}"
                    "\${CMAKE_C_COMPILER_}"
                    "\${CMAKE_CXX_COMPILER_}"
                    "-DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS_}"
                    "-DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS_}"
                    ${ANDROID_BUILD_OPTIONS}
                    "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
                    "-DCMAKE_CXX_COMPILER_LAUNCHER=${CMAKE_CXX_COMPILER_LAUNCHER}"
                    "-DMINION=ON"
                    "-DEPROSIMA_INSTALLER_MINION=${EPROSIMA_INSTALLER_MINION}"
                    "-DBIN_INSTALL_DIR:PATH=${BIN_INSTALL_DIR}"
                    "-DINCLUDE_INSTALL_DIR:PATH=${INCLUDE_INSTALL_DIR}"
                    "-DLIB_INSTALL_DIR:PATH=${LIB_INSTALL_DIR}"
                    "-DLICENSE_INSTALL_DIR:PATH=licenses"
                    "\${CMAKE_INSTALL_PREFIX_}"
                    "\${CMAKE_PREFIX_PATH_}"
                    "\${EPROSIMA_PACKAGE_EXTERNAL_DIR_}"
                    )
                list(APPEND ${package}_CMAKE_ARGS LIST_SEPARATOR "|")

                file(MAKE_DIRECTORY ${${package}ExternalDir})
                file(WRITE ${${package}ExternalDir}/CMakeLists.txt
                    "cmake_minimum_required(VERSION 2.8.12)\n"
                    "include(ExternalProject)\n"
                    "set(SOURCE_DIR_ \"${PROJECT_SOURCE_DIR}/thirdparty/${package}\")\n"
                    "set(GENERATOR_ -G \"${CMAKE_GENERATOR}\")\n"
                    "set(CMAKE_INSTALL_PREFIX_ \"-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX_}\")\n"
                    "set(CMAKE_PREFIX_PATH_ -DCMAKE_PREFIX_PATH=\"${CMAKE_PREFIX_PATH_}\")\n"
                    "set(CMAKE_C_COMPILER_ \"-DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}\")\n"
                    "set(CMAKE_CXX_COMPILER_ \"-DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}\")\n"
                    "set(EPROSIMA_PACKAGE_EXTERNAL_DIR_ \"-DEPROSIMA_PACKAGE_EXTERNAL_DIR:PATH=${EPROSIMA_PACKAGE_EXTERNAL_DIR}\")\n"
                    "ExternalProject_Add(${package}\n"
                    "CONFIGURE_COMMAND \"${CMAKE_COMMAND}\"\n"
                    "${${package}_CMAKE_ARGS}\n"
                    "DOWNLOAD_COMMAND \"\"\n"
                    "UPDATE_COMMAND cd \"${PROJECT_SOURCE_DIR}\" && git submodule update --recursive --init \"thirdparty/${package}\"\n"
                    "SOURCE_DIR \${SOURCE_DIR_}\n"
                    "BINARY_DIR \"${${package}ExternalDir}/build\"\n"
                    ")\n"
                    "install(CODE \"file(WRITE install.txt \\\"Installation completed\\\")\")\n")

                if(NOT "$ENV{CMAKE_MAKEFLAGS}" STREQUAL "")
                    set(ENV{MAKEFLAGS} "$ENV{CMAKE_MAKEFLAGS}")
                endif()

                execute_process(COMMAND ${CMAKE_COMMAND}
                    -G ${CMAKE_GENERATOR}
                    ${ANDROID_BUILD_OPTIONS}
                    WORKING_DIRECTORY ${${package}ExternalDir}
                    RESULT_VARIABLE EXECUTE_RESULT
                    )

                if(NOT EXECUTE_RESULT EQUAL 0)
                    message(FATAL_ERROR "Cannot configure Git submodule ${package}")
                endif()

                if(MSVC OR MSVC_IDE)
                    if("${CMAKE_BUILD_TYPE}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
                        set(BUILD_TYPE_GENERATION "Release")
                    else()
                        set(BUILD_TYPE_GENERATION ${CMAKE_BUILD_TYPE})
                    endif()

                    execute_process(COMMAND ${CMAKE_COMMAND} --build . --config Debug
                        WORKING_DIRECTORY ${${package}ExternalDir}
                        RESULT_VARIABLE EXECUTE_RESULT
                        )

                    if(NOT EXECUTE_RESULT EQUAL 0)
                        message(FATAL_ERROR "Cannot build Git submodule ${package} in Debug mode")
                    endif()

                    execute_process(COMMAND ${CMAKE_COMMAND} --build . --config ${BUILD_TYPE_GENERATION} --target install
                        WORKING_DIRECTORY ${${package}ExternalDir}
                        RESULT_VARIABLE EXECUTE_RESULT
                        )

                    if(NOT EXECUTE_RESULT EQUAL 0)
                        message(FATAL_ERROR "Cannot build Git submodule ${package} in ${BUILD_TYPE_GENERATION} mode")
                    endif()
                else()
                    execute_process(COMMAND ${CMAKE_COMMAND} --build . --target install
                        WORKING_DIRECTORY ${${package}ExternalDir}
                        RESULT_VARIABLE EXECUTE_RESULT
                        )

                    if(NOT EXECUTE_RESULT EQUAL 0)
                        message(FATAL_ERROR "Cannot build Git submodule ${package}")
                    endif()
                endif()
            endif()

            set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_INSTALL_PREFIX_})
        endif()

        find_package(${package} QUIET)

        if(${package}_FOUND)
            message(STATUS "${package} library found...")
        else()
            message(STATUS "${package} library not found...")
        endif()

    endif()
endmacro()

macro(find_eprosima_thirdparty package thirdparty_name)
    if(NOT (EPROSIMA_INSTALLER AND (MSVC OR MSVC_IDE)))

        option(THIRDPARTY_${package} "Activate the use of internal thirdparty ${package}" OFF)

        if(THIRDPARTY OR THIRDPARTY_${package})
            execute_process(
                COMMAND git submodule update --recursive --init "thirdparty/${thirdparty_name}"
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                RESULT_VARIABLE EXECUTE_RESULT
                )

            if(NOT EXECUTE_RESULT EQUAL 0)
                message(FATAL_ERROR "Cannot configure Git submodule ${package}")
            endif()
        endif()

        set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${PROJECT_SOURCE_DIR}/thirdparty/${thirdparty_name})
        set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${PROJECT_SOURCE_DIR}/thirdparty/${thirdparty_name}/${thirdparty_name})

        find_package(${package} REQUIRED)
    endif()
endmacro()

macro(install_eprosima_libraries)
    if((MSVC OR MSVC_IDE) AND THIRDPARTY AND NOT MINION)
        if(EPROSIMA_INSTALLER)
            # Install includes. Take from x64Win64VS2013
            install(DIRECTORY ${PROJECT_BINARY_DIR}/eprosima_installer/x64Win64VS2015/install/${INCLUDE_INSTALL_DIR}/
                DESTINATION ${INCLUDE_INSTALL_DIR}
                COMPONENT headers
                OPTIONAL
                )

            # Install licenses. Take from x64Win64VS2013
            install(DIRECTORY ${PROJECT_BINARY_DIR}/eprosima_installer/x64Win64VS2015/install/licenses/
                DESTINATION ${LICENSE_INSTALL_DIR}
                COMPONENT licenses
                OPTIONAL
                )
        else()
            if("${CMAKE_BUILD_TYPE}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
                set(BUILD_TYPE_INSTALLATION "Release")
            else()
                set(BUILD_TYPE_INSTALLATION ${CMAKE_BUILD_TYPE})
            endif()

            # Install includes
            install(DIRECTORY ${PROJECT_BINARY_DIR}/external/install/${INCLUDE_INSTALL_DIR}/
                DESTINATION ${INCLUDE_INSTALL_DIR}
                COMPONENT headers
                OPTIONAL
                )

            # Install libraries
            install(DIRECTORY ${PROJECT_BINARY_DIR}/external/install/${BIN_INSTALL_DIR}/
                DESTINATION ${BIN_INSTALL_DIR}
                COMPONENT libraries_${MSVC_ARCH}
                CONFIGURATIONS Debug
                OPTIONAL
                FILES_MATCHING
                PATTERN "*d.*"
                PATTERN "*d-*.*"
                )

            install(DIRECTORY ${PROJECT_BINARY_DIR}/external/install/${BIN_INSTALL_DIR}/
                DESTINATION ${BIN_INSTALL_DIR}
                COMPONENT libraries_${MSVC_ARCH}
                CONFIGURATIONS ${BUILD_TYPE_INSTALLATION}
                OPTIONAL
                FILES_MATCHING
                PATTERN "*"
                PATTERN "*d.*" EXCLUDE
                PATTERN "*d-*.*" EXCLUDE
                )

            install(DIRECTORY ${PROJECT_BINARY_DIR}/external/install/${LIB_INSTALL_DIR}/
                DESTINATION ${LIB_INSTALL_DIR}
                COMPONENT libraries_${MSVC_ARCH}
                CONFIGURATIONS Debug
                OPTIONAL
                FILES_MATCHING
                PATTERN "*d.*"
                PATTERN "*d-*.*"
                )

            install(DIRECTORY ${PROJECT_BINARY_DIR}/external/install/${LIB_INSTALL_DIR}/
                DESTINATION ${LIB_INSTALL_DIR}
                COMPONENT libraries_${MSVC_ARCH}
                CONFIGURATIONS ${BUILD_TYPE_INSTALLATION}
                OPTIONAL
                FILES_MATCHING
                PATTERN "*"
                PATTERN "*d.*" EXCLUDE
                PATTERN "*d-*.*" EXCLUDE
                )

            # Install licenses
            install(DIRECTORY ${PROJECT_BINARY_DIR}/external/install/licenses/
                DESTINATION ${LICENSE_INSTALL_DIR}
                COMPONENT licenses
                OPTIONAL
                )
        endif()
    elseif(UNIX AND THIRDPARTY AND NOT MINION AND NOT EPROSIMA_INSTALLER)
            # Install includes
            install(DIRECTORY ${PROJECT_BINARY_DIR}/external/install/${INCLUDE_INSTALL_DIR}/
                DESTINATION ${INCLUDE_INSTALL_DIR}
                COMPONENT headers
                OPTIONAL
                )

            # Install libraries
            install(DIRECTORY ${PROJECT_BINARY_DIR}/external/install/${LIB_INSTALL_DIR}/
                DESTINATION ${LIB_INSTALL_DIR}
                USE_SOURCE_PERMISSIONS
                COMPONENT libraries
                )

            # Install licenses
            install(DIRECTORY ${PROJECT_BINARY_DIR}/external/install/licenses/
                DESTINATION ${LICENSE_INSTALL_DIR}
                COMPONENT licenses
                OPTIONAL
                )
    endif()
endmacro()
