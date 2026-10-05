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

###############################################################################
# Reusable OBJECT libraries for the unit test suites.
#
# The source groups under src/cpp (fastdds::log, fastdds::xtypes::*, ...) are
# INTERFACE libraries: linking one of them does not reuse anything, it just
# appends its source files to the consumer, so every unit test executable
# recompiles the whole group from scratch.
#
# The group sources cannot simply be compiled once, because a test may inject
# header mocks (test/mock/...) or preprocessor definitions that change the code
# generated for those very sources.  What *can* be done is to compile them once
# per distinct combination of the settings the group is actually sensitive to,
# and share the resulting objects among every test using that combination.
#
# Usage:
#   * next to the sources, declare what the group is sensitive to:
#
#       fastdds_declare_object_group(fastdds-log
#           MOCKS   rtps/Log
#           DEFINES HAVE_LOG_NO_INFO FASTDDS_ENFORCE_LOG_INFO)
#
#     MOCKS   paths relative to test/mock of the mock directories holding a
#             header that is reachable from this group's translation units.
#     DEFINES preprocessor symbols this group's code branches on.
#
#   * in the test, after target_include_directories()/target_compile_definitions(),
#     replace the group in target_link_libraries() with:
#
#       fastdds_target_link_object_groups(MyTests fastdds::log)
#
# The flavour (mock set + definition set) is derived from the test target
# itself, so a test that adds or drops a mock automatically moves to the right
# object library with no extra bookkeeping.
#
# Set FASTDDS_REUSE_TEST_OBJECTS=OFF to fall back to the old behaviour (every
# test compiles its own copy), which is handy to measure the gain.
###############################################################################

if(COMMAND fastdds_declare_object_group)
    return()
endif()

option(FASTDDS_REUSE_TEST_OBJECTS
    "Share compiled objects of the src/cpp source groups among unit test suites" ON)

# Store in <out> the real target name behind <name> if <name> is an ALIAS
# target (e.g. fastdds::log -> fastdds-log); otherwise store <name> unchanged.
macro(_fastdds_robj_resolve out name)
    set(${out} ${name})
    if(TARGET ${name})
        get_target_property(_fastdds_robj_alias ${name} ALIASED_TARGET)
        if(_fastdds_robj_alias)
            set(${out} ${_fastdds_robj_alias})
        endif()
        unset(_fastdds_robj_alias)
    endif()
endmacro()

# Declare a source group as shareable, and state what it is sensitive to.
function(fastdds_declare_object_group group)
    cmake_parse_arguments(ARG "" "" "MOCKS;DEFINES" ${ARGN})

    _fastdds_robj_resolve(_group ${group})
    if(NOT TARGET ${_group})
        message(FATAL_ERROR "fastdds_declare_object_group: '${group}' is not a target")
    endif()

    set_target_properties(${_group} PROPERTIES
        FASTDDS_OBJ_GROUP TRUE
        FASTDDS_OBJ_MOCKS "${ARG_MOCKS}"
        FASTDDS_OBJ_DEFINES "${ARG_DEFINES}"
        )
endfunction()

# Collect the sources a group contributes, following the INTERFACE libraries it
# links against, exactly like the INTERFACE source propagation would.
function(_fastdds_robj_group_sources out group)
    set(_pending ${group})
    set(_seen "")
    set(_sources "")

    while(_pending)
        list(POP_FRONT _pending _current)
        if(_current IN_LIST _seen)
            continue()
        endif()
        list(APPEND _seen ${_current})

        if(NOT TARGET ${_current})
            continue()
        endif()

        get_target_property(_srcs ${_current} INTERFACE_SOURCES)
        if(_srcs)
            get_target_property(_dir ${_current} SOURCE_DIR)
            foreach(_s IN LISTS _srcs)
                if(NOT IS_ABSOLUTE "${_s}")
                    set(_s "${_dir}/${_s}")
                endif()
                list(APPEND _sources "${_s}")
            endforeach()
        endif()

        get_target_property(_deps ${_current} INTERFACE_LINK_LIBRARIES)
        if(_deps)
            foreach(_d IN LISTS _deps)
                if(TARGET ${_d})
                    _fastdds_robj_resolve(_dr ${_d})
                    get_target_property(_isgroup ${_dr} FASTDDS_OBJ_GROUP)
                    if(_isgroup)
                        list(APPEND _pending ${_dr})
                    endif()
                endif()
            endforeach()
        endif()
    endwhile()

    list(REMOVE_DUPLICATES _sources)
    set(${out} "${_sources}" PARENT_SCOPE)
