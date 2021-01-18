# ASIO_FOUND
# ASIO_INCLUDE_DIR

set(ASIO_REQUIRED_VERSION 1.13.0)

if (THIRDPARTY_ASIO STREQUAL "FORCE" OR ANDROID)
    find_path(ASIO_INCLUDE_DIR NAMES asio.hpp NO_CMAKE_FIND_ROOT_PATH)
else()
    find_path(ASIO_INCLUDE_DIR NAMES asio.hpp)

    # Not found a local version of Asio
    if(NOT ASIO_INCLUDE_DIR)
        # If THIRDPARTY_ASIO=ON the Asio version from thirdparty is used.
        if(THIRDPARTY_ASIO STREQUAL "ON")
            find_path(ASIO_INCLUDE_DIR NAMES asio.hpp NO_CMAKE_FIND_ROOT_PATH)
        # If THIRDPARTY_ASIO=OFF the search is stopped and an error is shown
        else()
            message(SEND_ERROR "Not found a local version of Asio installed.")
        endif()
    # An installed version of Asio has been found.
    # Check that the Asio version is equal to or greater than the minimum version required in Fast DDS.
    else()
        file(READ "${ASIO_INCLUDE_DIR}/asio/version.hpp" VERSION_INCLUDE)
        string(REGEX MATCH "#define ASIO_VERSION ([0-9]+)" REGEX_VERSION ${VERSION_INCLUDE})
        set(ASIO_VERSION ${CMAKE_MATCH_1})
        math(EXPR ASIO_PATCH_VERSION ${ASIO_VERSION}%100)
        math(EXPR ASIO_MINOR_VERSION ${ASIO_VERSION}/100%1000)
        math(EXPR ASIO_MAYOR_VERSION ${ASIO_VERSION}/100000)
        set(ASIO_VERSION "${ASIO_MAYOR_VERSION}.${ASIO_MINOR_VERSION}.${ASIO_PATCH_VERSION}")

        if(${ASIO_VERSION} VERSION_LESS ${ASIO_REQUIRED_VERSION})
            # If THIRDPARTY_ASIO=ON the Asio version from thirdparty is used.
            if(THIRDPARTY_ASIO STREQUAL "ON")
                find_path(ASIO_INCLUDE_DIR NAMES asio.hpp NO_CMAKE_FIND_ROOT_PATH)
            # If THIRDPARTY_ASIO=OFF the search is stopped and an error is shown
            else()
                message(SEND_ERROR
                    "Found Asio version ${ASIO_VERSION}, which is not compatible with Fast DDS. \n"
                    "Minimum required Asio version: ${ASIO_REQUIRED_VERSION}"
                )
            endif()
        endif()
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Asio DEFAULT_MSG ASIO_INCLUDE_DIR)
mark_as_advanced(ASIO_INCLUDE_DIR)
message(STATUS "Found Asio: ${ASIO_INCLUDE_DIR}/asio ")

