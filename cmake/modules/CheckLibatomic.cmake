# LINK_LIBATOMIC

# Check if platform needs to explicitly specify linking with libatomic

set(LINK_LIBATOMIC 0)

set(ATOMIC_TEST_CODE
	"#include <atomic>
	int main()
	{
		volatile std::atomic_ullong i(0xcafebabedeadbeeful);
		i += 0xfeedbadc0de8f00dul;
		return 0;
	}"
)

include(CheckLibraryExists)
include(CheckCXXSourceCompiles)

set(OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
set(CMAKE_REQUIRED_FLAGS "-std=c++11 ${CMAKE_REQUIRED_FLAGS}")

# Test linking without atomic
check_cxx_source_compiles(
	"${ATOMIC_TEST_CODE}"
	ATOMIC_WITHOUT_LIB
)

# Test linking with atomic
check_library_exists(atomic __atomic_load_8 "" HAVE_LIBATOMIC)
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

set(CMAKE_REQUIRED_FLAGS ${OLD_CMAKE_REQUIRED_FLAGS})


# Set LINK_LIBATOMIC to true only if linking with atomic is required
if (NOT ATOMIC_WITHOUT_LIB)
	if (ATOMIC_WITH_LIB)
		set(LINK_LIBATOMIC 1)
	else()
		message(FATAL_ERROR "Unable to create binaries with atomic dependencies")
	endif()
endif()
