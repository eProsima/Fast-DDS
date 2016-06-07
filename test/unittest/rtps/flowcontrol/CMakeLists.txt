if(NOT ((MSVC OR MSVC_IDE) AND EPROSIMA_INSTALLER))
    include(${PROJECT_SOURCE_DIR}/cmake/dev/gtest.cmake)
    check_gtest()

    if(GTEST_FOUND)
        include_directories(mock/)

        set(THROUGHPUTCONTROLLERTESTS_SOURCE 
            ThroughputControllerTests.cpp 
            mock/mock.RTPSParticipantImpl.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/AsyncWriterThread.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/AsyncInterestTree.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/FlowController.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/ThroughputController.cpp)

        add_executable(ThroughputControllerTests ${THROUGHPUTCONTROLLERTESTS_SOURCE})
        add_gtest(ThroughputControllerTests ${THROUGHPUTCONTROLLERTESTS_SOURCE})
        target_compile_definitions(ThroughputControllerTests PRIVATE FASTRTPS_NO_LIB BOOST_ALL_DYN_LINK)
        target_include_directories(ThroughputControllerTests PRIVATE ${Boost_INCLUDE_DIR} ${GTEST_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME})
        target_link_libraries(ThroughputControllerTests ${Boost_LIBRARIES} ${GTEST_LIBRARIES} ${MOCKS})
        if(MSVC OR MSVC_IDE)
            target_link_libraries(ThroughputControllerTests ${PRIVACY}
                ${Boost_LIBRARIES} iphlpapi Shlwapi
                )
        endif()
    endif()
endif()
