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

#include <vector>
#include "eprosimartps/common/types/common_types.h"
#include "eprosimartps/common/types/Time_t.h"
#include "eprosimartps/qos/ParameterTypes.h"

using namespace eprosima::rtps;

namespace eprosima{
namespace dds{

class QosPolicy{
public:
	QosPolicy():hasChanged(false),m_sendAlways(false){};
	QosPolicy(bool b_sendAlways):hasChanged(false),m_sendAlways(b_sendAlways){};
	virtual ~ QosPolicy(){};
	bool hasChanged;
	bool sendAlways(){return m_sendAlways;}
protected:
	bool m_sendAlways;

};

typedef enum DurabilityQosPolicyKind_t: octet{
	VOLATILE_DURABILITY_QOS  ,
	TRANSIENT_LOCAL_DURABILITY_QOS ,
	TRANSIENT_DURABILITY_QOS ,
	PERSISTENT_DURABILITY_QOS
}DurabilityQosPolicyKind_t;

#define PARAMETER_KIND_LENGTH 4

class DurabilityQosPolicy : private Parameter_t, public QosPolicy
{
public:
	DurabilityQosPolicy():Parameter_t(PID_DURABILITY,PARAMETER_KIND_LENGTH),kind(VOLATILE_DURABILITY_QOS){};
	virtual ~DurabilityQosPolicy(){};
	DurabilityQosPolicyKind_t kind;
	bool addToCDRMessage(CDRMessage_t* msg);
};


class DeadlineQosPolicy : private Parameter_t, public QosPolicy {
public:
	DeadlineQosPolicy():Parameter_t(PID_DEADLINE,PARAMETER_TIME_LENGTH){};
	virtual ~DeadlineQosPolicy(){};
	Duration_t period;
	bool addToCDRMessage(CDRMessage_t* msg);
};

class LatencyBudgetQosPolicy : private Parameter_t, public QosPolicy {
public:
	LatencyBudgetQosPolicy():Parameter_t(PID_LATENCY_BUDGET,PARAMETER_TIME_LENGTH){};
	virtual ~LatencyBudgetQosPolicy(){};
	Duration_t duration;
	bool addToCDRMessage(CDRMessage_t* msg);
};

enum LivelinessQosPolicyKind:octet {
	AUTOMATIC_LIVELINESS_QOS ,
			MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,
			MANUAL_BY_TOPIC_LIVELINESS_QOS
};

class LivelinessQosPolicy : private Parameter_t, public QosPolicy {
public:
	LivelinessQosPolicy():Parameter_t(PID_LIVELINESS,PARAMETER_KIND_LENGTH+PARAMETER_TIME_LENGTH),
						kind(AUTOMATIC_LIVELINESS_QOS){};
	virtual ~LivelinessQosPolicy(){};
	LivelinessQosPolicyKind kind;
	Duration_t lease_duration;
	bool addToCDRMessage(CDRMessage_t* msg);
};

enum ReliabilityQosPolicyKind:octet {
	BEST_EFFORT_RELIABILITY_QOS,
	RELIABLE_RELIABILITY_QOS
};

enum OwnershipQosPolicyKind:octet {
	SHARED_OWNERSHIP_QOS,
			EXCLUSIVE_OWNERSHIP_QOS
};

class OwnershipQosPolicy : private Parameter_t, public QosPolicy {
public:
	OwnershipQosPolicy():Parameter_t(PID_OWNERSHIP,PARAMETER_KIND_LENGTH),
						kind(SHARED_OWNERSHIP_QOS){};
	virtual ~OwnershipQosPolicy(){};
	OwnershipQosPolicyKind kind;
	bool addToCDRMessage(CDRMessage_t* msg);
};

enum DestinationOrderQosPolicyKind :octet{
	BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS,
	BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS
};

class ReliabilityQosPolicy : private Parameter_t, public QosPolicy
{
public:
	ReliabilityQosPolicy():	Parameter_t(PID_RELIABILITY,PARAMETER_KIND_LENGTH+PARAMETER_TIME_LENGTH),
							QosPolicy(true), //indicate send always
							kind(BEST_EFFORT_RELIABILITY_QOS){};
	virtual ~ReliabilityQosPolicy(){};
	ReliabilityQosPolicyKind kind;
	Duration_t max_blocking_time;
	bool addToCDRMessage(CDRMessage_t* msg);
};

class DestinationOrderQosPolicy : private Parameter_t, public QosPolicy {
public:
	DestinationOrderQosPolicyKind kind;
	DestinationOrderQosPolicy():Parameter_t(PID_DESTINATION_ORDER,PARAMETER_KIND_LENGTH),
									kind(BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS){};
	virtual ~DestinationOrderQosPolicy(){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

class UserDataQosPolicy : private Parameter_t, public QosPolicy{
	friend class ParameterList;
public:
	UserDataQosPolicy():Parameter_t(PID_USER_DATA,0){};
	virtual ~UserDataQosPolicy(){};
	std::string data;
	bool addToCDRMessage(CDRMessage_t* msg);
};

class TimeBasedFilterQosPolicy : private Parameter_t, public QosPolicy {
public:
	Duration_t minimum_separation;
	TimeBasedFilterQosPolicy():Parameter_t(PID_TIME_BASED_FILTER,PARAMETER_TIME_LENGTH){};
	virtual ~TimeBasedFilterQosPolicy(){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

enum PresentationQosPolicyAccessScopeKind:octet
{
	INSTANCE_PRESENTATION_QOS,
	TOPIC_PRESENTATION_QOS,
	GROUP_PRESENTATION_QOS
};

#define PARAMETER_PRESENTATION_LENGTH 12

class PresentationQosPolicy : private Parameter_t, public QosPolicy
{
public:
	PresentationQosPolicyAccessScopeKind access_scope;
	bool coherent_access;
	bool ordered_access;
	PresentationQosPolicy():Parameter_t(PID_PRESENTATION,PARAMETER_PRESENTATION_LENGTH),
			access_scope(INSTANCE_PRESENTATION_QOS),
						coherent_access(false),ordered_access(false){};
	virtual ~PresentationQosPolicy(){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

class PartitionQosPolicy : private Parameter_t, public QosPolicy
{
	friend class ParameterList;
public:
	PartitionQosPolicy():Parameter_t(PID_PARTITION,0){};
	virtual ~PartitionQosPolicy(){};
	std::vector<octet> name;
	bool addToCDRMessage(CDRMessage_t* msg);
};

class TopicDataQosPolicy : private Parameter_t, public QosPolicy
{
	friend class ParameterList;
public:
	std::vector<octet> value;
	TopicDataQosPolicy():Parameter_t(PID_TOPIC_DATA,0){};
	virtual ~TopicDataQosPolicy(){};
	bool addToCDRMessage(CDRMessage_t* msg);
};
class GroupDataQosPolicy : private Parameter_t, public QosPolicy
{
	friend class ParameterList;
public:
	GroupDataQosPolicy():Parameter_t(PID_GROUP_DATA,0){}
	virtual ~GroupDataQosPolicy(){};
	std::vector<octet> value;
	bool addToCDRMessage(CDRMessage_t* msg);
};

 enum HistoryQosPolicyKind:octet {
	KEEP_LAST_HISTORY_QOS,
	KEEP_ALL_HISTORY_QOS
};

class HistoryQosPolicy : private Parameter_t, public QosPolicy {
public:
	HistoryQosPolicyKind kind;
	int32_t depth;
	HistoryQosPolicy():Parameter_t(PID_HISTORY,PARAMETER_KIND_LENGTH+4),
						kind(KEEP_LAST_HISTORY_QOS),depth(0){};
	virtual ~HistoryQosPolicy(){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

class DurabilityServiceQosPolicy : private Parameter_t, public QosPolicy {
public:
	Duration_t service_cleanup_delay;
	HistoryQosPolicyKind history_kind;
	uint32_t history_depth;
	uint32_t max_samples;
	uint32_t max_instances;
	uint32_t max_samples_per_instance;
	DurabilityServiceQosPolicy():Parameter_t(PID_DURABILITY_SERVICE,PARAMETER_TIME_LENGTH+PARAMETER_KIND_LENGTH+4+4+4+4),
			history_kind(KEEP_LAST_HISTORY_QOS),
						history_depth(1),max_samples(0),max_instances(0),max_samples_per_instance(0){};
	virtual ~DurabilityServiceQosPolicy(){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

class LifespanQosPolicy : private Parameter_t, public QosPolicy {
public:
	Duration_t duration;
	LifespanQosPolicy():Parameter_t(PID_LIFESPAN,PARAMETER_TIME_LENGTH){};
	virtual ~LifespanQosPolicy(){};
	bool addToCDRMessage(CDRMessage_t* msg);
};


class OwnershipStrengthQosPolicy : private Parameter_t, public QosPolicy {
public:
	uint32_t value;
	OwnershipStrengthQosPolicy():Parameter_t(PID_OWNERSHIP_STRENGTH,4),value(0){};
	virtual ~OwnershipStrengthQosPolicy(){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

class ResourceLimitsQosPolicy : private Parameter_t, public QosPolicy {
public:
	uint32_t max_samples;
	uint32_t max_instances;
	uint32_t max_samples_per_instance;
	ResourceLimitsQosPolicy():Parameter_t(PID_RESOURCE_LIMITS,4+4+4),
			max_samples(0),max_instances(0),max_samples_per_instance(0){};
	virtual ~ResourceLimitsQosPolicy(){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

class TransportPriorityQosPolicy : private Parameter_t , public QosPolicy{
public:
	uint32_t value;
	TransportPriorityQosPolicy():Parameter_t(PID_TRANSPORT_PRIORITY,4),value(0){};
	virtual ~TransportPriorityQosPolicy(){};
	bool addToCDRMessage(CDRMessage_t* msg);
};


}
}



#endif /* DDS_QOS_POLICIES_H_ */
