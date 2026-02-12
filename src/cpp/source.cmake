set(${PROJECT_NAME}_module_dirs
    fastdds/log
    fastdds/xtypes
    )

set(${PROJECT_NAME}_source_files
    ${ALL_HEADERS}

    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/topic/DDSSQLFilter/DDSFilterCompoundCondition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/topic/DDSSQLFilter/DDSFilterExpression.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/topic/DDSSQLFilter/DDSFilterExpressionParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/topic/DDSSQLFilter/DDSFilterFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/topic/DDSSQLFilter/DDSFilterField.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/topic/DDSSQLFilter/DDSFilterParameter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/topic/DDSSQLFilter/DDSFilterPredicate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/topic/DDSSQLFilter/DDSFilterValue.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/builtin/type_lookup_service/detail/rpc_typesPubSubTypes.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/builtin/type_lookup_service/detail/TypeLookupTypesPubSubTypes.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/builtin/type_lookup_service/TypeLookupManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/builtin/type_lookup_service/TypeLookupRequestListener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/builtin/type_lookup_service/TypeLookupReplyListener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/core/condition/Condition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/core/condition/ConditionNotifier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/core/condition/GuardCondition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/core/condition/StatusCondition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/core/condition/StatusConditionImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/core/condition/WaitSet.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/core/condition/WaitSetImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/core/Entity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/core/policy/ParameterList.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/core/policy/QosPolicyUtils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/core/Time_t.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/domain/DomainParticipant.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/domain/DomainParticipantFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/domain/DomainParticipantImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/domain/qos/DomainParticipantFactoryQos.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/domain/qos/DomainParticipantQos.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/log/FileConsumer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/log/Log.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/log/OStreamConsumer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/log/StdoutConsumer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/log/StdoutErrConsumer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/publisher/DataWriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/publisher/DataWriterHistory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/publisher/DataWriterImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/publisher/Publisher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/publisher/PublisherImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/publisher/qos/DataWriterQos.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/publisher/qos/PublisherQos.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/publisher/qos/WriterQos.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/rpc/ServiceImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/rpc/ReplierImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/rpc/RequesterImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/rpc/RPCTypeObjectSupport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/rpc/ServiceTypeSupport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/subscriber/DataReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/subscriber/DataReaderImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/subscriber/history/DataReaderHistory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/subscriber/qos/DataReaderQos.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/subscriber/qos/ReaderQos.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/subscriber/qos/SubscriberQos.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/subscriber/ReadCondition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/subscriber/Subscriber.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/subscriber/SubscriberImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/topic/ContentFilteredTopic.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/topic/ContentFilteredTopicImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/topic/qos/TopicQos.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/topic/Topic.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/topic/TopicImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/topic/TopicProxyFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/topic/TypeSupport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/utils/QosConverters.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/utils/TypePropagation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/xtypes/dynamic_types/AnnotationDescriptorImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/xtypes/dynamic_types/DynamicDataFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/xtypes/dynamic_types/DynamicDataImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/xtypes/dynamic_types/DynamicDataFactoryImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/xtypes/dynamic_types/DynamicPubSubType.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/xtypes/dynamic_types/DynamicTypeImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/xtypes/dynamic_types/DynamicTypeBuilderFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/xtypes/dynamic_types/DynamicTypeBuilderImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/xtypes/dynamic_types/DynamicTypeBuilderFactoryImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/xtypes/dynamic_types/DynamicTypeMemberImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/xtypes/dynamic_types/MemberDescriptorImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/xtypes/dynamic_types/TypeDescriptorImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/xtypes/dynamic_types/VerbatimTextDescriptorImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/xtypes/exception/Exception.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/xtypes/serializers/idl/dynamic_type_idl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/xtypes/serializers/json/dynamic_data_json.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/xtypes/serializers/json/json_dynamic_data.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/xtypes/type_representation/dds_xtypes_typeobjectPubSubTypes.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/xtypes/type_representation/TypeObjectRegistry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/xtypes/type_representation/TypeObjectUtils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fastdds/xtypes/utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/attributes/EndpointSecurityAttributes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/attributes/PropertyPolicy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/attributes/RTPSParticipantAttributes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/attributes/ServerAttributes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/attributes/ThreadSettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/BuiltinProtocols.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/data/ParticipantBuiltinTopicData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/data/ParticipantProxyData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/data/PublicationBuiltinTopicData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/data/SubscriptionBuiltinTopicData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/data/ReaderProxyData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/data/WriterProxyData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/database/backup/SharedBackupFunctions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/database/DiscoveryDataBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/database/DiscoveryParticipantInfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/database/DiscoveryParticipantsAckStatus.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/database/DiscoverySharedInfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/endpoint/EDP.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/endpoint/EDPClient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/endpoint/EDPServer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/endpoint/EDPServerListeners.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/endpoint/EDPSimple.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/endpoint/EDPSimpleListeners.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/endpoint/EDPStatic.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/participant/DirectMessageSender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/participant/PDP.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/participant/PDPClient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/participant/PDPClientListener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/participant/PDPListener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/participant/PDPServer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/participant/PDPServerListener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/participant/PDPSimple.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/participant/simple/PDPStatelessWriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/participant/timedevent/DSClientEvent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/participant/timedevent/DServerEvent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/liveliness/WLP.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/liveliness/WLPListener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/common/GuidPrefix_t.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/common/SerializedPayload.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/common/LocatorWithMask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/common/Time_t.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/common/Token.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/DataSharing/DataSharingListener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/DataSharing/DataSharingNotification.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/DataSharing/DataSharingPayloadPool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/exceptions/Exception.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/flowcontrol/FlowControllerConsts.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/flowcontrol/FlowControllerFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/history/CacheChangePool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/history/History.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/history/ReaderHistory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/history/TopicPayloadPool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/history/TopicPayloadPoolRegistry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/history/WriterHistory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/messages/CDRMessage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/messages/MessageReceiver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/messages/RTPSGapBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/messages/RTPSMessageCreator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/messages/RTPSMessageGroup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/messages/SendBuffersManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/messages/submessages/AckNackMsg.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/messages/submessages/DataMsg.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/messages/submessages/GapMsg.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/messages/submessages/HeartbeatMsg.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/network/NetworkBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/network/NetworkConfiguration.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/network/NetworkFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/network/ReceiverResource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/network/utils/external_locators.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/network/utils/netmask_filter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/network/utils/network.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/participant/RTPSParticipant.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/participant/RTPSParticipantImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/persistence/PersistenceFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/reader/BaseReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/reader/reader_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/reader/RTPSReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/reader/StatefulPersistentReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/reader/StatefulReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/reader/StatelessPersistentReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/reader/StatelessReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/reader/WriterProxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/resources/ResourceEvent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/resources/TimedEvent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/resources/TimedEventImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/domain/RTPSDomain.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/domain/RTPSDomainExtras.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/ChainingTransport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/ChannelResource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/network/NetmaskFilterKind.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/network/NetworkInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/network/NetworkInterfaceWithFilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/PortBasedTransportDescriptor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/shared_mem/SharedMemTransportDescriptor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/tcp/RTCPMessageManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/tcp/TCPControlMessage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/TCPAcceptor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/TCPAcceptorBasic.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/TCPChannelResource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/TCPChannelResourceBasic.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/TCPTransportInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/TCPv4Transport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/TCPv6Transport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/test_UDPv4Transport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/TransportInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/UDPChannelResource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/UDPTransportInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/UDPv4Transport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/UDPv6Transport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/writer/BaseWriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/writer/LivelinessManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/writer/LocatorSelectorSender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/writer/PersistentWriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/writer/ReaderLocator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/writer/ReaderProxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/writer/RTPSWriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/writer/StatefulPersistentWriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/writer/StatefulWriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/writer/StatelessPersistentWriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/writer/StatelessWriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/statistics/fastdds/domain/DomainParticipant.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/statistics/fastdds/publisher/qos/DataWriterQos.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/statistics/fastdds/subscriber/qos/DataReaderQos.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/Host.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/IPFinder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/IPLocator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/md5.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/StringMatching.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/SystemInfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/TimedConditionVariable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/UnitsParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/xmlparser/attributes/TopicAttributes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/xmlparser/XMLDynamicParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/xmlparser/XMLElementParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/xmlparser/XMLEndpointParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/xmlparser/XMLParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/xmlparser/XMLParserCommon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/xmlparser/XMLParserExtras.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/xmlparser/XMLProfileManager.cpp

    # Ethernet transport
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/ethernet/eth_transport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/ethernet/EthernetSenderResource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/ethernet/InputChannelManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/ethernet/InputPort.cpp

    # TSN aware UDP transports
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/udp_tsn/udp_tsn_senders.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/udp_tsn/udp_tsn_transport_descriptors.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/udp_tsn/udp_tsn_transports.cpp

    # Low low-bandwidth transports
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/low-bandwidth/header-reduction/HeaderReductionTransport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/low-bandwidth/header-reduction/HeaderReductionImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/low-bandwidth/source-timestamp/SourceTimestampTransport.cpp
    )

