/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file dds_qos_policies.h
 *
 *  Created on: Apr 30, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef DDS_QOS_POLICIES_H_
#define DDS_QOS_POLICIES_H_

namespace eprosima{

namespace rtps{

typedef enum DurabilityQosPolicyKind_t: octet{
	VOLATILE_DURABILITY_QOS = 0x01,
			TRANSIENT_LOCAL_DURABILITY_QOS =0x02,
			TRANSIENT_DURABILITY_QOS = 0x04,
			PERSISTENT_DURABILITY_QOS = 0x08
};

typedef struct DurabilityQosPolicy
{
	DurabilityQosPolicyKind_t kind;
};

typedef struct DeadlineQosPolicy {
	Duration_t period;
};

typedef struct LatencyBudgetQosPolicy {
	Duration_t duration;
};

typedef enum LivelinessQosPolicyKind:octet {
	AUTOMATIC_LIVELINESS_QOS = 0x01,
			MANUAL_BY_PARTICIPANT_LIVELINESS_QOS=0x02,
			MANUAL_BY_TOPIC_LIVELINESS_QOS=0x04
};

typedef struct LivelinessQosPolicy {
	LivelinessQosPolicyKind kind;
	Duration_t lease_duration;
};

typedef enum ReliabilityQosPolicyKind:octet {
	BEST_EFFORT_RELIABILITY_QOS= 0x01,
	RELIABLE_RELIABILITY_QOS=0x02
};

typedef enum OwnershipQosPolicyKind:octet {
	SHARED_OWNERSHIP_QOS=0x01,
			EXCLUSIVE_OWNERSHIP_QOS=0x02
};
typedef struct OwnershipQosPolicy {
	OwnershipQosPolicyKind kind;
};

typedef enum DestinationOrderQosPolicyKind :octet{
	BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS,
	BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS
};

struct ReliabilityQosPolicy
{
	ReliabilityQosPolicyKind kind;
	Duration_t max_blocking_time;
};

typedef struct DestinationOrderQosPolicy {
	DestinationOrderQosPolicyKind kind;
};

typedef struct UserDataQosPolicy{
	std::string data;
};

typedef struct TimeBasedFilterQosPolicy {
	Duration_t minimum_separation;
};

typedef enum PresentationQosPolicyAccessScopeKind:octet
{
	INSTANCE_PRESENTATION_QOS=0x01,
	TOPIC_PRESENTATION_QOS=0x02,
	GROUP_PRESENTATION_QOS=0x04
};
typedef struct PresentationQosPolicy
{
	PresentationQosPolicyAccessScopeKind access_scope;
	bool coherent_access;
	bool ordered_access;
};

typedef struct PartitionQosPolicy
{
	std::vector<std::string> name;
};

typedef struct TopicDataQosPolicy
{
	std::vector<std::string> value;
};
typedef struct GroupDataQosPolicy
{
	std::vector<std::string> value;
};

typedef enum HistoryQosPolicyKind:octet {
	KEEP_LAST_HISTORY_QOS=0x01,
	KEEP_ALL_HISTORY_QOS=0x02
};

typedef struct HistoryQosPolicy {
HistoryQosPolicyKind kind;
long depth;
};

typedef struct DurabilityServiceQosPolicy {
	Duration_t service_cleanup_delay;
	HistoryQosPolicyKind history_kind;
	uint32_t history_depth;
	uint32_t max_samples;
	uint32_t max_instances;
	uint32_t max_samples_per_instance;
};

typedef struct LifespanQosPolicy {
	Duration_t duration;
};


typedef struct OwnershipStrengthQosPolicy {
	uint32_t value;
};

typedef struct ResourceLimitsQosPolicy {
	uint32_t max_samples;
	uint32_t max_instances;
	uint32_t max_samples_per_instance;
};

typedef struct TransportPriorityQosPolicy {
	uint32_t value;
};





}
}



#endif /* DDS_QOS_POLICIES_H_ */
