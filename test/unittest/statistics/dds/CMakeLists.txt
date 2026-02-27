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

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
    add_definitions(-DNOMINMAX)
endif()

# Only include tinyxml2 headers manually if the associated config predates the use of CMake targets
if(TINYXML2_INCLUDE_DIR)
    include_directories(${TINYXML2_INCLUDE_DIR})
endif(TINYXML2_INCLUDE_DIR)

## StatisticsDomainParticipantTests
set(STATISTICS_DOMAINPARTICIPANT_TESTS_SOURCE
    StatisticsDomainParticipantTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/monitorservice_typesPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/monitorservice_typesTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/typesPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/typesTypeObjectSupport.cxx
    )

add_executable(StatisticsDomainParticipantTests ${STATISTICS_DOMAINPARTICIPANT_TESTS_SOURCE})
target_compile_definitions(StatisticsDomainParticipantTests PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNAL_DEBUG> # Internal debug activated.
    )
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
target_compile_definitions(StatisticsQosTests PRIVATE BOOST_ASIO_STANDALONE ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNAL_DEBUG> # Internal debug activated.
    )
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

    add_executable(DomainParticipantStatisticsListenerTests
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Time_t.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/ThreadSettings.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/domain/DomainParticipantStatisticsListener.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/monitorservice_typesPubSubTypes.cxx
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/monitorservice_typesTypeObjectSupport.cxx
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/typesPubSubTypes.cxx
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/typesTypeObjectSupport.cxx
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/LocatorWithMask.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/network.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/utils/Host.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
        ${TINYXML2_SOURCES}
        ${DOMAINPARTICIPANTSTATISTICSLISTENER_TESTS_SOURCE})
    target_compile_definitions(DomainParticipantStatisticsListenerTests PRIVATE
        BOOST_ASIO_STANDALONE
        ASIO_STANDALONE
        $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
        $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNAL_DEBUG> # Internal debug activated.
        )
    target_include_directories(DomainParticipantStatisticsListenerTests PRIVATE
        ${Asio_INCLUDE_DIR}
        DomainParticipantStatisticsListenerTests/mock
        ${PROJECT_SOURCE_DIR}/test/mock/dds/DomainParticipantFactory/
        ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSDomainImpl/
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_BINARY_DIR}/include
        ${PROJECT_SOURCE_DIR}/src/cpp
        ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl)
    target_link_libraries(DomainParticipantStatisticsListenerTests
        ${TINYXML2_LIBRARY}
        fastcdr
        fastdds::log
        fastdds::xtypes::type-representation
        fastdds::xtypes::dynamic-types::impl
        GTest::gmock
        )
    gtest_discover_tests(DomainParticipantStatisticsListenerTests)
endif()

## StatisticsDomainParticipantMockTests
if (SQLITE3_SUPPORT AND FASTDDS_STATISTICS AND NOT QNX)
    # this test includes C sources
    enable_language(C)

    get_property(fastdds_sources_files TARGET ${PROJECT_NAME} PROPERTY SOURCES)
    list(REMOVE_ITEM fastdds_sources_files "${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/DomainParticipantImpl.cpp")
    list(REMOVE_ITEM fastdds_sources_files "${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/Publisher.cpp")
    get_property(fastdds_compile_definitions TARGET ${PROJECT_NAME} PROPERTY COMPILE_DEFINITIONS)
    list(FILTER fastdds_compile_definitions EXCLUDE REGEX ".*FASTDDS_DYN_LINK.*")
    get_property(fastdds_include_directories TARGET ${PROJECT_NAME} PROPERTY INCLUDE_DIRECTORIES)
    get_property(fastdds_link_libraries TARGET ${PROJECT_NAME} PROPERTY LINK_LIBRARIES)

    set(STATISTICS_DOMAINPARTICIPANT_MOCK_TESTS_SOURCE
        StatisticsDomainParticipantMockTests.cpp
        )

    add_executable(StatisticsDomainParticipantMockTests
        ${STATISTICS_DOMAINPARTICIPANT_MOCK_TESTS_SOURCE}
        )

    target_compile_definitions(StatisticsDomainParticipantMockTests PRIVATE
        ${fastdds_compile_definitions}
        )

    target_sources(StatisticsDomainParticipantMockTests
        PRIVATE
        ${fastdds_sources_files}
    )

    target_include_directories(StatisticsDomainParticipantMockTests PRIVATE
        StatisticsDomainParticipantMockTests/mock
        ${PROJECT_SOURCE_DIR}/test/mock/dds/Publisher/
        ${PROJECT_SOURCE_DIR}/test/mock/dds/DomainParticipantImpl/
        ${fastdds_include_directories}
        )

    target_link_libraries(StatisticsDomainParticipantMockTests PRIVATE
        GTest::gmock
        ${fastdds_link_libraries}
        )

    gtest_discover_tests(StatisticsDomainParticipantMockTests)

    list(REMOVE_ITEM fastdds_sources_files "${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/RTPSParticipantAttributes.cpp")
    list(REMOVE_ITEM fastdds_sources_files "${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/DataWriterImpl.cpp")

    set(STATISTICS_DOMAINPARTICIPANT_STATUS_QUERYABLE_TESTS_SOURCE
        StatisticsDomainParticipantStatusQueryableTests.cpp
        )

    add_executable(StatisticsDomainParticipantStatusQueryableTests ${STATISTICS_DOMAINPARTICIPANT_STATUS_QUERYABLE_TESTS_SOURCE})

    target_compile_definitions(StatisticsDomainParticipantStatusQueryableTests PRIVATE
        ${fastdds_compile_definitions}
        )

    target_sources(StatisticsDomainParticipantStatusQueryableTests
        PRIVATE
        ${fastdds_sources_files}
    )

    target_include_directories(StatisticsDomainParticipantStatusQueryableTests PRIVATE
        StatisticsDomainParticipantStatusQueryableTests/mock/
        ${PROJECT_SOURCE_DIR}/test/mock/dds/Publisher/
        ${PROJECT_SOURCE_DIR}/test/mock/dds/DomainParticipantImpl/
        ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSParticipantAttributes
        ${fastdds_include_directories}
        )

    target_link_libraries(StatisticsDomainParticipantStatusQueryableTests PRIVATE
        GTest::gmock
        ${fastdds_link_libraries}
        )

    gtest_discover_tests(StatisticsDomainParticipantStatusQueryableTests)

endif (SQLITE3_SUPPORT AND FASTDDS_STATISTICS AND NOT QNX)
