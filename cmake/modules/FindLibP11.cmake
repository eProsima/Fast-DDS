# FindLibP11
#
# Generates an imported target associated to an available pksc11 library:
#
#   + On linux relies on the apt package libp11-dev
#
#   + On Windows the library must be build from sources available at https://github.com/OpenSC/libp11.git
#     Given that each user must build its own binaries the following environment variables must be set to hint
#     where to locate headers and binaries (semicolon-separated list see https://cmake.org/cmake/help/v3.22/variable/PackageName_ROOT.html):
#       + LibP11_ROOT_32 -> to reference sources and 32 bit binaries location
#       + LibP11_ROOT_64 -> to reference sources and 64 bit binaries location

if(TARGET eProsima_p11)
    return()
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(LibP11_ROOT "$ENV{LibP11_ROOT_32}")
else()
    set(LibP11_ROOT "$ENV{LibP11_ROOT_64}")
endif()

find_path(LIBP11_INCLUDE_DIR NAMES libp11.h HINTS ${LibP11_ROOT})
find_library(LIBP11_LIBRARY NAMES libp11.a libp11.so libp11.lib HINTS ${LibP11_ROOT})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibP11 DEFAULT_MSG LIBP11_LIBRARY LIBP11_INCLUDE_DIR)

if(LibP11_FOUND)
    # add the target
    add_library(eProsima_p11 STATIC IMPORTED)

    # update the properties
    set_target_properties(eProsima_p11 PROPERTIES
        IMPORTED_LOCATION "${LIBP11_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${LIBP11_INCLUDE_DIR}"
    )
endif()

# clean local variables
unset(LIBP11_INCLUDE_DIR)
unset(LIBP11_LIBRARY)
unset(LibP11_ROOT)
