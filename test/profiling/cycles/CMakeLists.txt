find_package(PythonInterp 3 REQUIRED)

add_test(NAME cycles_profiling
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/profiling.py)

set_property(TEST memory_profiling PROPERTY LABELS "NoMemoryCheck")

if(WIN32)
    set_property(TEST cycles_profiling PROPERTY ENVIRONMENT
        "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;${BOOST_LIBRARYDIR}\\;$ENV{PATH}")
endif()
set_property(TEST cycles_profiling APPEND PROPERTY ENVIRONMENT
    "PROFILING_BINS=$<TARGET_FILE:RTPSCommsProfiling>\\;$<TARGET_FILE:RTPSPubSubProfiling>")
set_property(TEST cycles_profiling APPEND PROPERTY ENVIRONMENT
             "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
