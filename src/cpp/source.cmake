set(${PROJECT_NAME}_module_dirs
    fastdds/log
    fastdds/xtypes
    )

set(${PROJECT_NAME}_source_files
    ${ALL_HEADERS}

    ${FASTDDS_SOURCE_DIR}/fastdds/topic/DDSSQLFilter/DDSFilterCompoundCondition.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/topic/DDSSQLFilter/DDSFilterExpression.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/topic/DDSSQLFilter/DDSFilterExpressionParser.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/topic/DDSSQLFilter/DDSFilterFactory.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/topic/DDSSQLFilter/DDSFilterField.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/topic/DDSSQLFilter/DDSFilterParameter.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/topic/DDSSQLFilter/DDSFilterPredicate.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/topic/DDSSQLFilter/DDSFilterValue.cpp

    ${FASTDDS_SOURCE_DIR}/fastdds/builtin/type_lookup_service/detail/rpc_typesPubSubTypes.cxx
    ${FASTDDS_SOURCE_DIR}/fastdds/builtin/type_lookup_service/detail/TypeLookupTypesPubSubTypes.cxx
    ${FASTDDS_SOURCE_DIR}/fastdds/builtin/type_lookup_service/TypeLookupManager.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/builtin/type_lookup_service/TypeLookupRequestListener.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/builtin/type_lookup_service/TypeLookupReplyListener.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/core/condition/Condition.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/core/condition/ConditionNotifier.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/core/condition/GuardCondition.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/core/condition/StatusCondition.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/core/condition/StatusConditionImpl.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/core/condition/WaitSet.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/core/condition/WaitSetImpl.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/core/Entity.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/core/policy/ParameterList.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/core/policy/QosPolicyUtils.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/core/Time_t.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/domain/DomainParticipant.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/domain/DomainParticipantFactory.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/domain/DomainParticipantImpl.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/domain/qos/DomainParticipantFactoryQos.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/domain/qos/DomainParticipantQos.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/log/FileConsumer.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/log/Log.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/log/OStreamConsumer.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/log/StdoutConsumer.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/log/StdoutErrConsumer.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/publisher/DataWriter.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/publisher/DataWriterHistory.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/publisher/DataWriterImpl.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/publisher/Publisher.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/publisher/PublisherImpl.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/publisher/qos/DataWriterQos.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/publisher/qos/PublisherQos.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/publisher/qos/WriterQos.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/rpc/ServiceImpl.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/rpc/ReplierImpl.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/rpc/RequesterImpl.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/rpc/RPCTypeObjectSupport.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/rpc/ServiceTypeSupport.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/subscriber/DataReader.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/subscriber/DataReaderImpl.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/subscriber/history/DataReaderHistory.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/subscriber/qos/DataReaderQos.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/subscriber/qos/ReaderQos.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/subscriber/qos/SubscriberQos.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/subscriber/ReadCondition.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/subscriber/Subscriber.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/subscriber/SubscriberImpl.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/topic/ContentFilteredTopic.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/topic/ContentFilteredTopicImpl.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/topic/qos/TopicQos.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/topic/Topic.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/topic/TopicImpl.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/topic/TopicProxyFactory.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/topic/TypeSupport.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/utils/QosConverters.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/utils/TypePropagation.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/xtypes/dynamic_types/AnnotationDescriptorImpl.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/xtypes/dynamic_types/DynamicDataFactory.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/xtypes/dynamic_types/DynamicDataImpl.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/xtypes/dynamic_types/DynamicDataFactoryImpl.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/xtypes/dynamic_types/DynamicPubSubType.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/xtypes/dynamic_types/DynamicTypeImpl.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/xtypes/dynamic_types/DynamicTypeBuilderFactory.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/xtypes/dynamic_types/DynamicTypeBuilderImpl.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/xtypes/dynamic_types/DynamicTypeBuilderFactoryImpl.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/xtypes/dynamic_types/DynamicTypeMemberImpl.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/xtypes/dynamic_types/MemberDescriptorImpl.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/xtypes/dynamic_types/TypeDescriptorImpl.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/xtypes/dynamic_types/VerbatimTextDescriptorImpl.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/xtypes/exception/Exception.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/xtypes/serializers/idl/dynamic_type_idl.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/xtypes/serializers/json/dynamic_data_json.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/xtypes/serializers/json/json_dynamic_data.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/xtypes/type_representation/dds_xtypes_typeobjectPubSubTypes.cxx
    ${FASTDDS_SOURCE_DIR}/fastdds/xtypes/type_representation/TypeObjectRegistry.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/xtypes/type_representation/TypeObjectUtils.cpp
    ${FASTDDS_SOURCE_DIR}/fastdds/xtypes/utils.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/attributes/EndpointSecurityAttributes.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/attributes/PropertyPolicy.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/attributes/RTPSParticipantAttributes.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/attributes/ServerAttributes.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/attributes/ThreadSettings.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/BuiltinProtocols.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/data/ParticipantBuiltinTopicData.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/data/ParticipantProxyData.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/data/PublicationBuiltinTopicData.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/data/SubscriptionBuiltinTopicData.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/data/ReaderProxyData.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/data/WriterProxyData.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/discovery/database/backup/SharedBackupFunctions.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/discovery/database/DiscoveryDataBase.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/discovery/database/DiscoveryParticipantInfo.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/discovery/database/DiscoveryParticipantsAckStatus.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/discovery/database/DiscoverySharedInfo.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/discovery/endpoint/EDP.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/discovery/endpoint/EDPClient.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/discovery/endpoint/EDPServer.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/discovery/endpoint/EDPServerListeners.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/discovery/endpoint/EDPSimple.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/discovery/endpoint/EDPSimpleListeners.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/discovery/endpoint/EDPStatic.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/discovery/participant/DirectMessageSender.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/discovery/participant/PDP.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/discovery/participant/PDPClient.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/discovery/participant/PDPClientListener.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/discovery/participant/PDPListener.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/discovery/participant/PDPServer.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/discovery/participant/PDPServerListener.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/discovery/participant/PDPSimple.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/discovery/participant/simple/PDPStatelessWriter.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/discovery/participant/timedevent/DSClientEvent.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/discovery/participant/timedevent/DServerEvent.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/liveliness/WLP.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/liveliness/WLPListener.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/common/GuidPrefix_t.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/common/SerializedPayload.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/common/LocatorWithMask.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/common/Time_t.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/common/Token.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/DataSharing/DataSharingListener.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/DataSharing/DataSharingNotification.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/DataSharing/DataSharingPayloadPool.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/exceptions/Exception.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/flowcontrol/FlowControllerConsts.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/flowcontrol/FlowControllerFactory.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/history/CacheChangePool.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/history/History.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/history/ReaderHistory.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/history/TopicPayloadPool.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/history/TopicPayloadPoolRegistry.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/history/WriterHistory.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/messages/CDRMessage.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/messages/MessageReceiver.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/messages/RTPSGapBuilder.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/messages/RTPSMessageCreator.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/messages/RTPSMessageGroup.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/messages/SendBuffersManager.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/messages/submessages/AckNackMsg.hpp
    ${FASTDDS_SOURCE_DIR}/rtps/messages/submessages/DataMsg.hpp
    ${FASTDDS_SOURCE_DIR}/rtps/messages/submessages/GapMsg.hpp
    ${FASTDDS_SOURCE_DIR}/rtps/messages/submessages/HeartbeatMsg.hpp
    ${FASTDDS_SOURCE_DIR}/rtps/network/NetworkBuffer.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/network/NetworkConfiguration.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/network/NetworkFactory.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/network/ReceiverResource.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/network/utils/external_locators.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/network/utils/netmask_filter.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/network/utils/network.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/participant/RTPSParticipant.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/participant/RTPSParticipantImpl.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/persistence/PersistenceFactory.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/reader/BaseReader.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/reader/reader_utils.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/reader/RTPSReader.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/reader/StatefulPersistentReader.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/reader/StatefulReader.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/reader/StatelessPersistentReader.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/reader/StatelessReader.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/reader/WriterProxy.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/resources/ResourceEvent.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/resources/TimedEvent.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/resources/TimedEventImpl.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/domain/RTPSDomain.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/transport/ChainingTransport.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/transport/ChannelResource.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/transport/network/NetmaskFilterKind.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/transport/network/NetworkInterface.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/transport/network/NetworkInterfaceWithFilter.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/transport/PortBasedTransportDescriptor.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/transport/shared_mem/SharedMemTransportDescriptor.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/transport/tcp/RTCPMessageManager.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/transport/tcp/TCPControlMessage.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/transport/TCPAcceptor.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/transport/TCPAcceptorBasic.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/transport/TCPChannelResource.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/transport/TCPChannelResourceBasic.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/transport/TCPTransportInterface.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/transport/TCPv4Transport.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/transport/TCPv6Transport.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/transport/test_UDPv4Transport.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/transport/TransportInterface.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/transport/UDPChannelResource.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/transport/UDPTransportInterface.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/transport/UDPv4Transport.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/transport/UDPv6Transport.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/writer/BaseWriter.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/writer/LivelinessManager.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/writer/LocatorSelectorSender.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/writer/PersistentWriter.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/writer/ReaderLocator.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/writer/ReaderProxy.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/writer/RTPSWriter.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/writer/StatefulPersistentWriter.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/writer/StatefulWriter.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/writer/StatelessPersistentWriter.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/writer/StatelessWriter.cpp
    ${FASTDDS_SOURCE_DIR}/statistics/fastdds/domain/DomainParticipant.cpp
    ${FASTDDS_SOURCE_DIR}/statistics/fastdds/publisher/qos/DataWriterQos.cpp
    ${FASTDDS_SOURCE_DIR}/statistics/fastdds/subscriber/qos/DataReaderQos.cpp
    ${FASTDDS_SOURCE_DIR}/utils/Host.cpp
    ${FASTDDS_SOURCE_DIR}/utils/IPFinder.cpp
    ${FASTDDS_SOURCE_DIR}/utils/IPLocator.cpp
    ${FASTDDS_SOURCE_DIR}/utils/md5.cpp
    ${FASTDDS_SOURCE_DIR}/utils/StringMatching.cpp
    ${FASTDDS_SOURCE_DIR}/utils/SystemInfo.cpp
    ${FASTDDS_SOURCE_DIR}/utils/TimedConditionVariable.cpp
    ${FASTDDS_SOURCE_DIR}/utils/UnitsParser.cpp
    ${FASTDDS_SOURCE_DIR}/xmlparser/attributes/TopicAttributes.cpp
    ${FASTDDS_SOURCE_DIR}/xmlparser/XMLDynamicParser.cpp
    ${FASTDDS_SOURCE_DIR}/xmlparser/XMLElementParser.cpp
    ${FASTDDS_SOURCE_DIR}/xmlparser/XMLEndpointParser.cpp
    ${FASTDDS_SOURCE_DIR}/xmlparser/XMLParser.cpp
    ${FASTDDS_SOURCE_DIR}/xmlparser/XMLParserCommon.cpp
    ${FASTDDS_SOURCE_DIR}/xmlparser/XMLProfileManager.cpp
    )

