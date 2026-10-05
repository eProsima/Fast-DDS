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

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../profiles/test_xml_profile.xml
    ${CMAKE_CURRENT_BINARY_DIR}/test_xml_profile.xml
    COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../profiles/test_xml_for_string_profile.xml
        ${CMAKE_CURRENT_BINARY_DIR}/test_xml_for_string_profile.xml
        COPYONLY)

set(PUBLISHERTESTS_SOURCE PublisherTests.cpp)

file(GLOB DDSSQLFILTER_SOURCES ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/DDSSQLFilter/*.cpp)

#SQLITE3 persistence service sources
set(sqlite3_source_files
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/persistence/sqlite3.c
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/persistence/SQLite3PersistenceService.cpp
    )

set(DATAWRITERTESTS_SOURCE DataWriterTests.cpp
    ${DDSSQLFILTER_SOURCES}
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
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/DomainParticipant.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/DomainParticipantFactory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/DomainParticipantImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/qos/DomainParticipantFactoryQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/qos/DomainParticipantQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/DataWriter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/DataWriterImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/Publisher.cpp
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
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/DataSharing/DataSharingPayloadPool.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/FlowControllerConsts.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/FlowControllerFactory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/CacheChangePool.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/History.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/ReaderHistory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/TopicPayloadPool.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/TopicPayloadPoolRegistry.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/CDRMessage.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/RTPSMessageCreator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/NetworkBuffer.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/NetworkConfiguration.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/NetworkFactory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/external_locators.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/persistence/PersistenceFactory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/ResourceEvent.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEvent.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEventImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/ChainingTransport.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/ChannelResource.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/MulticastTransportInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetmaskFilterKind.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterfaceWithFilter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/PortBasedTransportDescriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/shared_mem/SharedMemTransportDescriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/tcp/RTCPMessageManager.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/tcp/TCPControlMessage.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPAcceptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPAcceptorBasic.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPChannelResource.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPChannelResourceBasic.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPTransportInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPv4Transport.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPv6Transport.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TransportInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/UDPChannelResource.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/UDPTransportInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/UDPv4Transport.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/UDPv6Transport.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/LivelinessManager.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/LocatorSelectorSender.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/ReaderProxy.cpp
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

    if(SQLITE3_SUPPORT)
        list(APPEND DATAWRITERTESTS_SOURCE ${sqlite3_source_files})
        # sqlite.c requires a C compiler
        enable_language(C)
    endif()

    if (FASTDDS_STATISTICS)

        set(statistics_sources
            ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/domain/DomainParticipant.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/domain/DomainParticipantImpl.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/domain/DomainParticipantStatisticsListener.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/publisher/qos/DataWriterQos.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/subscriber/qos/DataReaderQos.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/statistics/rtps/monitor-service/MonitorService.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/statistics/rtps/monitor-service/MonitorServiceListener.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/monitorservice_typesPubSubTypes.cxx
            ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/monitorservice_typesTypeObjectSupport.cxx
            ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/typesPubSubTypes.cxx
            ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/typesTypeObjectSupport.cxx
            )

        list(APPEND DATAWRITERTESTS_SOURCE ${statistics_sources})

    endif()

    # SHM Transport
    if(IS_THIRDPARTY_BOOST_OK)
        list(APPEND DATAWRITERTESTS_SOURCE
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/shared_mem/SharedMemTransport.cpp
            )
    endif()

    # TLS Support
    if(TLS_FOUND)
        list(APPEND DATAWRITERTESTS_SOURCE
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPChannelResourceSecure.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPAcceptorSecure.cpp
            )
    endif()

    # Security Support
    if(SECURITY)
        list(APPEND DATAWRITERTESTS_SOURCE
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/exceptions/Exception.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Token.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/security/exceptions/SecurityException.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/security/common/SharedSecretHandle.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/security/logging/Logging.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/security/logging/LoggingLevel.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/security/SecurityManager.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/security/artifact_providers/Pkcs11Provider.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/security/authentication/PKIDH.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/security/accesscontrol/Permissions.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/security/accesscontrol/DistinguishedName.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/security/cryptography/AESGCMGMAC.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/security/cryptography/AESGCMGMAC_KeyExchange.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/security/cryptography/AESGCMGMAC_KeyFactory.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/security/cryptography/AESGCMGMAC_Transform.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/security/cryptography/AESGCMGMAC_Types.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/security/artifact_providers/FileProvider.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/security/authentication/PKIIdentityHandle.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/security/authentication/PKIHandshakeHandle.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/security/accesscontrol/AccessPermissionsHandle.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/security/accesscontrol/CommonParser.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/security/accesscontrol/GovernanceParser.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/security/accesscontrol/PermissionsParser.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/security/logging/LogTopic.cpp
            )
    endif()

    # External sources
    if(TINYXML2_SOURCE_DIR)
        list(APPEND DATAWRITERTESTS_SOURCE
            ${TINYXML2_SOURCE_DIR}/tinyxml2.cpp
            )
    endif()

    # Only include tinyxml2 headers manually if the associated config predates the use of CMake targets
    if(TINYXML2_INCLUDE_DIR)
        include_directories(${TINYXML2_INCLUDE_DIR})
    endif(TINYXML2_INCLUDE_DIR)

    set(PUBLISHERTESTS_SOURCE PublisherTests.cpp
    )

if(ANDROID)
    if (ANDROID_NATIVE_API_LEVEL LESS 24)
        list(APPEND PUBLISHERTESTS_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
        list(APPEND DATAWRITERTESTS_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
    endif()
endif()

add_executable(PublisherTests ${PUBLISHERTESTS_SOURCE})
target_include_directories(PublisherTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    ${PROJECT_SOURCE_DIR}/test/utils
    )
target_link_libraries(PublisherTests fastdds fastcdr foonathan_memory
    GTest::gmock
    $<$<BOOL:${QNX}>:socket>
    ${CMAKE_DL_LIBS})
gtest_discover_tests(PublisherTests)

add_executable(DataWriterTests ${DATAWRITERTESTS_SOURCE})
target_include_directories(DataWriterTests PRIVATE
    $<$<BOOL:${OPENSSL_INCLUDE_DIR}>:${OPENSSL_INCLUDE_DIR}>
    ${PROJECT_SOURCE_DIR}/test/mock/dds/DataWriterHistory
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    )
target_link_libraries(DataWriterTests
    fastcdr
    foonathan_memory
    GTest::gmock
    ${CMAKE_DL_LIBS}
    ${TINYXML2_LIBRARY}
    $<$<BOOL:${LINK_SSL}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto>
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    $<$<BOOL:${QNX}>:socket>
    ${THIRDPARTY_BOOST_LINK_LIBS}
    $<$<BOOL:${LibP11_FOUND}>:eProsima_p11>  # $<TARGET_NAME_IF_EXISTS:eProsima_p11>
    eProsima_atomic
    )
fastdds_target_link_object_groups(DataWriterTests
    fastdds::log
    fastdds::xtypes::dynamic-types::impl
    fastdds::xtypes::type-representation
    fastdds::rtps::host-net
    fastdds::rtps::core-full
    )

gtest_discover_tests(DataWriterTests
    PROPERTIES ENVIRONMENT "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs")