# Statistics support
if (FASTDDS_STATISTICS)

    set(statistics_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/statistics/fastdds/domain/DomainParticipantImpl.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/statistics/fastdds/domain/DomainParticipantStatisticsListener.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/statistics/rtps/monitor-service/MonitorService.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/statistics/rtps/monitor-service/MonitorServiceListener.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/statistics/rtps/reader/StatisticsReaderImpl.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/statistics/rtps/StatisticsBase.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/statistics/rtps/writer/StatisticsWriterImpl.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/statistics/types/monitorservice_typesPubSubTypes.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/statistics/types/monitorservice_typesTypeObjectSupport.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/statistics/types/typesPubSubTypes.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/statistics/types/typesTypeObjectSupport.cxx
        )

    list(APPEND ${PROJECT_NAME}_source_files
        ${statistics_sources}
        )

endif()

# SHM Transport
if(IS_THIRDPARTY_BOOST_OK)
    list(APPEND ${PROJECT_NAME}_source_files
        ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/shared_mem/test_SharedMemTransport.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/shared_mem/SharedMemTransport.cpp
        )
endif()

# TLS Support
if(TLS_FOUND)
    list(APPEND ${PROJECT_NAME}_source_files
        ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/TCPChannelResourceSecure.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/TCPAcceptorSecure.cpp
        )
