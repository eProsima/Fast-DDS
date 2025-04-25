Forthcoming
-----------

Version v3.2.2
--------------

* Improve `IDLParser` adding support to new types:
  * Add support for parsing sequence types.
  * Move logic for managing `state["type"]` changes to `action<scoped_name>`.
  * Refactor union types parsing to be similar to array types.

Version v3.2.0
--------------

* Added implementation for:
  * `DataWriter::get_matched_subscription_data()`
  * `DataWriter::get_matched_subscriptions()`
  * `DataReader::get_matched_publication_data()`
  * `DataReader::get_matched_publications()`
  * `DynamicTypeBuilderFactory::create_type_w_uri`
* Added ``Extended Incompatible QoS`` feature for monitor service.
* Machine UUID added to the Data(p) to check Participants in same host, instead of using GUIDs.
* Windows ci example testing infrastructure and `hello world` example.
* New property to configure the preferred key agreement algorithm.
* Refactor benchmark example.
* Extended CLI ``shm`` command with ``-f`` param to clean Data Sharing segments.
* New Easy Mode:
  * New `ROS2_EASY_MODE` environment variable
  * Extended CLI ``discovery`` command to handle Fast DDS daemon.
  * Added P2P builtin transport.
  * Added new `easy_mode_ip` XML tag and `easy_mode` setter in `WireProtocolConfigQos` for Easy Mode IP configuration through XML and code.
* Added RPC over DDS internal API:
  * New classes: `Service`, `Requester`, `Replier`, `ServiceTypeSupport`
  * Added methods in DomainParticipant public API
* Added builtin flow controller for builtin writers (PDP, EDP, WLP and Type Look Up Service).

Version v3.1.0
--------------

* Allow `PERSISTENT_DURABILITY` behaving as `TRANSIENT_DURABILITY`. Fallback to `TRANSIENT_LOCAL_DURABILITY` if no persistence guid is set.
* Fix DomainParticipantQos equality operator by using the new `DomainParticipantQos::compare_flow_controllers`.
* Add new XML QoS overloads for ``DomainParticipant``, ``DataWriter`` and ``DataReader``:
  * ``get_X_qos_from_xml`` (without profile name)
  * ``get_X_qos_from_xml`` (profile name given)
  * ``get_default_X_qos_from_xml``
* Add complete support for dynamic network interfaces.

Version v3.0.0
--------------

* Rename project to `fastdds`.
* Rename environment variable to `FASTDDS_DEFAULT_PROFILES_FILE` and rename default XML profiles file to `FASTDDS_DEFAULT_PROFILES`.
* Remove API marked as deprecated.
* Removed deprecated FastRTPS API tests.
* Removed no longer supported `FASTRTPS_API_TESTS` CMake options.
* RTPS layer APIs refactor:
  * RTPSReader, ReaderListener, ReaderAttributes:
    * Several methods that were meant for internal use have been removed from public API
    * All public methods now have `snake_case` names
    * All public attributes now have `snake_case` names
  * RTPSWriter, WriterListener, WriterAttributes, WriterHistory:
    * The responsibility of managing both the `CacheChange` and `Payload` pools has been moved to the `WriterHistory`.
    * Several methods that were meant for internal use have been removed from public API
    * All public methods now have `snake_case` names
    * All public attributes now have `snake_case` names
  * RTPSParticipant:
    * Some methods changed to `snake_case`: `register_reader`, `register_writer`, `update_reader`, `update_writer`.
	* Register methods take a `TopicDescription` instead of `TopicAttributes`.
	* Update methods no longer take `TopicAttributes`.
  * Endpoint creation fails if the `EntityId` is inconsistent with the topic kind.
* Discovery callbacks refactor:
  * on_participant_discovery now receives a `ParticipantDiscoveryStatus` and a `ParticipantBuiltinTopicData` instead of a `ParticipantDiscoveryInfo`
  * on_data_reader_discovery now receives a `ReaderDiscoveryStatus` and a `SubscriptionBuiltinTopicData` instead of a `ReaderDiscoveryInfo`
  * on_data_writer_discovery now receives a `WriterDiscoveryStatus` and a `PublicationBuiltinTopicData` instead of a `WriterDiscoveryInfo`
* New methods to get local discovery information:
  * `DataWriter::get_publication_builtin_topic_data`
  * `DataReader::get_subscription_builtin_topic_data`
