# Copyright (C) 2025, Proyectos y Sistemas de Mantenimiento SL (eProsima)
#
# This program is commercial software licensed under the terms of the
# eProsima Software License Agreement Rev 03 (the "License")
#
# You may obtain a copy of the License at
# https://www.eprosima.com/licenses/LICENSE-REV03

if(WIN32)
    add_definitions(
        -D_WIN32_WINNT=0x0601
        -D_CRT_SECURE_NO_WARNINGS
        )
endif()

add_executable(ip_mobility_tests
    ip_mobility_tests.cpp

    ${PROJECT_SOURCE_DIR}/test/blackbox/types/HelloWorldPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/blackbox/types/HelloWorldTypeObjectSupport.cxx

    ${PROJECT_SOURCE_DIR}/test/blackbox/utils/data_generators.cpp
    ${PROJECT_SOURCE_DIR}/test/blackbox/utils/lambda_functions.cpp
    ${PROJECT_SOURCE_DIR}/test/blackbox/utils/print_functions.cpp
)

target_include_directories(ip_mobility_tests
    PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/test/blackbox/api/dds-pim
    ${PROJECT_SOURCE_DIR}/test/blackbox/common
    ${PROJECT_SOURCE_DIR}/test/blackbox/types
)

target_link_libraries(ip_mobility_tests
    PRIVATE
    fastcdr
    fastdds
    GTest::gtest
)
gtest_discover_tests(ip_mobility_tests)