# Statistics support
if (FASTDDS_STATISTICS)

    set(statistics_sources
        ${FASTDDS_SOURCE_DIR}/statistics/fastdds/domain/DomainParticipantImpl.cpp
        ${FASTDDS_SOURCE_DIR}/statistics/fastdds/domain/DomainParticipantStatisticsListener.cpp
        ${FASTDDS_SOURCE_DIR}/statistics/rtps/monitor-service/MonitorService.cpp
        ${FASTDDS_SOURCE_DIR}/statistics/rtps/monitor-service/MonitorServiceListener.cpp
        ${FASTDDS_SOURCE_DIR}/statistics/rtps/reader/StatisticsReaderImpl.cpp
        ${FASTDDS_SOURCE_DIR}/statistics/rtps/StatisticsBase.cpp
        ${FASTDDS_SOURCE_DIR}/statistics/rtps/writer/StatisticsWriterImpl.cpp
        ${FASTDDS_SOURCE_DIR}/statistics/types/monitorservice_typesPubSubTypes.cxx
        ${FASTDDS_SOURCE_DIR}/statistics/types/monitorservice_typesTypeObjectSupport.cxx
        ${FASTDDS_SOURCE_DIR}/statistics/types/typesPubSubTypes.cxx
        ${FASTDDS_SOURCE_DIR}/statistics/types/typesTypeObjectSupport.cxx
        )

    list(APPEND ${PROJECT_NAME}_source_files
        ${statistics_sources}
        )

