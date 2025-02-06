# Migration Guide from Fast DDS v2 to Fast DDS v3

This document aims to help during the migration process from eProsima *Fast DDS version* 2 to *Fast DDS version* 3.
For more information about all the updates, please refer to the [release notes](https://fast-dds.docs.eprosima.com/en/latest/notes/notes.html).

> [!WARNING]
> Fast DDS v3 introduces a new feature [XTypes](https://fast-dds.docs.eprosima.com/en/latest/fastdds/xtypes/xtypes.html#xtypes) that allows to discover remote types.
> In consequence, discovery traffic can be increased during start up.
> If you are experiencing high load during discovery, try disabling the new feature.
> Please refer to [disable type propagation](https://fast-dds.docs.eprosima.com/en/latest/fastdds/property_policies/non_consolidated_qos.html#type-propagation) to learn how to do it.

## Migration Steps

The following steps describe the possible changes that your project may require to migrate to *Fast DDS v3.0.0*:

- [Step 1: Update the Package Name and CMake Configuration](#step-1-update-the-package-name-and-cmake-configuration)
- [Step 2: Update Dependencies](#step-2-update-dependencies)
- [Step 3: Ensure Compability with Related Products](#step-3-ensure-compability-with-related-products)
- [Step 4: Apply Namespace Changes](#step-4-apply-namespace-changes)
- [Step 5: Migrate Public Headers](#step-5-migrate-public-headers)
- [Step 6: Handle Removed or Private Headers](#step-6-handle-removed-or-private-headers)
- [Step 7: Update API Methods](#step-7-update-api-methods)
- [Step 8: Update Structs, Enums, and Variables](#step-8-update-structs-enums-and-variables)
- [Step 9: Refactor Examples](#step-9-refactor-examples)

### Step 1: Update the Package Name and CMake Configuration

1. CMake Project Name: Rename the CMake project from `fastrtps` to `fastdds`.
2. Environment Variables:

    * Rename `FASTRTPS_DEFAULT_PROFILES_FILE` to `FASTDDS_DEFAULT_PROFILES_FILE`.
    * The configuration file for loading profiles is now `DEFAULT_FASTDDS_PROFILES.xml`.
3. Update CMake File Names on Windows:

    * Rename fastrtps.manifest.in to fastdds.manifest.in
    * Rename fastrtps-config.cmake to fastdds-config.cmake
    * Rename fastrtps.rc to fastdds.rc


### Step 2: Update Dependencies

Fast DDS v3 is only compatible with Fast CDR v2.
If you are not using Fast CDR as [third-party](https://fast-dds.docs.eprosima.com/en/latest/installation/configuration/cmake_options.html#third-party-libraries-options), please ensure that your local dependencies are up-to-date.
Refer to the [library deprendencies table](https://fast-dds.docs.eprosima.com/en/latest/notes/versions.html#library-dependencies) to verify version compatibility for all Fast DDS library dependencies.

### Step 3: Ensure Compability with Related Products

Fast DDS v3 requires Fast DDS Gen v4 for code generation. Make sure to regenerate types using this compatible version.

For other compatibility requirements with related products (e.g., Shapes Demo, Discovery Server), refer to the table of [products compatibility](https://fast-dds.docs.eprosima.com/en/latest/notes/versions.html#eprosima-products-compatibility), which outlines version compatibility across the Fast DDS ecosystem.
Verify these versions and update accordingly to avoid any integration issues.

### Step 4: Apply Namespace Changes

1. Namespace Migration:

    * Update all `eprosima::fastrtps::` namespace references to `eprosima::fastdds::`.
    * Move built-in topics `SubscriptionBuiltinTopicData`, `PublicationBuiltinTopicData` and `ParticipantBuiltinTopicData` from `fastdds::dds::builtin::` to `fastdds::dds::`.
    * Move `Duration_t` and `c_TimeInfinite` references to `dds::`.
    * Move `Time_t.hpp` references from `eprosima::fastdds::` to `eprosima::fastdds::dds`.

Ensure you update these namespace references across your code to avoid compilation errors.

2. Renamed Types:

    * Change `EventKindBits::` references to`EventKind::`.
    * Change `EventKindEntityId::` references to`EntityId::`.
    * Change `StatisticsEventKind::` references to `statistics::EventKind::`.

Refactor the type references as outlined above to maintain compatibility with the new version.

### Step 5: Migrate Public Headers

1. File Location headers:

    All the headers in `include/fastrtps` were migrated to `include/fastdds`.
    In particular, the following list includes headers that have been relocated to different paths or whose implementations have been incorporated into other headers:

    | Fast DDS 2.x file *include* path | Fast DDS v3.0.0 file *include* path |
    |----------------------------------|-------------------------------------|
    | fastdds/rtps/resources/ResourceManagement.hpp | fastdds/rtps/attributes/ResourceManagement.hpp |
    | fastrtps/eProsima_auto_link.h | fastdds/fastdds_auto_link.hpp |
    | fastrtps/attributes/ParticipantAttributes.h | fastdds/rtps/DomainParticipantQos.hpp |
    | fastrtps/Domain.h | fastdds/dds/domain/DomainParticipantFactory.hpp |
    | fastrtps/log/Log.h | fastdds/dds/log/Log.hpp |
    | fastrtps/qos/DeadlineMissedStatus.h	| fastdds/dds/core/status/DeadlineMissedStatus.hpp |
    | fastrtps/qos/IncompatibleQosStatus.hpp | fastdds/dds/core/status/IncompatibleQosStatus.hpp |
    | fastrtps/qos/LivelinessChangedStatus.h | fastdds/dds/core/status/LivelinessChangedStatus.hpp |
    | fastrtps/qos/QosPolicies.h | fastdds/dds/core/policy/QosPolicies.hpp |
    | fastrtps/qos/ReaderQos.h | fastdds/dds/subscriber/qos/ReaderQos.hpp |
    | fastrtps/qos/WriterQos.h | fastdds/dds/publisher/qos/WriterQos.hpp |
    | fastrtps/qos/SampleRejectedStatus.hpp | fastdds/dds/core/status/SampleRejectedStatus.hpp |
    | fastrtps/participant/Participant.h | fastdds/rtps/participant/RTPSParticipant.hpp |
    | fastrtps/transport/TCPv4TransportDescriptor.h | fastdds/rtps/transport/TCPv4TransportDescriptor.hpp |
    | fastrtps/transport/TCPv6TransportDescriptor.h| 	fastdds/rtps/transport/ TCPv6TransportDescriptor.hpp |
    | fastrtps/transport/UDPv4TransportDescriptor.h | fastdds/rtps/transport/ UDPv4TransportDescriptor.hpp |
    | fastrtps/transport/UDPv6TransportDescriptor.h | fastdds/rtps/transport/ UDPv6TransportDescriptor.hpp |
    | fastrtps/transport/UDPTransportDescritpor.h | fastdds/rtps/transport/UDPTransportDescritpor.hpp |
    | fastrtps/transport/TCPTransportDescritpor.h | fastdds/rtps/transport/TCPTransportDescritpor.hpp |
    | fastdds/rtps/common/Time_t.hpp in namespace{fastdds} | fastdds/dds/core/Time_t.hpp in namespace{fastdds::dds} |

    Also, the `fixed_size_string.hpp` implementation has been migrated from Fast DDS package to Fast CDR.

2. File Extentions:

    Rename file extensions from `.h` to `.hpp`.

### Step 6: Handle Removed or Private Headers

The following list contains headers that were previously in the `include` folder and have been relocated to the `src/cpp` folder.
Since they are no longer public, it is not possible to include them in external projects:

* ParticipantAttributes.hpp
* ReplierAttributes.hpp
* RequesterAttributes.hpp
* PublisherAttributes.hpp
* SubscriberAttributes.hpp
* ProxyPool.hpp
* Semaphore.hpp
* MessageReceiver.hpp
* BuiltinProtocols.hpp
* shared_mutex.hpp
* StringMatching.hpp
* TimeConversion.hpp
* DBQueue.hpp
* ResourceEvent.hpp
* TimedEvent.hpp
* WriterProxyData.hpp
* ReaderProxyData.hpp
* ParticipantProxyData.hpp
* XML Parser API
* UnitsParser.hpp
* RTPSMessageGroup.hpp
* RTPSMessageCreator.hpp
* CDRMessage.hpp
* StatefulPersistentReader.hpp
* StatefulReader.hpp
* StatelessPersistentReader.hpp
* StatelessReader.hpp
* PersistentWriter.hpp
* StatefulPersistentWriter.hpp
* StatefulWriter.hpp
* StatelessPersistentWriter.hpp
* StatelessWriter.hpp
* logging.h
* Exception.h
* Cryptography.h
* Authentication.h
* AccessControl.h
* SecurityException.h
* ChangeForReader.hpp
* ReaderLocator.hpp
* ReaderProxy.hpp
* ServerAttributes.hpp
* TopicAttributes.hpp
* TypeLookupService.hpp

If your project previously included any of these headers, you will need to modify your implementation.
Since these headers are now private, you should replace their usage with public alternatives or refactor the related code to ensure it does not depend on private headers.

### Step 7: Update API Methods

The table below contains the list of API changes, showing the previous methods and the corresponding new ones introduced in Fast DDS v3.
The new API methods achieve the same functionality, even though the signature of the method is different from the deprecated one.

|        Deprecated methods           |             New methods                |
|----------------------------------|-------------------------------------|
| xmlparser::XMLProfileManager::library_settings(`LibrarySettingsAttributes`&) | DomainParticipantFactory::get_instance()->set_library_settings(const `LibrarySettings`&) |
| fill_discovery_data_from_cdr_message(`ReaderProxyData`&, `MonitorServiceStatusData`&) |fill_discovery_data_from_cdr_message(`SubscriptionBuiltinTopicData`&, `MonitorServiceStatusData`&) |
| fill_discovery_data_from_cdr_message(`WriterProxyData`&, `MonitorServiceStatusData`&) | fill_discovery_data_from_cdr_message(`PublicationBuiltinTopicData`&,`MonitorServiceStatusData`&) |
| fill_discovery_data_from_cdr_message(`ParticipantProxyData`&, `MonitorServiceStatusData`&) | fill_discovery_data_from_cdr_message(`ParticipantBuiltinTopicData`&,`MonitorServiceStatusData`&) |
| on_participant_discovery(`DomainParticipant`*, `ParticipantDiscoveryInfo`&&, bool) |on_participant_discovery(`DomainParticipant`*, `ParticipantDiscoveryStatus`, `ParticipantBuiltinTopicData`&, bool&) |
| on_subscriber_discovery(`DomainParticipant`*, `ReaderDiscoveryInfo`&&, bool) | on_data_reader_discovery(`DomainParticipant`*, `ReaderDiscoveryStatus`, `SubscriptionBuiltinTopicData`&, bool&) |
| on_publisher_discovery(`DomainParticipant`*, `WriterDiscoveryInfo`&&, bool) | on_data_writer_discovery(`DomainParticipant`*, `WriterDiscoveryStatus`, `PublicationBuiltinTopicData`&, bool&) |
| onReaderDiscovery(`RTPSParticipant`*, `ReaderDiscoveryInfo`&&, bool) | on_reader_discovery(`RTPSParticipant`*,  `ReaderDiscoveryStatus`, `SubscriptionBuiltinTopicData`&, bool&) |
| onWriterDiscovery(`RTPSParticipant`*, `WriterDiscoveryInfo`&&, bool) | on_writer_discovery(`RTPSParticipant`*, `WriterDiscoveryStatus`, `PublicationBuiltinTopicData`&, bool&) |
| onParticipantDiscovery(`RTPSParticipant`*, `ParticipantDiscoveryInfo`&&, bool) | on_participant_discovery(`RTPSParticipant`*, `ParticipantDiscoveryStatus`, `ParticipantBuiltinTopicData`&, bool&) |
| XMLProfileManager::loadXMLFile(string&) | DomainParticipantFactory::get_instance()->load_XML_profiles_file(string) |
| XMLProfileManager::loadDefaultXMLFile() | load_profiles() |
| XMLProfileManager::loadXMLFile(string) | load_XML_profiles_file(string&) |
| XMLProfileManager::loadXMLString(const char*, size_t) | load_XML_profiles_string(const char*, size_t) |
| XMLProfileManager::fillParticipantAttributes(const string&, `ParticipantAttributes`&, bool) | get_participant_qos_from_profile(string&, `DomainParticipantQos`&) |
|DynamicTypeBuilder XMLProfileManager::getDynamicTypeByName(string&) | get_dynamic_type_builder_from_xml_by_name(string&, `DynamicTypeBuilder::_ref_type`&) |
| XMLProfileManager::fillRequesterAttributes(string&, RequesterAttributes&) | get_requester_qos_from_profile(string&, RequesterQos&) |
| XMLParser::getXMLThroughputController(`tinyxml2::XMLElement`*, `ThroughputControllerDescriptor`&, uint8_t) | XMLParser::getXMLFlowControllerDescriptorList(`tinyxml2::XMLElement`*, `FlowControllerDescriptorList`&, uint8_t) |
| add_throughput_controller_descriptor_to_pparams(`FlowControllerSchedulerPolicy`, uint32_t, uint32_t) | add_flow_controller_descriptor_to_pparams(`FlowControllerSchedulerPolicy`, uint32_t, uint32_t) |
| get_payload(uint32_t, `CacheChange_t`&) | get_payload(uint32_t, `SerializedPayload_t`&) |
| release_payload(`CacheChange_t`&) | release_payload(`SerializedPayload_t`&) |
| registerWriter(`RTPSWriter`*, const `TopicAttributes`&, const `WriterQos`&) |  register_writer(`RTPSWriter`*, cosnt `PublicationBuiltinTopicData`&) |
| registerReader(`RTPSReader`*, `TopicAttributes`&, `ReaderQos`&) |  register_reader(`RTPSReader`*, const `SubscriptionBuiltinTopicData`&, const `ContentFilterProperty`*) |
| updateWriter(`RTPSWriter`*, const `TopicAttributes`&, const `WriterQos`&) | update_writer(`RTPSWriter`*, const `WriterQos`&) |
| updateReader(`RTPSReader`*, const `TopicAttributes`&, const `ReaderQos`&, const `ContentFilterProperty`*) | update_reader(`RTPSReader`*, const `ReaderQos`, const ContentFilterProperty*) |
| getRTPSParticipantAttributes() | get_attributes() |
| bool write(void*) | ReturnCode_t write(void*) |
| bool write(void*, `WriteParams`&) | ReturnCode_t write(void*, `WriteParams`&) |
| SenderResource::send(const octet*, uint32_t, `LocatorsIterator`*, `LocatorsIterator`*, const chrono::steady_clock::time_point&) | SenderResource::send(`vector<NetworkBuffer>`, uint32_t, `LocatorsIterator`*, `LocatorsIterator`*, const chrono::steady_clock::time_point&) |
| RTPSMessageSenderInterface::send(`CDRMessage_t`*, chrono::steady_clock::time_point) | RTPSMessageSenderInterface::send(`vector<NetworkBuffer>`&, uint32_t&, chrono::steady_clock::time_point) |
| createRTPSWriter(`RTPSParticipant`*, `EntityId_t`&, `WriterAttributes`&, `shared_ptr<IPayloadPool>`&, `shared_ptr<IChangePool>`&, `WriterHistory`*, `WriterListener`*) | createRTPSWriter(`RTPSParticipant`*, `WriterAttributes`&, `WriterHistory`*, `WriterListener`*) |
| RTPSWriter::new_change(const `function<uint32_t()>& dataCdrSerializedSize`, `ChangeKind_t`, `InstanceHandle_t`) | WriterHistory::create_change(uint32_t, `ChangeKind_t`, `InstanceHandle_t`) |
| RTPSWriter::new_change(`ChangeKind_t`, `InstanceHandle_t`) | WriterHistory::create_change(`ChangeKind_t`, `InstanceHandle_t`) |
| RTPSWriter::release_change(`CacheChange_t`*) | WriterHistory::release_change(`CacheChange_t`*) |
| RTPSWriter::remove_older_changes(unsigned int) | WriterHistory::remove_min_change() |
| RTPSWriter::is_acked_by_all(const `CacheChange_t`*) | RTPSWriter::is_acked_by_all(const `SequenceNumber_t`&) |
| RTPSWriter::updateAttributes(const `WriterAttributes`&) | RTPSWriter::update_attributes(const `WriterAttributes`&)|
| RTPSWriter::getListener() | RTPSWriter::get_listener() |
| RTPSWriter::isAsync() | RTPSWriter::is_async() |
| WriterListener::onWriterMatched(`RTPSWriter`*,  `MatchingInfo`&) | WriterListener::on_writer_matched(`RTPSWriter`*, const `MatchingInfo`&) |
| WriterListener::onWriterChangeReceivedByAll(`RTPSWriter`*,  `CacheChange_t`*) | WriterListener::on_writer_change_received_by_all(`RTPSWriter`*,`CacheChange_t`*) |
| TypeLookupReplyListener::onWriterChangeReceivedByAll(`RTPSWriter`*, `CacheChange_t`*) | TypeLookupReplyListener::on_writer_change_received_by_all(`RTPSWriter`*, `CacheChange_t`*) |
| RTPSReader::getListener() | RTPSReader::get_listener() |
| RTPSReader::setListener() | RTPSReader::set_listener() |
| RTPSReader::expectsInlineQos() | RTPSReader::expects_inline_qos() |
| RTPSReader::isInCleanState() | RTPSReader::is_in_clean_state() |
| RTPSReader::getHistory() | RTPSReader::get_history() |
| RTPSReader::nextUnreadCache(`CacheChange_t`**, `WriterProxy` **) | RTPSReader::next_unread_cache() |
| RTPSReader::nextUntakenCache(`CacheChange_t`**, `WriterProxy` **) | RTPSReader::next_untaken_cache() |
| ReaderListener::onReaderMatched(`RTPSReader`*, `MatchingInfo`&) | ReaderListener::on_reader_matched(`RTPSReader`*, `MatchingInfo`&) |
| ReaderListener::onNewCacheChangeAdded(`RTPSReader`*, const `CacheChange_t`* const) | ReaderListener::on_new_cache_change_added(`RTPSReader`*, cont `CacheChange_t`* const) |
| TopicDataType::getSerializedSizeProvider(const void* const, `DataRepresentationId_t`) | TopicDataType::calculate_serialized_size(const void* const, `DataRepresentationId_t`) |
| TopicDataType::createData() | TopicDataType::create_data() |
| TopicDataType::deleteData(void*) | TopicDataType::delete_data(void*) |
| TopicDataType::getKey(const void* const, `InstanceHand`*, bool) | TopicDataType::compute_key(const void* const, `InstanceHand`&, bool) |
| TopicDataType::setName(const char*) | TopicDataType::set_name(const string&) |
| char* TopicDataType::getName() | string& TopicDataType::get_name() |
| TypeSupport::calculate_serialized_size_provider(const void* const, `DataRepresentationId_t`) | TypeSupport::calculate_serialized_size(const void* const, `DataRepresentationId_t`) |
|   get_key(void, `InstanceHandle_t`*, bool) | compute_key(`SerializedPayload_t`&, `InstanceHandle_t`&, bool) |
| DynamicPubSubType::createData() | DynamicPubSubType::create_data() |
| DynamicPubSubType::deleteData(void*) | DynamicPubSubType::delete_data(void*) |
| DynamicPubSubType::getKey(const void* const, `InstanceHand`*, bool) | DynamicPubSubType::compute_key(const void* const, `InstanceHand`&, bool) |
| DynamicPubSubType::getSerializedSizeProvider(const void* const, `DataRepresentationId_t`) | DynamicPubSubType::calculate_serialized_size(const void* const, `DataRepresentationId_t`) |

Review your code for any APIs marked with the `FASTDDS_DEPRECATED` and `FASTDDS_TODO_BEFORE` macros.
Note that these deprecated APIs have been removed in Fast DDS v3.
Make the necessary updates to your implementation to ensure compatibility with the new version.

### Step 8: Update Structs, Enums, and Variables

As part of the Fast DDS migration, several structs, enums, and variables have been updated. You will need to modify your code to reflect these changes:

1. Enum and Variable Changes:

    * `DiscoveryProtocol_t` is now `DiscoveryProtocol`.
    * Rename `initialHeartbeatDelay` to `initial_heartbeat_delay`.
    * Rename `heartbeatPeriod` to `heartbeat_period`.
    * Rename `nackResponseDelay` to `nack_response_delay`.
    * Rename `nackSupressionDuration` to `nack_supression_duration`.
    * Rename `heartbeatResponseDelay` to `heartbeat_response_delay`.
    * Rename `initialAcknackDelay` to `initial_acknack_delay`.
    * Rename `expectsInlineQos` to `expects_inline_qos`.
    * Rename `m_typeSize` to `max_serialized_type_size`.
    * Rename `m_isGetKeyDefined` to `is_compute_key_provided`.
    * Rename `m_topicDataTypeName` to `topic_data_typename`.

2. Extend Built-in Topics:

    * `SubscriptionBuiltinTopicData` has been extended with additional fields to mimic those of `ReaderProxyData`.
    * `PublicationBuiltinTopicData` has been extended with additional fields to mimic those of `WriterProxyData`.
    * `ParticipantBuiltinTopicData` has been extended to include the product version and fields from `ParticipantProxyData`.

3. Other Struct Changes:

    * `SendBuffersAllocationAttributes` has a new attribute to define the allocation configuration of the `NetworkBuffers`.
    * `TypeConsistencyQos` has been removed from `DataReader`, and the `TypeConsistencyEnforcementQosPolicy` and `DataRepresentationQosPolicy` have been added.

### Step 9: Refactor Examples

All examples have been refactored to follow a consistent structure across the Fast DDS project.
This includes renaming files, restructuring classes, and updating the overall format.
Additionally, it is important to note that some examples have been removed, renamed, or had significant changes to their options and configurations.
If you have integrated any of these examples into your own implementation, carefully review the updated examples to ensure compatibility with your project.

All the examples have been refactored to follow the same structure:
* File names, guards, and classes follow new format.
* Detailed and well-formed README.md with example explanation.
* Example structured in applications, stopped by `SIGTERM` signal.

#### Hello World
Refactor the HelloWorld example with the current new example format.
In this hello world example, the key changes are:
* The XML profile is loaded from the environment (if defined), and the `--env` CLI option has been removed.
* Add a subscriber implementing the waitsets mechanism.
* Provide XML profiles examples targeting several scenarios (e.g., SampleConfig_Controller, Events, Multimedia).

#### X-Types Examples
In this X-Types example, a type is defined at runtime on the publisher side using the Dynamic Types API.
The subscriber discovers the type, creates a reader for it, and prints the received data.
This example is type compatible with the Hello World example.

#### Configuration
In the configuration example, the key changes are:
* Included LargeData as an option (builtin transport argument).
* Included all previous QoS examples:
    * Deadline
    * Disable positive ACKs
    * Lifespan
    * Liveliness
    * Ownership (strength)
    * Partitions

#### Content Filter
Refactor the ContentFilteresTopicExample example with the current new example format.
In this content filter example, the main changes are:
* Added	option to select filter type: Default, Custom, or None.
* Customizable lower-bound and upper-bound options of the filter as arguments.
    * For the Custom filter, they represent the maximum and minimum values of the message indexes that are filtered out through the filter.
    * For the Default filter, they represent the maximum and minimum value message indexes that are read.

#### Custom Payload Pool
Refactor the CustomPayloadPoolExample example with the current new example format.

#### Delivery Mechanisms
In this delivery mechanisms example, the key changes are:
* Loans and data-sharing compatible: bounded types, final extensibility.
* Loans mechanism for data management.
* Option to select all delivery mechanisms.

#### Discovery Server
Refactor the DiscoveryServerExample example with the current new example format.

#### Flow Controller
Refactor the FlowControlExample example with the current new example format.
In this Flow Controller example, the key changes are:
* Publishers continuously send samples. The user can set the number of samples to send.
* User can set the following QoS and properties for the Flow Controller:
    * Scheduler policy used by the flow controller.
    * Maximum number of bytes to be sent to the network per period.
    * Period of time in milliseconds during which the flow controller is allowed to send the maximum number of bytes per period.
    * Property fastdds.sfc.priority.
    * Property fastdds.sfc.bandwidth_reservation.

#### Request-Reply
Refactor the Request-Reply example with the current new example format.

#### Static EDP Discovery
Refactor the Static EDP Discovery example with the new example format.

#### Security
Refactor the SecureHelloWorld example with the current new example format.

#### RTPS Entities
Refactor the rtps/Registered example with the current new example format.
This RTPS example demonstrates a basic RTPS deployment.
The main change is that serialization and deserialization are done with overload methods from fastcdr.
