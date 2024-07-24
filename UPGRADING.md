# Migration Guide to Fast DDS 3.0.x

This document aims to help during the migration process from *eProsima Fast DDS* version 2.x to *Fast DDS v3.0.0*.
The new release includes more changes than the included in this document, so refer to the [release notes](https://fast-dds.docs.eprosima.com/en/latest/notes/notes.html) for further information.

As announced in previous minor releases, it is always *recommended* to regenerate the types (if applies) with the latest *eProsima Fast DDS Gen* release.
In this case, the type regeneration is **required** (if applies), using *Fast DDS Gen* v4.0.0.

The following sections describe in detail the possible changes that your project would require to migrate to *Fast DDS v3.0.0*.

- [Library management](#library-management)
- [Namespace migrations and changes](#namespace-migrations-and-changes)
- [Fast RTPS public headers migration to Fast DDS](#fast-rtps-public-headers-migration-to-fast-dds)
- [Headers migrated to private](#headers-migrated-to-private)
- [API changes](#api-changes)
- [Struct, Enum, Variable](#struct-enum-variable)
- [Examples](#examples)
- [Compatibility with Fast CDR](#compatibility-with-fast-cdr)


## Library management

The below list of changes expose the changes related to the library name, library environment variables and other library usages.

* The package has been renamed to `fastdds`.
* Deprecated APIs marked with `FASTDDS_DEPRECATED` and `FASTDDS_TODO_BEFORE` macros have been removed.
* XML profiles loading environment variables have been renamed to: `FASTDDS_DEFAULT_PROFILES_FILE`.
* The configuration file has been renamed to `DEFAULT_FASTDDS_PROFILES.xml`.
* XML Schema namespace in fastdds_profiles.xsd has been updated to http://www.eprosima.com.
* The API exporting macro has been renamed to `FASTDDS_EXPORTED_API`.
* CMake Windows file names have been changed:

    * fastdds.manifest.in
    * fastdds-config.cmake

* The fastrtps.rc file has been renamed as fastdds.rc.


## Namespace migrations and changes

The following list contains the namespace changes and migrations:

* All `eprosima::fastrtps` namespace references were migrated to `eprosima::fastdds::rtps`.
* `SubscriptionBuiltinTopicData`, `PublicationBuiltinTopicData` and `ParticipantBuiltinTopicData` were migrated from `fastdds::dds::builtin` to `fastdds::dds`.
* `EventKindBits` references changed to`EventKind`.
* `EventKindEntityId` references changed to`EntityId`.
* `StatisticsEventKind` references changed to `statistics::EventKind`.
* `Duration_t` reference moved to `fastdds::dds`


## Fast RTPS public headers migration to Fast DDS

All public header extensions have been changed to `.hpp`.
Also, the `fixed_size_string.hpp` feature has been migrated from Fast DDS package to Fast CDR.
All the headers in `include/fastrtps` were migrated to `include/fastdds`.
In particular, he following list includes headers that have been relocated to different paths or whose implementations have been incorporated into other headers.

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


## Public headers migrated to private

The following list contains the files that were in the `include` folder that have been migrated to `src/cpp` folder:

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


## API changes

|        Deprecated APIs           |             New APIs                |
|----------------------------------|-------------------------------------|
| **xmlparser::XMLProfileManager::library_settings(LibrarySettingsAttributes)** | **DomainParticipantFactory::get_instance()->set_library_settings(LibrarySettings)** |
| **fill_discovery_data_from_cdr_message(`ReaderProxyData`, `MonitorServiceStatusData`)** |**fill_discovery_data_from_cdr_message(`SubscriptionBuiltinTopicData`, `MonitorServiceStatusData`)** |
| **fill_discovery_data_from_cdr_message(`WriterProxyData`, `MonitorServiceStatusData`)** | **fill_discovery_data_from_cdr_message(`PublicationBuiltinTopicData`,`MonitorServiceStatusData`)** |
| **fill_discovery_data_from_cdr_message(`ParticipantProxyData`, `MonitorServiceStatusData`)** | **fill_discovery_data_from_cdr_message(`ParticipantBuiltinTopicData`,`MonitorServiceStatusData`)** |
| **on_participant_discovery(`DomainParticipant`, `ParticipantDiscoveryInfo`, bool)** |**on_participant_discovery(`DomainParticipant`, `ParticipantDiscoveryStatus`, `ParticipantBuiltinTopicData`, `should_be_ignored`)** |
| **on_subscriber_discovery(`DomainParticipant`, `ReaderDiscoveryInfo`, bool)** | **on_data_reader_discovery(`DomainParticipant`, `ReaderDiscoveryStatus`, `SubscriptionBuiltinTopicData`, `should_be_ignored`)** |
| **on_publisher_discovery(`DomainParticipant`, `WriterDiscoveryInfo`, bool)** | **on_data_writer_discovery(`DomainParticipant`, `WriterDiscoveryStatus`, `PublicationBuiltinTopicData`, `should_be_ignored`)** |
| **onReaderDiscovery(`RTPSParticipant`, `ReaderDiscoveryInfo`, bool)** | **on_reader_discovery(`RTPSParticipant`,  `ReaderDiscoveryStatus`, `SubscriptionBuiltinTopicData`, bool)** |
| **onWriterDiscovery(`RTPSParticipant`, `WriterDiscoveryInfo`, bool)** | **on_writer_discovery(`RTPSParticipant`, `WriterDiscoveryStatus`, `PublicationBuiltinTopicData`, bool)** |
| **onParticipantDiscovery(`RTPSParticipant`, `ParticipantDiscoveryInfo`, bool)** | **on_participant_discovery(`RTPSParticipant`, `ParticipantDiscoveryStatus`, `ParticipantBuiltinTopicData`, bool)** |
| **xmlparser::XMLProfileManager::loadXMLFile(string)** | **DomainParticipantFactory::get_instance()->load_XML_profiles_file(string)** |
| **xmlparser::XMLProfileManager::loadDefaultXMLFile()** | **load_profiles()** |
| **xmlparser::XMLProfileManager::loadXMLFile(string)** | **load_XML_profiles_file(string)** |
| **xmlparser::XMLProfileManager::loadXMLString(const char, size_t)** | **load_XML_profiles_string(const char, size_t)** |
| **xmlparser::XMLProfileManager::fillParticipantAttributes(string, `ParticipantAttributes`, bool)** | **get_participant_qos_from_profile(string, `DomainParticipantQos`)** |
|**xmlparser::XMLProfileManager::getDynamicTypeBuilderByName(`DynamicTypeBuilder::_ref_type`, string)** | **get_dynamic_type_builder_from_xml_by_name(`DynamicTypeBuilder::_ref_type`, string)** |
| **xmlparser::XMLProfileManager::fillRequesterAttributes(string, RequesterAttributes)** | **get_requester_qos_from_profile(string, RequesterQos)** |
| **XMLParser::getXMLThroughputController(`tinyxml2::XMLElement`, `ThroughputControllerDescriptor`, uint8_t)** | **XMLParser::getXMLFlowControllerDescriptorList(`tinyxml2::XMLElement`, `FlowControllerDescriptorList`, uint8_t)** |
| **add_throughput_controller_descriptor_to_pparams(`FlowControllerSchedulerPolicy`, uint32_t, uint32_t)** | **add_flow_controller_descriptor_to_pparams(`FlowControllerSchedulerPolicy`, uint32_t, uint32_t)** |
| **get_payload(uint32_t, `CacheChange_t`)** | **get_payload(uint32_t, `SerializedPayload_t`)** |
| **release_payload(`CacheChange_t`)** | **release_payload(`SerializedPayload_t`)** |

Moreover, `DataWriter::write()` methods return only `ReturnCode_t`.


## Struct, Enum, Variable

* Extend SubscriptionBuiltinTopicData with additional fields to mimic those of ReaderProxyData.
* Extend PublicationBuiltinTopicData with additional fields to mimic those of WriterProxyData.
* Extend  ParticipantBuiltinTopicData 	with additional fields to mimic those of ParticipantProxyData.
* DiscoveryProtocol_t	is DiscoveryProtocol.
* Extend SendBuffersAllocationAttributes with a new attribute defining the allocation configuration of the NetworkBuffers.


## Examples

### Hello World
Refactor the HelloWorld example with the current new example format.
In this hello world example, the key changes are:
* Removed --env option and set that environment behavior as default.
* Added subscriber waitsets class and its usage.
* Provided 	XML profiles examples that target several scenarios (e.g.,SampleConfig_Controller, Events, Multimedia).

### X-Types Examples
In this X-Types example, a type is defined at runtime on the publisher side using the Dynamic Types API, and the subscriber discovers the type, creates a reader for it, and prints the received data. This example is type compatible with the Hello World example, and a compatibility test has been added for this.

### Configuration
In this configuration example, the key changes are:
* Included LargeData as an option (builtin transport argument).
* Included all previous QoS examples:
    * Deadline
    * Disable positive ACKs
    * Lifespan
    * Liveliness
    * Ownership (strength)
    * Partitions

### Content Filter
Refactor the ContentFilteresTopicExample example with the current new example format.
In this content filter example, the main changes are:
* Added	option to select filter type: Default, Custom, or None.
* Customizable lower-bound and upper-bound options of the filter as arguments.
    * For the Custom filter, they represent the maximum and minimum values of the message indexes that are filtered out through the filter.
    * For the Default filter, they represent the maximum and minimum value message indexes that are read.

### Custom Payload Pool
Refactor the CustomPayloadPoolExample example with the current new example format.

### Delivery Mechanisms
In this delivery mechanisms example, the key changes are:
* Loans and data-sharing compatible: bounded types, final extensibility.
* Loans mechanism for data management.
* Option to select all delivery mechanisms.

### Discovery Server
Refactor the DiscoveryServerExample example with the current new example format.
This refactors the DiscoveryServerExample to the new example format. It also adds proper tests for the examples testing automation.

### Flow Controller
Refactor the FlowControlExample example with the current new example format.
In this Flow Controller example, the key changes are:
* Publishers continuously send samples. The user can set the number of samples to 	send.
* User can set the following QoS and properties for the Flow Controller:
    * Scheduler policy used by the flow controller.
    * Maximum number of bytes to be sent to the network per period.
    * Period of time in milliseconds during which the flow controller is allowed 	to send the maximum number of bytes per period.
    * Property fastdds.sfc.priority.
    * Property fastdds.sfc.bandwidth_reservation.

### Request-Reply
Refactor the Request-Reply example with the current new example format.

### Static EDP Discovery
Refactor the Static EDP Discovery example with the new example format.

### Security
Refactor the SecureHelloWorld example with the current new example format.
A security folder created in examples with a modified version of hello world, supporting security plugins.

### RTPS Entities
Refactor the rtps/Registered example with the current new example format.
This RTPS example demonstrates a basic RTPS deployment. The main change is that serialization and deserialization are done with overload methods from fastcdr.

## Compatibility with Fast CDR

Fast DDS v3.x.x is only compatible with Fast CDR v2.x.x.
