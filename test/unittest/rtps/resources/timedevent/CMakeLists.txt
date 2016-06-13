if(NOT ((MSVC OR MSVC_IDE) AND EPROSIMA_INSTALLER))
    include(${PROJECT_SOURCE_DIR}/cmake/dev/gtest.cmake)
    check_gtest()

    if(GTEST_FOUND)
        find_package(Threads REQUIRED)

        set(TIMEDEVENTTESTS_SOURCE mock/MockEvent.cpp
            mock/MockParentEvent.cpp
            TimedEventTests.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEventImpl.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEvent.cpp
            )

        if(WIN32)
            add_definitions(-D_WIN32_WINNT=0x0601)
        endif()

        add_executable(TimedEventTests ${TIMEDEVENTTESTS_SOURCE})
        add_gtest(TimedEventTests ${TIMEDEVENTTESTS_SOURCE})
        target_compile_definitions(TimedEventTests PRIVATE BOOST_ALL_DYN_LINK FASTRTPS_NO_LIB)
        target_include_directories(TimedEventTests PRIVATE ${Boost_INCLUDE_DIR} ${GTEST_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME})
        target_link_libraries(TimedEventTests ${Boost_LIBRARIES} ${GTEST_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
    endif()
endif()
