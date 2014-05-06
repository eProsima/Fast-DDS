/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file DDSQosPolicies.h
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

/**
 * @addtogroup PARAMETERMODULE
 * @{
 */



typedef enum DurabilityQosPolicyKind: octet{
	VOLATILE_DURABILITY_QOS = 0x01,        //!< VOLATILE_DURABILITY_QOS
			TRANSIENT_LOCAL_DURABILITY_QOS =0x02,//!< TRANSIENT_LOCAL_DURABILITY_QOS
			TRANSIENT_DURABILITY_QOS = 0x04,     //!< TRANSIENT_DURABILITY_QOS
			PERSISTENT_DURABILITY_QOS = 0x08     //!< PERSISTENT_DURABILITY_QOS
}DurabilityQosPolicyKind;

#define PARAMETER_KIND_LENGTH 4

class DurabilityQosPolicy : public Parameter_t
{
	DurabilityQosPolicy():Parameter_t(PID_DURABILITY,PARAMETER_KIND_LENGTH),kind(VOLATILE_DURABILITY_QOS){};
	DurabilityQosPolicyKind kind;
	bool addToCDRMessage(CDRMessage_t* msg);
};


class DeadlineQosPolicy : public Parameter_t {
	DeadlineQosPolicy():Parameter_t(PID_DEADLINE,PARAMETER_TIME_LENGTH){};
	Duration_t period;
	bool addToCDRMessage(CDRMessage_t* msg);
};

class LatencyBudgetQosPolicy : public Parameter_t {
	LatencyBudgetQosPolicy():Parameter_t(PID_LATENCY_BUDGET,PARAMETER_TIME_LENGTH){};
	Duration_t duration;
	bool addToCDRMessage(CDRMessage_t* msg);
};

typedef enum LivelinessQosPolicyKind:octet {
	AUTOMATIC_LIVELINESS_QOS = 0x01,
			MANUAL_BY_PARTICIPANT_LIVELINESS_QOS=0x02,
			MANUAL_BY_TOPIC_LIVELINESS_QOS=0x04
}LivelinessQosPolicyKind;

class LivelinessQosPolicy : public Parameter_t {
	LivelinessQosPolicy():Parameter_t(PID_LIVELINESS,PARAMETER_KIND_LENGTH+PARAMETER_TIME_LENGTH),
			kind(AUTOMATIC_LIVELINESS_QOS){};
	LivelinessQosPolicyKind kind;
	Duration_t lease_duration;
	bool addToCDRMessage(CDRMessage_t* msg);
};

typedef enum ReliabilityQosPolicyKind:octet {
	BEST_EFFORT_RELIABILITY_QOS= 0x01,
	RELIABLE_RELIABILITY_QOS=0x02
}ReliabilityQosPolicyKind;

typedef enum OwnershipQosPolicyKind:octet {
	SHARED_OWNERSHIP_QOS=0x01,
			EXCLUSIVE_OWNERSHIP_QOS=0x02
}OwnershipQosPolicyKind;

class OwnershipQosPolicy : public Parameter_t {
	OwnershipQosPolicy():Parameter_t(PID_OWNERSHIP,PARAMETER_KIND_LENGTH),
			kind(SHARED_OWNERSHIP_QOS){};
	OwnershipQosPolicyKind kind;
	bool addToCDRMessage(CDRMessage_t* msg);
};

typedef enum DestinationOrderQosPolicyKind :octet{
	BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS,
	BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS
}DestinationOrderQosPolicyKind;

struct ReliabilityQosPolicy : public Parameter_t
{
	ReliabilityQosPolicy():	Parameter_t(PID_RELIABILITY,PARAMETER_KIND_LENGTH+PARAMETER_TIME_LENGTH),
			kind(BEST_EFFORT_RELIABILITY_QOS){};
	ReliabilityQosPolicyKind kind;
	Duration_t max_blocking_time;
	bool addToCDRMessage(CDRMessage_t* msg);
};

