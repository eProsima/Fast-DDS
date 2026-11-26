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

#[=[
Create an object library with the sources of a mock.

  add_mock_library(<name>
      SOURCES <source>...
      [DEPENDS <mock name>...]
      [REPLACES <unit>]
      )

Creates the target fastdds-mock-<name> (with :: replaced by -) and its alias fastdds::mock::<name>. Sources are
compiled once, with the compile definitions of the unit tests, and reused by every unit test linking the alias. The
directory calling this function is the public include directory of the mock, where tests find the headers of its
companion gmock classes (e.g. mock/rtps/resources/TimedEventMock.hpp).

DEPENDS lists other mocks (by <name>) whose headers and objects this mock needs. For each of them:
 * It is linked as public, so its usage requirements reach this mock and its consumers.
 * Its objects are added to the consumers as interface sources, because object files are not propagated through
   linking. target_link_units() does not link it directly when it is a dependency of another linked mock, so its
   objects are not linked twice.

REPLACES names the unit (see fastdds_add_unit()) this mock replaces as a whole: target_link_units() links the mock
instead of that unit, and does not follow the unit's dependencies.

A mock named <unit>::<name> (e.g. core::DataWriterHistory) replaces only some objects of that unit, and cannot use
REPLACES: each of its compiled sources (.c, .cc, .cpp, .cxx) replaces the source of the unit with the same file name.
target_link_units() links the rest of the unit's objects and the mock ones, and follows the unit's dependencies.
Objects are matched by file name, the only part all the generators keep, so each compiled source of the mock must
match exactly one source of the unit.
#]=]
function(add_mock_library name)
    cmake_parse_arguments(MOCK "" "REPLACES" "SOURCES;DEPENDS" ${ARGN})

    if(NOT MOCK_SOURCES)
        message(FATAL_ERROR "add_mock_library(${name}): SOURCES is required")
    endif()

    # The unit a <unit>::<name> mock replaces some objects of.
    set(partial_unit)
    if(name MATCHES "^(.+)::[^:]+$")
        set(partial_unit ${CMAKE_MATCH_1})
        if(NOT TARGET fastdds::${partial_unit})
            message(FATAL_ERROR "add_mock_library(${name}): unknown unit ${partial_unit}")
        endif()
        if(MOCK_REPLACES)
            message(FATAL_ERROR "add_mock_library(${name}): a mock named <unit>::<name> replaces some objects of "
                "${partial_unit}, it cannot replace a whole unit with REPLACES")
        endif()
    elseif(MOCK_REPLACES AND NOT TARGET fastdds::${MOCK_REPLACES})
        message(FATAL_ERROR "add_mock_library(${name}): unknown unit ${MOCK_REPLACES}")
    endif()

    string(REPLACE "::" "-" target "fastdds-mock-${name}")

    add_library(${target} OBJECT EXCLUDE_FROM_ALL)
    add_library(fastdds::mock::${name} ALIAS ${target})

    target_compile_features(${target} PRIVATE cxx_std_11)

    # Compile as position independent code like the fastdds target does when it is a shared library.
    if(BUILD_SHARED_LIBS)
        set_target_properties(${target} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    endif()

    set(dependency_targets)
    set(dependency_objects)
    foreach(dependency IN LISTS MOCK_DEPENDS)
        list(APPEND dependency_targets fastdds::mock::${dependency})
        list(APPEND dependency_objects $<TARGET_OBJECTS:fastdds::mock::${dependency}>)
    endforeach()

    # Same include directories as the fastdds target.
    target_include_directories(${target}
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        PRIVATE
        $<GENEX_EVAL:$<TARGET_PROPERTY:fastdds,INCLUDE_DIRECTORIES>>
        )

    # Same compile definitions as the unit tests (see test/unittest/CMakeLists.txt): the ones of the fastdds target,
    # without FASTDDS_SOURCE and FASTDDS_DYN_LINK. On Windows they turn the API into dllexport, and MSVC then emits
    # every inline member of the exported classes, referencing library symbols the unit tests do not link.
    target_compile_definitions(${target} PRIVATE
        $<FILTER:$<TARGET_PROPERTY:fastdds,COMPILE_DEFINITIONS>,EXCLUDE,^FASTDDS_SOURCE$|^FASTDDS_DYN_LINK$>
        FASTDDS_NO_LIB
        $<$<BOOL:${WIN32}>:_CRT_SECURE_NO_WARNINGS>
        )

    # Same libraries as the fastdds target, without its units (fastdds::<unit>): target_link_units() decides which of
    # their objects, or the mock ones, each unit test links.
    target_link_libraries(${target}
        PUBLIC
        fastdds-mock-common
        ${dependency_targets}
        PRIVATE
        $<FILTER:$<GENEX_EVAL:$<TARGET_PROPERTY:fastdds,LINK_LIBRARIES>>,EXCLUDE,^fastdds::>
        GTest::gmock
        )

    # Regular expression matching the objects of the replaced sources, e.g. /DataWriterHistory.cpp.o with Ninja and
    # Makefiles, /DataWriterHistory.obj with Visual Studio.
    set(replaced_objects)
    if(partial_unit)
        get_target_property(unit_sources fastdds::${partial_unit} SOURCES)
        set(unit_file_names)
        foreach(unit_source IN LISTS unit_sources)
            get_filename_component(unit_file_name "${unit_source}" NAME)
            list(APPEND unit_file_names ${unit_file_name})
        endforeach()
        set(object_names)
        foreach(source IN LISTS MOCK_SOURCES)
            if(NOT source MATCHES "\\.(c|cc|cpp|cxx)$")
                continue()
            endif()
            get_filename_component(file_name "${source}" NAME)
            set(count 0)
            foreach(unit_file_name IN LISTS unit_file_names)
                if(unit_file_name STREQUAL file_name)
                    math(EXPR count "${count} + 1")
                endif()
            endforeach()
            if(NOT count EQUAL 1)
                message(FATAL_ERROR "add_mock_library(${name}): unit ${partial_unit} has ${count} sources named "
                    "${file_name}, it must have exactly one for the mock to replace its object")
            endif()
            get_filename_component(object_name "${source}" NAME_WLE)
            get_filename_component(object_extension "${source}" LAST_EXT)
            list(APPEND object_names "${object_name}(${object_extension})?")
        endforeach()
        if(NOT object_names)
            message(FATAL_ERROR "add_mock_library(${name}): a mock named <unit>::<name> needs a compiled source")
        endif()
        list(JOIN object_names "|" object_names)
        string(REPLACE "." "\\." object_names "${object_names}")
        set(replaced_objects "[/\\\\](${object_names})\\.(o|obj)$")
        set_target_properties(${target} PROPERTIES
            FASTDDS_MOCK_REPLACES "${partial_unit}"
            FASTDDS_MOCK_REPLACES_OBJECTS "${replaced_objects}"
            )
    else()
        set_target_properties(${target} PROPERTIES FASTDDS_MOCK_REPLACES "${MOCK_REPLACES}")
    endif()
    set_target_properties(${target} PROPERTIES FASTDDS_MOCK_DEPENDS "${MOCK_DEPENDS}")

    if(dependency_objects)
        target_sources(${target} INTERFACE ${dependency_objects})
    endif()

    target_sources(${target} PRIVATE ${MOCK_SOURCES})
endfunction()
