// Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include <fastrtps/xmlparser/XMLParserCommon.h>

namespace eprosima {
namespace fastrtps {
namespace xmlparser {

const char* DEFAULT_FASTRTPS_PROFILES = "DEFAULT_FASTRTPS_PROFILES.xml";

const char* ROOT = "dds";
const char* PROFILES = "profiles";
const char* TRANSPORT_DESCRIPTORS = "transport_descriptors";
const char* PROFILE_NAME = "profile_name";
const char* DEFAULT_PROF = "is_default_profile";
const char* PARTICIPANT = "participant";
const char* PUBLISHER = "publisher";
const char* SUBSCRIBER = "subscriber";
const char* RTPS = "rtps";
const char* TYPES = "types";

const char* TRANSPORT_DESCRIPTOR = "transport_descriptor";
const char* TRANSPORT_ID = "transport_id";
const char* UDP_OUTPUT_PORT = "output_port";
const char* TCP_WAN_ADDR = "wan_addr";
const char* RECEIVE_BUFFER_SIZE = "receiveBufferSize";
const char* SEND_BUFFER_SIZE = "sendBufferSize";
const char* TTL = "TTL";
const char* WHITE_LIST = "interfaceWhiteList";
const char* MAX_MESSAGE_SIZE = "maxMessageSize";
const char* MAX_INITIAL_PEERS_RANGE = "maxInitialPeersRange";
const char* KEEP_ALIVE_FREQUENCY = "keep_alive_frequency_ms";
const char* KEEP_ALIVE_TIMEOUT = "keep_alive_timeout_ms";
const char* MAX_LOGICAL_PORT = "max_logical_port";
const char* LOGICAL_PORT_RANGE = "logical_port_range";
const char* LOGICAL_PORT_INCREMENT = "logical_port_increment";
const char* METADATA_LOGICAL_PORT = "metadata_logical_port";
const char* LISTENING_PORTS = "listening_ports";

const char* QOS_PROFILE = "qos_profile";
const char* APPLICATION = "application";
const char* TYPE = "type";
// const char* TOPIC = "topic";
const char* DATA_WRITER = "data_writer";
const char* DATA_READER = "data_reader";

/// RTPS Participant attributes
const char* DEF_UNI_LOC_LIST = "defaultUnicastLocatorList";
const char* DEF_MULTI_LOC_LIST = "defaultMulticastLocatorList";
const char* SEND_SOCK_BUF_SIZE = "sendSocketBufferSize";
const char* LIST_SOCK_BUF_SIZE = "listenSocketBufferSize";
const char* BUILTIN = "builtin";
const char* PORT = "port";
const char* PORTS = "ports_";
const char* LOGICAL_PORT = "logical_port";
const char* PHYSICAL_PORT = "physical_port";
const char* USER_DATA = "userData";
const char* PART_ID = "participantID";
const char* THROUGHPUT_CONT = "throughputController";
const char* USER_TRANS = "userTransports";
const char* USE_BUILTIN_TRANS = "useBuiltinTransports";
const char* PROPERTIES_POLICY = "propertiesPolicy";
const char* NAME = "name";

/// Publisher-subscriber attributes
const char* TOPIC = "topic";
const char* QOS = "qos";
const char* TIMES = "times";
const char* UNI_LOC_LIST = "unicastLocatorList";
const char* MULTI_LOC_LIST = "multicastLocatorList";
const char* REM_LOC_LIST = "remoteLocatorList";
//const char* THROUGHPUT_CONT = "throughputController";
const char* EXP_INLINE_QOS = "expectsInlineQos";
const char* HIST_MEM_POLICY = "historyMemoryPolicy";
//const char* PROPERTIES_POLICY = "propertiesPolicy";
const char* USER_DEF_ID = "userDefinedID";
const char* ENTITY_ID = "entityID";

///
const char* PROPERTIES = "properties";
const char* BIN_PROPERTIES = "binary_properties";
const char* PROPERTY = "property";
const char* VALUE = "value";
const char* PROPAGATE = "propagate";
const char* PREALLOCATED = "PREALLOCATED";
const char* PREALLOCATED_WITH_REALLOC = "PREALLOCATED_WITH_REALLOC";
const char* DYNAMIC = "DYNAMIC";
const char* LOCATOR = "locator";
const char* KIND = "kind";
const char* ADDRESS = "address";
const char* IPV6_ADDRESS = "ipv6_address";
const char* ADDRESSES = "addresses_";
const char* UNIQUE_LAN_ID = "unique_lan_id";
const char* WAN_ADDRESS = "wan_address";
const char* IP_ADDRESS = "ip_address";
const char* RESERVED = "RESERVED";
const char* UDPv4 = "UDPv4";
const char* UDPv6 = "UDPv6";
const char* TCPv4 = "TCPv4";
const char* TCPv6 = "TCPv6";
const char* INIT_ACKNACK_DELAY = "initialAcknackDelay";
const char* HEARTB_RESP_DELAY = "heartbeatResponseDelay";
const char* INIT_HEARTB_DELAY = "initialHeartbeatDelay";
const char* HEARTB_PERIOD = "heartbeatPeriod";
const char* NACK_RESP_DELAY = "nackResponseDelay";
const char* NACK_SUPRESSION = "nackSupressionDuration";
const char* BY_NAME = "durationbyname";
const char* BY_VAL = "durationbyval";
const char* _INFINITE = "INFINITE";
const char* ZERO = "ZERO";
const char* INVALID = "INVALID";
const char* SECONDS = "seconds";
const char* FRACTION = "fraction";
const char* SHARED = "SHARED";
const char* EXCLUSIVE = "EXCLUSIVE";

/// QOS
const char* DURABILITY = "durability";
const char* DURABILITY_SRV = "durabilityService";
const char* DEADLINE = "deadline";
const char* LATENCY_BUDGET = "latencyBudget";
const char* LIVELINESS = "liveliness";
const char* RELIABILITY = "reliability";
const char* LIFESPAN = "lifespan";
const char* TIME_FILTER = "timeBasedFilter";
const char* OWNERSHIP = "ownership";
const char* OWNERSHIP_STRENGTH = "ownershipStrength";
const char* DEST_ORDER = "destinationOrder";
const char* PRESENTATION = "presentation";
const char* PARTITION = "partition";
const char* TOPIC_DATA = "topicData";
const char* GROUP_DATA = "groupData";
const char* PUB_MODE = "publishMode";

const char* SYNCHRONOUS = "SYNCHRONOUS";
const char* ASYNCHRONOUS = "ASYNCHRONOUS";
const char* NAMES = "names";
const char* INSTANCE = "INSTANCE";
const char* GROUP = "GROUP";
const char* COHERENT_ACCESS = "coherent_access";
const char* ORDERED_ACCESS = "ordered_access";
const char* BY_RECEPTION_TIMESTAMP = "BY_RECEPTION_TIMESTAMP";
const char* BY_SOURCE_TIMESTAMP = "BY_SOURCE_TIMESTAMP";
const char* MIN_SEPARATION = "minimum_separation";
const char* DURATION = "duration";
const char* MAX_BLOCK_TIME = "max_blocking_time";
const char* _BEST_EFFORT = "BEST_EFFORT";
const char* _RELIABLE = "RELIABLE";
const char* AUTOMATIC = "AUTOMATIC";
const char* MANUAL_BY_PARTICIPANT = "MANUAL_BY_PARTICIPANT";
const char* MANUAL_BY_TOPIC = "MANUAL_BY_TOPIC";
const char* LEASE_DURATION = "lease_duration";
const char* ANNOUNCE_PERIOD = "announcement_period";
const char* PERIOD = "period";
const char* SRV_CLEAN_DELAY = "service_cleanup_delay";
const char* HISTORY_KIND = "history_kind";
const char* HISTORY_DEPTH = "history_depth";
const char* MAX_SAMPLES = "max_samples";
const char* MAX_INSTANCES = "max_instances";
const char* MAX_SAMPLES_INSTANCE = "max_samples_per_instance";
const char* _VOLATILE = "VOLATILE";
const char* _TRANSIENT_LOCAL = "TRANSIENT_LOCAL";
const char* _TRANSIENT = "TRANSIENT";
const char* _PERSISTENT = "PERSISTENT";
const char* KEEP_LAST = "KEEP_LAST";
const char* KEEP_ALL = "KEEP_ALL";
const char* _NO_KEY = "NO_KEY";
const char* _WITH_KEY = "WITH_KEY";
const char* DATA_TYPE = "dataType";
const char* HISTORY_QOS = "historyQos";
const char* RES_LIMITS_QOS = "resourceLimitsQos";
const char* DEPTH = "depth";
const char* ALLOCATED_SAMPLES = "allocated_samples";
const char* BYTES_PER_SECOND = "bytesPerPeriod";
const char* PERIOD_MILLISECS = "periodMillisecs";
const char* PORT_BASE = "portBase";
const char* DOMAIN_ID_GAIN = "domainIDGain";
const char* PARTICIPANT_ID_GAIN = "participantIDGain";
const char* OFFSETD0 = "offsetd0";
const char* OFFSETD1 = "offsetd1";
const char* OFFSETD2 = "offsetd2";
const char* OFFSETD3 = "offsetd3";
const char* SIMPLE_RTPS_PDP = "use_SIMPLE_RTPS_PDP";
const char* WRITER_LVESS_PROTOCOL = "use_WriterLivelinessProtocol";
const char* _EDP = "EDP";
const char* DOMAIN_ID = "domainId";
const char* LEASEDURATION = "leaseDuration";
const char* LEASE_ANNOUNCE = "leaseAnnouncement";
const char* SIMPLE_EDP = "simpleEDP";
const char* META_UNI_LOC_LIST = "metatrafficUnicastLocatorList";
const char* META_MULTI_LOC_LIST = "metatrafficMulticastLocatorList";
const char* INIT_PEERS_LIST = "initialPeersList";
const char* SIMPLE = "SIMPLE";
const char* STATIC = "STATIC";
const char* PUBWRITER_SUBREADER = "PUBWRITER_SUBREADER";
const char* PUBREADER_SUBWRITER = "PUBREADER_SUBWRITER";
const char* STATIC_ENDPOINT_XML = "staticEndpointXMLFilename";
const char* READER_HIST_MEM_POLICY = "readerHistoryMemoryPolicy";
const char* WRITER_HIST_MEM_POLICY = "writerHistoryMemoryPolicy";
const char* ACCESS_SCOPE = "access_scope";

// Endpoint parser
const char* STATICDISCOVERY = "staticdiscovery";
const char* READER = "reader";
const char* WRITER = "writer";
const char* USER_ID = "userId";
const char* EXPECT_INLINE_QOS = "expectsInlineQos";
const char* TOPIC_NAME = "topicName";
const char* TOPIC_DATA_TYPE = "topicDataType";
const char* TOPIC_KIND = "topicKind";
const char* RELIABILITY_QOS = "reliabilityQos";
const char* UNICAST_LOCATOR = "unicastLocator";
const char* MULTICAST_LOCATOR = "multicastLocator";
const char* _RELIABLE_RELIABILITY_QOS = "RELIABLE_RELIABILITY_QOS";
const char* _BEST_EFFORT_RELIABILITY_QOS = "BEST_EFFORT_RELIABILITY_QOS";
const char* DURABILITY_QOS = "durabilityQos";
const char* _TRANSIENT_LOCAL_DURABILITY_QOS = "TRANSIENT_LOCAL_DURABILITY_QOS";
const char* _VOLATILE_DURABILITY_QOS = "VOLATILE_DURABILITY_QOS";
const char* OWNERSHIP_QOS = "ownershipQos";
const char* OWNERSHIP_KIND_NOT_PRESENT = "OWNERHSIP kind NOT PRESENT";
const char* _SHARED_OWNERSHIP_QOS = "SHARED_OWNERSHIP_QOS";
const char* _EXCLUSIVE_OWNERSHIP_QOS = "EXCLUSIVE_OWNERSHIP_QOS";
const char* PARTITION_QOS = "partitionQos";
const char* LIVELINESS_QOS = "livelinessQos";
const char* LIVELINESS_KIND_NOT_PRESENT = "LIVELINESS kind NOT PRESENT";
const char* _AUTOMATIC_LIVELINESS_QOS = "AUTOMATIC_LIVELINESS_QOS";
const char* _MANUAL_BY_PARTICIPANT_LIVELINESS_QOS = "MANUAL_BY_PARTICIPANT_LIVELINESS_QOS";
const char* _MANUAL_BY_TOPIC_LIVELINESS_QOS = "MANUAL_BY_TOPIC_LIVELINESS_QOS";
const char* LEASE_DURATION_MS = "leaseDuration_ms";
const char* _INF = "INF";
const char* EPROSIMA_UNKNOWN_STRING = "EPROSIMA_UNKNOWN_STRING";
const char* _OWNERSHIP_KIND_NOT_PRESENT = "OWNERSHIP_KIND_NOT_PRESENT";
const char* STRENGTH = "strength";

// TYPES parser
const char* STRUCT = "struct";
const char* UNION = "union";
const char* BOOLEAN = "boolean";
const char* CHAR = "char";
const char* WCHAR = "wchar";
const char* OCTET = "octet";
const char* SHORT = "short";
const char* LONG = "long";
const char* USHORT = "unsignedshort";
const char* ULONG = "unsignedlong";
const char* LONGLONG = "longlong";
const char* ULONGLONG = "unsignedlonglong";
const char* FLOAT = "float";
const char* DOUBLE = "double";
const char* LONGDOUBLE = "longdouble";
const char* STRING = "string";
const char* WSTRING = "wstring";
const char* BOUNDEDSTRING = "boundedString";
const char* BOUNDEDWSTRING = "boundedWString";
const char* SEQUENCE = "sequence";
const char* MAP = "map";
const char* TYPEDEF = "typedef";
const char* ENUM = "enum";
const char* LITERAL = "literal";
const char* CASE = "case";
const char* CASEVALUE = "caseValue";
const char* DEFAULT = "default";
const char* DISCRIMINATOR = "discriminator";
const char* DIMENSIONS = "dimensions";
const char* KEY = "key";
const char* KEY_TYPE = "key_type";
const char* VALUE_TYPE = "value_type";
const char* LENGHT = "length";
const char* MAXLENGTH = "maxLength";


} /* xmlparser */
} /* namespace */
} /* namespace eprosima */
