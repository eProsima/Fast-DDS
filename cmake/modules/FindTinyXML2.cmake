# TINYXML2_FOUND
# TINYXML2_INCLUDE_DIR
# TINYXML2_SOURCE_DIR

option(TINYXML2_FROM_SOURCE "Integrate TinyXML2 source code inside Fast RTPS" OFF)

if(NOT (TINYXML2_FROM_SOURCE OR THIRDPARTY))
    if(DEFINED ENV{ROS_DISTRO})
        set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${ROS_PACKAGE_PATH}/tinyxml2_vendor/cmake/Modules/FindTinyXML2.cmake")
        find_package(TinyXML2 CONFIG QUIET)
    else()
        find_path(TINYXML2_INCLUDE_DIR "tinyxml2.h")
        find_library(TINYXML2_LIBRARY NAMES "tinyxml2")

        if(TINYXML2_INCLUDE_DIR AND TINYXML2_LIBRARY)
            set(TinyXML2_FOUND TRUE CACHE BOOL "TinyXML2 was found or not" FORCE)
        endif()
    endif()
endif()

if(TinyXML2_FOUND AND NOT THIRDPARTY)
    message(STATUS "Found TinyXML2: ${TINYXML2_INCLUDE_DIR}")
    include_directories(${TINYXML2_INCLUDE_DIR})
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
    if(THIRDPARTY OR ANDROID)
        set(TINYXML2_FROM_SOURCE ON)
    endif()

    if(THIRDPARTY OR ANDROID)
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
        find_package_handle_standard_args(tinyxml2 DEFAULT_MSG TINYXML2_SOURCE_DIR TINYXML2_INCLUDE_DIR)
        mark_as_advanced(TINYXML2_INCLUDE_DIR TINYXML2_SOURCE_DIR)
    else()
        find_package_handle_standard_args(tinyxml2 DEFAULT_MSG TINYXML2_LIBRARY TINYXML2_INCLUDE_DIR)
        mark_as_advanced(TINYXML2_INCLUDE_DIR TINYXML2_LIBRARY)
    endif()
endif()
