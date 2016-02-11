if(NOT ((MSVC OR MSVC_IDE) AND EPROSIMA_INSTALLER))
    find_package(Threads REQUIRED)

    include(${PROJECT_SOURCE_DIR}/cmake/dev/gtest.cmake)

    set(TIMEDEVENTTESTS_SOURCE MockEvent.cpp
        TimedEventTests.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEventImpl.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEvent.cpp
        )

    add_executable(TimedEventTests ${TIMEDEVENTTESTS_SOURCE})
    add_gtest(TimedEventTests ${BLACKBOXTESTS_SOURCE})
    target_compile_definitions(TimedEventTests PRIVATE BOOST_ALL_DYN_LINK)
    target_include_directories(TimedEventTests PRIVATE ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME})
    target_link_libraries(TimedEventTests ${Boost_LIBRARIES} ${GTEST_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
endif()
