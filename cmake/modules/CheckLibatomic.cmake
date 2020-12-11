# LINK_LIBATOMIC

# Check if platform needs to explicitly specify linking with libatomic

set(LINK_LIBATOMIC 0)

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

# make the compiler receive for this testing the actual build flags
set(OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
set(CMAKE_REQUIRED_FLAGS ${FASTDDS_REQUIRED_FLAGS})

# Test linking without atomic
check_cxx_source_compiles(
    "${ATOMIC_TEST_CODE}"
    ATOMIC_WITHOUT_LIB
)

# Test linking with atomic, note that it will try first build using C if the C compiler is enabled (as is the case in
# fastrtps project). C compiler will complain if C++ flags are passed via CMAKE_REQUIRED_FLAGS ruining the check. We
# must locally change CMAKE_C_COMPILER_LOADED value to force the use of C++ compiler. Note that future changes in
# CheckLibraryExists module may alter this workaround.
set(OLD_CMAKE_C_COMPILER_LOADED ${CMAKE_C_COMPILER_LOADED})
set(CMAKE_C_COMPILER_LOADED 0)

check_library_exists(atomic __atomic_load_8 "" HAVE_LIBATOMIC)

set(CMAKE_C_COMPILER_LOADED ${OLD_CMAKE_C_COMPILER_LOADED})

if (HAVE_LIBATOMIC)
    set(OLD_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
    set(CMAKE_REQUIRED_LIBRARIES atomic ${CMAKE_REQUIRED_LIBRARIES})
    check_cxx_source_compiles(
        "${ATOMIC_TEST_CODE}"
        ATOMIC_WITH_LIB
    )

    set(CMAKE_REQUIRED_LIBRARIES ${OLD_CMAKE_REQUIRED_LIBRARIES})
else()
    set(ATOMIC_WITH_LIB 0)
endif()

# Set LINK_LIBATOMIC to true only if linking with atomic is required
if (NOT ATOMIC_WITHOUT_LIB)
    if (ATOMIC_WITH_LIB)
        set(LINK_LIBATOMIC 1)
    else()
        message(FATAL_ERROR "Unable to create binaries with atomic dependencies")
    endif()

# restore original flags to avoid interference in other tests 
set(CMAKE_REQUIRED_FLAGS ${OLD_CMAKE_REQUIRED_FLAGS})

endif()