endfunction()

# Work out the flavour <target> needs of <group>: the subset of the group's
# sensitive mocks/definitions that <target> actually uses.
function(_fastdds_robj_flavour target group out_mocks out_defines)
    get_target_property(_mocks ${group} FASTDDS_OBJ_MOCKS)
    get_target_property(_defines ${group} FASTDDS_OBJ_DEFINES)

    set(_used_mocks "")
    get_target_property(_incs ${target} INCLUDE_DIRECTORIES)
    if(_mocks AND _incs)
        foreach(_m IN LISTS _mocks)
            foreach(_i IN LISTS _incs)
                if("${_i}" MATCHES "/test/mock/${_m}/?$")
                    list(APPEND _used_mocks "${_m}")
                    break()
                endif()
            endforeach()
        endforeach()
    endif()

    set(_used_defines "")
    get_target_property(_tdefs ${target} COMPILE_DEFINITIONS)
    get_directory_property(_ddefs COMPILE_DEFINITIONS)
    set(_candidates "")
    foreach(_list _tdefs _ddefs)
        if(${_list})
            list(APPEND _candidates ${${_list}})
        endif()
    endforeach()
    if(_defines AND _candidates)
        foreach(_d IN LISTS _defines)
            foreach(_c IN LISTS _candidates)
                string(REGEX REPLACE "^-D" "" _c "${_c}")
                if("${_c}" MATCHES "^${_d}(=.*)?$")
                    list(APPEND _used_defines "${_c}")
                endif()
            endforeach()
        endforeach()
    endif()

    foreach(_v _used_mocks _used_defines)
        if(${_v})
            list(REMOVE_DUPLICATES ${_v})
            list(SORT ${_v})
        endif()
    endforeach()

    set(${out_mocks} "${_used_mocks}" PARENT_SCOPE)
    set(${out_defines} "${_used_defines}" PARENT_SCOPE)
endfunction()

# Drop from <target>'s own source list every file also provided by a group.
function(_fastdds_robj_strip_sources target group_sources)
    get_target_property(_tsrcs ${target} SOURCES)
    if(NOT _tsrcs)
        return()
    endif()
    get_target_property(_tdir ${target} SOURCE_DIR)

    set(_group_real "")
    foreach(_s IN LISTS group_sources)
        if("${_s}" MATCHES "^\\$<")
            continue()
        endif()
        file(REAL_PATH "${_s}" _rp BASE_DIRECTORY "${_tdir}")
        list(APPEND _group_real "${_rp}")
    endforeach()

    set(_kept "")
    set(_dropped "")
    foreach(_s IN LISTS _tsrcs)
        if("${_s}" MATCHES "^\\$<")
            list(APPEND _kept "${_s}")
            continue()
        endif()
        file(REAL_PATH "${_s}" _rp BASE_DIRECTORY "${_tdir}")
        if("${_rp}" IN_LIST _group_real)
            list(APPEND _dropped "${_s}")
        else()
            list(APPEND _kept "${_s}")
        endif()
    endforeach()

    if(_dropped)
        set_target_properties(${target} PROPERTIES SOURCES "${_kept}")
        list(LENGTH _dropped _n)
        message(STATUS
            "Reusable objects: ${target} no longer compiles ${_n} file(s) already"
            " provided by a shared source group")
    endif()
endfunction()

