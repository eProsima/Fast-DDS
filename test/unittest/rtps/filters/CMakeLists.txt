if(NOT ((MSVC OR MSVC_IDE) AND EPROSIMA_INSTALLER))
    include(${PROJECT_SOURCE_DIR}/cmake/dev/gtest.cmake)
    check_gtest()

    if(GTEST_FOUND)
        find_package(Threads REQUIRED)

        set(THROTTLEFILTERTESTS_SOURCE 
            ThrottleFilterTests.cpp 
            mock/mock.RTPSMessageGroup.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/filters/FlowFilter.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/filters/ThrottleFilter.cpp)

        include_directories(mock/)

        add_executable(ThrottleFilterTests ${THROTTLEFILTERTESTS_SOURCE})
        add_gtest(ThrottleFilterTests ${THROTTLEFILTERTESTS_SOURCE})
        target_compile_definitions(ThrottleFilterTests PRIVATE FASTRTPS_NO_LIB)
        target_include_directories(ThrottleFilterTests PRIVATE ${GTEST_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME})
        target_link_libraries(ThrottleFilterTests ${GTEST_LIBRARIES} ${MOCKS} ${CMAKE_THREAD_LIBS_INIT})
    endif()
endif()