class DestinationOrderQosPolicy : public Parameter_t {
	DestinationOrderQosPolicyKind kind;
	DestinationOrderQosPolicy():Parameter_t(PID_DESTINATION_ORDER,PARAMETER_KIND_LENGTH),
			kind(BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

class UserDataQosPolicy : public ParameterString_t{
	UserDataQosPolicy():ParameterString_t(PID_USER_DATA,0){};
};

class TimeBasedFilterQosPolicy : public Parameter_t {
	Duration_t minimum_separation;
	TimeBasedFilterQosPolicy():Parameter_t(PID_TIME_BASED_FILTER,PARAMETER_TIME_LENGTH){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

typedef enum PresentationQosPolicyAccessScopeKind:octet
{
	INSTANCE_PRESENTATION_QOS=0x01,
	TOPIC_PRESENTATION_QOS=0x02,
	GROUP_PRESENTATION_QOS=0x04
}PresentationQosPolicyAccessScopeKind;

#define PARAMETER_PRESENTATION_LENGTH 12

class PresentationQosPolicy : public Parameter_t
{
	PresentationQosPolicyAccessScopeKind access_scope;
	bool coherent_access;
	bool ordered_access;
	PresentationQosPolicy():Parameter_t(PID_PRESENTATION,PARAMETER_PRESENTATION_LENGTH),
			access_scope(INSTANCE_PRESENTATION_QOS),
			coherent_access(false),ordered_access(false){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

class PartitionQosPolicy : public Parameter_t
{
	PartitionQosPolicy():Parameter_t(PID_PARTITION,0){};
	std::vector<std::string> name;
	bool addToCDRMessage(CDRMessage_t* msg);
};

class TopicDataQosPolicy : public Parameter_t
{
	std::vector<std::string> value;
	TopicDataQosPolicy():Parameter_t(PID_TOPIC_DATA,0){};
	bool addToCDRMessage(CDRMessage_t* msg);
};
class GroupDataQosPolicy : public Parameter_t
{
	GroupDataQosPolicy():Parameter_t(PID_GROUP_DATA,0){}
	std::vector<std::string> value;
	bool addToCDRMessage(CDRMessage_t* msg);
};

typedef enum HistoryQosPolicyKind:octet {
	KEEP_LAST_HISTORY_QOS=0x01,
	KEEP_ALL_HISTORY_QOS=0x02
}HistoryQosPolicyKind;

class HistoryQosPolicy : public Parameter_t {
	HistoryQosPolicyKind kind;
	int32_t depth;
	HistoryQosPolicy():Parameter_t(PID_HISTORY,PARAMETER_KIND_LENGTH+4),
			kind(KEEP_LAST_HISTORY_QOS),depth(0){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

class DurabilityServiceQosPolicy : public Parameter_t {
	Duration_t service_cleanup_delay;
	HistoryQosPolicyKind history_kind;
	uint32_t history_depth;
	uint32_t max_samples;
	uint32_t max_instances;
	uint32_t max_samples_per_instance;
	DurabilityServiceQosPolicy():Parameter_t(PID_DURABILITY_SERVICE,PARAMETER_TIME_LENGTH+PARAMETER_KIND_LENGTH+4+4+4+4),
			history_kind(KEEP_LAST_HISTORY_QOS),
			history_depth(0),max_samples(0),max_instances(0),max_samples_per_instance(0){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

class LifespanQosPolicy : public Parameter_t {
	Duration_t duration;
	LifespanQosPolicy():Parameter_t(PID_LIFESPAN,PARAMETER_TIME_LENGTH){};
	bool addToCDRMessage(CDRMessage_t* msg);
};


class OwnershipStrengthQosPolicy : public Parameter_t {
	uint32_t value;
	OwnershipStrengthQosPolicy():Parameter_t(PID_OWNERSHIP_STRENGTH,4),value(0){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

class ResourceLimitsQosPolicy : public Parameter_t {
	uint32_t max_samples;
	uint32_t max_instances;
	uint32_t max_samples_per_instance;
	ResourceLimitsQosPolicy():Parameter_t(PID_RESOURCE_LIMITS,4+4+4),
			max_samples(0),max_instances(0),max_samples_per_instance(0){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

class TransportPriorityQosPolicy : public Parameter_t {
	uint32_t value;
	TransportPriorityQosPolicy():Parameter_t(PID_TRANSPORT_PRIORITY,4),value(0){};
	bool addToCDRMessage(CDRMessage_t* msg);
};


///@}


}
}



#endif /* DDS_QOS_POLICIES_H_ */