# Make <target> use the shared objects of the given source groups.
# Must be called after the target's include directories and compile definitions
# have been set, since the flavour is derived from them.
function(fastdds_target_link_object_groups target)
    foreach(_g IN LISTS ARGN)
        _fastdds_robj_resolve(_group ${_g})
        if(NOT TARGET ${_group})
            message(FATAL_ERROR "fastdds_target_link_object_groups: unknown group '${_g}'")
        endif()
        get_target_property(_isgroup ${_group} FASTDDS_OBJ_GROUP)
        if(NOT _isgroup)
            message(FATAL_ERROR
                "fastdds_target_link_object_groups: '${_g}' was not declared with "
                "fastdds_declare_object_group()")
        endif()

        _fastdds_robj_group_sources(_gsrcs ${_group})

        # Some suites also list part of the group explicitly in their own source
        # list. INTERFACE source propagation used to merge both, sharing objects
        # would instead link the same symbols twice, so drop them here.
        _fastdds_robj_strip_sources(${target} "${_gsrcs}")

        if(NOT FASTDDS_REUSE_TEST_OBJECTS)
            # Old behaviour: compile the group into the consumer.
            target_sources(${target} PRIVATE ${_gsrcs})
            continue()
        endif()

        _fastdds_robj_flavour(${target} ${_group} _mocks _defines)

        string(MD5 _hash "${_group}|${_mocks}|${_defines}")
        string(SUBSTRING "${_hash}" 0 8 _hash)
        set(_objlib "${_group}-obj-${_hash}")

        get_property(_known GLOBAL PROPERTY FASTDDS_REUSABLE_OBJECTS)
        if(NOT "${_objlib}" IN_LIST _known)
            set_property(GLOBAL APPEND PROPERTY FASTDDS_REUSABLE_OBJECTS "${_objlib}")
            set_property(GLOBAL PROPERTY FASTDDS_ROBJ_${_objlib}_GROUP "${_group}")
            set_property(GLOBAL PROPERTY FASTDDS_ROBJ_${_objlib}_MOCKS "${_mocks}")
            set_property(GLOBAL PROPERTY FASTDDS_ROBJ_${_objlib}_DEFINES "${_defines}")
        endif()

        # $<TARGET_OBJECTS:> instead of target_link_libraries() so that this
        # works no matter whether the test uses the plain or the keyword
        # signature of target_link_libraries().  The object library itself is
        # created later, by fastdds_create_reusable_objects().
        target_sources(${target} PRIVATE $<TARGET_OBJECTS:${_objlib}>)
        set_property(GLOBAL APPEND PROPERTY FASTDDS_ROBJ_${_objlib}_USERS "${target}")
    endforeach()
endfunction()

# Materialize every object library requested so far.  Call it once, from a
# dedicated directory added after all the test directories, so the libraries do
# not inherit directory level settings of whichever test happened to ask first.
# They do inherit the definitions common to every unit test, set by the parent
# directory (test/unittest/CMakeLists.txt).
function(fastdds_create_reusable_objects)
    if(NOT FASTDDS_REUSE_TEST_OBJECTS)
        return()
    endif()

    get_property(_objs GLOBAL PROPERTY FASTDDS_REUSABLE_OBJECTS)
    if(NOT _objs)
        return()
    endif()

    foreach(_objlib IN LISTS _objs)
        get_property(_group GLOBAL PROPERTY FASTDDS_ROBJ_${_objlib}_GROUP)
        get_property(_mocks GLOBAL PROPERTY FASTDDS_ROBJ_${_objlib}_MOCKS)
        get_property(_defines GLOBAL PROPERTY FASTDDS_ROBJ_${_objlib}_DEFINES)

        _fastdds_robj_group_sources(_srcs ${_group})

        set(_mock_includes "")
        foreach(_m IN LISTS _mocks)
            list(APPEND _mock_includes "${PROJECT_SOURCE_DIR}/test/mock/${_m}")
        endforeach()

        add_library(${_objlib} OBJECT ${_srcs})

        target_compile_features(${_objlib} PRIVATE cxx_std_11)

        # Mock directories first, they are meant to shadow the real headers.
        target_include_directories(${_objlib} PRIVATE
            ${_mock_includes}
            ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_BINARY_DIR}/include
            ${PROJECT_SOURCE_DIR}/src/cpp
            ${Asio_INCLUDE_DIR}
            ${THIRDPARTY_BOOST_INCLUDE_DIR}
            ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
            ${TINYXML2_INCLUDE_DIR}
            $<$<BOOL:${OPENSSL_INCLUDE_DIR}>:${OPENSSL_INCLUDE_DIR}>
            $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
            )

        # Only the flavour definitions. The common ones are inherited from the
        # directory, which must be the same one setting them for the test
        # targets (test/unittest), so objects and tests compile alike.
        if(_defines)
            target_compile_definitions(${_objlib} PRIVATE ${_defines})
        endif()

        # Only for the usage requirements (include directories) of the headers
        # these sources pull in.
        target_link_libraries(${_objlib} PRIVATE
            fastcdr
            foonathan_memory
            GTest::gmock
            )

        set_target_properties(${_objlib} PROPERTIES FOLDER "ReusableObjects")

        get_property(_users GLOBAL PROPERTY FASTDDS_ROBJ_${_objlib}_USERS)
        foreach(_u IN LISTS _users)
            add_dependencies(${_u} ${_objlib})
        endforeach()

        list(LENGTH _users _n)
        message(STATUS
            "Reusable objects: ${_objlib} shared by ${_n} test target(s)"
            " mocks=[${_mocks}] defines=[${_defines}]")
    endforeach()
endfunction()
