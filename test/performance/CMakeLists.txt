if(NOT ((MSVC OR MSVC_IDE) AND EPROSIMA_INSTALLER) AND fastcdr_FOUND)
    find_package(Threads REQUIRED)
    find_package(PythonInterp 3 REQUIRED)
    find_package(Boost COMPONENTS program_options)

    if(PYTHONINTERP_FOUND)
        ###############################################################################
        # Binaries
        ###############################################################################
        if(Boost_PROGRAM_OPTIONS_FOUND)
            set(LATENCYTEST_SOURCE LatencyTestPublisher.cpp
                LatencyTestSubscriber.cpp
                LatencyTestTypes.cpp
                main_LatencyTest.cpp
                )
            add_executable(LatencyTest ${LATENCYTEST_SOURCE})
            target_include_directories(LatencyTest PRIVATE ${Boost_INCLUDE_DIR})
            target_link_libraries(LatencyTest fastrtps ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
        endif()

        set(THROUGHPUTTEST_SOURCE ThroughputPublisher.cpp
            ThroughputSubscriber.cpp
            ThroughputTypes.cpp
            main_ThroughputTest.cpp
            )
        add_executable(ThroughputTest ${THROUGHPUTTEST_SOURCE})
        target_include_directories(ThroughputTest PRIVATE ${Boost_INCLUDE_DIR})
        target_link_libraries(ThroughputTest fastrtps ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

        ###############################################################################
        # LatencyTest
        ###############################################################################	
        if(Boost_PROGRAM_OPTIONS_FOUND)
            add_test(NAME LatencyTest
                COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/latency_tests.py)

            # Set test with label NoMemoryCheck
            set_property(TEST LatencyTest PROPERTY LABELS "NoMemoryCheck")

            if(WIN32)
                set_property(TEST LatencyTest PROPERTY ENVIRONMENT
                    "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;${BOOST_LIBRARYDIR}\\;$ENV{PATH}")
            endif()
            set_property(TEST LatencyTest APPEND PROPERTY ENVIRONMENT
                "LATENCY_TEST_BIN=$<TARGET_FILE:LatencyTest>")
        endif()

        ###############################################################################
        # ThroughputTest
        ###############################################################################
        add_test(NAME ThroughputTest
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/throughput_tests.py)

        # Set test with label NoMemoryCheck
        set_property(TEST ThroughputTest PROPERTY LABELS "NoMemoryCheck")

        if(WIN32)
            set_property(TEST ThroughputTest PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;${BOOST_LIBRARYDIR}\\;$ENV{PATH}")
        endif()
        set_property(TEST ThroughputTest APPEND PROPERTY ENVIRONMENT
            "THROUGHPUT_TEST_BIN=$<TARGET_FILE:ThroughputTest>")
        set_property(TEST ThroughputTest APPEND PROPERTY ENVIRONMENT
            "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")

    endif()
endif()

