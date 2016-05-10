# CMake generated Testfile for 
# Source directory: /home/pablo/repos/rtps/test/profiling/cycles
# Build directory: /home/pablo/repos/rtps/build/Debug/test/profiling/cycles
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(profiling "/usr/bin/python3" "/home/pablo/repos/rtps/test/profiling/cycles/profiling.py")
set_tests_properties(profiling PROPERTIES  ENVIRONMENT "PROFILING_BINS=/home/pablo/repos/rtps/build/Debug/test/profiling/RTPSCommsProfiling;CMAKE_CURRENT_SOURCE_DIR=/home/pablo/repos/rtps/test/profiling/cycles")
