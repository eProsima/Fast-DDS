# Copyright (C) 2025, Proyectos y Sistemas de Mantenimiento SL (eProsima)
#
# This program is commercial software licensed under the terms of the
# eProsima Software License Agreement Rev 03 (the "License")
#
# You may obtain a copy of the License at
# https://www.eprosima.com/licenses/LICENSE-REV03

configure_file(payload_compression.xml ${CMAKE_CURRENT_BINARY_DIR}/payload_compression.xml COPYONLY)
configure_file(header_reduction.xml ${CMAKE_CURRENT_BINARY_DIR}/header_reduction.xml COPYONLY)
configure_file(source_timestamp.xml ${CMAKE_CURRENT_BINARY_DIR}/source_timestamp.xml COPYONLY)
configure_file(all_low_transports.xml ${CMAKE_CURRENT_BINARY_DIR}/all_low_transports.xml COPYONLY)

add_executable(low_bandwidth_transport_tests
    low_bandwidth_transport_tests.cpp

    ${PROJECT_SOURCE_DIR}/test/blackbox/types/HelloWorldPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/blackbox/types/HelloWorldTypeObjectSupport.cxx
)

target_include_directories(low_bandwidth_transport_tests
    PRIVATE
    ${PROJECT_SOURCE_DIR}/test/blackbox/types
)

target_link_libraries(low_bandwidth_transport_tests
    PRIVATE
    fastcdr
    fastdds
    GTest::gtest
)
gtest_discover_tests(low_bandwidth_transport_tests)
