# ASIO_FOUND
# ASIO_INCLUDE_DIR

find_package(Asio CONFIG QUIET)
if(Asio_FOUND)
    message(STATUS "Found Asio")
else()
    set(NO_FIND_ROOT_PATH_ "")
    if(ANDROID)
        set(NO_FIND_ROOT_PATH_ NO_CMAKE_FIND_ROOT_PATH)
    endif()

    find_path(ASIO_INCLUDE_DIR NAMES asio.hpp ${NO_FIND_ROOT_PATH_})

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(asio DEFAULT_MSG ASIO_INCLUDE_DIR)

    mark_as_advanced(ASIO_INCLUDE_DIR)
endif()
