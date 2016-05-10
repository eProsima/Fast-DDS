find_package(PythonInterp 3 REQUIRED)

add_test(NAME profiling
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/profiling.py)

if(WIN32)
    set_property(TEST profiling PROPERTY ENVIRONMENT
        "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;${BOOST_LIBRARYDIR}\\;$ENV{PATH}")
endif()
set_property(TEST profiling APPEND PROPERTY ENVIRONMENT
    "PROFILING_BINS=$<TARGET_FILE:RTPSCommsProfiling>")
set_property(TEST profiling APPEND PROPERTY ENVIRONMENT
             "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
