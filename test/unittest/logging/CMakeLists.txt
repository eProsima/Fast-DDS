# Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

add_subdirectory(log_macros)

set(LOG_COMMON_SOURCE
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetmaskFilterKind.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterfaceWithFilter.cpp
    )

# LOG TEST
set(LOGTESTS_TEST_SOURCE LogTests.cpp)

set(LOGTESTS_SOURCE
    ${LOG_COMMON_SOURCE}
    ${LOGTESTS_TEST_SOURCE})

include_directories(mock/)

add_executable(LogTests ${LOGTESTS_SOURCE})
target_compile_definitions(LogTests PRIVATE
    HAVE_LOG_NO_INFO=0 FASTDDS_ENFORCE_LOG_INFO # logInfo is used in some tests
    )

target_include_directories(LogTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp)
target_link_libraries(LogTests
    fastcdr
    GTest::gtest
    ${MOCKS}
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    )
fastdds_target_link_object_groups(LogTests
    fastdds::log
    fastdds::rtps::host-net
    )

gtest_discover_tests(LogTests)

# LOG FILE TEST
set(LOGFILETESTS_TEST_SOURCE LogFileTests.cpp)

set(LOGFILETESTS_SOURCE
    ${LOG_COMMON_SOURCE}
    ${LOGFILETESTS_TEST_SOURCE}
    )

# External sources
if(TINYXML2_SOURCE_DIR)
    list(APPEND LOGFILETESTS_SOURCE
        ${TINYXML2_SOURCE_DIR}/tinyxml2.cpp
        )
endif()

# Only include tinyxml2 headers manually if the associated config predates the use of CMake targets
if(TINYXML2_INCLUDE_DIR)
    include_directories(${TINYXML2_INCLUDE_DIR})
endif(TINYXML2_INCLUDE_DIR)

add_executable(LogFileTests ${LOGFILETESTS_SOURCE})
target_include_directories(LogFileTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp)
target_link_libraries(LogFileTests GTest::gtest ${MOCKS}
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    ${TINYXML2_LIBRARY}
    fastcdr
    )
fastdds_target_link_object_groups(LogFileTests
    fastdds::log
    fastdds::rtps::host-net
    )

gtest_discover_tests(LogFileTests)
