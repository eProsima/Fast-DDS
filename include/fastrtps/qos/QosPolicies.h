/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file QosPolicies.h
 *
*/

#ifndef QOS_POLICIES_H_
#define QOS_POLICIES_H_

#include <vector>
#include "fastrtps/rtps/common/Types.h"
#include "fastrtps/rtps/common/Time_t.h"
#include "fastrtps/qos/ParameterTypes.h"
#include "fastrtps/config/eprosima_stl_exports.hpp"
using namespace eprosima::fastrtps::rtps;


namespace eprosima{
namespace fastrtps{

/**
 * QosPolicy is a base class for all the different QoS defined to the Writers and Readers.
 */
class RTPS_DllAPI QosPolicy{
public:
	QosPolicy():hasChanged(false),m_sendAlways(false){};
	QosPolicy(bool b_sendAlways):hasChanged(false),m_sendAlways(b_sendAlways){};
	virtual ~ QosPolicy(){};
	bool hasChanged;
	bool sendAlways(){return m_sendAlways;}
protected:
	bool m_sendAlways;

};
/**
 * Enum DurabilityQosPolicyKind_t, different types of durability.
 */
typedef enum DurabilityQosPolicyKind_t: octet{
	VOLATILE_DURABILITY_QOS  ,      //!< VOLATILE_DURABILITY_QOS
	TRANSIENT_LOCAL_DURABILITY_QOS ,//!< TRANSIENT_LOCAL_DURABILITY_QOS
	TRANSIENT_DURABILITY_QOS ,      //!< TRANSIENT_DURABILITY_QOS, NOT IMPLEMENTED
	PERSISTENT_DURABILITY_QOS       //!< PERSISTENT_DURABILITY_QOS, NOT IMPLEMENTED
}DurabilityQosPolicyKind_t;

#define PARAMETER_KIND_LENGTH 4

class RTPS_DllAPI DurabilityQosPolicy : private Parameter_t, public QosPolicy
{
public:
	DurabilityQosPolicy():Parameter_t(PID_DURABILITY,PARAMETER_KIND_LENGTH),QosPolicy(true),kind(VOLATILE_DURABILITY_QOS){};
	virtual ~DurabilityQosPolicy(){};
	DurabilityQosPolicyKind_t kind;
	bool addToCDRMessage(CDRMessage_t* msg);
};


class RTPS_DllAPI DeadlineQosPolicy : private Parameter_t, public QosPolicy {
public:
	DeadlineQosPolicy():Parameter_t(PID_DEADLINE,PARAMETER_TIME_LENGTH),QosPolicy(true),period(c_TimeInfinite){	};
	virtual ~DeadlineQosPolicy(){};
	Duration_t period;
	bool addToCDRMessage(CDRMessage_t* msg);
};

class RTPS_DllAPI LatencyBudgetQosPolicy : private Parameter_t, public QosPolicy {
public:
	LatencyBudgetQosPolicy():Parameter_t(PID_LATENCY_BUDGET,PARAMETER_TIME_LENGTH),QosPolicy(true),duration(c_TimeZero){};
	virtual ~LatencyBudgetQosPolicy(){};
	Duration_t duration;
	bool addToCDRMessage(CDRMessage_t* msg);
};

/**
 * Different types of Liveliness
 */
enum LivelinessQosPolicyKind:octet {
	AUTOMATIC_LIVELINESS_QOS ,             //!< AUTOMATIC_LIVELINESS_QOS
			MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,//!< MANUAL_BY_RTPSParticipant_LIVELINESS_QOS
			MANUAL_BY_TOPIC_LIVELINESS_QOS       //!< MANUAL_BY_TOPIC_LIVELINESS_QOS
};

class RTPS_DllAPI LivelinessQosPolicy : private Parameter_t, public QosPolicy {
public:
	LivelinessQosPolicy():Parameter_t(PID_LIVELINESS,PARAMETER_KIND_LENGTH+PARAMETER_TIME_LENGTH),QosPolicy(true),
						kind(AUTOMATIC_LIVELINESS_QOS){lease_duration = c_TimeInfinite; announcement_period = c_TimeInfinite;};
	virtual ~LivelinessQosPolicy(){};
	LivelinessQosPolicyKind kind;
	Duration_t lease_duration;
	Duration_t announcement_period;
	bool addToCDRMessage(CDRMessage_t* msg);
};

enum ReliabilityQosPolicyKind:octet {
	BEST_EFFORT_RELIABILITY_QOS = 0x01,
	RELIABLE_RELIABILITY_QOS = 0x02
};

enum OwnershipQosPolicyKind:octet {
	SHARED_OWNERSHIP_QOS,
			EXCLUSIVE_OWNERSHIP_QOS
};

class RTPS_DllAPI OwnershipQosPolicy : private Parameter_t, public QosPolicy {
public:
	OwnershipQosPolicy():Parameter_t(PID_OWNERSHIP,PARAMETER_KIND_LENGTH),QosPolicy(true),
						kind(SHARED_OWNERSHIP_QOS){};
	virtual ~OwnershipQosPolicy(){};
	OwnershipQosPolicyKind kind;
	bool addToCDRMessage(CDRMessage_t* msg);
};

enum DestinationOrderQosPolicyKind :octet{
	BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS,
	BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS
};

class RTPS_DllAPI ReliabilityQosPolicy : private Parameter_t, public QosPolicy
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

class RTPS_DllAPI DestinationOrderQosPolicy : private Parameter_t, public QosPolicy {
public:
	DestinationOrderQosPolicyKind kind;
	DestinationOrderQosPolicy():Parameter_t(PID_DESTINATION_ORDER,PARAMETER_KIND_LENGTH),QosPolicy(true),
									kind(BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS){};
	virtual ~DestinationOrderQosPolicy(){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

class RTPS_DllAPI UserDataQosPolicy : private Parameter_t, public QosPolicy{
	friend class ParameterList;
public:
	UserDataQosPolicy():Parameter_t(PID_USER_DATA,0),QosPolicy(false){};
	virtual ~UserDataQosPolicy(){};
	std::vector<octet> dataVec;
	bool addToCDRMessage(CDRMessage_t* msg);
};

class RTPS_DllAPI TimeBasedFilterQosPolicy : private Parameter_t, public QosPolicy {
public:

	TimeBasedFilterQosPolicy():Parameter_t(PID_TIME_BASED_FILTER,PARAMETER_TIME_LENGTH),QosPolicy(false),minimum_separation(c_TimeZero){};
	virtual ~TimeBasedFilterQosPolicy(){};
	Duration_t minimum_separation;
	bool addToCDRMessage(CDRMessage_t* msg);
};

enum PresentationQosPolicyAccessScopeKind:octet
{
	INSTANCE_PRESENTATION_QOS,
	TOPIC_PRESENTATION_QOS,
	GROUP_PRESENTATION_QOS
};

#define PARAMETER_PRESENTATION_LENGTH 8

class RTPS_DllAPI PresentationQosPolicy : private Parameter_t, public QosPolicy
{
public:
	PresentationQosPolicyAccessScopeKind access_scope;
	bool coherent_access;
	bool ordered_access;
	PresentationQosPolicy():Parameter_t(PID_PRESENTATION,PARAMETER_PRESENTATION_LENGTH),QosPolicy(false),
			access_scope(INSTANCE_PRESENTATION_QOS),
						coherent_access(false),ordered_access(false){};
	virtual ~PresentationQosPolicy(){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

class RTPS_DllAPI PartitionQosPolicy : private Parameter_t, public QosPolicy
{
	friend class ParameterList;
public:
	PartitionQosPolicy():Parameter_t(PID_PARTITION,0),QosPolicy(false){};
	virtual ~PartitionQosPolicy(){};
	//std::vector<octet> name;
	std::vector<std::string> names;
	bool addToCDRMessage(CDRMessage_t* msg);
};

class RTPS_DllAPI TopicDataQosPolicy : private Parameter_t, public QosPolicy
{
	friend class ParameterList;
public:
	std::vector<octet> value;
	TopicDataQosPolicy():Parameter_t(PID_TOPIC_DATA,0),QosPolicy(false){};
	virtual ~TopicDataQosPolicy(){};
	bool addToCDRMessage(CDRMessage_t* msg);
};
class RTPS_DllAPI GroupDataQosPolicy : private Parameter_t, public QosPolicy
{
	friend class ParameterList;
public:
	GroupDataQosPolicy():Parameter_t(PID_GROUP_DATA,0),QosPolicy(false){}
	virtual ~GroupDataQosPolicy(){};
	std::vector<octet> value;
	bool addToCDRMessage(CDRMessage_t* msg);
};

 enum HistoryQosPolicyKind:octet {
	KEEP_LAST_HISTORY_QOS,
	KEEP_ALL_HISTORY_QOS
};

class RTPS_DllAPI HistoryQosPolicy : private Parameter_t, public QosPolicy {
public:
	HistoryQosPolicyKind kind;
	int32_t depth;
	HistoryQosPolicy():Parameter_t(PID_HISTORY,PARAMETER_KIND_LENGTH+4),QosPolicy(true),
						kind(KEEP_LAST_HISTORY_QOS),depth(1000){};
	virtual ~HistoryQosPolicy(){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

class RTPS_DllAPI DurabilityServiceQosPolicy : private Parameter_t, public QosPolicy {
public:
	Duration_t service_cleanup_delay;
	HistoryQosPolicyKind history_kind;
	int32_t history_depth;
	int32_t max_samples;
	int32_t max_instances;
	int32_t max_samples_per_instance;
	DurabilityServiceQosPolicy():Parameter_t(PID_DURABILITY_SERVICE,PARAMETER_TIME_LENGTH+PARAMETER_KIND_LENGTH+4+4+4+4),QosPolicy(false),
			history_kind(KEEP_LAST_HISTORY_QOS),
						history_depth(1),max_samples(-1),max_instances(-1),max_samples_per_instance(-1){};
	virtual ~DurabilityServiceQosPolicy(){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

class RTPS_DllAPI LifespanQosPolicy : private Parameter_t, public QosPolicy {
public:
	LifespanQosPolicy():Parameter_t(PID_LIFESPAN,PARAMETER_TIME_LENGTH),QosPolicy(true),duration(c_TimeInfinite){};
	virtual ~LifespanQosPolicy(){};
	Duration_t duration;
	bool addToCDRMessage(CDRMessage_t* msg);
};


class RTPS_DllAPI OwnershipStrengthQosPolicy : private Parameter_t, public QosPolicy {
public:
	uint32_t value;
	OwnershipStrengthQosPolicy():Parameter_t(PID_OWNERSHIP_STRENGTH,4),QosPolicy(false),value(0){};
	virtual ~OwnershipStrengthQosPolicy(){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

class RTPS_DllAPI ResourceLimitsQosPolicy : private Parameter_t, public QosPolicy {
public:
	int32_t max_samples;
	int32_t max_instances;
	int32_t max_samples_per_instance;
	int32_t allocated_samples;
	ResourceLimitsQosPolicy():Parameter_t(PID_RESOURCE_LIMITS,4+4+4),QosPolicy(false),
			max_samples(1000),max_instances(5),max_samples_per_instance(200),allocated_samples(1000){};
	virtual ~ResourceLimitsQosPolicy(){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

class RTPS_DllAPI TransportPriorityQosPolicy : private Parameter_t , public QosPolicy{
public:
	uint32_t value;
	TransportPriorityQosPolicy():Parameter_t(PID_TRANSPORT_PRIORITY,4),QosPolicy(false),value(0){};
	virtual ~TransportPriorityQosPolicy(){};
	bool addToCDRMessage(CDRMessage_t* msg);
};


}
}



#endif /* QOS_POLICIES_H_ */
