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

#include "eprosimartps/qos/ParameterTypes.h"

using namespace eprosima::rtps;

namespace eprosima{

namespace dds{


typedef enum DurabilityQosPolicyKind_t: octet{
	VOLATILE_DURABILITY_QOS = 0x01,
			TRANSIENT_LOCAL_DURABILITY_QOS =0x02,
			TRANSIENT_DURABILITY_QOS = 0x04,
			PERSISTENT_DURABILITY_QOS = 0x08
};


class DurabilityQosPolicy : public Parameter_t
{
	DurabilityQosPolicy():kind(VOLATILE_DURABILITY_QOS){};
	DurabilityQosPolicyKind_t kind;
};


class DeadlineQosPolicy : public Parameter_t {
	Duration_t period;
};

class LatencyBudgetQosPolicy : public Parameter_t {
	Duration_t duration;
};

typedef enum LivelinessQosPolicyKind:octet {
	AUTOMATIC_LIVELINESS_QOS = 0x01,
			MANUAL_BY_PARTICIPANT_LIVELINESS_QOS=0x02,
			MANUAL_BY_TOPIC_LIVELINESS_QOS=0x04
};

class LivelinessQosPolicy : public Parameter_t {
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
class OwnershipQosPolicy : public Parameter_t {
	OwnershipQosPolicyKind kind;
};

typedef enum DestinationOrderQosPolicyKind :octet{
	BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS,
	BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS
};

struct ReliabilityQosPolicy : public Parameter_t
{
	ReliabilityQosPolicyKind kind;
	Duration_t max_blocking_time;
};

class DestinationOrderQosPolicy : public Parameter_t {
	DestinationOrderQosPolicyKind kind;
};

class UserDataQosPolicy : public Parameter_t{
	std::string data;
};

class TimeBasedFilterQosPolicy : public Parameter_t {
	Duration_t minimum_separation;
};

typedef enum PresentationQosPolicyAccessScopeKind:octet
{
	INSTANCE_PRESENTATION_QOS=0x01,
	TOPIC_PRESENTATION_QOS=0x02,
	GROUP_PRESENTATION_QOS=0x04
};
class PresentationQosPolicy : public Parameter_t
{
	PresentationQosPolicyAccessScopeKind access_scope;
	bool coherent_access;
	bool ordered_access;
};

class PartitionQosPolicy : public Parameter_t
{
	std::vector<std::string> name;
};

class TopicDataQosPolicy : public Parameter_t
{
	std::vector<std::string> value;
};
class GroupDataQosPolicy : public Parameter_t
{
	std::vector<std::string> value;
};

typedef enum HistoryQosPolicyKind:octet {
	KEEP_LAST_HISTORY_QOS=0x01,
	KEEP_ALL_HISTORY_QOS=0x02
};

class HistoryQosPolicy : public Parameter_t {
HistoryQosPolicyKind kind;
long depth;
};

class DurabilityServiceQosPolicy : public Parameter_t {
	Duration_t service_cleanup_delay;
	HistoryQosPolicyKind history_kind;
	uint32_t history_depth;
	uint32_t max_samples;
	uint32_t max_instances;
	uint32_t max_samples_per_instance;
};

class LifespanQosPolicy : public Parameter_t {
	Duration_t duration;
};


class OwnershipStrengthQosPolicy : public Parameter_t {
	uint32_t value;
};

class ResourceLimitsQosPolicy : public Parameter_t {
	uint32_t max_samples;
	uint32_t max_instances;
	uint32_t max_samples_per_instance;
};

class TransportPriorityQosPolicy : public Parameter_t {
	uint32_t value;
};





}
}



#endif /* DDS_QOS_POLICIES_H_ */