* Public API that is no longer public:
  * XML Parser API no longer public.
  * ReaderProxyData
  * ReaderDiscoveryInfo
  * WriterProxyData
  * WriterDiscoveryInfo
  * ParticiantProxyData
  * ParticiantDiscoveryInfo
  * ParticipantAttributes
  * ReplierAttributes
  * RequesterAttributes
  * PublisherAttributes
  * SubscriberAttributes
  * TopicAttributes
  * All discovery implementation related API
  * ProxyPool
  * Semaphore
  * MessageReceiver
  * BuiltinProtocols
  * Liveliness implementation related API
  * shared_mutex
  * StringMatching
  * TimeConversion
  * TypeLookupService
  * DBQueue
  * UnitsParser
  * ReaderLocator
  * ReaderProxy
  * ChangeForReader
  * ServerAttributes
* Public API headers .h have been renamed to .hpp
* Added create participant methods that use environment XML profile for participant configuration.
* New TypeObjectRegistry class to register/query TypeObjects/TypeIdentifiers.
* New TypeObjectUtils class providing API to build and register TypeObjects/TypeIdentifiers.
* Refactor Dynamic Language Binding API according to OMG XTypes v1.3 specification.
* Refactor ReturnCode complying with OMG DDS specification.
* Calling `DataReader::return_loan` returns `ReturnCode_t::RETCODE_OK` both for empty sequences and for sequences that were not loaned.
* Refactor examples:
  * Hello world example with wait-sets and environment XML profiles.
  * X-Types example with dynamic type discovery and Hello world example compatibility.
  * Configuration example that condenses multiple QoS examples. Multiple configurations allowed through argument parsing.
  * Custom content filter example with lower and upper bounds for data based on the index.
  * Custom payload pool example that uses a user-defined payload pool instead of the default
  * Delivery mechanisms example with SHM, UDP, TCP, data-sharing and intra-process mechanisms.
  * Discovery server example.
  * Flow Controller example with `FlowControllersQos` and property settings.
  * Request-reply example to showcase RPC paradigms over Fast DDS.
  * Security example with environment XML profiles.
  * Static EDP discovery example to avoid EDP meta-traffic.
  * Topic instances example, compatible with _ShapesDemo_ app.
  * RTPS example to show the creation of entities in the RTPS layer.
* Removed `TypeConsistencyQos` from DataReader, and included `TypeConsistencyEnforcementQosPolicy` and `DataRepresentationQosPolicy`
* Added new `flow_controller_descriptor_list` XML configuration, remove `ThroughtputController`.
* Migrate `#define`s within `BuiltinEndpoints.hpp` to namespaced `constexpr` variables.
* Make `StdoutErrConsumer` the default log consumer.
* IPayloadPool refactor:
  * `payload_owner` moved from `CacheChange_t` to `SerializedPayload_t`.
  * `SerializedPayload_t` copies are now forbidden.
  * Refactor of `get_payload` methods.
* Use `PID_DOMAIN_ID` during PDP.
* Creation of RTPS messages refactor:
  * New Gather-send method is now used by default, avoiding an extra copy during the creation of the RTPS message.
  * New attribute in `SendBuffersAllocationAttributes` to configure allocation of `NetworkBuffer` vector.
  * `SenderResource` and Transport APIs now receive a collection of `NetworkBuffer` on their `send` method.
* Migrate fastrtps namespace to fastdds
* Migrate fastrtps `ResourceManagement` API from `rtps/resources` to `rtps/attributes`.
* `const` qualify all data related inputs in DataWriter APIs
* New `DomainParticipantExtendedQos` that includes both `DomainId` and `DomainParticipantQos` (extends `DomainParticipantFactory` API).
* Make Blackbox tests not include any private API.
* Remove all the private API include from Blackbox tests.
* Discovery Server refactor:
  * Clients and Servers no longer need a known GUID to connect between them.
  * Servers are now configured specifying a LocatorList, instead of a RemoteServerAttributes list.
  * Servers connect through a mesh topology. For a new server to join the topology, it must be connected to any server in it.
  * Servers only redirect discovery information of their direct clients.
  * Remote Discovery servers connection list can now be updated and modified at runtime without restrictions.
  * Fast DDS CLI has been updated to allow the creation of servers without GUID.
  * Servers are responsible of answering TypeLookupRequests of others servers when working with X-Types.
  * Backup server is not compatible with X-Types.
