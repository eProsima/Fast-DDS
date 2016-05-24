if(NOT ((MSVC OR MSVC_IDE) AND EPROSIMA_INSTALLER))
    include(${PROJECT_SOURCE_DIR}/cmake/dev/gtest.cmake)
    check_gtest()

    if(GTEST_FOUND)
        find_package(Threads REQUIRED)

        include_directories(mock/)

        set(THROTTLEFILTERTESTS_SOURCE 
            ThrottleFilterTests.cpp 
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/filters/FlowFilter.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/filters/ThrottleFilter.cpp)

        add_executable(ThrottleFilterTests ${THROTTLEFILTERTESTS_SOURCE})
        add_gtest(ThrottleFilterTests ${THROTTLEFILTERTESTS_SOURCE})
        target_compile_definitions(ThrottleFilterTests PRIVATE FASTRTPS_NO_LIB)
        target_include_directories(ThrottleFilterTests PRIVATE ${GTEST_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME})
        target_link_libraries(ThrottleFilterTests ${Boost_LIBRARIES} ${GTEST_LIBRARIES} ${MOCKS} ${CMAKE_THREAD_LIBS_INIT})

        set(QUANTITYFILTERTESTS_SOURCE 
            QuantityFilterTests.cpp 
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/filters/FlowFilter.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/filters/QuantityFilter.cpp)

        add_executable(QuantityFilterTests ${QUANTITYFILTERTESTS_SOURCE})
        add_gtest(QuantityFilterTests ${QUANTITYFILTERTESTS_SOURCE})
        target_compile_definitions(QuantityFilterTests PRIVATE FASTRTPS_NO_LIB)
        target_include_directories(QuantityFilterTests PRIVATE ${GTEST_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME})
        target_link_libraries(QuantityFilterTests ${Boost_LIBRARIES} ${GTEST_LIBRARIES} ${MOCKS} ${CMAKE_THREAD_LIBS_INIT})

        set(SIZEFILTERTESTS_SOURCE 
            SizeFilterTests.cpp 
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/filters/FlowFilter.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/filters/SizeFilter.cpp)

        add_executable(SizeFilterTests ${SIZEFILTERTESTS_SOURCE})
        add_gtest(SizeFilterTests ${SIZEFILTERTESTS_SOURCE})
        target_compile_definitions(SizeFilterTests PRIVATE FASTRTPS_NO_LIB)
        target_include_directories(SizeFilterTests PRIVATE ${GTEST_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME})
        target_link_libraries(SizeFilterTests ${Boost_LIBRARIES} ${GTEST_LIBRARIES} ${MOCKS} ${CMAKE_THREAD_LIBS_INIT})
    endif()
endif()
