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

set(CONDITION_TESTS_SOURCE
    ConditionTests.cpp)

add_executable(ConditionTests ${CONDITION_TESTS_SOURCE})
target_include_directories(ConditionTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    )
target_link_units(ConditionTests UNITS dds::condition)
target_link_libraries(ConditionTests
    fastcdr
    GTest::gtest
    )
gtest_discover_tests(ConditionTests)

### ConditionNotifier ###
set(CONDITION_NOTIFIER_TESTS_SOURCE
    ConditionNotifierTests.cpp)

add_executable(ConditionNotifierTests ${CONDITION_NOTIFIER_TESTS_SOURCE})
target_include_directories(ConditionNotifierTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${GTEST_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    )
target_link_units(ConditionNotifierTests UNITS dds::condition MOCKS dds::condition::WaitSetImpl)
target_link_libraries(ConditionNotifierTests
    fastcdr
    GTest::gtest
    GTest::gmock
    )
gtest_discover_tests(ConditionNotifierTests)

### StatusConditionImpl ###
set(STATUS_CONDITION_IMPL_TESTS_SOURCE
    StatusConditionImplTests.cpp)

add_executable(StatusConditionImplTests ${STATUS_CONDITION_IMPL_TESTS_SOURCE})
target_include_directories(StatusConditionImplTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${GTEST_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    )
target_link_units(StatusConditionImplTests UNITS dds::condition MOCKS dds::condition::ConditionNotifier)
target_link_libraries(StatusConditionImplTests
    fastcdr
    GTest::gtest
    GTest::gmock
    )
gtest_discover_tests(StatusConditionImplTests)

### WaitSetImpl ###
set(WAITSET_IMPL_TESTS_SOURCE
    WaitSetImplTests.cpp)

add_executable(WaitSetImplTests ${WAITSET_IMPL_TESTS_SOURCE})
target_include_directories(WaitSetImplTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${GTEST_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    )
target_link_units(WaitSetImplTests UNITS dds::condition MOCKS dds::condition::ConditionNotifier)
target_link_libraries(WaitSetImplTests
    fastcdr
    GTest::gtest
    GTest::gmock
    )
gtest_discover_tests(WaitSetImplTests)
