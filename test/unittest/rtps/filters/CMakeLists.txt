if(NOT ((MSVC OR MSVC_IDE) AND EPROSIMA_INSTALLER))
    include(${PROJECT_SOURCE_DIR}/cmake/dev/gtest.cmake)
    check_gtest()

    if(GTEST_FOUND)
        include_directories(mock/)

        set(SIZEFILTERTESTS_SOURCE 
            SizeFilterTests.cpp 
            mock/mock.RTPSParticipantImpl.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/AsyncWriterThread.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/AsyncInterestTree.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/filters/FlowFilter.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/filters/SizeFilter.cpp)

        add_executable(SizeFilterTests ${SIZEFILTERTESTS_SOURCE})
        add_gtest(SizeFilterTests ${SIZEFILTERTESTS_SOURCE})
        target_compile_definitions(SizeFilterTests PRIVATE FASTRTPS_NO_LIB BOOST_ALL_DYN_LINK)
        target_include_directories(SizeFilterTests PRIVATE ${Boost_INCLUDE_DIR} ${GTEST_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME})
        target_link_libraries(SizeFilterTests ${Boost_LIBRARIES} ${GTEST_LIBRARIES} ${MOCKS})
        if(MSVC OR MSVC_IDE)
            target_link_libraries(SizeFilterTests ${PRIVACY}
                ${Boost_LIBRARIES} iphlpapi Shlwapi
                )
        endif()
    endif()
endif()
