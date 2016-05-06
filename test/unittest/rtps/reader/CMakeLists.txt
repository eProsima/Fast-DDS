if(NOT ((MSVC OR MSVC_IDE) AND EPROSIMA_INSTALLER))
    include(${PROJECT_SOURCE_DIR}/cmake/dev/gtest.cmake)
    check_gtest()
    check_gmock()

    if(GTEST_FOUND)
        find_package(Threads REQUIRED)

        set(WRITERPROXYTESTS_SOURCE WriterProxyTests.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/reader/WriterProxy.cpp
            )

        if(WIN32)
            add_definitions(-D_WIN32_WINNT=0x0501)
        endif()

        add_executable(WriterProxyTests ${WRITERPROXYTESTS_SOURCE})
        add_gtest(WriterProxyTests ${WRITERPROXYTESTS_SOURCE})
        target_compile_definitions(WriterProxyTests PRIVATE BOOST_ALL_DYN_LINK FASTRTPS_NO_LIB)
        target_include_directories(WriterProxyTests PRIVATE
            ${Boost_INCLUDE_DIR} ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/StatefulReader
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/HeartbeatResponseDelay
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/WriterProxyLiveliness
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME})
        target_link_libraries(WriterProxyTests ${Boost_LIBRARIES}
            ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES}
            ${CMAKE_THREAD_LIBS_INIT})
    endif()
endif()
