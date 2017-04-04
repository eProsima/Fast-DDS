#ifndef XML_PROFILE_PARSER_COMMON_H_
#define XML_PROFILE_PARSER_COMMON_H_

namespace eprosima{
namespace fastrtps{

#define dibuja(ident, texto, ...) for (uint8_t i = ident + 1; i > 0; i--) (i == 1)? printf(texto, ## __VA_ARGS__): printf("\t")


static const char* default_profile_file = "DEFAULT_FASTRTPS_PROFILES.xml";


static const char* PROFILES = "profiles";
static const char* PROFILE_NAME = "profile_name";
static const char* PARTICIPANT = "participant";
static const char* PUBLISHER = "publisher";
static const char* SUBSCRIBER = "subscriber";
static const char* RTPS = "rtps";

/// RTPS Participant attributes
static const char* DEF_UNI_LOC_LIST = "defaultUnicastLocatorList";
static const char* DEF_MULTI_LOC_LIST = "defaultMulticastLocatorList";
static const char* DEF_OUT_LOC_LIST= "defaultOutLocatorList";
static const char* DEF_SEND_PORT = "defaultSendPort";
static const char* SEND_SOCK_BUF_SIZE = "sendSocketBufferSize";
static const char* LIST_SOCK_BUF_SIZE = "listenSocketBufferSize";
static const char* BUILTIN = "builtin";
static const char* PORT = "port";
static const char* USER_DATA = "userData";
static const char* PART_ID = "participantID";
static const char* IP4_TO_SEND = "use_IP4_to_send";
static const char* IP6_TO_SEND = "use_IP6_to_send";
static const char* THROUGHPUT_CONT = "throughputController";
static const char* USER_TRANS = "userTransports";
static const char* USE_BUILTIN_TRANS = "useBuiltinTransports";
static const char* PROPERTIES_POLICY = "propertiesPolicy";
static const char* NAME = "name";

/// Publisher-subscriber attributes
static const char* TOPIC = "topic";
static const char* QOS = "qos";
static const char* TIMES = "times";
static const char* UNI_LOC_LIST = "unicastLocatorList";
static const char* MULTI_LOC_LIST = "multicastLocatorList";
static const char* OUT_LOC_LIST = "outLocatorList";
//static const char* THROUGHPUT_CONT = "throughputController";
static const char* EXP_INLINE_QOS = "expectsInlineQos";
static const char* HIST_MEM_POLICY = "historyMemoryPolicy";
//static const char* PROPERTIES_POLICY = "propertiesPolicy";
static const char* USER_DEF_ID = "userDefinedID";
static const char* ENTITY_ID = "entityID";

///
static const char* PROPERTIES = "properties";
static const char* BIN_PROPERTIES = "binary_properties";
static const char* PROPERTY = "property";
static const char* VALUE = "value";
static const char* PROPAGATE = "propagate";
static const char* PREALLOCATED = "PREALLOCATED";
static const char* PREALLOCATED_WITH_REALLOC = "PREALLOCATED_WITH_REALLOC";
static const char* DYNAMIC = "DYNAMIC";
static const char* LOCATOR = "locator";
static const char* KIND = "kind";
static const char* ADDRESS = "address";
static const char* RESERVED = "RESERVED";
static const char* UDPv4 = "UDPv4";
static const char* UDPv6 = "UDPv6";
static const char* INIT_ACKNACK_DELAY = "initialAcknackDelay";
static const char* HEARTB_RESP_DELAY = "heartbeatResponseDelay";
static const char* INIT_HEARTB_DELAY = "initialHeartbeatDelay";
static const char* HEARTB_PERIOD = "heartbeatPeriod";
static const char* NACK_RESP_DELAY = "nackResponseDelay";
static const char* NACK_SUPRESSION = "nackSupressionDuration";
static const char* BY_NAME = "durationbyname";
static const char* BY_VAL = "durationbyval";
static const char* INFINITE = "INFINITE";
static const char* ZERO = "ZERO";
static const char* INVALID = "INVALID";
static const char* SECONDS = "seconds";
static const char* FRACTION = "fraction";
static const char* SHARED = "SHARED";
static const char* EXCLUSIVE = "EXCLUSIVE";

/// QOS
static const char* DURABILITY = "durability";
static const char* DURABILITY_SRV = "durabilityService";
static const char* DEADLINE = "deadline";
static const char* LATENCY_BUDGET = "latencyBudget";
static const char* LIVELINESS = "liveliness";
static const char* RELIABILITY = "reliability";
static const char* LIFESPAN = "lifespan";
static const char* TIME_FILTER = "timeBasedFilter";
static const char* OWNERSHIP = "ownership";
static const char* OWNERSHIP_STRENGTH = "ownershipStrength";
static const char* DEST_ORDER = "destinationOrder";
static const char* PRESENTATION = "presentation";
static const char* PARTITION = "partition";
static const char* TOPIC_DATA = "topicData";
static const char* GROUP_DATA = "groupData";
static const char* PUB_MODE = "publishMode";

static const char* SYNCHRONOUS = "SYNCHRONOUS";
static const char* ASYNCHRONOUS = "ASYNCHRONOUS";
static const char* NAMES = "names";
static const char* INSTANCE = "INSTANCE";
static const char* GROUP = "GROUP";
static const char* COHERENT_ACCESS = "coherent_access";
static const char* ORDERED_ACCESS = "ordered_access";
static const char* BY_RECEPTION_TIMESTAMP = "BY_RECEPTION_TIMESTAMP";
static const char* BY_SOURCE_TIMESTAMP = "BY_SOURCE_TIMESTAMP";
static const char* MIN_SEPARATION = "minimum_separation";
static const char* DURATION = "duration";
static const char* MAX_BLOCK_TIME = "max_blocking_time";
static const char* _BEST_EFFORT = "BEST_EFFORT";
static const char* _RELIABLE = "RELIABLE";
static const char* AUTOMATIC = "AUTOMATIC";
static const char* MANUAL_BY_PARTICIPANT = "MANUAL_BY_PARTICIPANT";
static const char* MANUAL_BY_TOPIC = "MANUAL_BY_TOPIC";
static const char* LEASE_DURATION = "lease_duration";
static const char* ANNOUNCE_PERIOD = "announcement_period";
static const char* PERIOD = "period";
static const char* SRV_CLEAN_DELAY = "service_cleanup_delay";
static const char* HISTORY_KIND = "history_kind";
static const char* HISTORY_DEPTH = "history_depth";
static const char* MAX_SAMPLES = "max_samples";
static const char* MAX_INSTANCES = "max_instances";
static const char* MAX_SAMPLES_INSTANCE = "max_samples_per_instance";
static const char* _VOLATILE = "VOLATILE";
static const char* _TRANSIENT_LOCAL = "TRANSIENT_LOCAL";
static const char* _TRANSIENT = "TRANSIENT";
static const char* _PERSISTENT = "PERSISTENT";
static const char* KEEP_LAST = "KEEP_LAST";
static const char* KEEP_ALL = "KEEP_ALL";
static const char* _NO_KEY = "NO_KEY";
static const char* _WITH_KEY = "WITH_KEY";
static const char* DATA_TYPE = "dataType";
static const char* HISTORY_QOS = "historyQos";
static const char* RES_LIMITS_QOS = "resourceLimitsQos";
static const char* DEPTH = "depth";
static const char* ALLOCATED_SAMPLES = "allocated_samples";
static const char* BYTES_PER_SECOND = "bytesPerPeriod";
static const char* PERIOD_MILLISECS = "periodMillisecs";
static const char* PORT_BASE = "portBase";
static const char* DOMAIN_ID_GAIN = "domainIDGain";
static const char* PARTICIPANT_ID_GAIN = "participantIDGain";
static const char* OFFSETD0 = "offsetd0";
static const char* OFFSETD1 = "offsetd1";
static const char* OFFSETD2 = "offsetd2";
static const char* OFFSETD3 = "offsetd3";
static const char* SIMPLE_RTPS_PDP = "use_SIMPLE_RTPS_PDP";
static const char* WRITER_LVESS_PROTOCOL = "use_WriterLivelinessProtocol";
static const char* _EDP = "EDP";
static const char* DOMAIN_ID = "domainId";
static const char* LEASEDURATION = "leaseDuration";
static const char* LEASE_ANNOUNCE = "leaseAnnouncement";
static const char* SIMPLE_EDP = "simpleEDP";
static const char* META_UNI_LOC_LIST = "metatrafficUnicastLocatorList";
static const char* META_MULTI_LOC_LIST = "metatrafficMulticastLocatorList";
static const char* INIT_PEERS_LIST = "initialPeersList";
static const char* SIMPLE = "SIMPLE";
static const char* STATIC = "STATIC";
static const char* PUBWRITER_SUBREADER = "PUBWRITER_SUBREADER";
static const char* PUBREADER_SUBWRITER = "PUBREADER_SUBWRITER";
static const char* STATIC_ENDPOINT_XML = "staticEndpointXMLFilename";
static const char* ACCESS_SCOPE = "access_scope";

} /* namespace */
} /* namespace eprosima */

#endif
