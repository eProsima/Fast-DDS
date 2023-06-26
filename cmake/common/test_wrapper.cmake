find_package(PythonInterp 3)

# Clean first so the exit code comes from the second process
execute_process(COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../tools/fastdds/fastdds.py shm clean)
execute_process(COMMAND ${ACTUAL_TEST} ${ACTUAL_ARGS})
