# TINYXML2_FOUND
# TINYXML2_INCLUDE_DIR
# TINYXML2_SOURCE_DIR

find_package(TinyXML2 CONFIG QUIET)
if(TinyXML2_FOUND)
    message(STATUS "Found TinyXML2")
else()
    if(THIRDPARTY OR ANDROID)
        find_path(TINYXML2_INCLUDE_DIR NAMES tinyxml2.h NO_CMAKE_FIND_ROOT_PATH)
    else()
        find_path(TINYXML2_INCLUDE_DIR NAMES tinyxml2.h)
    endif()


    if(THIRDPARTY)
            find_path(TINYXML2_SOURCE_DIR NAMES tinyxml2.cpp NO_CMAKE_FIND_ROOT_PATH)
    else()
        find_library(TINYXML2_LIBRARY tinyxml2)
    endif()

    include(FindPackageHandleStandardArgs)

    if(THIRDPARTY)
        find_package_handle_standard_args(tinyxml2 DEFAULT_MSG TINYXML2_SOURCE_DIR TINYXML2_INCLUDE_DIR)

        mark_as_advanced(TINYXML2_INCLUDE_DIR TINYXML2_SOURCE_DIR)
    else()
        find_package_handle_standard_args(tinyxml2 DEFAULT_MSG TINYXML2_LIBRARY TINYXML2_INCLUDE_DIR)

        mark_as_advanced(TINYXML2_INCLUDE_DIR TINYXML2_LIBRARY)
    endif()
endif()