* Refactor in XML Parser to return DynamicTypeBuilder instead of DynamicType
* Setting vendor_id in the received CacheChange_t for Data and DataFrag.
* Added new DynamicData to JSON serializer (`json_serialize`).
* Added new DynamicType to IDL serializer (`idl_serialize`).
* DDS implementation of `eprosima::fastdds::Time_t` moved to `eprosima::fastdds::dds::Time_t`.
* `TopicDataType::auto_fill_type_information` has been removed in favor of `fastdds.type_propagation` participant property.
* Add new custom pid PID_PRODUCT_VERSION.
* SHM locator kind is now linked to Fast DDS' major version.

Version 2.14.0
--------------

* Added authentication handshake properties.
* Added methods OpenOutputChannels and CloseOutputChannels to TransportInterface with LocatorSelectorEntry argument.
* Added `netmask_filter`, `allowlist` and `blocklist` transport configuration options.
* Added extra configuration options for the builitin transports.
* Limit the amount of listening ports for TCP servers to 1.

Version 2.13.0
--------------

* Added monitor service feature.
* Enable configuration of thread setting for all threads.
* Added the possibility to define a listening port equal to 0 in TCP Transport
* Added support for TCP to Fast DDS CLI and environment variable
* Enable Discovery Server example through TCP.
* Define a discovery server' super client by environment variable.
* Added the possibility to define interfaces in the whitelist by interface name.
* Added configuration of builtin transports through DomainParticipantQos, environment variable and XML.
* Enable support for DataRepresentationQos to select the CDR encoding.

Version 2.12.0
--------------

* Added participant property to configure SHM transport metatraffic behavior.
  No metatraffic over SHM transport by default.
* Exposed custom payload pool on DDS endpoints declaration
* Processing environment variables on XML text
* Upgrade to support Fast CDR v2.0.0.
  Default encoding is XCDRv1 to maintain interoperability with previous Fast DDS versions.
  Some concerns:
    - Data type source code generated with Fast DDS-Gen v2 should be regenerated using Fast DDS-Gen v3.
    - **API break**. Changed a `MEMBER_INVALID` identifier from a `#define` to a `constexpr`.
      Although this is not a new major version, using a `#define` is a bad conduct which was decided to be changed.
      Note that new `constexpr` is inside namespace `eprosima::fastdds::types`.

Version 2.11.0
--------------

* Added Participant ignore local endpoints feature.
* Remove `FASTDDS_STATIC` CMake option.
  Please, use `BUILD_SHARED_LIBS=OFF` instead.
* Fixed exported symbols on ContentFilteredTopic (ABI break)
* Deprecated the DDS:Crypto:AES-GCM-GMAC plugin configuration through the DomainParticipant PropertyPolicyQos (security vulnerability).
* `DomainParticipantListener::on_participant_discovery` changed behavior (fix API break in 2.10.0).
* Included XML schema for static discovery profile configuration.
* Extend DynamicDataHelper API providing `print` overload with `std::ostream` parameter (API extension in Dynamic Types).
* TypeLookup Service configuration through XML.

Version 2.10.1
--------------

* Added `ignore_participant` implementation.

Version 2.10.0
--------------

* Enabled secure communications on Discovery Server (ABI break on RTPS layer)
* Added non-standard DataWriterListener callback `on_unacknowledged_sample_removed` (API extension on DDS layer).
* Added `RTPSWriter::has_been_delivered` virtual method (ABI break on RTPS layer).
* Refactor `StatefulWriter::get_disable_positive_acks` as virtual method of `RTPSWriter` (ABI break on RTPS layer).
* Network headers made private (ABI break on RTPS layer).
* Added ignore RTPS entity API in RTPSParticipant (ABI break on RTPS layer).
* Overload `PDP::removeWriterProxyData` and `PDP::removeReaderProxyData` (ABI break on RTPS layer).
* Overload RTPS discovery callbacks in RTPSParticipantListener (ABI break on RTPS layer).
* Overload DDS discovery callbacks in DomainParticipantListener (ABI break on DDS layer).
* Added on_incompatible_type to RTPS listeners (ABI break on RTPS layer).
* Added support for QNX 7.1 build.

Version 2.9.0
-------------

