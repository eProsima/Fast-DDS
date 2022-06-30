# Check if platform needs to explicitly specify linking with libatomic.
# It creates an interface target eProsima_atomic that will include the dependency if needed.
# The variable FASTDDS_REQUIRED_FLAGS can be used to pass specific compiler flags use in the build
# and that we want to mimick in the testing like --std=c++20

if(TARGET eProsima_atomic)
    return()
endif()

set(Atomic_FOUND FALSE CACHE BOOL "The atomic module testing has already been performed")

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

include(CheckCXXSourceCompiles)

# Test linking without atomic
unset(ATOMIC_WITHOUT_LIB)
check_cxx_source_compiles(
    "${ATOMIC_TEST_CODE}"
    ATOMIC_WITHOUT_LIB
)

if(NOT ATOMIC_WITHOUT_LIB) 
    unset(ATOMIC_WITH_LIB)
    set(CMAKE_REQUIRED_LIBRARIES -latomic)
    # Test linking with atomic
    check_cxx_source_compiles(
        "${ATOMIC_TEST_CODE}"
        ATOMIC_WITH_LIB
    )
endif()

# add interface target associated (cannot be imported to avoid propagation on static libraries)
add_library(eProsima_atomic INTERFACE)

# Populate the interface target properties
if (ATOMIC_WITH_LIB)
    # force to link to atomic when the dummy target is present
    target_link_libraries(eProsima_atomic INTERFACE atomic)
elseif(NOT ATOMIC_WITHOUT_LIB)
    message(FATAL_ERROR "Unable to create binaries with atomic dependencies")
endif()

# clean local variables
unset(ATOMIC_TEST_CODE)
unset(ATOMIC_WITH_LIB)
unset(ATOMIC_WITHOUT_LIB)
unset(HAVE_LIBATOMIC)
