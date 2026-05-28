# Copyright (C) 2025, Proyectos y Sistemas de Mantenimiento SL (eProsima)
#
# This program is commercial software licensed under the terms of the
# eProsima Software License Agreement Rev 03 (the "License")
#
# You may obtain a copy of the License at
# https://www.eprosima.com/licenses/LICENSE-REV03


add_executable(licenses_tests
    licenses_tests.cpp

    ${PROJECT_SOURCE_DIR}/test/blackbox/types/HelloWorldPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/blackbox/types/HelloWorldTypeObjectSupport.cxx
)

target_include_directories(licenses_tests
    PRIVATE
    ${PROJECT_SOURCE_DIR}/test/blackbox/types
)

target_link_libraries(licenses_tests
    PRIVATE
    fastcdr
    fastdds
    GTest::gtest
)
gtest_discover_tests(licenses_tests
    PROPERTIES
            ENVIRONMENT "LICENSES_PATH=${CMAKE_CURRENT_SOURCE_DIR}/tests"
)