* Default memory management policy set to `PREALLOCATED_WITH_REALLOC_MEMORY_MODE` (behaviour change)
* Statistics metrics are only calculated/accumulated when their corresponding DataWriter is enabled (behaviour change)
* Added new log macros `EPROSIMA_LOG_INFO`, `EPROSIMA_LOG_WARNING` and `EPROSIMA_LOG_ERROR`, and change all old macros `logInfo`, `logWarning`, and `logError` in the project.
* Added `ENABLE_OLD_LOG_MACROS` CMake option to support disabling the compilation of old log macros `logInfo`, `logWarning`, and `logError`.
* FASTDDS_STATISTICS build option set to ON by default
* Added XML profile validation option as a CLI new verb task: `"fastdds xml validate <xml_file(s)_path>"`. Added tests due to this new feature.

Version 2.8.0
-------------

* Added API get the WAN address of TCPv4 transport descriptors (API extension)
* Support `propagate` attribute for Properties in PropertyQoSPolicies so they could be
  set by user and sent in PDP
* Added possibility to configure Ownership and Ownership Strength QoS Policies from XML profiles file
* Added Server Name Indication (SNI) support for TLS-TCP communication
* Changes to allow running tests on Android emulator or device
* Added configuration settings for announcing locators on external LANs (ABI break)

Version 2.7.1
-------------

* ReadCondition implementation according to DDS version 1.4 standard document
* Added a new CMake option to allow users to force the use of our third party shared_mutex

Version 2.7.0
-------------

* Implementation of DataWriter methods write_w_timestamp, register_instance_w_timestamp,
  unregister_instance_w_timestamp, and dispose_w_timestamp (ABI break)
* Support of `SampleRejectedStatus` in DDS API (API extensions on RTPS layer)
* Implementation of DomainParticipant method find_topic
* Include Server Name Indication (SNI) empty API (ABI break on transport layer)
* CacheChange_t destructor made virtual (ABI break on RTPS layer)
* Added DDS APIs for ReadCondition (API extension)
* Added bulk notification interface to RTPS reader listener (API extension)
* Refactor of auxiliary utils class DBQueue (ABI break)
* Added configuration data for external locators feature (ABI break)

Version 2.6.1
-------------

* Writer side content filter.
* Implementation for DataWriter::get_key_value.
* Implementation for DataReader::lookup_instance.
* Support of `SampleLostStatus` in DDS API.

Version 2.6.0
-------------

* New TransportInterface and NetworkFactory API to allow updating the network interfaces at runtime (ABI breaks on RTPS
  and transport layers)
* Removed dll export for constructors and destructors of factory created entities (breaks ABI)
* Allow modifying the remote server locator in runtime.
* Add physical information in DATA[p] using properties
* Extension of `DISCOVERY_TOPIC` to include physical information about the discovered entity (ABI break)
* Added methods getting `fastdds::Time_t` as parameters instead of `fastdds::rtps::Time_t` (API extension, API
  deprecations).
* Changed signature of eprosima::fastdds::dds::DataWriter::dispose_w_timestamp (ABI break).
* Added method getting `std::vector<InstanceHandle_t>&` instead of `std::vector<InstanceHandle_t*>&` (API extension, API
  deprecations).
* Added RTPS APIs for content filter discovery information (API extension)
* Added RTPS APIs for endpoint discovery (API extension)
* Added RTPS APIs for on_sample_lost feature (API extension)

Version 2.5.0
-------------

* Allow zero-valued InstanceHandle_t (ABI break)
* Allow concatenation of transports (new exported symbols + ABI breaks on transport layer)
* New DomainParticipantFactory API for loading XML profiles from a string data buffer (extends DomainParticipantFactory
  API, implies ABI break)
* New DataWriter API allowing to wait for acknowledgements for a specific instance (extends DataWriter API, implies ABI
  break)
* Generation of GUID on entity creation (ABI break on RTPS layer)
* New DataReader history with correct implementation of instance_state and view_state (ABI break on RTPS layer)
* Support for PKCS#11 format URIs for private keys

Version 2.4.0
-------------

* Implementation of WaitSet, GuardCondition and StatusCondition.
* Flow controllers (ABI breaks on RTPS layer).
* Resolve DNS names on locators.
* Allow addition of discovery servers in runtime (ABI breaks on RTPS layer).
* Allow setting custom folder for data sharing files.
* Allow setting persistence guid with static discovery.

Version 2.3.0
-------------

