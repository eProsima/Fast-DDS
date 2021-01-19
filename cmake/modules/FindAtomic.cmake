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

# preserve some framework variables values
set(OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
set(OLD_CMAKE_C_COMPILER_LOADED ${CMAKE_C_COMPILER_LOADED})
set(OLD_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})

# Test linking with atomic, note that it will try first build using C if the C compiler is enabled (as is the case in
# fastrtps project). C compiler will complain if C++ flags are passed via CMAKE_REQUIRED_FLAGS ruining the check. We
# must locally change CMAKE_C_COMPILER_LOADED value to force the use of C++ compiler. Note that future changes in
# CheckLibraryExists module may alter this workaround.
set(CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS} ${FASTDDS_REQUIRED_FLAGS})
set(CMAKE_C_COMPILER_LOADED 0)

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
        if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.11.4")
            target_link_libraries(eProsima_atomic INTERFACE atomic)
        else()
            set_property(TARGET eProsima_atomic PROPERTY INTERFACE_LINK_LIBRARIES atomic)
        endif()
    else()
        message(FATAL_ERROR "Unable to create binaries with atomic dependencies")
    endif()
endif()

# restore some framework variables values
set(CMAKE_REQUIRED_FLAGS ${OLD_CMAKE_REQUIRED_FLAGS})
set(CMAKE_C_COMPILER_LOADED ${OLD_CMAKE_C_COMPILER_LOADED})
set(CMAKE_REQUIRED_LIBRARIES ${OLD_CMAKE_REQUIRED_LIBRARIES})

# clean local variables
unset(ATOMIC_TEST_CODE)
unset(ATOMIC_WITH_LIB)
unset(ATOMIC_WITHOUT_LIB)
unset(HAVE_LIBATOMIC)
