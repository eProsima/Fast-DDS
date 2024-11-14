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

cmake_minimum_required(VERSION 3.20)

project(fast-discovery-server VERSION 1.0.1 LANGUAGES CXX)

###############################################################################
# Load external dependencies
###############################################################################

if(NOT fastdds_FOUND)
    find_package(fastdds 3 REQUIRED)
endif()

###############################################################################
# Compilation
###############################################################################

if (MSVC)

    # For isolated builds provide some IDE aids
    if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

        # Add fast headers to enable class view on visual studio IDE
        get_target_property(FAST_INCLUDE_DIR fastdds INTERFACE_INCLUDE_DIRECTORIES)
        list(GET FAST_INCLUDE_DIR 0 FAST_INCLUDE_DIR)

        file(GLOB_RECURSE ALL_FAST_HEADERS
            "${FAST_INCLUDE_DIR}/**/*.h"
            "${FAST_INCLUDE_DIR}/**/*.hpp"
            "${FAST_INCLUDE_DIR}/**/*.hxx"
            )

        source_group("Fast DDS Headers" FILES ${ALL_FAST_HEADERS})

    endif()

    # Add project headers to enable class view on visual studio IDE
    set(FAST_CLI_DISCOVERY_HEADERS CliDiscoveryParser.hpp CliDiscoveryManager.hpp)

endif(MSVC)


add_executable(${PROJECT_NAME}
    CliDiscoveryEntrypoint.cpp
    CliDiscoveryManager.cpp
    ${FAST_CLI_DISCOVERY_HEADERS}
    ${ALL_FAST_HEADERS})

# Pass the version as a preprocessor definition
target_compile_definitions(${PROJECT_NAME}
    PRIVATE FAST_SERVER_VERSION=\"${PROJECT_VERSION}\"
    PRIVATE FAST_SERVER_BINARY=\"$<TARGET_FILE_NAME:${PROJECT_NAME}>\" )

target_link_libraries(${PROJECT_NAME} fastdds fastcdr fastdds::optionparser)

# Properties that change bin names depending on current config.
# This is convenient because on installation all bins share folder
set_target_properties(${PROJECT_NAME} PROPERTIES RELEASE_POSTFIX -${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES MINSIZEREL_POSTFIX m-${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES RELWITHDEBINFO_POSTFIX rd-${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX d-${PROJECT_VERSION})

###############################################################################
# Installation
###############################################################################

# If not isolated integrate
if(CMAKE_PROJECT_NAME STREQUAL "fastdds" )
    set(BIN_INSTALL_DIR tools/fds/${BIN_INSTALL_DIR} CACHE PATH "Installation directory for binaries within Fast DDS")
else()
    set(BIN_INSTALL_DIR bin/ CACHE PATH "Installation directory for binaries")
endif()

install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}-targets
        RUNTIME DESTINATION ${BIN_INSTALL_DIR}${MSVCARCH_DIR_EXTENSION}
        COMPONENT discovery
        )

    if(INSTALLER_PLATFORM)
        set(INSTALL_DESTINATION_PATH ${DATA_INSTALL_DIR}/${CMAKE_PROJECT_NAME}-${INSTALLER_PLATFORM}/cmake)
    else()
        set(INSTALL_DESTINATION_PATH ${DATA_INSTALL_DIR}/${CMAKE_PROJECT_NAME}/cmake)
    endif()

# export library
install(EXPORT ${PROJECT_NAME}-targets
        DESTINATION ${INSTALL_DESTINATION_PATH}
        NAMESPACE fastdds::
        COMPONENT cmake
        )

# symlink creation requires:
#   - install( CODE using generator expressions (legacy code is provided for all cmake versions)
#   - on windows privileges to create symlinks (a .bat file is provided on unprivileged installations)
if( WIN32 )
    # Use powershell to generate the link
    install(
      CODE "execute_process( COMMAND PowerShell -Command \"if( test-path ${PROJECT_NAME}.exe -PathType Leaf ) { rm ${PROJECT_NAME}.exe } ; New-Item -ItemType SymbolicLink -Target $<TARGET_FILE_NAME:${PROJECT_NAME}> -Path ${PROJECT_NAME}.exe \" ERROR_QUIET RESULTS_VARIABLE SYMLINK_FAILED WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}/${BIN_INSTALL_DIR}\") \n if( SYMLINK_FAILED ) \n message(STATUS \"Windows requires admin installation rights to create symlinks. Build again with privileges to create symlink. Tool will try to find executable if no symlink is found.\") \n endif()"
      COMPONENT discovery)
else()
    # Use ln to create the symbolic link. We remove the version from the file name but keep the debug suffix
    install(
        CODE "execute_process(COMMAND bash -c \"if [[ -h ${PROJECT_NAME} ]]; then rm ${PROJECT_NAME}; fi;  ln -s $<TARGET_FILE_NAME:${PROJECT_NAME}> ${PROJECT_NAME}\" WORKING_DIRECTORY \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${BIN_INSTALL_DIR}${MSVCARCH_DIR_EXTENSION}\")"
        COMPONENT discovery)
endif()