* New Fast DDS Statistics module
* New discovery "super-client" kind
* Added methods to get sending locators for writers and listening locators for readers (ABI break)
* Added support for unique network flows
* Added reception_timestamp to `eprosima::fastdds::dds::SampleInfo` (ABI break)
* Added `eprosima::fastdds::dds::DataReader::get_unread_count` (ABI break)
* Refactor `eprosima::fastdds::type::ReturnCode_t`. Now the constant global objects are no longer available (ABI break)
* Performance tests refactored to use DDS-PIM high-level API

Version 2.2.0
-------------

* TopicDataType interface extended (ABI break)
* Upgrade to Quality Level 1
* New DataWriter API for loaning samples (extends DataWriter API, implies ABI break)
* New template classes for loanable sequences
* Added DataReader read and take APIs (implies ABI break)
* Complete DDS traditional C++ API (implies ABI breaks)
* Data sharing delivery (ABI breaks)

Version 2.1.0
-------------

This minor release is API compatible with the previous minor release, but introduces **ABI breaks** on
two of the three public APIs:
* Methods and attributes have been added on several classes of the DDS-PIM high-level API, so indexes of
  symbols on dynamic libraries may have changed
* Methods and attributes have been added on several classes of the RTPS low-level API, so indexes of
  symbols on dynamic libraries may have changed
* Old Fast-RTPS high-level API remains ABI compatible.

Users of the RTPS low-level API should also be aware of the following **API deprecations**:
* History::reserve_Cache has been deprecated
  * Methods RTPSWriter::new_change or RTPSReader::reserveCache should be used instead
* History::release_Cache has been deprecated
  * Methods RTPSWriter::release_change or RTPSReader::releaseCache should be used instead

This release adds the follwing **features**:
* Support persistence for large data
* Added support for `on_requested_incompatible_qos` and `on_offered_incompatible_qos`
* SKIP_DEFAULT_XML environment variable
* Added FORCE value to THIRDPARTY cmake options
* New log consumer (StdOutErrConsumer)
* Added methods to get qos defined in XML Profile
* Support for persistence on TRANSIENT_LOCAL

It also includes the following **improvements**:
* Internal refactor for intra-process performance boost
* Allow usage of foonathan/memory library built without debug tool
* Large data support on performance tests
* Reduced flakiness of several tests

Some important **bugfixes** are also included:
* Fixed behavior of several DDS API methods
* Fixed interoperability issues with RTI connext
* Fixed DLL export of some methods
* Avoid redefinition of compiler defined macros
* Fixed some intra-process related segmentation faults and deadlocks
* Fixed large data payload protection issues on intra-process
* Fixed C++17 and VS 2019 warnings
* Fixed linker problems on some platforms
* Fixed transient local retransmission after participant drop
* Fixed assertion failure on persistent writers

Version 2.0.2
-------------

This release includes the following **improvements**:

* Improve QNX support
* Security improvements
* Fast DDS Quality Declaration (QL 2)
* Large traffic reduction when using Discovery Server (up to 85-90% for large deployments)
* Configuration of Clients of Discovery Server using an environment variable
* A CLI for Fast DDS:
  * This can be used to launch a discovery server
  * Clean SHM directories with one command
* Shared memory transport enabled by default
* Solved edge-case interoperability issue with CycloneDDS

Version 2.0.1
-------------

This release includes the following bug fixes:

* Fixed sending GAPs to late joiners
* Fixed asserting liveliness on data reception
* Avoid calling OpenSSL_add_all_algorithms when not required

Other improvements:

* Fixing warnings

Version 2.0.0
-------------

This release has the following **API breaks**:

* eClock API, which was deprecated on v1.9.1, has been removed
* `eprosima::fastdds::rtps::RTPSDomain::createParticipant` methods now have an additional first argument `domain_id`
* Data member `domainId` has been removed from `eprosima::fastdds::rtps::RTPSParticipantAttributes` and added to
  `eprosima::fastdds::ParticipantAttributes`

Users should also be aware of the following **deprecation announcement**:

* All classes inside the namespace `eprosima::fastdds` should be considered deprecated.
  Equivalent functionality is offered through namespace `eprosima::fastdds`.
* Namespaces beneath `eprosima::fastdds` are not included in this deprecation, i.e.
  `eprosima::fastdds::rtps` can still be used)

This release adds the following **features**:

* Added support for register/unregister/dispose instance
* Added DDS compliant API. This new API exposes all the functionality of the Publisher-Subscriber Fast RTPS API
  adhering to the `Data Distribution Service (DDS) version 1.4 specification <https://www.omg.org/spec/DDS/1.4>`_
