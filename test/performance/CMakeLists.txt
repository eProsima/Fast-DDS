if(NOT ((MSVC OR MSVC_IDE) AND EPROSIMA_INSTALLER) AND fastcdr_FOUND)
    # If Windows, link against export dll functions
    if(WIN32)
        add_definitions(-DFASTRTPS_DYN_LINK -DFASTCDR_DYN_LINK)
    endif()


    ###############################################################################
    # Unit tests
    ###############################################################################
    set(LATENCYTEST_SOURCE LatencyTestPublisher.cpp
        LatencyTestSubscriber.cpp
        LatencyTestTypes.cpp
        main_LatencyTest.cpp
        )
    add_executable(LatencyTest ${LATENCYTEST_SOURCE})
	target_include_directories(LatencyTest PRIVATE ${Boost_INCLUDE_DIR})
    target_link_libraries(LatencyTest fastrtps ${Boost_LIBRARIES})

    set(THROUGHPUTTEST_SOURCE ThroughputPublisher.cpp
        ThroughputSubscriber.cpp
        ThroughputTypes.cpp
        main_ThroughputTest.cpp
        )
    add_executable(ThroughputTest ${THROUGHPUTTEST_SOURCE})
	target_include_directories(ThroughputTest PRIVATE ${Boost_INCLUDE_DIR})
	target_link_libraries(ThroughputTest fastrtps ${Boost_LIBRARIES})

endif()

