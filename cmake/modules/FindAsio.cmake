set(ASIO_REQUIRED_VERSION ${FIND_VERSION})

if (THIRDPARTY_Asio STREQUAL "FORCE" OR ANDROID)
    find_path(Asio_INCLUDE_DIR NAMES asio.hpp NO_CMAKE_FIND_ROOT_PATH)
else()
    find_package(Asio CONFIG QUIET)

    # If Asio was not found, search for the header file asio.hpp in the common CMake directories.
    if(NOT Asio_FOUND)
        if(QNX)
            find_path(Asio_INCLUDE_DIR NAMES asio.hpp HINTS ${QNX_INSTALL_ROOT}/usr/include)
        else()
            find_path(Asio_INCLUDE_DIR NAMES asio.hpp)
        endif()
    else()
        set(Asio_FOUND_PACKAGE ON)
    endif()

    # Asio local version not found
    if(NOT Asio_INCLUDE_DIR)
        # If THIRDPARTY_Asio=ON the Asio version from thirdparty is used.
        if(THIRDPARTY_Asio STREQUAL "ON")
            find_path(Asio_INCLUDE_DIR NAMES asio.hpp NO_CMAKE_FIND_ROOT_PATH)
        # If THIRDPARTY_Asio=OFF the search is stopped and an error is shown
        else()
            message(FATAL_ERROR "Not found a local version of Asio installed.")
        endif()
    # An installed version of Asio has been found.
    # Check that the Asio version is equal to or greater than the minimum version required in Fast DDS.
    else()
        file(READ "${Asio_INCLUDE_DIR}/asio/version.hpp" VERSION_INCLUDE)
        string(REGEX MATCH "#define ASIO_VERSION ([0-9]+)" REGEX_VERSION ${VERSION_INCLUDE})
        set(ASIO_VERSION ${CMAKE_MATCH_1})
        math(EXPR ASIO_PATCH_VERSION ${ASIO_VERSION}%100)
        math(EXPR ASIO_MINOR_VERSION ${ASIO_VERSION}/100%1000)
        math(EXPR ASIO_MAYOR_VERSION ${ASIO_VERSION}/100000)
        set(ASIO_VERSION "${ASIO_MAYOR_VERSION}.${ASIO_MINOR_VERSION}.${ASIO_PATCH_VERSION}")

        if(${ASIO_VERSION} VERSION_LESS ${ASIO_REQUIRED_VERSION})
            # If THIRDPARTY_Asio=ON the Asio version from thirdparty is used.
            unset(Asio_INCLUDE_DIR)
            unset(Asio_FOUND)
            set(Asio_FOUND_PACKAGE OFF)
            if(THIRDPARTY_Asio STREQUAL "ON")
                find_path(Asio_INCLUDE_DIR NAMES asio.hpp NO_CMAKE_FIND_ROOT_PATH)
            # If THIRDPARTY_Asio=OFF the search is stopped and an error is shown
            else()
                message(FATAL_ERROR
                    "Found Asio version ${ASIO_VERSION}, which is not compatible with Fast DDS. "
                    "Minimum required Asio version: ${ASIO_REQUIRED_VERSION}"
                )
            endif()
        endif()
    endif()
endif()

# The fact that at this point Asio has not yet been found means that the package is not among the Fast DDS
# submodules.
# It could be updated later and retry the search for the package among the local dependencies.
if(Asio_INCLUDE_DIR AND (NOT Asio_FOUND_PACKAGE))
    include(FindPackageHandleStandardArgs)
    # Asio is considered to be found if Asio_INCLUDE_DIR is valid.
    find_package_handle_standard_args(Asio DEFAULT_MSG Asio_INCLUDE_DIR)
    mark_as_advanced(Asio_INCLUDE_DIR)
    message(STATUS "Found Asio ${ASIO_VERSION}: ${Asio_INCLUDE_DIR}")
else()
    message(STATUS "Cannot find package Asio")
endif()