* Added Security Logging Plugin (contributed by Cannonical Ltd.)
* Bump to FastCDR v1.0.14

It also includes the following bug fixes and improvements:

* Support for OpenSSL 1.1.1d and higher
* Support for latest versions of gtest
* Support for FreeBSD
* Fault tolerance improvements to Shared Memory transport
* Fixed segfault when no network interfaces are detected
* Correctly ignoring length of `PID_SENTINEL` on parameter list
* Improved traffic on PDP simple mode
* Reduced CPU and memory usage

Version 1.10.0
--------------

This release adds the following features:

* New built-in :ref:`comm-transports-shm`
* Transport API refactored to support locator iterators
* Added subscriber API to retrieve info of first non-taken sample
* Added parameters to fully avoid dynamic allocations
* History of built-in endpoints can be configured
* Bump to FastCDR v1.0.13.
* Bump to Fast-RTPS-Gen v1.0.4.
* Require CMake 3.5 but use policies from 3.13

It also includes the following bug fixes and improvements:

* Fixed alignment on parameter lists
* Fixed error sending more than 256 fragments.
* Fix handling of STRICT_REALTIME.
* Fixed submessage_size calculation on last data_frag.
* Solved an issue when recreating a publishing participant with the same GUID.
* Solved an issue where a publisher could block on write for a long time when a new
  subscriber (late joiner) is matched, if the publisher had already sent a large number
  of messages.
* Correctly handling the case where lifespan expires at the same time on several samples.
* Solved some issues regarding liveliness on writers with no readers.
* Correctly removing changes from histories on keyed topics.
* Not reusing cache change when sample does not fit.
* Fixed custom wait_until methods when time is in the past.
* Several data races and ABBA locks fixed.
* Reduced CPU and memory usage.
* Reduced flakiness of liveliness tests.
* Allow for more use cases on performance tests.

Several bug fixes on discovery server:

* Fixed local host communications.
* Correctly trimming server history.
* Fixed backup server operation.
* Fixed timing issues.

Version 1.9.4
-------------

This release adds the following features:

* Intra-process delivery mechanism is now active by default.
* Synchronous writers are now allowed to send fragments.
* New memory mode DYNAMIC_RESERVE on history pool.
* Performance tests can now be run on Windows and Mac.
* XML profiles for requester and replier.

It also includes the following bug fixes and improvements:

* Bump to FastCDR v1.0.12.
* Bump to Fast-RTPS-Gen v1.0.3.
* Fixed deadlock between PDP and StatefulReader.
* Improved CPU usage and allocations on timed events management.
* Performance improvements on reliable writers.
* Fixing bugs when Intra-process delivery is activated.
* Reducing dynamic allocations and memory footprint.
* Improvements and fixes on performance tests.
* Other minor bug fixes and improvements.

Version 1.9.3
-------------

This release adds the following features:

* Participant discovery filtering flags.
* Intra-process delivery mechanism opt-in.

It also includes the following bug fixes and improvements:

* Bump to Fast-RTPS-Gen v1.0.2.
* Bring back compatibility with XTypes 1.1 on PID_TYPE_CONSISTENCY.
* Ensure correct alignment when reading a parameter list.
* Add CHECK_DOCUMENTATION *cmake* option.
* EntityId_t and GuidPrefix_t have now their own header files.
* Fix potential race conditions and deadlocks.
* Improve the case where *check_acked_status* is called between reader matching process and its acknack reception.
* RTPSMessageGroup_t instances now use the thread-local storage.
* FragmentedChangePitStop manager removed.
* Remove the data fragments vector on CacheChange_t.
* Only call find_package for TinyXML2 if third-party options are off
* Allow XMLProfileManager methods to not show error log messages if a profile is not found.


Version 1.9.2
-------------

This release includes the following feature:

* Multiple initial PDP announcements.
* Flag to avoid builtin multicast.

It also adds the following bug fixes and improvements:

* Bump to Fast-RTPS-Gen v1.0.1.
* Bump to IDL-Parser v1.0.1.

Version 1.9.1
-------------

This release includes the following features:

* Fast-RTPS-Gen is now an independent project.
* Header **eClock.h** is now marked as deprecated.

It also adds the following bug fixes and improvements:

