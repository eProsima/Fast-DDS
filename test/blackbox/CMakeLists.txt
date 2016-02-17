if(NOT ((MSVC OR MSVC_IDE) AND EPROSIMA_INSTALLER) AND fastcdr_FOUND)
    include(${PROJECT_SOURCE_DIR}/cmake/dev/gtest.cmake)
    check_gtest()
	
    if(GTEST_FOUND)
        # If Windows, link against export dll functions
        if(WIN32)
            add_definitions(-DFASTRTPS_DYN_LINK -DFASTCDR_DYN_LINK)
        endif()


        ###############################################################################
        # Unit tests
        ###############################################################################
        set(BLACKBOXTESTS_SOURCE BlackboxTests.cpp
            types/HelloWorld.cpp
            types/HelloWorldType.cpp
            RTPSAsSocketReader.cpp
            RTPSAsSocketWriter.cpp
            RTPSWithRegistrationReader.cpp
            RTPSWithRegistrationWriter.cpp
            PubSubHelloWorldReader.cpp
            PubSubHelloWorldWriter.cpp
            ReqRepHelloWorldRequester.cpp
            ReqRepHelloWorldReplier.cpp
            )
        add_executable(BlackboxTests ${BLACKBOXTESTS_SOURCE})
        add_gtest(BlackboxTests ${BLACKBOXTESTS_SOURCE})
        target_include_directories(BlackboxTests PRIVATE ${GTEST_INCLUDE_DIRS})
        target_link_libraries(BlackboxTests fastrtps fastcdr ${GTEST_LIBRARIES})
    endif()
endif()

