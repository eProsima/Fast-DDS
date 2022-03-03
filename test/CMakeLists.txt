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

option(PERFORMANCE_TESTS "Activate the building and execution of performance tests" OFF)
option(SYSTEM_TESTS "Activate the building and execution of system tests" OFF)
option(PROFILING_TESTS "Activate the building and execution of profiling tests" OFF)
option(EPROSIMA_BUILD_TESTS "Activate the building and execution unit tests and integral tests" OFF)

if(EPROSIMA_BUILD)
    set(EPROSIMA_BUILD_TESTS ON)
    set(SYSTEM_TESTS ON)
endif()

file(TO_CMAKE_PATH "${PROJECT_SOURCE_DIR}/valgrind.supp" MEMORYCHECK_SUPPRESSIONS_FILE_TMP)
set(MEMORYCHECK_SUPPRESSIONS_FILE ${MEMORYCHECK_SUPPRESSIONS_FILE_TMP} CACHE FILEPATH "Valgrind suppresions file")

###############################################################################
# Testing
###############################################################################
if(EPROSIMA_BUILD_TESTS)
    find_package(GTest CONFIG REQUIRED)
    include(${PROJECT_SOURCE_DIR}/cmake/common/gtest.cmake)

    add_subdirectory(blackbox)
    add_subdirectory(communication)
    add_subdirectory(unittest)
    add_subdirectory(xtypes)
    add_subdirectory(dds/communication)
    add_subdirectory(dds/discovery)

    if(UNIX AND NOT APPLE AND STRICT_REALTIME)
        add_subdirectory(realtime)
    endif()
endif()

###############################################################################
# Profiling tests using valgrind
###############################################################################
if(NOT ((MSVC OR MSVC_IDE)) AND PROFILING_TESTS)
   find_program(CTEST_MEMORYCHECK_COMMAND NAMES valgrind)
   if(CTEST_MEMORYCHECK_COMMAND)
      add_subdirectory(profiling)
   endif()
endif()

###############################################################################
# Performance tests
###############################################################################
if(PERFORMANCE_TESTS)
    add_subdirectory(performance)
endif()

###############################################################################
# System tests
###############################################################################
if(SYSTEM_TESTS)
    add_subdirectory(system/tools/fastdds)
    add_subdirectory(system/tools/fds)
endif()

