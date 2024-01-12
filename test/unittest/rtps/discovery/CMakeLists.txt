# Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#EDP TESTS
set(EDPTESTS_SOURCE EdpTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/AnnotationDescriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/DynamicData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/DynamicDataFactory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/DynamicDataPtr.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/DynamicPubSubType.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/DynamicType.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/DynamicTypeBuilder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/DynamicTypeBuilderFactory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/DynamicTypeBuilderPtr.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/DynamicTypeMember.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/DynamicTypePtr.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/MemberDescriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/TypeDescriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/Log.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/OStreamConsumer.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/StdoutConsumer.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/StdoutErrConsumer.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/qos/WriterQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/qos/ReaderQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/TopicDataType.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/xtypes/exception/Exception.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/xtypes/type_representation/dds_xtypes_typeobjectPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/xtypes/type_representation/TypeObjectRegistry.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/xtypes/type_representation/TypeObjectUtils.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/ThreadSettings.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/discovery/endpoint/EDP.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/FlowControllerConsts.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/ThroughputControllerDescriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/string_convert.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/StringMatching.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
    )

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

add_executable(EdpTests ${EDPTESTS_SOURCE})
target_compile_definitions(EdpTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(EdpTests PRIVATE
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/PDP
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/ExternalLocatorsProcessor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSParticipantImpl
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/Endpoint
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/ReaderProxyData
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSReader
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/StatefulReader
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/ReaderHistory
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/WriterProxyData
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSWriter
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/StatefulWriter
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/WriterHistory
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/TimedEvent
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/ResourceEvent
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/SecurityManager
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSDomainImpl
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${Asio_INCLUDE_DIR}
    )
target_link_libraries(EdpTests foonathan_memory
    GTest::gmock
    ${CMAKE_DL_LIBS})
if(QNX)
    target_link_libraries(EdpTests socket)
endif()
if(MSVC OR MSVC_IDE)
    target_link_libraries(EdpTests ${PRIVACY} fastcdr iphlpapi Shlwapi ws2_32)
else()
    target_link_libraries(EdpTests ${PRIVACY} fastcdr)
endif()

gtest_discover_tests(EdpTests)

#PDP TESTS
set(PDPTESTS_SOURCE PDPTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/discovery/participant/PDP.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/qos/WriterQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/qos/ReaderQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/TopicDataType.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/xtypes/type_representation/dds_xtypes_typeobjectPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/ParticipantProxyData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/policy/ParameterList.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/PropertyPolicy.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/NetworkFactory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/Log.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/StringMatching.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/string_convert.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/ThreadSettings.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/WriterProxyData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/ReaderProxyData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/ThroughputControllerDescriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/FlowControllerConsts.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/OStreamConsumer.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/StdoutConsumer.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/StdoutErrConsumer.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/AnnotationDescriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/DynamicData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/DynamicDataFactory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/DynamicType.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/DynamicPubSubType.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/DynamicTypePtr.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/DynamicDataPtr.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/DynamicTypeBuilder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/DynamicTypeBuilderPtr.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/DynamicTypeBuilderFactory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/DynamicTypeMember.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/TypeDescriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/MemberDescriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
    )

add_executable(PDPTests ${PDPTESTS_SOURCE})

target_compile_definitions(PDPTests PRIVATE FASTRTPS_NO_LIB
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(PDPTests PRIVATE
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/BuiltinProtocols
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/ExternalLocatorsProcessor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSParticipantImpl
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/Endpoint
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/EDP
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/WLP
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/ReceiverResource
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSReader
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/StatefulReader
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/StatelessReader
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/ReaderHistory
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSWriter
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/StatefulWriter
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/StatelessWriter
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/WriterHistory
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/TimedEvent
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/ResourceEvent
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/SecurityManager
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/TypeLookupManager
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${Asio_INCLUDE_DIR}
    )

target_link_libraries(PDPTests foonathan_memory
    GTest::gmock
    ${CMAKE_DL_LIBS})
if(QNX)
    target_link_libraries(PDPTests socket)
endif()
if(MSVC OR MSVC_IDE)
    target_link_libraries(PDPTests ${PRIVACY} fastcdr iphlpapi Shlwapi ws2_32)
else()
    target_link_libraries(PDPTests ${PRIVACY} fastcdr)
endif()

gtest_discover_tests(PDPTests)

