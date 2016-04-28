if(NOT ((MSVC OR MSVC_IDE) AND EPROSIMA_INSTALLER))
    include(${PROJECT_SOURCE_DIR}/cmake/dev/gtest.cmake)
    check_gtest()

    if(GTEST_FOUND)
        set(UDPV4TESTS_SOURCE 
            UDPv4Tests.cpp 
            ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/log/Log.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/transport/UDPv4Transport.cpp)

        set(UDPV6TESTS_SOURCE 
            UDPv6Tests.cpp 
            ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/log/Log.cpp)
            #${PROJECT_SOURCE_DIR}/src/cpp/transport/UDPv6Transport.cpp)

        include_directories(mock/)

        add_executable(UDPv4Tests ${UDPV4TESTS_SOURCE})
        add_gtest(UDPv4Tests ${UDPV4TESTS_SOURCE})
        target_compile_definitions(UDPv4Tests PRIVATE FASTRTPS_NO_LIB)
        target_include_directories(UDPv4Tests PRIVATE ${GTEST_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME})
        target_link_libraries(UDPv4Tests ${GTEST_LIBRARIES} ${Boost_LIBRARIES} ${MOCKS} -lpthread)

        add_executable(UDPv6Tests ${UDPV6TESTS_SOURCE})
        add_gtest(UDPv6Tests ${UDPV6TESTS_SOURCE})
        target_compile_definitions(UDPv6Tests PRIVATE FASTRTPS_NO_LIB)
        target_include_directories(UDPv6Tests PRIVATE ${GTEST_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME})
        target_link_libraries(UDPv6Tests ${GTEST_LIBRARIES} ${Boost_LIBRARIES} ${MOCKS} -lpthread)
    endif()
endif()
