add_subdirectory(mutex_testing_tool)


if(GTEST_FOUND)
    set(USER_THREAD_NONBLOCKED_TEST UserThreadNonBlockedTest.cpp)

    add_executable(user_thread_nonblocked_test ${USER_THREAD_NONBLOCKED_TEST})
    target_include_directories(user_thread_nonblocked_test PRIVATE ${GTEST_INCLUDE_DIRS})
    target_link_libraries(user_thread_nonblocked_test mutex_testing_tool fastrtps fastcdr ${GTEST_LIBRARIES})

    STRING(REPLACE " " "\\ " MUTEX_PRELOAD_LIBRARY_FILE "$<TARGET_FILE:mutex_testing_tool_preload>")
    add_gtest(NAME UserThreadNonBlockedTest COMMAND user_thread_nonblocked_test SOURCES ${USER_THREAD_NONBLOCKED_TEST}
        ENVIRONMENTS
        "LD_LIBRARY_PATH=$<TARGET_FILE_DIR:mutex_testing_tool_preload>"
        "LD_PRELOAD=$<TARGET_FILE_NAME:mutex_testing_tool_preload>"
        LABELS "NoMemoryCheck"
        )
endif()
