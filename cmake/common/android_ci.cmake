###############################################################################
# Android testing post-installation steps
###############################################################################
message(STATUS "Fixing CI Paths")
message(STATUS "Vars:" ${ANDROID} " " ${ANDROID_TESTING_ROOT})
if(ANDROID)
    if(ANDROID_TESTING_ROOT)
        file(GLOB_RECURSE CTEST_GENERATED_FILES RELATIVE ${CMAKE_BINARY_DIR} "CTestTestfile.cmake")
        message(STATUS "Vars:" ${location} " " ${CTEST_GENERATED_FILES})
        foreach(CTEST_FILE ${CTEST_GENERATED_FILES})
            file(READ ${CTEST_FILE} contents)
            message(STATUS "Vars:" ${CTEST_FILE} " " ${CMAKE_BINARY_DIR} " " ${ANDROID_TESTING_ROOT})
            string(REGEX REPLACE "${CMAKE_BINARY_DIR}" "${ANDROID_TESTING_ROOT}" contents "${contents}")
            file(REMOVE ${CTEST_FILE})
            file(WRITE  ${CTEST_FILE} "${contents}")
        endforeach()
    endif()
endif()

