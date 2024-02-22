# TINYXML2_FOUND
# TINYXML2_INCLUDE_DIR
# TINYXML2_SOURCE_DIR

option(TINYXML2_FROM_SOURCE "Integrate TinyXML2 source code inside Fast DDS" OFF)

# Option for evaluating whether we are looking in for tinyxml2 in submodule
set(TINYXML2_FROM_THIRDPARTY OFF)
if (
    (THIRDPARTY_TinyXML2 STREQUAL "ON") OR
    (THIRDPARTY_TinyXML2 STREQUAL "FORCE")
)
    set(TINYXML2_FROM_THIRDPARTY ON)
endif()

if(NOT (TINYXML2_FROM_SOURCE OR TINYXML2_FROM_THIRDPARTY))
    find_package(TinyXML2 CONFIG QUIET)
endif()

if(TinyXML2_FOUND AND NOT TINYXML2_FROM_THIRDPARTY)
    message(STATUS "Found TinyXML2: ${TinyXML2_DIR}")
    if(NOT TINYXML2_LIBRARY)
        # in this case, we're probably using TinyXML2 version 5.0.0 or greater
        # in which case tinyxml2 is an exported target and we should use that
        if(TARGET tinyxml2)
          set(TINYXML2_LIBRARY tinyxml2)
        elseif(TARGET tinyxml2::tinyxml2)
          set(TINYXML2_LIBRARY tinyxml2::tinyxml2)
        endif()
    endif()
else()
    if(TINYXML2_FROM_THIRDPARTY OR ANDROID)
        set(TINYXML2_FROM_SOURCE ON)
        find_path(TINYXML2_INCLUDE_DIR NAMES tinyxml2.h NO_CMAKE_FIND_ROOT_PATH)
    else()
        find_path(TINYXML2_INCLUDE_DIR NAMES tinyxml2.h)
    endif()

    if(TINYXML2_FROM_SOURCE)
        find_path(TINYXML2_SOURCE_DIR NAMES tinyxml2.cpp NO_CMAKE_FIND_ROOT_PATH)
    else()
        find_library(TINYXML2_LIBRARY tinyxml2)
    endif()

    include(FindPackageHandleStandardArgs)

    if(TINYXML2_FROM_SOURCE)
        find_package_handle_standard_args(TinyXML2 DEFAULT_MSG TINYXML2_SOURCE_DIR TINYXML2_INCLUDE_DIR)
        mark_as_advanced(TINYXML2_INCLUDE_DIR TINYXML2_SOURCE_DIR)
    else()
        find_package_handle_standard_args(TinyXML2 DEFAULT_MSG TINYXML2_LIBRARY TINYXML2_INCLUDE_DIR)
        mark_as_advanced(TINYXML2_INCLUDE_DIR TINYXML2_LIBRARY)
    endif()
endif()
