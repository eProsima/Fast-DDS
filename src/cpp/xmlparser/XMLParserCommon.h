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
#ifndef XML_PARSER_COMMON_H_
#define XML_PARSER_COMMON_H_

namespace eprosima {
namespace fastdds {
namespace xmlparser {

/**
 * Enum class XMLP_ret, used to provide a strongly typed result from the operations within this module.
 * @ingroup XMLPARSER_MODULE
 */
enum class XMLP_ret
{
    XML_ERROR,
    XML_OK,
    XML_NOK
};


extern const char* DEFAULT_FASTDDS_ENV_VARIABLE;
extern const char* DEFAULT_FASTDDS_PROFILES;
extern const char* DEFAULT_STATISTICS_DATAWRITER_PROFILE;
extern const char* SKIP_DEFAULT_XML_FILE;

extern const char* ROOT;
extern const char* PROFILES;
extern const char* LIBRARY_SETTINGS;
extern const char* TRANSPORT_DESCRIPTORS;
extern const char* PROFILE_NAME;
extern const char* DEFAULT_PROF;
extern const char* DOMAINPARTICIPANT_FACTORY;
extern const char* PARTICIPANT;
extern const char* PUBLISHER;
extern const char* SUBSCRIBER;
extern const char* RTPS;
extern const char* TYPES;
extern const char* LOG;
extern const char* REQUESTER;
extern const char* REPLIER;

extern const char* TRANSPORT_DESCRIPTOR;
extern const char* TRANSPORT_ID;
extern const char* UDP_OUTPUT_PORT;
extern const char* TCP_WAN_ADDR;
extern const char* RECEIVE_BUFFER_SIZE;
extern const char* SEND_BUFFER_SIZE;
extern const char* TTL;
extern const char* NON_BLOCKING_SEND;
extern const char* WHITE_LIST;
extern const char* NETWORK_INTERFACE;
extern const char* NETMASK_FILTER;
extern const char* NETWORK_INTERFACES;
extern const char* ALLOWLIST;
extern const char* BLOCKLIST;
extern const char* MAX_MESSAGE_SIZE;
extern const char* MAX_INITIAL_PEERS_RANGE;
extern const char* KEEP_ALIVE_FREQUENCY;
extern const char* KEEP_ALIVE_TIMEOUT;
extern const char* MAX_LOGICAL_PORT;
extern const char* LOGICAL_PORT_RANGE;
extern const char* LOGICAL_PORT_INCREMENT;
extern const char* ENABLE_TCP_NODELAY;
extern const char* METADATA_LOGICAL_PORT;
extern const char* LISTENING_PORTS;
extern const char* CALCULATE_CRC;
extern const char* CHECK_CRC;
extern const char* KEEP_ALIVE_THREAD;
extern const char* ACCEPT_THREAD;
extern const char* TCP_NEGOTIATION_TIMEOUT;
extern const char* SEGMENT_SIZE;
extern const char* PORT_QUEUE_CAPACITY;
extern const char* PORT_OVERFLOW_POLICY;
extern const char* SEGMENT_OVERFLOW_POLICY;
extern const char* HEALTHY_CHECK_TIMEOUT_MS;
extern const char* DISCARD;
extern const char* FAIL;
extern const char* RTPS_DUMP_FILE;
extern const char* DEFAULT_RECEPTION_THREADS;
extern const char* RECEPTION_THREADS;
extern const char* RECEPTION_THREAD;
extern const char* DUMP_THREAD;
extern const char* ON;
extern const char* AUTO;
extern const char* THREAD_SETTINGS;
extern const char* SCHEDULING_POLICY;
extern const char* PRIORITY;
extern const char* AFFINITY;
extern const char* STACK_SIZE;

// IntraprocessDeliveryType
extern const char* OFF;
extern const char* USER_DATA_ONLY;
extern const char* FULL;

extern const char* QOS_PROFILE;
extern const char* APPLICATION;
extern const char* TYPE;
// extern const char* TOPIC;
extern const char* DATA_WRITER;
extern const char* DATA_READER;

/// LibrarySettings attributes
extern const char* INTRAPROCESS_DELIVERY;

/// DomainParticipantFactory Qos
extern const char* ENTITY_FACTORY;
extern const char* AUTOENABLE_CREATED_ENTITIES;
extern const char* SHM_WATCHDOG_THREAD;
extern const char* FILE_WATCH_THREADS;

/// RTPS Participant attributes
extern const char* ALLOCATION;
extern const char* PREFIX;
extern const char* DEF_UNI_LOC_LIST;
extern const char* DEF_EXT_UNI_LOC_LIST;
extern const char* DEF_MULTI_LOC_LIST;
extern const char* SEND_SOCK_BUF_SIZE;
extern const char* LIST_SOCK_BUF_SIZE;
extern const char* BUILTIN;
extern const char* PORT;
extern const char* PORTS;
extern const char* LOGICAL_PORT;
extern const char* PHYSICAL_PORT;
extern const char* USER_DATA;
extern const char* PART_ID;
extern const char* EASY_MODE_IP;
extern const char* IP4_TO_SEND;
extern const char* IP6_TO_SEND;
extern const char* FLOW_CONTROLLER_DESCRIPTOR_LIST;
extern const char* USER_TRANS;
extern const char* USE_BUILTIN_TRANS;
extern const char* BUILTIN_TRANS;
extern const char* MAX_MSG_SIZE_LARGE_DATA;
extern const char* SOCKETS_SIZE_LARGE_DATA;
extern const char* NON_BLOCKING_LARGE_DATA;
extern const char* PROPERTIES_POLICY;
extern const char* NAME;
extern const char* REMOTE_LOCATORS;
extern const char* MAX_UNICAST_LOCATORS;
extern const char* MAX_MULTICAST_LOCATORS;
extern const char* TOTAL_PARTICIPANTS;
extern const char* TOTAL_READERS;
extern const char* TOTAL_WRITERS;
extern const char* SEND_BUFFERS;
extern const char* PREALLOCATED_NUMBER;
extern const char* DYNAMIC_LC;
extern const char* NETWORK_BUFFERS_CONFIG;
extern const char* MAX_PROPERTIES;
extern const char* MAX_USER_DATA;
extern const char* MAX_PARTITIONS;
extern const char* TIMED_EVENTS_THREAD;
extern const char* DISCOVERY_SERVER_THREAD;
extern const char* TYPELOOKUP_SERVICE_THREAD;
extern const char* SECURITY_LOG_THREAD;
extern const char* BUILTIN_TRANSPORTS_RECEPTION_THREADS;
extern const char* BUILTIN_CONTROLLERS_SENDER_THREAD;

/// Publisher-subscriber attributes
extern const char* TOPIC;
extern const char* QOS;
extern const char* TIMES;
extern const char* EXT_UNI_LOC_LIST;
extern const char* UNI_LOC_LIST;
extern const char* MULTI_LOC_LIST;
extern const char* REM_LOC_LIST;
extern const char* EXP_INLINE_QOS;
extern const char* HIST_MEM_POLICY;
//extern const char* PROPERTIES_POLICY;
extern const char* USER_DEF_ID;
extern const char* ENTITY_ID;
extern const char* MATCHED_SUBSCRIBERS_ALLOCATION;
extern const char* MATCHED_PUBLISHERS_ALLOCATION;
extern const char* DATA_SHARING_LISTENER_THREAD;

///
extern const char* IGN_NON_MATCHING_LOCS;
extern const char* PROPERTIES;
extern const char* BIN_PROPERTIES;
extern const char* PROPERTY;
extern const char* VALUE;
extern const char* PROPAGATE;
extern const char* PREALLOCATED;
extern const char* PREALLOCATED_WITH_REALLOC;
extern const char* DYNAMIC;
extern const char* DYNAMIC_REUSABLE;
extern const char* LOCATOR;
extern const char* UDPv4_LOCATOR;
extern const char* UDPv6_LOCATOR;
extern const char* TCPv4_LOCATOR;
extern const char* TCPv6_LOCATOR;
extern const char* KIND;
extern const char* ADDRESS;
extern const char* UNIQUE_LAN_ID;
extern const char* WAN_ADDRESS;
extern const char* RESERVED;
extern const char* UDPv4;
extern const char* UDPv6;
extern const char* TCPv4;
extern const char* TCPv6;
extern const char* SHM;
extern const char* DEFAULT_C;
extern const char* DEFAULTv6;
extern const char* LARGE_DATA;
extern const char* LARGE_DATAv6;
extern const char* P2P;
extern const char* INIT_ACKNACK_DELAY;
extern const char* HEARTB_RESP_DELAY;
extern const char* INIT_HEARTB_DELAY;
extern const char* HEARTB_PERIOD;
extern const char* NACK_RESP_DELAY;
extern const char* NACK_SUPRESSION;
extern const char* BY_NAME;
extern const char* BY_VAL;
extern const char* DURATION_INFINITY;
extern const char* DURATION_INFINITE_SEC;
extern const char* DURATION_INFINITE_NSEC;
extern const char* SECONDS;
extern const char* NANOSECONDS;
extern const char* SHARED;
extern const char* EXCLUSIVE;

/// QOS
extern const char* DURABILITY;
extern const char* DURABILITY_SRV;
extern const char* DEADLINE;
extern const char* LATENCY_BUDGET;
extern const char* LIVELINESS;
extern const char* RELIABILITY;
extern const char* LIFESPAN;
extern const char* TIME_FILTER;
extern const char* OWNERSHIP;
extern const char* OWNERSHIP_STRENGTH;
extern const char* DEST_ORDER;
extern const char* PRESENTATION;
extern const char* PARTITION;
extern const char* TOPIC_DATA;
extern const char* GROUP_DATA;
extern const char* PUB_MODE;
extern const char* DISABLE_POSITIVE_ACKS;
extern const char* DISABLE_HEARTBEAT_PIGGYBACK;
extern const char* DATA_SHARING;

extern const char* SYNCHRONOUS;
extern const char* ASYNCHRONOUS;
extern const char* NAMES;
extern const char* INSTANCE;
extern const char* GROUP;
extern const char* COHERENT_ACCESS;
extern const char* ORDERED_ACCESS;
extern const char* BY_RECEPTION_TIMESTAMP;
extern const char* BY_SOURCE_TIMESTAMP;
extern const char* MIN_SEPARATION;
extern const char* DURATION;
extern const char* MAX_BLOCK_TIME;
extern const char* _BEST_EFFORT;
extern const char* _RELIABLE;
extern const char* AUTOMATIC;
extern const char* MANUAL_BY_PARTICIPANT;
extern const char* MANUAL_BY_TOPIC;
extern const char* LEASE_DURATION;
extern const char* ANNOUNCE_PERIOD;
extern const char* COUNT;
extern const char* PERIOD;
extern const char* SRV_CLEAN_DELAY;
extern const char* HISTORY_KIND;
extern const char* HISTORY_DEPTH;
extern const char* MAX_SAMPLES;
extern const char* MAX_INSTANCES;
extern const char* MAX_SAMPLES_INSTANCE;
extern const char* _VOLATILE;
extern const char* _TRANSIENT_LOCAL;
extern const char* _TRANSIENT;
extern const char* _PERSISTENT;
extern const char* KEEP_LAST;
extern const char* KEEP_ALL;
extern const char* _NO_KEY;
extern const char* _WITH_KEY;
extern const char* DATA_TYPE;
extern const char* HISTORY_QOS;
extern const char* RES_LIMITS_QOS;
extern const char* DEPTH;
extern const char* ALLOCATED_SAMPLES;
extern const char* EXTRA_SAMPLES;
extern const char* FLOW_CONTROLLER_DESCRIPTOR;
extern const char* SCHEDULER;
extern const char* SENDER_THREAD;
extern const char* MAX_BYTES_PER_PERIOD;
extern const char* FLOW_CONTROLLER_NAME;
extern const char* FIFO;
extern const char* HIGH_PRIORITY;
extern const char* ROUND_ROBIN;
extern const char* PRIORITY_WITH_RESERVATION;
extern const char* FLOW_CONTROLLER_NAME;
extern const char* PERIOD_MILLISECS;
extern const char* PORT_BASE;
extern const char* DOMAIN_ID_GAIN;
extern const char* PARTICIPANT_ID_GAIN;
extern const char* OFFSETD0;
extern const char* OFFSETD1;
extern const char* OFFSETD2;
extern const char* OFFSETD3;
extern const char* OFFSETD4;
extern const char* RTPS_PDP_TYPE;
extern const char* NONE;
extern const char* CLIENT;
extern const char* SERVER;
extern const char* BACKUP;
extern const char* SUPER_CLIENT;
extern const char* IGNORE_PARTICIPANT_FLAGS;
extern const char* FILTER_DIFFERENT_HOST;
extern const char* FILTER_DIFFERENT_PROCESS;
extern const char* FILTER_SAME_PROCESS;
extern const char* WRITER_LVESS_PROTOCOL;
extern const char* DISCOVERY_SETTINGS;
extern const char* _EDP;
extern const char* DOMAIN_ID;
extern const char* LEASEDURATION;
extern const char* LEASE_ANNOUNCE;
extern const char* INITIAL_ANNOUNCEMENTS;
extern const char* AVOID_BUILTIN_MULTICAST;
extern const char* SIMPLE_EDP;
extern const char* META_EXT_UNI_LOC_LIST;
extern const char* META_UNI_LOC_LIST;
extern const char* META_MULTI_LOC_LIST;
extern const char* INIT_PEERS_LIST;
extern const char* CLIENTANNOUNCEMENTPERIOD;
extern const char* SERVER_LIST;
extern const char* RSERVER;
extern const char* SIMPLE;
extern const char* STATIC;
extern const char* PUBWRITER_SUBREADER;
extern const char* PUBREADER_SUBWRITER;
extern const char* STATIC_ENDPOINT_XML;
extern const char* STATIC_ENDPOINT_XML_URI;
extern const char* READER_HIST_MEM_POLICY;
extern const char* WRITER_HIST_MEM_POLICY;
extern const char* READER_PAYLOAD_SIZE;
extern const char* WRITER_PAYLOAD_SIZE;
extern const char* MUTATION_TRIES;
extern const char* ACCESS_SCOPE;
extern const char* ENABLED;
extern const char* DOMAIN_IDS;
extern const char* SHARED_DIR;
extern const char* MAX_DOMAINS;

// Endpoint parser
extern const char* STATICDISCOVERY;
extern const char* READER;
extern const char* WRITER;
extern const char* USER_ID;
extern const char* EXPECT_INLINE_QOS;
extern const char* TOPIC_NAME;
extern const char* TOPIC_DATA_TYPE;
extern const char* TOPIC_KIND;
extern const char* RELIABILITY_QOS;
extern const char* UNICAST_LOCATOR;
extern const char* MULTICAST_LOCATOR;
extern const char* _RELIABLE_RELIABILITY_QOS;
extern const char* _BEST_EFFORT_RELIABILITY_QOS;
extern const char* DURABILITY_QOS;
extern const char* _PERSISTENT_DURABILITY_QOS;
extern const char* _TRANSIENT_DURABILITY_QOS;
extern const char* _TRANSIENT_LOCAL_DURABILITY_QOS;
extern const char* _VOLATILE_DURABILITY_QOS;
extern const char* OWNERSHIP_QOS;
extern const char* OWNERSHIP_KIND_NOT_PRESENT;
extern const char* _SHARED_OWNERSHIP_QOS;
extern const char* _EXCLUSIVE_OWNERSHIP_QOS;
extern const char* PARTITION_QOS;
extern const char* LIVELINESS_QOS;
extern const char* LIVELINESS_KIND_NOT_PRESENT;
extern const char* _AUTOMATIC_LIVELINESS_QOS;
extern const char* _MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
extern const char* _MANUAL_BY_TOPIC_LIVELINESS_QOS;
extern const char* LEASE_DURATION_MS;
extern const char* _INF;
extern const char* EPROSIMA_UNKNOWN_STRING;
extern const char* _TRANSIENT_LOCAL_DURABILITY_QOS;
extern const char* _VOLATILE_DURABILITY_QOS;
extern const char* STRENGTH;

// LOG
extern const char* USE_DEFAULT;
extern const char* CONSUMER;
extern const char* CLASS;

// Allocation config
extern const char* INITIAL;
extern const char* MAXIMUM;
extern const char* INCREMENT;

// TLS Config
extern const char* TLS;
extern const char* TLS_PASSWORD;
extern const char* TLS_OPTIONS;
extern const char* TLS_CERT_CHAIN_FILE;
extern const char* TLS_PRIVATE_KEY_FILE;
extern const char* TLS_TMP_DH_FILE;
extern const char* TLS_VERIFY_FILE;
extern const char* TLS_VERIFY_MODE;
extern const char* TLS_VERIFY_PATHS;
extern const char* TLS_DEFAULT_VERIFY_PATH;
extern const char* TLS_VERIFY_DEPTH;
extern const char* TLS_RSA_PRIVATE_KEY_FILE;
extern const char* TLS_HANDSHAKE_ROLE;
extern const char* TLS_SERVER_NAME;

// TLS HandShake Role
extern const char* TLS_HANDSHAKE_ROLE_DEFAULT;
extern const char* TLS_HANDSHAKE_ROLE_CLIENT;
extern const char* TLS_HANDSHAKE_ROLE_SERVER;

// TLS Verify Stuff
extern const char* TLS_VERIFY_PATH;
extern const char* TLS_VERIFY;

// TLS Options
extern const char* TLS_OPTION;
extern const char* TLS_DEFAULT_WORKAROUNDS;
extern const char* TLS_NO_COMPRESSION;
extern const char* TLS_NO_SSLV2;
extern const char* TLS_NO_SSLV3;
extern const char* TLS_NO_TLSV1;
extern const char* TLS_NO_TLSV1_1;
extern const char* TLS_NO_TLSV1_2;
extern const char* TLS_NO_TLSV1_3;
extern const char* TLS_SINGLE_DH_USE;

// TLS Verify Mode
extern const char* TLS_VERIFY_NONE;
extern const char* TLS_VERIFY_PEER;
extern const char* TLS_VERIFY_FAIL_IF_NO_PEER_CERT;
extern const char* TLS_VERIFY_CLIENT_ONCE;

// Requester and Replier
extern const char* SERVICE_NAME;
extern const char* REQUEST_TYPE;
extern const char* REPLY_TYPE;
extern const char* REQUEST_TOPIC_NAME;
extern const char* REPLY_TOPIC_NAME;

} /* xmlparser */
} /* namespace */
} /* namespace eprosima */

#endif // ifndef XML_PARSER_COMMON_H_
