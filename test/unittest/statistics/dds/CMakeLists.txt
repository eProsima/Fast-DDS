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
    ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/monitorservice_typesPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/monitorservice_typesTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/typesPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/typesTypeObjectSupport.cxx
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

    add_executable(DomainParticipantStatisticsListenerTests
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/ThreadSettings.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/domain/DomainParticipantStatisticsListener.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
        ${TINYXML2_SOURCES}
        ${DOMAINPARTICIPANTSTATISTICSLISTENER_TESTS_SOURCE})
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
        GTest::gmock
        )
    fastdds_target_link_object_groups(DomainParticipantStatisticsListenerTests
        fastdds::log
        fastdds::xtypes::dynamic-types::impl
        fastdds::xtypes::type-representation
        fastdds::rtps::host-net
        fastdds::time
        fastdds::statistics::types
        )

    gtest_discover_tests(DomainParticipantStatisticsListenerTests)
endif()

## StatisticsDomainParticipantMockTests
if (SQLITE3_SUPPORT AND FASTDDS_STATISTICS AND NOT QNX)
    # this test includes C sources
    enable_language(C)

    file(GLOB STATISTICS_DOMAINPARTICIPANT_MOCK_TESTS_SOURCE
        StatisticsDomainParticipantMockTests.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/builtin/type_lookup_service/detail/rpc_typesPubSubTypes.cxx
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/builtin/type_lookup_service/detail/TypeLookupTypesPubSubTypes.cxx
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/builtin/type_lookup_service/TypeLookupReplyListener.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/builtin/type_lookup_service/TypeLookupRequestListener.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/Condition.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/ConditionNotifier.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/StatusCondition.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/StatusConditionImpl.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/WaitSetImpl.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Entity.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/policy/ParameterList.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/policy/QosPolicyUtils.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/policy/QosPolicyUtils.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/DomainParticipant.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/DomainParticipantFactory.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/qos/DomainParticipantFactoryQos.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/qos/DomainParticipantQos.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/FileConsumer.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/Log.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/OStreamConsumer.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/StdoutConsumer.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/StdoutErrConsumer.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/DataWriter.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/DataWriterHistory.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/DataWriterImpl.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/PublisherImpl.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/qos/DataWriterQos.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/qos/PublisherQos.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/qos/WriterQos.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/rpc/ServiceTypeSupport.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/DataReader.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/DataReaderImpl.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/history/DataReaderHistory.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/qos/DataReaderQos.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/qos/ReaderQos.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/qos/SubscriberQos.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/ReadCondition.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/Subscriber.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/SubscriberImpl.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/ContentFilteredTopic.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/ContentFilteredTopicImpl.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/qos/TopicQos.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/Topic.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/TopicImpl.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/TopicProxyFactory.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/TypeSupport.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/utils/QosConverters.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/utils/TypePropagation.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/EndpointSecurityAttributes.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/PropertyPolicy.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/RTPSParticipantAttributes.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/ServerAttributes.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/ThreadSettings.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/ParticipantBuiltinTopicData.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/ParticipantProxyData.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/PublicationBuiltinTopicData.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/SubscriptionBuiltinTopicData.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/ReaderProxyData.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/WriterProxyData.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/discovery/endpoint/EDP.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/discovery/participant/PDP.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/GuidPrefix_t.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/DataSharing/DataSharingPayloadPool.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/FlowControllerConsts.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/FlowControllerFactory.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/CacheChangePool.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/History.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/ReaderHistory.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/TopicPayloadPool.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/TopicPayloadPoolRegistry.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/CDRMessage.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/external_locators.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/ResourceEvent.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEvent.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEventImpl.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/PortBasedTransportDescriptor.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/shared_mem/SharedMemTransportDescriptor.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/LivelinessManager.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/LocatorSelectorSender.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/ReaderProxy.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/domain/DomainParticipant.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/domain/DomainParticipantImpl.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/domain/DomainParticipantStatisticsListener.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/publisher/qos/DataWriterQos.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/utils/StringMatching.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/utils/TimedConditionVariable.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/utils/UnitsParser.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLDynamicParser.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLElementParser.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLEndpointParser.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLParser.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLParserCommon.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLParserExtras.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLProfileManager.cpp
        )

    set(STATISTICS_DOMAINPARTICIPANT_STATUS_QUERYABLE_TESTS_SOURCE
        StatisticsDomainParticipantStatusQueryableTests.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/builtin/type_lookup_service/detail/rpc_typesPubSubTypes.cxx
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/builtin/type_lookup_service/detail/TypeLookupTypesPubSubTypes.cxx
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/builtin/type_lookup_service/TypeLookupReplyListener.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/builtin/type_lookup_service/TypeLookupRequestListener.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Entity.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/Condition.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/ConditionNotifier.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/StatusCondition.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/StatusConditionImpl.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/WaitSetImpl.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Entity.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/policy/ParameterList.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/policy/QosPolicyUtils.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/DomainParticipant.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/DomainParticipantFactory.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/qos/DomainParticipantFactoryQos.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/qos/DomainParticipantQos.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/DataWriter.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/DataWriterHistory.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/PublisherImpl.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/qos/DataWriterQos.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/qos/PublisherQos.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/qos/WriterQos.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/rpc/ServiceTypeSupport.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/DataReader.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/DataReaderImpl.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/history/DataReaderHistory.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/qos/DataReaderQos.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/qos/ReaderQos.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/qos/SubscriberQos.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/ReadCondition.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/Subscriber.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/SubscriberImpl.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/ContentFilteredTopic.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/ContentFilteredTopicImpl.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/qos/TopicQos.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/Topic.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/TopicImpl.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/TopicProxyFactory.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/TypeSupport.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/utils/QosConverters.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/utils/TypePropagation.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/EndpointSecurityAttributes.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/PropertyPolicy.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/RTPSParticipantAttributes.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/ServerAttributes.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/ThreadSettings.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/ParticipantBuiltinTopicData.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/ParticipantProxyData.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/PublicationBuiltinTopicData.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/SubscriptionBuiltinTopicData.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/ReaderProxyData.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/WriterProxyData.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/discovery/endpoint/EDP.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/discovery/participant/PDP.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/GuidPrefix_t.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/DataSharing/DataSharingPayloadPool.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/FlowControllerConsts.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/FlowControllerFactory.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/CacheChangePool.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/History.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/ReaderHistory.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/TopicPayloadPool.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/TopicPayloadPoolRegistry.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/CDRMessage.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/external_locators.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/ResourceEvent.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEvent.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEventImpl.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/PortBasedTransportDescriptor.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/shared_mem/SharedMemTransportDescriptor.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/LivelinessManager.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/LocatorSelectorSender.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/ReaderProxy.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/domain/DomainParticipant.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/domain/DomainParticipantImpl.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/domain/DomainParticipantStatisticsListener.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/publisher/qos/DataWriterQos.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/utils/StringMatching.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/utils/TimedConditionVariable.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/utils/UnitsParser.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLDynamicParser.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLElementParser.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLEndpointParser.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLParser.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLParserCommon.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLParserExtras.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLProfileManager.cpp
        )

    # Security Support
    if(SECURITY)
        file(GLOB security_sources
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/exceptions/Exception.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Token.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/security/exceptions/SecurityException.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/security/common/SharedSecretHandle.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/security/logging/Logging.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/security/logging/LoggingLevel.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/security/*.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/security/accesscontrol/*.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/security/artifact_providers/*.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/security/authentication/*.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/security/cryptography/*.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/security/logging/*.cpp
            )
        list(APPEND STATISTICS_DOMAINPARTICIPANT_MOCK_TESTS_SOURCE ${security_sources})
        list(APPEND STATISTICS_DOMAINPARTICIPANT_STATUS_QUERYABLE_TESTS_SOURCE ${security_sources})
    endif()

    # External sources
    if(TINYXML2_SOURCE_DIR)
        list(APPEND STATISTICS_DOMAINPARTICIPANT_MOCK_TESTS_SOURCE
            ${TINYXML2_SOURCES}
            )
        list(APPEND STATISTICS_DOMAINPARTICIPANT_STATUS_QUERYABLE_TESTS_SOURCE
            ${TINYXML2_SOURCE_DIR}/tinyxml2.cpp
            )
    endif()

    add_executable(StatisticsDomainParticipantMockTests
        ${STATISTICS_DOMAINPARTICIPANT_MOCK_TESTS_SOURCE}
        )
    target_include_directories(StatisticsDomainParticipantMockTests PRIVATE
        $<$<BOOL:${OPENSSL_INCLUDE_DIR}>:${OPENSSL_INCLUDE_DIR}>
        StatisticsDomainParticipantMockTests/mock
        ${PROJECT_SOURCE_DIR}/test/mock/dds/Publisher/
        ${PROJECT_SOURCE_DIR}/test/mock/dds/DomainParticipantImpl/
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_BINARY_DIR}/include
        ${Asio_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/src/cpp
        ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
        ${THIRDPARTY_BOOST_INCLUDE_DIR})
    target_link_libraries(StatisticsDomainParticipantMockTests PRIVATE
        fastcdr
        foonathan_memory
        GTest::gmock
        ${CMAKE_DL_LIBS}
        ${TINYXML2_LIBRARY}
        $<$<BOOL:${LINK_SSL}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto>
        $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
        ${THIRDPARTY_BOOST_LINK_LIBS}
        eProsima_atomic
        )
    fastdds_target_link_object_groups(StatisticsDomainParticipantMockTests
        fastdds::log
        fastdds::xtypes::dynamic-types::impl
        fastdds::xtypes::type-representation
        fastdds::rtps::host-net
        fastdds::rtps::core-full
        fastdds::rtps::transport-core
        fastdds::rtps::network-factory
        fastdds::rtps::transport-tcp
        fastdds::rtps::transport-tcpv6
        fastdds::rtps::transport-udp
        fastdds::rtps::transport-udpv6
        fastdds::rtps::transport-shm
        fastdds::rtps::net-interface
        fastdds::time
        fastdds::rtps::netmask-filter
        fastdds::statistics::types
        fastdds::statistics::monitor-service
        fastdds::security::pki
        fastdds::security::access-control
        fastdds::security::crypto
        fastdds::rtps::persistence
        )


    gtest_discover_tests(StatisticsDomainParticipantMockTests)

    add_executable(StatisticsDomainParticipantStatusQueryableTests ${STATISTICS_DOMAINPARTICIPANT_STATUS_QUERYABLE_TESTS_SOURCE})
    target_include_directories(StatisticsDomainParticipantStatusQueryableTests PRIVATE
        $<$<BOOL:${OPENSSL_INCLUDE_DIR}>:${OPENSSL_INCLUDE_DIR}>
        StatisticsDomainParticipantStatusQueryableTests/mock/
        ${PROJECT_SOURCE_DIR}/test/mock/dds/Publisher/
        ${PROJECT_SOURCE_DIR}/test/mock/dds/DomainParticipantImpl/
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_BINARY_DIR}/include
        ${Asio_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/src/cpp
        ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
        ${THIRDPARTY_BOOST_INCLUDE_DIR})
    target_link_libraries(StatisticsDomainParticipantStatusQueryableTests PRIVATE
        fastcdr
        foonathan_memory
        GTest::gmock
        ${CMAKE_DL_LIBS}
        ${TINYXML2_LIBRARY}
        $<$<BOOL:${LINK_SSL}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto>
        $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
        ${THIRDPARTY_BOOST_LINK_LIBS}
        eProsima_atomic
        )
    fastdds_target_link_object_groups(StatisticsDomainParticipantStatusQueryableTests
        fastdds::log
        fastdds::xtypes::dynamic-types::impl
        fastdds::xtypes::type-representation
        fastdds::rtps::host-net
        fastdds::rtps::core-full
        fastdds::rtps::transport-core
        fastdds::rtps::network-factory
        fastdds::rtps::transport-tcp
        fastdds::rtps::transport-tcpv6
        fastdds::rtps::transport-udp
        fastdds::rtps::transport-udpv6
        fastdds::rtps::transport-shm
        fastdds::rtps::net-interface
        fastdds::time
        fastdds::rtps::netmask-filter
        fastdds::statistics::types
        fastdds::statistics::monitor-service
        fastdds::security::pki
        fastdds::security::access-control
        fastdds::security::crypto
        fastdds::rtps::persistence
        )


    gtest_discover_tests(StatisticsDomainParticipantStatusQueryableTests)

endif (SQLITE3_SUPPORT AND FASTDDS_STATISTICS AND NOT QNX)
