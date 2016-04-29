if(NOT ((MSVC OR MSVC_IDE) AND EPROSIMA_INSTALLER))
    include(${PROJECT_SOURCE_DIR}/cmake/dev/gtest.cmake)
    check_gtest()

    if(GTEST_FOUND)
        find_package(Threads REQUIRED)

        set(SEQUENCENUMBERTESTS_SOURCE SequenceNumberTests.cpp)

        add_executable(SequenceNumberTests ${SEQUENCENUMBERTESTS_SOURCE})
        add_gtest(SequenceNumberTests ${SEQUENCENUMBERTESTS_SOURCE})
        target_compile_definitions(SequenceNumberTests PRIVATE FASTRTPS_NO_LIB)
        target_include_directories(SequenceNumberTests PRIVATE ${GTEST_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME})
        target_link_libraries(SequenceNumberTests ${GTEST_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
    endif()
endif()