endif()

# SHM Transport
if(IS_THIRDPARTY_BOOST_OK)
    list(APPEND ${PROJECT_NAME}_source_files
        ${FASTDDS_SOURCE_DIR}/rtps/transport/shared_mem/test_SharedMemTransport.cpp
        ${FASTDDS_SOURCE_DIR}/rtps/transport/shared_mem/SharedMemTransport.cpp
        )
endif()

# TLS Support
if(TLS_FOUND)
    list(APPEND ${PROJECT_NAME}_source_files
        ${FASTDDS_SOURCE_DIR}/rtps/transport/TCPChannelResourceSecure.cpp
        ${FASTDDS_SOURCE_DIR}/rtps/transport/TCPAcceptorSecure.cpp
        )
endif()

# Security sources
set(${PROJECT_NAME}_security_source_files
    ${FASTDDS_SOURCE_DIR}/rtps/security/exceptions/SecurityException.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/security/common/SharedSecretHandle.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/security/logging/Logging.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/security/logging/LoggingLevel.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/security/SecurityManager.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/security/SecurityPluginFactory.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/builtin/discovery/participant/DS/PDPSecurityInitiatorListener.cpp
    ${FASTDDS_SOURCE_DIR}/security/authentication/PKIDH.cpp
    ${FASTDDS_SOURCE_DIR}/security/accesscontrol/Permissions.cpp
    ${FASTDDS_SOURCE_DIR}/security/accesscontrol/DistinguishedName.cpp
    ${FASTDDS_SOURCE_DIR}/security/cryptography/AESGCMGMAC.cpp
    ${FASTDDS_SOURCE_DIR}/security/cryptography/AESGCMGMAC_KeyExchange.cpp
    ${FASTDDS_SOURCE_DIR}/security/cryptography/AESGCMGMAC_KeyFactory.cpp
    ${FASTDDS_SOURCE_DIR}/security/cryptography/AESGCMGMAC_Transform.cpp
    ${FASTDDS_SOURCE_DIR}/security/cryptography/AESGCMGMAC_Types.cpp
    ${FASTDDS_SOURCE_DIR}/security/authentication/PKIIdentityHandle.cpp
    ${FASTDDS_SOURCE_DIR}/security/authentication/PKIHandshakeHandle.cpp
    ${FASTDDS_SOURCE_DIR}/security/accesscontrol/AccessPermissionsHandle.cpp
    ${FASTDDS_SOURCE_DIR}/security/accesscontrol/CommonParser.cpp
    ${FASTDDS_SOURCE_DIR}/security/accesscontrol/GovernanceParser.cpp
    ${FASTDDS_SOURCE_DIR}/security/accesscontrol/PermissionsParser.cpp
    ${FASTDDS_SOURCE_DIR}/security/logging/LogTopic.cpp
    ${FASTDDS_SOURCE_DIR}/security/artifact_providers/FileProvider.cpp
    ${FASTDDS_SOURCE_DIR}/security/artifact_providers/Pkcs11Provider.cpp
    )

