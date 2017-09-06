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

macro(gradle_build directory)
    find_package(Java 1.6 COMPONENTS Runtime REQUIRED)
    if(WIN32)
        find_program(GRADLE_EXE gradle.bat)
    else()
        find_program(GRADLE_EXE gradle)
    endif()

    if(GRADLE_EXE)
        message(STATUS "Found Gradle: ${GRADLE_EXE}")
    else()
        message(FATAL_ERROR "gradle is needed to build the java application. Please install it correctly")
    endif()


    if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION} VERSION_LESS 3.1.3)
        add_custom_target(java ALL
            COMMAND "${GRADLE_EXE}" -Pcustomversion=${PROJECT_VERSION} build
            WORKING_DIRECTORY ${directory}
            COMMENT "Generating Java application" VERBATIM)
    else()
        get_filename_component(Java_JAVA_EXECUTABLE_DIR ${Java_JAVA_EXECUTABLE} DIRECTORY)
        file(TO_NATIVE_PATH "${Java_JAVA_EXECUTABLE_DIR}" Java_JAVA_EXECUTABLE_DIR_NATIVE)

        if(WIN32)
            set(delimiter_ ";")
        else()
            set(delimiter_ ":")
        endif()

        add_custom_target(java ALL
            COMMAND ${CMAKE_COMMAND} -E env
            --unset=JAVA_HOME
            "PATH=${Java_JAVA_EXECUTABLE_DIR_NATIVE}${delimiter_}$<JOIN:$ENV{PATH},${delimiter_}>"
            "${GRADLE_EXE}" -Pcustomversion=${PROJECT_VERSION} build
            WORKING_DIRECTORY ${directory}
            COMMENT "Generating Java application" VERBATIM)
    endif()



    set(THIRDPARTY_FOUND false)
    foreach(arg ${ARGN})
        if("${arg}" STREQUAL "THIRDPARTY_DEPENDENCY")
            set(THIRDPARTY_FOUND true)
        else()
            if(THIRDPARTY_FOUND)
                add_custom_target(git_submodule_update_${arg}
                    COMMAND git submodule update --recursive --init thirdparty/${arg}
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                    COMMENT "Updating Git module ${arg}" VERBATIM)
                add_dependencies(java git_submodule_update_${arg})
                set(THIRDPARTY_FOUND false)
            else()
                message(FATAL_ERROR "Bad use of gradle_build command")
            endif()
        endif()
    endforeach()
endmacro()