* Bump to FastCDR v1.0.11.
* Installation from sources documentation fixed.
* Fixed assertion on WriterProxy.
* Fixed potential fall through while parsing Parameters.
* Removed deprecated guards causing compilation errors in some 32 bits platforms.
* *addTOCDRMessage* method is now exported in the DLL, fixing issues related with Parameters' constructors.
* Improve windows performance by avoiding usage of *_Cnd_timedwait* method.
* Fixed reported communication issues by sending multicast through *localhost* too.
* Fixed potential race conditions and deadlocks.
* Eliminating use of *acceptMsgDirectTo*.
* Discovery Server framework reconnect/recreate strategy.
* Removed unused folders.
* Restored subscriber API.
* SequenceNumber_t improvements.
* Added STRICT_REALTIME *cmake* option.
* SubscriberHistory improvements.
* Assertion of participant liveliness by receiving RTPS messages from the remote participant.
* Fixed error while setting next deadline event in *create_new_change_with_params*.

Version 1.9.0
-------------

This release includes the following features:

* Partial implementation of allocation QoS.
* Implementation of Discovery Server.
* Implementation of non-blocking calls.

It also adds the following bug fixes and improvements:

* Added sliding window to BitmapRange.
* Modified default behavior for unknown writers.
* A `Flush()` method was added to the logger to ensure all info is logged.
* A test for loading `Duration_t` from XML was added.
* Optimized WLP when removing local writers.
* Some liveliness tests were updated so that they are more stable on Windows.
* A fix was added to `CMakeLists.txt` for installing static libraries.
* A fix was added to performance tests so that they can run on the RT kernel.
* Fix for race condition on built-in protocols creation.
* Fix for setting *nullptr* in a *fixed_string*.
* Fix for v1.8.1 not building with -DBUILD_JAVA=ON.
* Fix for GAP messages not being sent in some cases.
* Fix for coverity report.
* Several memory issues fixes.
* `fastrtps.repos` file was updated.
* Documentation for building with Colcon was added.
* Change CMake configuration directory if INSTALLER_PLATFORM is set.
* IDL sub-module updated to current version.

Version 1.8.4
-------------

This release adds the following **feature**:

* XML profiles for `requester` and `replier`

It also has the following **important bug fixes**:

* Solved an issue when recreating a publishing participant with the same GUID (either on purpose or by chance)
* Solved an issue where a publisher could block on `write` for a long time when, after a large number of samples
  have been sent, a new subscriber is matched.

Version 1.8.3
-------------

This release adds the following bug fixes and improvements:

* Fix serialization of TypeConsistencyEnforcementQosPolicy.
* Bump to Fast-RTPS-Gen v1.0.2.
* Bump to IDL-Parser v1.0.1.

Version 1.8.2
-------------

This release includes the following features:

* Modified unknown writers default behavior.
* Multiple initial PDP announcements.
* Flag to avoid builtin multicast.
* *STRICT_REALTIME* compilation flag.

It also adds the following bug fixes and improvements:

* Fix for setting `nullptr` in a fixed string.
* Fix for not sending GAP in several cases.
* Solve *Coverity* report issues.
* Fix issue of *fastrtpsgen* failing to open *IDL.g4* file.
* Fix unnamed lock in *AESGCMGMAC_KeyFactory.cpp*.
* Improve *XMLProfiles* example.
* Multicast is now sent through *localhost* too.
* *BitmapRange* now implements sliding window.
* Improve *SequenceNumber_t* struct.
* Participant's liveliness is now asserted when receiving any RTPS message.
* Fix leak on RemoteParticipantLeaseDuration.
* Modified default values to improve behavior in *Wi-Fi* scenarios.
* *SubscriberHistory* improvements.
* Removed use of *acceptMsgDirectTo*.
* *WLP* improvements.

Version 1.8.1
-------------

This release includes the following features:

* Implementation of :ref:`livelinessqospolicy` QoS.

It also adds the following bug fixes and improvements:

* Fix for get_change on history, which was causing issues during discovery.
* Fix for announcement of participant state, which was sending ParticipantBuiltinData twice.
* Fix for closing multicast UDP channel.
* Fix for race conditions in SubscriberHistory, UDPTransportInterface and StatefulReader.
* Fix for lroundl error on Windows in Time_t.
* CDR & IDL submodules update.
* Use of java 1.8 or greater for fastrtpsgen.jar generation.

Version 1.8.0
-------------

This release included the following features:

* Implementation of IDL 4.2.
* Implementation of :ref:`deadlineqospolicy` QoS.
* Implementation of :ref:`lifespanqospolicy` QoS.
* Implementation of :ref:`disablepositiveacksqospolicy` QoS.
* Secure sockets on TCP transport (:ref:`TLS`).

