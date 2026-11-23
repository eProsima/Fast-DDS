# Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Only include tinyxml2 headers manually if the associated config predates the use of CMake targets
if(TINYXML2_INCLUDE_DIR)
    include_directories(${TINYXML2_INCLUDE_DIR})
endif(TINYXML2_INCLUDE_DIR)

# These tests narrow a base DomainParticipant to statistics::dds::DomainParticipant and call members
# on it. That is intentional UB (the object is never constructed as the derived type, to avoid
# emitting its vtable and breaking ABI), so disable the vptr (bad-downcast) check for just these
# translation units
if(SANITIZER STREQUAL "UNDEFINED")
    set_source_files_properties(
        StatisticsDomainParticipantTests.cpp
        StatisticsQosTests.cpp
        StatisticsDomainParticipantStatusQueryableTests.cpp
        StatisticsDomainParticipantMockTests.cpp
        PROPERTIES COMPILE_OPTIONS "-fno-sanitize=vptr")
endif()

## StatisticsDomainParticipantTests
set(STATISTICS_DOMAINPARTICIPANT_TESTS_SOURCE
    StatisticsDomainParticipantTests.cpp
    )

add_executable(StatisticsDomainParticipantTests ${STATISTICS_DOMAINPARTICIPANT_TESTS_SOURCE})
target_include_directories(StatisticsDomainParticipantTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include ${PROJECT_SOURCE_DIR}/src/cpp)
target_link_libraries(StatisticsDomainParticipantTests
    fastcdr
    fastdds
    GTest::gtest
    )
gtest_discover_tests(StatisticsDomainParticipantTests)

## StatisticsQosTests
set(STATISTICS_QOS_TESTS_SOURCE
    StatisticsQosTests.cpp)

# External sources
if(TINYXML2_SOURCE_DIR)
    set(TINYXML2_SOURCES
        ${TINYXML2_SOURCE_DIR}/tinyxml2.cpp
        )
    list(APPEND STATISTICS_QOS_TESTS_SOURCE
        ${TINYXML2_SOURCES}
        )
endif()

add_executable(StatisticsQosTests ${STATISTICS_QOS_TESTS_SOURCE})
target_include_directories(StatisticsQosTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include ${PROJECT_SOURCE_DIR}/src/cpp ${THIRDPARTY_BOOST_INCLUDE_DIR})
target_link_libraries(StatisticsQosTests
    fastcdr
    fastdds
    foonathan_memory
    ${TINYXML2_LIBRARY}
    GTest::gtest)
gtest_discover_tests(StatisticsQosTests)

## StatisticsDomainParticipantListenerTests
if (FASTDDS_STATISTICS)
    set(DOMAINPARTICIPANTSTATISTICSLISTENER_TESTS_SOURCE
        DomainParticipantStatisticsListenerTests.cpp)

    add_executable(DomainParticipantStatisticsListenerTests ${DOMAINPARTICIPANTSTATISTICSLISTENER_TESTS_SOURCE})
    target_link_units(DomainParticipantStatisticsListenerTests UNITS statistics::types dds::condition dds::participant-qos
        rtps::common xtypes xmlparser::common statistics::participant-listener MOCKS DataWriter
        DomainParticipantFactory RTPSDomainImpl TypeSupport xtypes::TypeObjectUtils XMLProfileManager)
    target_include_directories(DomainParticipantStatisticsListenerTests PRIVATE
        ${Asio_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_BINARY_DIR}/include
        ${PROJECT_SOURCE_DIR}/src/cpp
        ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl)
    target_link_libraries(DomainParticipantStatisticsListenerTests
        ${TINYXML2_LIBRARY}
        fastcdr
        GTest::gmock
        )
    gtest_discover_tests(DomainParticipantStatisticsListenerTests)
endif()

## StatisticsDomainParticipantMockTests
if (SQLITE3_SUPPORT AND FASTDDS_STATISTICS AND NOT QNX)
    # this test includes C sources
    enable_language(C)

    get_property(fastdds_include_directories TARGET ${PROJECT_NAME} PROPERTY INCLUDE_DIRECTORIES)

    set(STATISTICS_DOMAINPARTICIPANT_MOCK_TESTS_SOURCE
        StatisticsDomainParticipantMockTests.cpp
        )

    add_executable(StatisticsDomainParticipantMockTests
        ${STATISTICS_DOMAINPARTICIPANT_MOCK_TESTS_SOURCE}
        )

    target_link_units(StatisticsDomainParticipantMockTests UNITS core MOCKS dds::entities::Publisher)

    target_include_directories(StatisticsDomainParticipantMockTests PRIVATE
        ${fastdds_include_directories}
        )

    target_link_libraries(StatisticsDomainParticipantMockTests
        GTest::gmock
        )

    gtest_discover_tests(StatisticsDomainParticipantMockTests)

    set(STATISTICS_DOMAINPARTICIPANT_STATUS_QUERYABLE_TESTS_SOURCE
        StatisticsDomainParticipantStatusQueryableTests.cpp
        )

    add_executable(StatisticsDomainParticipantStatusQueryableTests ${STATISTICS_DOMAINPARTICIPANT_STATUS_QUERYABLE_TESTS_SOURCE})

    target_link_units(StatisticsDomainParticipantStatusQueryableTests UNITS core)

    target_include_directories(StatisticsDomainParticipantStatusQueryableTests PRIVATE
        ${fastdds_include_directories}
        )

    target_link_libraries(StatisticsDomainParticipantStatusQueryableTests
        GTest::gmock
        )

    gtest_discover_tests(StatisticsDomainParticipantStatusQueryableTests)

endif (SQLITE3_SUPPORT AND FASTDDS_STATISTICS AND NOT QNX)
