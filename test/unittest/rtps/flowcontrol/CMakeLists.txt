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

set(FLOWCONTROLLER_COMMON_SOURCE
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/PropertyPolicy.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/FlowControllerConsts.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/LocatorSelectorSender.cpp
    )

set(FLOWCONTROLLERFACTORYTESTS_SOURCE
    ${FLOWCONTROLLER_COMMON_SOURCE}
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/FlowControllerFactory.cpp
    FlowControllerFactoryTests.cpp
    )

add_executable(FlowControllerFactoryTests ${FLOWCONTROLLERFACTORYTESTS_SOURCE})
target_include_directories(FlowControllerFactoryTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSWriter
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/Endpoint
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSReader
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSMessageGroup
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSParticipantImpl
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    )
target_link_libraries(FlowControllerFactoryTests
    fastcdr
    GTest::gmock
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    )
fastdds_target_link_object_groups(FlowControllerFactoryTests
    fastdds::log
    fastdds::rtps::host-net
    fastdds::rtps::net-interface
    fastdds::time
    )

gtest_discover_tests(FlowControllerFactoryTests)

set(FLOWCONTROLLERPUBLISHMODESTESTS_SOURCE
    ${FLOWCONTROLLER_COMMON_SOURCE}
    FlowControllerPublishModesOnPureSyncTests.cpp
    FlowControllerPublishModesOnSyncTests.cpp
    FlowControllerPublishModesOnAsyncTests.cpp
    FlowControllerPublishModesOnLimitedAsyncTests.cpp
    FlowControllerPublishModesTests.cpp
    )

add_executable(FlowControllerPublishModesTests ${FLOWCONTROLLERPUBLISHMODESTESTS_SOURCE})
target_include_directories(FlowControllerPublishModesTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSWriter
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/Endpoint
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSReader
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSMessageGroup
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSParticipantImpl
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    )
target_link_libraries(FlowControllerPublishModesTests
    fastcdr
    GTest::gmock
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    )
fastdds_target_link_object_groups(FlowControllerPublishModesTests
    fastdds::log
    fastdds::rtps::host-net
    fastdds::rtps::net-interface
    fastdds::time
    )

gtest_discover_tests(FlowControllerPublishModesTests)

set(FLOWCONTROLLERSCHEDULERSTESTS_SOURCE
    ${FLOWCONTROLLER_COMMON_SOURCE}
    FlowControllerSchedulersTests.cpp
    )

add_executable(FlowControllerSchedulersTests ${FLOWCONTROLLERSCHEDULERSTESTS_SOURCE})
target_include_directories(FlowControllerSchedulersTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSWriter
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/Endpoint
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSReader
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSMessageGroup
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSParticipantImpl
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    )
target_link_libraries(FlowControllerSchedulersTests
    fastcdr
    GTest::gmock
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    )
fastdds_target_link_object_groups(FlowControllerSchedulersTests
    fastdds::log
    fastdds::rtps::host-net
    fastdds::rtps::net-interface
    fastdds::time
    )

gtest_discover_tests(FlowControllerSchedulersTests)