It also adds the following improvements and bug fixes:

* Real-time improvements: non-blocking write calls for best-effort writers, addition of fixed size strings,
  fixed size bitmaps, resource limited vectors, etc.
* Duration parameters now use nanoseconds.
* Configuration of participant mutation tries.
* Automatic calculation of the port when a value of 0 is received on the endpoint custom locators.
* Non-local addresses are now filtered from whitelists.
* Optimization of check for acked status for stateful writers.
* Linked libs are now not exposed when the target is a shared lib.
* Limitation on the domain ID has been added.
* UDP non-blocking send is now optional and configurable via XML.
* Fix for non-deterministic tests.
* Fix for ReaderProxy history being reloaded incorrectly in some cases.
* Fix for RTPS domain hostid being potentially not unique.
* Fix for participants with different lease expiration times failing to reconnect.

**Known issues**

* When using TPC transport, sometimes callbacks are not invoked when removing a participant due to a bug in ASIO.

Version 1.7.2
-------------

This release fixes an important bug:

* Allocation limits on subscribers with a KEEP_LAST QoS was taken from resource limits configuration
  and didn't take history depth into account.

It also has the following improvements:

* Vendor FindThreads.cmake from CMake 3.14 release candidate to help with sanitizers.
* Fixed format of gradle file.

Some other minor bugs and performance improvements.

Version 1.7.1
-------------

This release included the following features:

* LogFileConsumer added to the logging system.
* Handle FASTRTPS_DEFAULT_PROFILES_FILE environment variable indicating the default profiles XML file.
* XML parser made more restrictive and with better error messages.

It also fixes some important bugs:
* Fixed discovery issues related to the selected network interfaces on Windows.
* Improved discovery times.
* Workaround ASIO issue with multicast on QNX systems.
* Improved TCP transport performance.
* Improved handling of key-only data submessages.

Some other minor bugs and performance improvements.

**KNOWN ISSUES**

* Allocation limits on subscribers with a KEEP_LAST QoS is taken from resource limits configuration
  and doesn't take history depth into account.

Version 1.7.0
-------------

This release included the following features:

* :ref:`comm-transports-tcp`.
* :ref:`dynamic-types`.
* Security 1.1 compliance.

Also bug fixing, allocation and performance improvements.

Version 1.6.0
-------------

This release included the following features:

* :ref:`persistence`.
* Security access control plugin API and builtin :ref:`access-permissions` plugin.

Also bug fixing.

Version 1.5.0
-------------

This release included the following features:

* Configuration of Fast RTPS entities through XML profiles.
* Added heartbeat piggyback support.

Also bug fixing.

Version 1.4.0
-------------

This release included the following:

* Added secure communications.
* Removed all Boost dependencies. Fast RTPS is not using Boost libraries anymore.
* Added compatibility with Android.
* Bug fixing.

Version 1.3.1
-------------

This release included the following:

* New examples that illustrate how to tweak Fast RTPS towards different applications.
* Improved support for embedded Linux.
* Bug fixing.

Version 1.3.0
-------------

This release introduced several new features:

* Unbound Arrays support: Now you can send variable size data arrays.
* Extended Fragmentation Configuration: It allows you to setup a Message/Fragment max size different to the standard
  64Kb limit.
* Improved logging system: Get even more introspection about the status of your communications system.
* Static Discovery: Use XML to map your network and keep discovery traffic to a minimum.
* Stability and performance improvements: A new iteration of our built-in performance tests will make benchmarking
  easier for you.
* ReadTheDocs Support: We improved our documentation format and now our installation and user manuals are available
  online on ReadTheDocs.

Version 1.2.0
-------------

This release introduced two important new features:

* Flow Controllers: A mechanism to control how you use the available bandwidth avoiding data bursts.
  The controllers allow you to specify the maximum amount of data to be sent in a specific period of time.
  This is very useful when you are sending large messages requiring fragmentation.
* Discovery Listeners: Now the user can subscribe to the discovery information to know the entities present in the
  network (Topics, Publishers & Subscribers) dynamically without prior knowledge of the system.
  This enables the creation of generic tools to inspect your system.

But there is more:

* Full ROS2 Support: Fast RTPS is used by ROS2, the upcoming release of the Robot Operating System (ROS).
* Better documentation: More content and examples.
* Improved performance.
* Bug fixing.
