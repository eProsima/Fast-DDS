# Script that builds an external dummy project to check standard enforcement
include(CMakePrintHelpers)

# generate a per-test directory test to allow parallel execution
string(MD5 TEST_DIR "${FORCE_CXX} ${TEST_FALLBACK} ${BUILD_DIR}")
get_filename_component(BUILD_DIR ${BUILD_DIR} ABSOLUTE)
string(JOIN "/" BUILD_DIR ${BUILD_DIR} ${TEST_DIR})

# clean build dir if exits
if(EXISTS "${BUILD_DIR}")
    file(REMOVE_RECURSE "${BUILD_DIR}" )
endif()

execute_process(COMMAND ${CMAKE_COMMAND} 
                        -DFORCE_CXX=${FORCE_CXX} -DTEST_FALLBACK=${TEST_FALLBACK}
                        -DEPROSIMA_MODULE_PATH=${EPROSIMA_MODULE_PATH}
                        -B${BUILD_DIR} .
                WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR} 
                OUTPUT_VARIABLE my_output
                ERROR_VARIABLE error_log
                RESULT_VARIABLE fail_to_build)

if(fail_to_build)
    message(FATAL_ERROR "The dummy test fail to generate!!!")
    cmake_print_variables(my_output error_log)
endif()

execute_process(COMMAND ${CMAKE_COMMAND}
                        --build ${BUILD_DIR} --target std_force_test
                OUTPUT_VARIABLE my_output
                ERROR_VARIABLE error_log
                RESULT_VARIABLE fail_to_build)

if(fail_to_build OR (error_log MATCHES "[Ww][Aa][Rr][Nn][Ii][Nn][Gg]"))
    message(FATAL_ERROR "The dummy test fail to build!!!")
    cmake_print_variables(my_output error_log)
endif()
