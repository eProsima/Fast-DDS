# Check if platform needs to explicitly specify linking with libatomic.
# It creates an interface target eProsima_atomic that will include the dependency if needed.
# The variable FASTDDS_REQUIRED_FLAGS can be used to pass specific compiler flags use in the build
# and that we want to mimick in the testing like --std=c++20

if(TARGET eProsima_atomic)
    return()
endif()

set(Atomic_FOUND FALSE)

set(ATOMIC_TEST_CODE
    "#define _ENABLE_ATOMIC_ALIGNMENT_FIX

    #include <atomic>
    #include <cstdint>

    int main()
    {
        using namespace std;

        volatile atomic_ullong i(0xcafebabedeadbeeful);
        i += 0xfeedbadc0de8f00dul;

        struct Pointer
        {
            uint32_t write_p;
            uint32_t free_cells;
        };

        Pointer dummy{1,1};
        atomic<Pointer> pointer;
        pointer = dummy;

        return 0;
    }"
)

include(CheckLibraryExists)
include(CheckCXXSourceCompiles)

# Test linking without atomic
check_cxx_source_compiles(
    "${ATOMIC_TEST_CODE}"
    ATOMIC_WITHOUT_LIB
)

check_library_exists(atomic __atomic_load_8 "" HAVE_LIBATOMIC)
set(Atomic_FOUND HAVE_LIBATOMIC)

if (HAVE_LIBATOMIC)
    set(CMAKE_REQUIRED_LIBRARIES atomic ${CMAKE_REQUIRED_LIBRARIES})
    check_cxx_source_compiles(
        "${ATOMIC_TEST_CODE}"
        ATOMIC_WITH_LIB
    )
endif()

# add interface target associated (cannot be imported to avoid propagation on static libraries)
add_library(eProsima_atomic INTERFACE)

# Populate the interface target properties
if (NOT ATOMIC_WITHOUT_LIB)
    if (ATOMIC_WITH_LIB)
        # force to link to atomic when the dummy target is present
        if (NOT (CMAKE_VERSION VERSION_LESS "3.11.4"))
            target_link_libraries(eProsima_atomic INTERFACE atomic)
        else()
            set_property(TARGET eProsima_atomic PROPERTY INTERFACE_LINK_LIBRARIES atomic)
        endif()
    else()
        message(FATAL_ERROR "Unable to create binaries with atomic dependencies")
    endif()
endif()

# clean local variables
unset(ATOMIC_TEST_CODE)
unset(ATOMIC_WITH_LIB)
unset(ATOMIC_WITHOUT_LIB)
unset(HAVE_LIBATOMIC)
