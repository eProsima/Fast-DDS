add_subdirectory(mutex_testing_tool)

set(USER_THREAD_NONBLOCKED_TEST UserThreadNonBlockedTest.cpp)

add_executable(user_thread_nonblocked_test ${USER_THREAD_NONBLOCKED_TEST})
target_compile_definitions(user_thread_nonblocked_test PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(user_thread_nonblocked_test PRIVATE ${CMAKE_SOURCE_DIR}/src/cpp/utils)
target_link_libraries(user_thread_nonblocked_test mutex_testing_tool fastdds fastcdr GTest::gtest)

STRING(REPLACE " " "\\ " MUTEX_PRELOAD_LIBRARY_FILE "$<TARGET_FILE:mutex_testing_tool_preload>")
gtest_discover_tests(user_thread_nonblocked_test
    PROPERTIES
        ENVIRONMENT "LD_LIBRARY_PATH=$<TARGET_FILE_DIR:mutex_testing_tool_preload>"
        ENVIRONMENT "LD_PRELOAD=$<TARGET_FILE_NAME:mutex_testing_tool_preload>"
    LABELS "NoMemoryCheck"
    )

set(TIMED_CONDITION_VARIABLE_UNIT_TEST_SOURCE TimedConditionVariableUnitTest.cpp)

add_executable(TimedConditionVariableUnitTest ${TIMED_CONDITION_VARIABLE_UNIT_TEST_SOURCE})
target_link_libraries(TimedConditionVariableUnitTest fastdds GTest::gtest)
gtest_discover_tests(TimedConditionVariableUnitTest)
