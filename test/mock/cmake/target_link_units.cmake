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
Link fastdds units created by fastdds_add_unit(), and optionally mocks replacing some of them, to a target.

  target_link_units(<target>
      UNITS <unit>...
      [MOCKS <mock name>...]
      )

The given units and, recursively, the units they depend on (DEPENDS of fastdds_add_unit()) are linked once each, so
the target gets all the objects it needs without listing them.

A unit replaced by one of the mocks (REPLACES of add_mock_library()), or by a mock they depend on, is not linked and
its dependencies are not followed: the mock provides those symbols instead. A unit some of whose objects are replaced
by a mock named <unit>::<name> is not linked either: the target gets the unit objects but the replaced ones, the
libraries the unit links and a dependency on the unit, and the unit dependencies are followed. That unit is linked
even if it is not listed in UNITS nor reached through their dependencies.

The given mocks are linked, which brings their objects, the objects of the mocks they depend on (DEPENDS of
add_mock_library()) and the include directories of their companion headers. A given mock other given mocks depend on
is not linked directly, so its objects are not linked twice.

On Windows, rtps::basic and rtps::common are always linked, with the units they depend on (see below).

target_link_libraries() is called with the plain signature, so the target must use it too.
#]]

# Append to <out_var> the given mocks and, recursively, the mocks they depend on (DEPENDS of add_mock_library()).
function(_get_mocks_closure out_var)
    set(closure ${${out_var}})
    foreach(mock IN LISTS ARGN)
        if(NOT mock IN_LIST closure)
            if(NOT TARGET fastdds::mock::${mock})
                message(FATAL_ERROR "target_link_units(): unknown mock ${mock}")
            endif()
            list(APPEND closure ${mock})
            get_target_property(dependencies fastdds::mock::${mock} FASTDDS_MOCK_DEPENDS)
            if(dependencies)
                _get_mocks_closure(closure ${dependencies})
            endif()
        endif()
    endforeach()
    set(${out_var} ${closure} PARENT_SCOPE)
endfunction()

function(target_link_units target)
    cmake_parse_arguments(LINK "" "" "UNITS;MOCKS" ${ARGN})

    if(NOT LINK_UNITS)
        message(FATAL_ERROR "target_link_units(${target}): UNITS is required")
    endif()

    # Units replaced by the mocks, including the mocks they depend on, and regular expressions matching the objects
    # replaced in partially replaced units (stored in replaced_objects_<unit>).
    set(mocks)
    _get_mocks_closure(mocks ${LINK_MOCKS})
    set(replaced)
    set(partially_replaced)
    foreach(mock IN LISTS mocks)
        get_target_property(unit fastdds::mock::${mock} FASTDDS_MOCK_REPLACES)
        get_target_property(objects fastdds::mock::${mock} FASTDDS_MOCK_REPLACES_OBJECTS)
        if(unit AND objects)
            string(REPLACE "::" "-" unit_var "${unit}")
            if(DEFINED replaced_objects_${unit_var})
                set(replaced_objects_${unit_var} "${replaced_objects_${unit_var}}|${objects}")
            else()
                set(replaced_objects_${unit_var} "${objects}")
            endif()
            list(APPEND partially_replaced ${unit})
        elseif(unit)
            list(APPEND replaced ${unit})
        endif()
    endforeach()

    # On Windows the units are compiled with the API as dllexport, and MSVC emits every inline member of the exported
    # classes in each object including them. Some of those members call the ones of GuidPrefix_t (rtps::common) and
    # SerializedPayload_t and TransportInterface (rtps::basic), so these units are always linked.
    set(windows_units)
    if(WIN32)
        set(windows_units rtps::basic rtps::common)
    endif()

    set(units)
    set(pending ${LINK_UNITS} ${partially_replaced} ${windows_units})
    while(pending)
        list(POP_FRONT pending unit)
        if(unit IN_LIST units OR unit IN_LIST replaced)
            continue()
        endif()
        if(NOT TARGET fastdds::${unit})
            message(FATAL_ERROR "target_link_units(${target}): unknown unit ${unit}")
        endif()
        list(APPEND units ${unit})
        get_target_property(dependencies fastdds::${unit} FASTDDS_UNIT_DEPENDS)
        if(dependencies)
            list(APPEND pending ${dependencies})
        endif()
    endwhile()

    # Mocks other given mocks depend on already come with them.
    set(indirect_mocks)
    foreach(mock IN LISTS LINK_MOCKS)
        get_target_property(dependencies fastdds::mock::${mock} FASTDDS_MOCK_DEPENDS)
        if(dependencies)
            _get_mocks_closure(indirect_mocks ${dependencies})
        endif()
    endforeach()
    set(mock_targets)
    foreach(mock IN LISTS LINK_MOCKS)
        if(NOT mock IN_LIST indirect_mocks)
            list(APPEND mock_targets fastdds::mock::${mock})
        endif()
    endforeach()

    set(unit_targets)
    foreach(unit IN LISTS units)
        if(unit IN_LIST partially_replaced)
            # Objects are only propagated by linking the unit, which would bring the replaced ones too.
            string(REPLACE "::" "-" unit_var "${unit}")
            string(REPLACE "::" "-" unit_target "fastdds-${unit}")
            target_sources(${target} PRIVATE
                "$<FILTER:$<TARGET_OBJECTS:${unit_target}>,EXCLUDE,${replaced_objects_${unit_var}}>")
            # Read now: $<TARGET_PROPERTY> would not evaluate the generator expressions the property contains.
            get_target_property(unit_links ${unit_target} INTERFACE_LINK_LIBRARIES)
            if(unit_links)
                list(APPEND unit_targets ${unit_links})
            endif()
            add_dependencies(${target} ${unit_target})
        else()
            list(APPEND unit_targets fastdds::${unit})
        endif()
    endforeach()

    target_link_libraries(${target} ${mock_targets} ${unit_targets})
endfunction()
