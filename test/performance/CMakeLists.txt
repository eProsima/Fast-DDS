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
        # ThroughputTest16
        ###############################################################################
        add_test(NAME ThroughputTest16
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/throughput_tests_16.py)

        # Set test with label NoMemoryCheck
        set_property(TEST ThroughputTest16 PROPERTY LABELS "NoMemoryCheck")

        if(WIN32)
            set_property(TEST ThroughputTest16 PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;${BOOST_LIBRARYDIR}\\;$ENV{PATH}")
        endif()
        set_property(TEST ThroughputTest16 APPEND PROPERTY ENVIRONMENT
            "THROUGHPUT_TEST_BIN=$<TARGET_FILE:ThroughputTest>")
        set_property(TEST ThroughputTest16 APPEND PROPERTY ENVIRONMENT
            "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
			
        ###############################################################################
        # ThroughputTest32
        ###############################################################################
        add_test(NAME ThroughputTest32
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/throughput_tests_32.py)

        # Set test with label NoMemoryCheck
        set_property(TEST ThroughputTest32 PROPERTY LABELS "NoMemoryCheck")

        if(WIN32)
            set_property(TEST ThroughputTest32 PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;${BOOST_LIBRARYDIR}\\;$ENV{PATH}")
        endif()
        set_property(TEST ThroughputTest32 APPEND PROPERTY ENVIRONMENT
            "THROUGHPUT_TEST_BIN=$<TARGET_FILE:ThroughputTest>")
        set_property(TEST ThroughputTest32 APPEND PROPERTY ENVIRONMENT
            "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
			
		###############################################################################
        # ThroughputTest64
        ###############################################################################
        add_test(NAME ThroughputTest64
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/throughput_tests_64.py)

        # Set test with label NoMemoryCheck
        set_property(TEST ThroughputTest64 PROPERTY LABELS "NoMemoryCheck")

        if(WIN32)
            set_property(TEST ThroughputTest64 PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;${BOOST_LIBRARYDIR}\\;$ENV{PATH}")
        endif()
        set_property(TEST ThroughputTest64 APPEND PROPERTY ENVIRONMENT
            "THROUGHPUT_TEST_BIN=$<TARGET_FILE:ThroughputTest>")
        set_property(TEST ThroughputTest64 APPEND PROPERTY ENVIRONMENT
            "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
			
		###############################################################################
        # ThroughputTest128
        ###############################################################################
        add_test(NAME ThroughputTest128
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/throughput_tests_128.py)

        # Set test with label NoMemoryCheck
        set_property(TEST ThroughputTest128 PROPERTY LABELS "NoMemoryCheck")

        if(WIN32)
            set_property(TEST ThroughputTest128 PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;${BOOST_LIBRARYDIR}\\;$ENV{PATH}")
        endif()
        set_property(TEST ThroughputTest128 APPEND PROPERTY ENVIRONMENT
            "THROUGHPUT_TEST_BIN=$<TARGET_FILE:ThroughputTest>")
        set_property(TEST ThroughputTest128 APPEND PROPERTY ENVIRONMENT
            "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
			
        ###############################################################################
        # ThroughputTest256
        ###############################################################################
        add_test(NAME ThroughputTest256
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/throughput_tests_256.py)

        # Set test with label NoMemoryCheck
        set_property(TEST ThroughputTest256 PROPERTY LABELS "NoMemoryCheck")

        if(WIN32)
            set_property(TEST ThroughputTest256 PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;${BOOST_LIBRARYDIR}\\;$ENV{PATH}")
        endif()
        set_property(TEST ThroughputTest256 APPEND PROPERTY ENVIRONMENT
            "THROUGHPUT_TEST_BIN=$<TARGET_FILE:ThroughputTest>")
        set_property(TEST ThroughputTest256 APPEND PROPERTY ENVIRONMENT
            "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
			
        ###############################################################################
        # ThroughputTest512
        ###############################################################################
        add_test(NAME ThroughputTest512
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/throughput_tests_512.py)

        # Set test with label NoMemoryCheck
        set_property(TEST ThroughputTest512 PROPERTY LABELS "NoMemoryCheck")

        if(WIN32)
            set_property(TEST ThroughputTest512 PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;${BOOST_LIBRARYDIR}\\;$ENV{PATH}")
        endif()
        set_property(TEST ThroughputTest512 APPEND PROPERTY ENVIRONMENT
            "THROUGHPUT_TEST_BIN=$<TARGET_FILE:ThroughputTest>")
        set_property(TEST ThroughputTest512 APPEND PROPERTY ENVIRONMENT
            "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
			
		###############################################################################
        # ThroughputTest1024
        ###############################################################################
        add_test(NAME ThroughputTest1024
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/throughput_tests_1024.py)

        # Set test with label NoMemoryCheck
        set_property(TEST ThroughputTest1024 PROPERTY LABELS "NoMemoryCheck")

        if(WIN32)
            set_property(TEST ThroughputTest1024 PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;${BOOST_LIBRARYDIR}\\;$ENV{PATH}")
        endif()
        set_property(TEST ThroughputTest1024 APPEND PROPERTY ENVIRONMENT
            "THROUGHPUT_TEST_BIN=$<TARGET_FILE:ThroughputTest>")
        set_property(TEST ThroughputTest1024 APPEND PROPERTY ENVIRONMENT
            "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
			
		###############################################################################
        # ThroughputTest2048
        ###############################################################################
        add_test(NAME ThroughputTest2048
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/throughput_tests_2048.py)

        # Set test with label NoMemoryCheck
        set_property(TEST ThroughputTest2048 PROPERTY LABELS "NoMemoryCheck")

        if(WIN32)
            set_property(TEST ThroughputTest2048 PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;${BOOST_LIBRARYDIR}\\;$ENV{PATH}")
        endif()
        set_property(TEST ThroughputTest2048 APPEND PROPERTY ENVIRONMENT
            "THROUGHPUT_TEST_BIN=$<TARGET_FILE:ThroughputTest>")
        set_property(TEST ThroughputTest2048 APPEND PROPERTY ENVIRONMENT
            "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
			
		###############################################################################
        # ThroughputTest4096
        ###############################################################################
        add_test(NAME ThroughputTest4096
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/throughput_tests_4096.py)

        # Set test with label NoMemoryCheck
        set_property(TEST ThroughputTest4096 PROPERTY LABELS "NoMemoryCheck")

        if(WIN32)
            set_property(TEST ThroughputTest4096 PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;${BOOST_LIBRARYDIR}\\;$ENV{PATH}")
        endif()
        set_property(TEST ThroughputTest4096 APPEND PROPERTY ENVIRONMENT
            "THROUGHPUT_TEST_BIN=$<TARGET_FILE:ThroughputTest>")
        set_property(TEST ThroughputTest4096 APPEND PROPERTY ENVIRONMENT
            "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
			
		###############################################################################
        # ThroughputTest8192
        ###############################################################################
        add_test(NAME ThroughputTest8192
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/throughput_tests_8192.py)

        # Set test with label NoMemoryCheck
        set_property(TEST ThroughputTest8192 PROPERTY LABELS "NoMemoryCheck")

        if(WIN32)
            set_property(TEST ThroughputTest8192 PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;${BOOST_LIBRARYDIR}\\;$ENV{PATH}")
        endif()
        set_property(TEST ThroughputTest8192 APPEND PROPERTY ENVIRONMENT
            "THROUGHPUT_TEST_BIN=$<TARGET_FILE:ThroughputTest>")
        set_property(TEST ThroughputTest8192 APPEND PROPERTY ENVIRONMENT
            "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")

    endif()
endif()

