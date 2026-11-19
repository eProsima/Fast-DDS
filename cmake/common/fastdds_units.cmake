# Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#[[
Create an object library (a unit) grouping part of the fastdds sources.

  fastdds_add_unit(<name>
      SOURCES <source>...
      [DEPENDS <unit>...]
      [INCLUDE_DIRECTORIES <directory>...]
      [LINK_LIBRARIES <item>...]
      )

<name> uses :: as separator (e.g. rtps::events). Creates the target fastdds-<name> (with :: replaced by -) and its
alias fastdds::<name>, and appends the alias to the FASTDDS_UNITS global property. Sources are compiled once, with the
compile definitions of the fastdds target, and the objects are embedded in fastdds and reused by the unit tests.

DEPENDS lists other units (by <name>) whose objects this unit needs at link time. They are only recorded in the
FASTDDS_UNIT_DEPENDS property and resolved by target_link_units(): units are not linked to each other, so a unit test
can replace any of them by a mock.

INCLUDE_DIRECTORIES are added as private, after the fastdds include directories.

LINK_LIBRARIES are passed as is to target_link_libraries(), so they must start with a PUBLIC, PRIVATE or INTERFACE
keyword, and must only name external libraries.
#]]
function(fastdds_add_unit name)
    cmake_parse_arguments(UNIT "" "" "SOURCES;DEPENDS;INCLUDE_DIRECTORIES;LINK_LIBRARIES" ${ARGN})

    if(NOT UNIT_SOURCES)
        message(FATAL_ERROR "fastdds_add_unit(${name}): SOURCES is required")
    endif()

    string(REPLACE "::" "-" target "fastdds-${name}")

    add_library(${target} OBJECT EXCLUDE_FROM_ALL)
    add_library(fastdds::${name} ALIAS ${target})
    set_property(GLOBAL APPEND PROPERTY FASTDDS_UNITS fastdds::${name})

    target_compile_features(${target} PRIVATE cxx_std_11)

    # Compile as position independent code like the fastdds target does when it is a shared library.
    if(BUILD_SHARED_LIBS)
        set_target_properties(${target} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    endif()

    target_include_directories(${target} PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_BINARY_DIR}/include
        ${PROJECT_SOURCE_DIR}/src/cpp
        ${UNIT_INCLUDE_DIRECTORIES}
        )

    # Same compile definitions as the fastdds target, which is created after the units.
    # fastdds_EXPORTS is not part of COMPILE_DEFINITIONS: CMake only adds it to the fastdds shared library.
    target_compile_definitions(${target} PRIVATE
        $<TARGET_PROPERTY:fastdds,COMPILE_DEFINITIONS>
        )

    if(UNIT_LINK_LIBRARIES)
        target_link_libraries(${target} ${UNIT_LINK_LIBRARIES})
    endif()

    set_target_properties(${target} PROPERTIES FASTDDS_UNIT_DEPENDS "${UNIT_DEPENDS}")

    target_sources(${target} PRIVATE ${UNIT_SOURCES})
endfunction()
