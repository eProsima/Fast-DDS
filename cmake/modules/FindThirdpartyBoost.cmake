# Test compilation of boost on thirdparty directory
# and set 

# (This file is almost an identical copy of the original FindGTest.cmake file,
#  feel free to use it as it is or modify it for your own needs.)

set(THIRDPARTY_BOOST_INCLUDE_DIR 
    ${PROJECT_SOURCE_DIR}/thirdparty/boost/include 
    CACHE 
    FILEPATH
    "Path to thirdparty/boost"
)

find_package(Threads REQUIRED)

if(WIN32 OR APPLE)
    set(THIRDPARTY_BOOST_LINK_LIBS ${CMAKE_THREAD_LIBS_INIT})
else() # Posix
    set(THIRDPARTY_BOOST_LINK_LIBS ${CMAKE_THREAD_LIBS_INIT} rt)
endif()

try_compile(IS_THIRDPARTY_BOOST_OK
        ${CMAKE_BINARY_DIR}
        ${PROJECT_SOURCE_DIR}/thirdparty/boost/test/ThirdpartyBoostCompile_test.cpp
         CMAKE_FLAGS "-DINCLUDE_DIRECTORIES=${THIRDPARTY_BOOST_INCLUDE_DIR}"
         LINK_LIBRARIES ${THIRDPARTY_BOOST_LINK_LIBS}
         OUTPUT_VARIABLE OUT
    )

if(NOT IS_THIRDPARTY_BOOST_OK)
    message(FATAL_ERROR "Couldn't compile thirdparty/boost with current configuration!!!\n" ${OUT})
else()
    message(STATUS "Thirdparty/boost compiled OK")
    mark_as_advanced(THIRDPARTY_BOOST_INCLUDE_DIR)
endif()
