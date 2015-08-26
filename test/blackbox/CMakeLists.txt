if(NOT ((MSVC OR MSVC_IDE) AND EPROSIMA_INSTALLER))
    include(${PROJECT_SOURCE_DIR}/cmake/dev/gtest.cmake)

    # If Windows, link against export dll functions
    if(WIN32)
        add_definitions(-DFASTRTPS_DYN_LINK)
    endif()


    ###############################################################################
    # Unit tests
    ###############################################################################
    set(BLACKBOXTESTS_SOURCE BlackboxTests.cpp
        RTPSAsSocketReader.cpp
        RTPSAsSocketWriter.cpp
        RTPSWithRegistrationReader.cpp
        RTPSWithRegistrationWriter.cpp
        )
    add_executable(BlackboxTests ${BLACKBOXTESTS_SOURCE})
    add_gtest(BlackboxTests ${BLACKBOXTESTS_SOURCE})
    target_include_directories(BlackboxTests PRIVATE ${GTEST_INCLUDE_DIRS})
    target_link_libraries(BlackboxTests fastrtps ${GTEST_LIBRARIES})
endif()