endif()

# Security sources
set(${PROJECT_NAME}_security_source_files
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/security/exceptions/SecurityException.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/security/common/SharedSecretHandle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/security/logging/Logging.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/security/logging/LoggingLevel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/security/SecurityManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/security/SecurityPluginFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/participant/DS/PDPSecurityInitiatorListener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/security/authentication/PKIDH.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/security/accesscontrol/Permissions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/security/accesscontrol/DistinguishedName.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/security/cryptography/AESGCMGMAC.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/security/cryptography/AESGCMGMAC_KeyExchange.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/security/cryptography/AESGCMGMAC_KeyFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/security/cryptography/AESGCMGMAC_Transform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/security/cryptography/AESGCMGMAC_Types.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/security/authentication/PKIIdentityHandle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/security/authentication/PKIHandshakeHandle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/security/accesscontrol/AccessPermissionsHandle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/security/accesscontrol/CommonParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/security/accesscontrol/GovernanceParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/security/accesscontrol/PermissionsParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/security/logging/LogTopic.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/security/artifact_providers/FileProvider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/security/artifact_providers/Pkcs11Provider.cpp
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
        ${CMAKE_CURRENT_SOURCE_DIR}/fastdds.rc
        )
endif()


#SQLITE3 persistence service sources
set(${PROJECT_NAME}_sqlite3_source_files
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/persistence/SQLite3PersistenceService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/persistence/sqlite3.c
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

# Payload compression transport
if(BZIP2_FOUND OR ZLIB_FOUND)
    list(APPEND ${PROJECT_NAME}_source_files
        rtps/transport/low-bandwidth/payload-compression/PayloadCompressionTransport.cpp
        rtps/transport/low-bandwidth/payload-compression/PayloadCompressionImpl.cpp
    )
endif()

if(ANDROID)
    if((ANDROID_PLATFORM LESS_EQUAL 23) OR (ANDROID_NATIVE_API_LEVEL LESS_EQUAL 23))
        list(APPEND ${PROJECT_NAME}_source_files
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
    endif()
endif()