if(SECURITY)
    list(APPEND ${PROJECT_NAME}_source_files
        ${${PROJECT_NAME}_security_source_files}
        )
    set(HAVE_SECURITY 1)
else()
    set(HAVE_SECURITY 0)
endif()

if(WIN32 AND (MSVC OR MSVC_IDE))
    list(APPEND ${PROJECT_NAME}_source_files
        ${FASTDDS_SOURCE_DIR}/fastdds.rc
        )
endif()


#SQLITE3 persistence service sources
set(${PROJECT_NAME}_sqlite3_source_files
    ${FASTDDS_SOURCE_DIR}/rtps/persistence/SQLite3PersistenceService.cpp
    ${FASTDDS_SOURCE_DIR}/rtps/persistence/sqlite3.c
    )

if(SQLITE3_SUPPORT)
    list(APPEND ${PROJECT_NAME}_source_files
        ${${PROJECT_NAME}_sqlite3_source_files}
        )
    set(HAVE_SQLITE3 1)
    # sqlite.c requires a C compiler
    enable_language(C)
else()
    set(HAVE_SQLITE3 0)
endif()


# External sources
if(TINYXML2_SOURCE_DIR)
    set(TINYXML2_SOURCE_DIR_ ${TINYXML2_SOURCE_DIR})
    list(APPEND ${PROJECT_NAME}_source_files
        ${TINYXML2_SOURCE_DIR_}/tinyxml2.cpp
        )
endif()

if(ANDROID)
    if((ANDROID_PLATFORM LESS_EQUAL 23) OR (ANDROID_NATIVE_API_LEVEL LESS_EQUAL 23))
        list(APPEND ${PROJECT_NAME}_source_files
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
    endif()
endif()