set_property(TARGET BlackboxTests_RTPS PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/blackbox; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib;TOPIC_RANDOM_NUMBER=${TOPIC_RANDOM_NUMBER};W_UNICAST_PORT_RANDOM_NUMBER=${W_UNICAST_PORT_RANDOM_NUMBER};R_UNICAST_PORT_RANDOM_NUMBER=${R_UNICAST_PORT_RANDOM_NUMBER};MULTICAST_PORT_RANDOM_NUMBER=${MULTICAST_PORT_RANDOM_NUMBER}")
#set_property(TARGET BlackboxTests_FastRTPS PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/blackbox; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
#set_property(TARGET BlackboxTests_DDS_PIM PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/blackbox; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET SimpleCommunicationPublisher PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/communication; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET SimpleCommunicationSubscriber PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/communication; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET SimpleCommunicationPubSub PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/communication; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET DDSSimpleCommunicationDynamicPublisher PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/dds/communication; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET DDSSimpleCommunicationDynamicSubscriber PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/dds/communication; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET DDSCommunicationPublisher PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/dds/communication; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET DDSCommunicationSubscriber PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/dds/communication; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET DDSCommunicationPubSub PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/dds/communication; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET DDSParticipantDiscovery PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/dds/discovery; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")

#set_property(TARGET MemoryTest PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
#set_property(TARGET AllocationTest PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
#set_property(TARGET user_thread_nonblocked_test PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
#set_property(TARGET TimedConditionVariableUnitTest PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
#set_property(TARGET mutex_testing_tool_preload PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
#set_property(TARGET TMutexTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")

set_property(TARGET UDPv4Tests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/transport; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET UDPv4Tests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/transport; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET UDPv6Tests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/transport; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET UDPv6Tests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/transport; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET TCPv6Tests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/transport; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET TCPv6Tests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/transport; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET TCPv6Tests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/transport; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET test_UDPv4Tests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/transport; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET test_UDPv4Tests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/transport; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET test_UDPv4Tests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/transport; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET TCPv4Tests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/transport; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET TCPv4Tests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/transport; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET TCPv4Tests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/transport; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET SharedMemTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/transport; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET SharedMemTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/transport; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET SharedMemTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/transport; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET DynamicTypesTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/dynamic_types; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET DynamicComplexTypesTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/dynamic_types; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET DynamicTypes_4_2_Tests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/dynamic_types; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")

set_property(TARGET BuiltinAESGCMGMAC PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib;CERTS_PATH=/data/local/tmp/test/certs")
set_property(TARGET AccessControlTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib;CERTS_PATH=/data/local/tmp/test/certs")
set_property(TARGET AccessControlTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib;CERTS_PATH=/data/local/tmp/test/certs")
set_property(TARGET BuiltinLogging PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib;CERTS_PATH=/data/local/tmp/test/certs")
set_property(TARGET BuiltinPKIDH PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib;CERTS_PATH=/data/local/tmp/test/certs")
#
set_property(TARGET EntityTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/dds/core/entity; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET ConditionTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/dds/core/condition; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET ConditionNotifierTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/dds/core/condition; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET StatusConditionImplTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/dds/core/condition; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET WaitSetImplTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/dds/core/condition; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET ListenerTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/dds/status; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET ListenerTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/dds/status; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET TopicTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/dds/topic; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET ParticipantTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/dds/participant; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET SubscriberTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/dds/subscriber; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET DataReaderTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/dds/subscriber; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET PublisherTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/dds/publisher; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET DataWriterTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/dds/publisher; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET DataWriterTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/dds/publisher; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET LoanableSequenceTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/dds/collections; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET LogTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/logging; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET LogFileTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/logging; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET LogMacrosAllActiveTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/logging/log_macros; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET LogMacrosNoInfoTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/logging/log_macros; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET LogMacrosNoWarningTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/logging/log_macros; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET LogMacrosNoErrorTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/logging/log_macros; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET LogMacrosDefaultTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/logging/log_macros; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET LogMacrosInternalDebugOffTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/logging/log_macros; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET XMLProfileParserTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/xmlparser; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET XMLParserTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/xmlparser; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET XMLTreeTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/xmlparser; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET XMLEndpointParserTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET StringMatchingTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/utils; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET StringMatchingTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/utils; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET FixedSizeStringTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/utils; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET BitmapRangeTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/utils; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET ResourceLimitedVectorTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/utils; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET LocatorTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/utils; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET FixedSizeQueueTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/utils; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET SystemInfoTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/utils; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET XTypesTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/xtypes; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET XTypesTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/xtypes; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET XTypesTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/xtypes; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET FlowControllerFactoryTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/rtps/flowcontrol; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET FlowControllerFactoryTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/rtps/flowcontrol; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET FlowControllerPublishModesTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/rtps/flowcontrol; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET FlowControllerPublishModesTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/rtps/flowcontrol; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET FlowControllerSchedulersTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/rtps/flowcontrol; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET FlowControllerSchedulersTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/rtps/flowcontrol; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET ReaderProxyTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/rtps/writer; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET LivelinessManagerTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/rtps/writer; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET RTPSWriterTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/rtps/writer; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET PersistenceTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/rtps/persistence; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET PersistenceTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/rtps/persistence; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET BuiltinDataSerializationTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/rtps/builtin; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET BuiltinDataSerializationTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/rtps/builtin; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET BuiltinDataSerializationTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/rtps/builtin; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET TimedEventTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/rtps/resources/timedevent; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")

set_property(TARGET SecurityAuthentication PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib;;CERTS_PATH=/data/local/tmp/test/certs")

set_property(TARGET ReaderHistoryTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/rtps/history; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET BasicPoolsTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/rtps/history; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET CacheChangePoolTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/rtps/history; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET TopicPayloadPoolTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/rtps/history; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET EdpTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/rtps/discovery; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET EdpTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/rtps/discovery; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET EdpTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/rtps/discovery; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET CacheChangeTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/rtps/common; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET GuidUtilsTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/rtps/common; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET GuidUtilsTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/rtps/common; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET SequenceNumberTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/rtps/common; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET PortParametersTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/rtps/common; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET WriterProxyTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/rtps/reader; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET NetworkFactoryTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/rtps/network; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET NetworkFactoryTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/unittest/rtps/network; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")

set_property(TARGET StatisticsDomainParticipantTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
set_property(TARGET StatisticsQosTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
# set_property(TARGET DomainParticipantStatisticsListenerTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
# set_property(TARGET StatisticsDomainParticipantMockTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
# set_property(TARGET StatisticsDomainParticipantMockTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")
# set_property(TARGET RTPSStatisticsTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")

set_property(TARGET XTypesBlackBoxTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR}/xtypes; ;&&;LD_LIBRARY_PATH=${ANDROID_TESTING_ROOT}/install/x86_64/lib")


install(CODE "set(ANDROID \"${ANDROID}\")")
install(CODE "set(ANDROID_TESTING_ROOT \"${ANDROID_TESTING_ROOT}\")")
install(CODE "set(CMAKE_BINARY_DIR \"${CMAKE_BINARY_DIR}\")")

install(SCRIPT ${CMAKE_SOURCE_DIR}/cmake/common/android_ci.cmake)

install(CODE "execute_process(COMMAND adb push ${CMAKE_INSTALL_PREFIX} ${ANDROID_TESTING_ROOT}/install)")
install(CODE "execute_process(COMMAND adb push ${CMAKE_BINARY_DIR}/test ${ANDROID_TESTING_ROOT})")

