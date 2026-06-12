# Copyright (C) 2026, Proyectos y Sistemas de Mantenimiento SL (eProsima)
#
# This program is commercial software licensed under the terms of the
# eProsima Software License Agreement Rev 03 (the "License")
#
# You may obtain a copy of the License at
# https://www.eprosima.com/licenses/LICENSE-REV03

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

set(CONGESTION_CONTROL_COMMON_SOURCE
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/utils/TypePropagation.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/PropertyPolicy.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/SubscriptionBuiltinTopicData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/LocatorWithMask.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/FlowControllerConsts.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/FlowControllerFactory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/network.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/ResourceEvent.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEvent.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEventImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetmaskFilterKind.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterfaceWithFilter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/LocatorSelectorSender.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/Host.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
    )

set(CONGESTION_CONTROL_BASIC_TESTS_SOURCE
    ${CONGESTION_CONTROL_COMMON_SOURCE}
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/congestion-control/CongestionControlFactory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/congestion-control/basic/CongestionControlBasic.cpp
    CongestionControlBasicTests.cpp
    )

add_executable(CongestionControlBasicTests ${CONGESTION_CONTROL_BASIC_TESTS_SOURCE})
target_compile_definitions(CongestionControlBasicTests PRIVATE
    FASTDDS_SOURCE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(CongestionControlBasicTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSWriter
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/Endpoint
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSReader
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSMessageGroup
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/NetworkFactory
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSParticipantImpl
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/GrainedFlowController
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/fastdds/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/fastdds/src/cpp
    )
target_link_libraries(CongestionControlBasicTests
    fastcdr
    fastdds::log
    GTest::gmock
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    )
gtest_discover_tests(CongestionControlBasicTests)
