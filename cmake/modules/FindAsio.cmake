# ASIO_FOUND
# ASIO_INCLUDE_DIR

if(NOT THIRDPARTY)
    find_path(Asio_INCLUDE_DIR NAMES asio.hpp)
    if(Asio_INCLUDE_DIR)
        set(Asio_FOUND TRUE CACHE BOOL "Asio was found or not" FORCE)
    endif()
endif()

if(Asio_FOUND AND NOT THIRDPARTY)
    message(STATUS "Found ASIO: ${Asio_INCLUDE_DIR}")
    include_directories(${Asio_INCLUDE_DIR})
else()
    if(THIRDPARTY OR ANDROID)
        find_path(ASIO_INCLUDE_DIR NAMES asio.hpp NO_CMAKE_FIND_ROOT_PATH)
    else()
        find_path(ASIO_INCLUDE_DIR NAMES asio.hpp)
    endif()

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(Asio DEFAULT_MSG ASIO_INCLUDE_DIR)

    mark_as_advanced(ASIO_INCLUDE_DIR)
endif()
