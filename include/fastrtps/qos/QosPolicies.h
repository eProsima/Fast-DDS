/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file QosPolicies.h
 *
 */

#ifndef QOS_POLICIES_H_
#define QOS_POLICIES_H_

#include <vector>
#include "../rtps/common/Types.h"
#include "../rtps/common/Time_t.h"
#include "ParameterTypes.h"
using namespace eprosima::fastrtps::rtps;


namespace eprosima{
namespace fastrtps{

namespace rtps{
class EDP;
}

/**
 * Class QosPolicy is a base class for all the different QoS defined to the Writers and Readers.
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
 * Enum DurabilityQosPolicyKind_t, different kinds of durability for DurabilityQosPolicy.
 */
typedef enum DurabilityQosPolicyKind: octet{
	VOLATILE_DURABILITY_QOS  ,      //!< Volatile Durability (default for Subscribers).
	TRANSIENT_LOCAL_DURABILITY_QOS ,//!< Transient Local Durability (default for Publishers).
	TRANSIENT_DURABILITY_QOS ,      //!< NOT IMPLEMENTED.
	PERSISTENT_DURABILITY_QOS       //!< NOT IMPLEMENTED.
}DurabilityQosPolicyKind_t;

#define PARAMETER_KIND_LENGTH 4

/**
 * Class DurabilityQosPolicy, to indicate the durability of the samples.
 * kind: Default value for Subscribers: VOLATILE_DURABILITY_QOS, for Publishers TRANSIENT_LOCAL_DURABILITY_QOS
 */
class RTPS_DllAPI DurabilityQosPolicy : private Parameter_t, public QosPolicy
{
public:
	DurabilityQosPolicy():Parameter_t(PID_DURABILITY,PARAMETER_KIND_LENGTH),QosPolicy(true),kind(VOLATILE_DURABILITY_QOS){};
	virtual ~DurabilityQosPolicy(){};
	DurabilityQosPolicyKind_t kind;
	bool addToCDRMessage(CDRMessage_t* msg);
};

/**
 * Class DeadlineQosPolicy, to indicate the Deadline of the samples.
 * This QosPolicy can be defined and is transmitted to the rest of the network but is not implemented in this version.
 * period: Default value c_TimeInifinite.
 */
class RTPS_DllAPI DeadlineQosPolicy : private Parameter_t, public QosPolicy {
public:
	DeadlineQosPolicy():Parameter_t(PID_DEADLINE,PARAMETER_TIME_LENGTH),QosPolicy(true),period(c_TimeInfinite){	};
	virtual ~DeadlineQosPolicy(){};
	Duration_t period;
	bool addToCDRMessage(CDRMessage_t* msg);
};

/**
 * Class LatencyBudgetQosPolicy, to indicate the LatencyBudget of the samples.
 * This QosPolicy can be defined and is transmitted to the rest of the network but is not implemented in this version.
 * period: Default value c_TimeZero.
 */
class RTPS_DllAPI LatencyBudgetQosPolicy : private Parameter_t, public QosPolicy {
public:
	LatencyBudgetQosPolicy():Parameter_t(PID_LATENCY_BUDGET,PARAMETER_TIME_LENGTH),QosPolicy(true),duration(c_TimeZero){};
	virtual ~LatencyBudgetQosPolicy(){};
	Duration_t duration;
	bool addToCDRMessage(CDRMessage_t* msg);
};

/**
 * Enum LivelinessQosPolicyKind, different kinds of liveliness for LivelinessQosPolicy
 */
typedef enum LivelinessQosPolicyKind:octet {
	AUTOMATIC_LIVELINESS_QOS ,             //!< Automatic Liveliness, default value.
	MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,//!< MANUAL_BY_PARTICIPANT_LIVELINESS_QOS
	MANUAL_BY_TOPIC_LIVELINESS_QOS       //!< MANUAL_BY_TOPIC_LIVELINESS_QOS
}LivelinessQosPolicyKind;

/**
 * Class LivelinessQosPolicy, to indicate the Liveliness of the Writers.
 * This QosPolicy can be defined for the Subscribers and is transmitted but only the Writer Liveliness protocol
 * is implemented in this version. The user should set the lease_duration and the announcement_period with values that differ
 * in at least 30%. Values too close to each other may cause the failure of the writer liveliness assertion in networks
 * with high latency or with lots of communication errors.
 * kind: Default value AUTOMATIC_LIVELINESS_QOS
 * lease_duration: Default value c_TimeInfinite.
 * announcement_period: Default value c_TimeInfinite (must be < lease_duration).
 */
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

/**
 * Enum ReliabilityQosPolicyKind, different kinds of reliability for ReliabilityQosPolicy.
 */
typedef enum ReliabilityQosPolicyKind:octet {
	BEST_EFFORT_RELIABILITY_QOS = 0x01, //!< Best Effort reliability (default for Subscribers).
			RELIABLE_RELIABILITY_QOS = 0x02 //!< Reliable reliability (default for Publishers).
}ReliabilityQosPolicyKind;

/**
 * Class ReliabilityQosPolicy, to indicate the reliability of the endpoints.
 * kind: Default value BEST_EFFORT_RELIABILITY_QOS for ReaderQos and RELIABLE_RELIABILITY_QOS for WriterQos.
 * max_blocking_time: Not Used in this version.
 */
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



/**
 * Enum OwnershipQosPolicyKind, different kinds of ownership for OwnershipQosPolicy.
 */
enum OwnershipQosPolicyKind:octet {
	SHARED_OWNERSHIP_QOS, //!< Shared Ownership, default value.
	EXCLUSIVE_OWNERSHIP_QOS //!< Exclusive ownership
};

/**
 * Class OwnershipQosPolicy, to indicate the ownership kind of the endpoints.
 * kind: Default value SHARED_OWNERSHIP_QOS.
 */
class RTPS_DllAPI OwnershipQosPolicy : private Parameter_t, public QosPolicy {
public:
	OwnershipQosPolicy():Parameter_t(PID_OWNERSHIP,PARAMETER_KIND_LENGTH),QosPolicy(true),
	kind(SHARED_OWNERSHIP_QOS){};
	virtual ~OwnershipQosPolicy(){};
	OwnershipQosPolicyKind kind;
	bool addToCDRMessage(CDRMessage_t* msg);
};

/**
 * Enum OwnershipQosPolicyKind, different kinds of destination order for DestinationOrderQosPolicy.
 */
enum DestinationOrderQosPolicyKind :octet{
	BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS, //!< By Reception Timestamp, default value.
	BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS //!< By Source Timestamp.
};



/**
 * Class DestinationOrderQosPolicy, to indicate the Destination Order Qos.
 * This QosPolicy can be defined and is transmitted to the rest of the network but is not implemented in this version.
 * kind: Default value BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS
 */
class RTPS_DllAPI DestinationOrderQosPolicy : private Parameter_t, public QosPolicy {
public:
	DestinationOrderQosPolicyKind kind;
	DestinationOrderQosPolicy():Parameter_t(PID_DESTINATION_ORDER,PARAMETER_KIND_LENGTH),QosPolicy(true),
			kind(BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS){};
	virtual ~DestinationOrderQosPolicy(){};
	bool addToCDRMessage(CDRMessage_t* msg);
};


/**
 * Class UserDataQosPolicy, to transmit user data during the discovery phase.
 */
class UserDataQosPolicy : private Parameter_t, public QosPolicy{
	friend class ParameterList;
public:
	RTPS_DllAPI UserDataQosPolicy() :Parameter_t(PID_USER_DATA, 0), QosPolicy(false){};
	RTPS_DllAPI virtual ~UserDataQosPolicy(){};

	RTPS_DllAPI bool addToCDRMessage(CDRMessage_t* msg);

	RTPS_DllAPI inline std::vector<octet> getDataVec(){ return dataVec; };
	RTPS_DllAPI inline void setDataVec(std::vector<octet>& vec){ dataVec = vec; };
private:
	std::vector<octet> dataVec;
};

/**
 * Class TimeBasedFilterQosPolicy, to indicate the Time Based Filter Qos.
 * This QosPolicy can be defined and is transmitted to the rest of the network but is not implemented in this version.
 * minimum_separation: Default value c_TimeZero
 */
class RTPS_DllAPI TimeBasedFilterQosPolicy : private Parameter_t, public QosPolicy {
public:

	TimeBasedFilterQosPolicy():Parameter_t(PID_TIME_BASED_FILTER,PARAMETER_TIME_LENGTH),QosPolicy(false),minimum_separation(c_TimeZero){};
	virtual ~TimeBasedFilterQosPolicy(){};
	Duration_t minimum_separation;
	bool addToCDRMessage(CDRMessage_t* msg);
};

/**
 * Enum PresentationQosPolicyAccessScopeKind, different kinds of Presentation Policy order for PresentationQosPolicy.
 */
enum PresentationQosPolicyAccessScopeKind:octet
{
	INSTANCE_PRESENTATION_QOS, //!< Instance Presentation, default value.
	TOPIC_PRESENTATION_QOS, //!< Topic Presentation.
	GROUP_PRESENTATION_QOS //!< Group Presentation.
};

#define PARAMETER_PRESENTATION_LENGTH 8

/**
 * Class PresentationQosPolicy, to indicate the Presentation Qos Policy.
 * This QosPolicy can be defined and is transmitted to the rest of the network but is not implemented in this version.
 * access_scope: Default value INSTANCE_PRESENTATION_QOS
 * coherent_access: Default value false.
 * ordered_access: Default value false.
 */
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


/**
 * Class PartitionQosPolicy, to indicate the Partition Qos.
 */
class  PartitionQosPolicy : private Parameter_t, public QosPolicy
{
	friend class ParameterList;
	friend class rtps::EDP;
public:
	RTPS_DllAPI PartitionQosPolicy() :Parameter_t(PID_PARTITION, 0), QosPolicy(false){};
	RTPS_DllAPI virtual ~PartitionQosPolicy(){};
	RTPS_DllAPI bool addToCDRMessage(CDRMessage_t* msg);
    RTPS_DllAPI inline void push_back(const char* oc){ names.push_back(std::string(oc)); hasChanged=true; };
	RTPS_DllAPI inline void clear(){ names.clear(); };
	RTPS_DllAPI inline std::vector<std::string> getNames(){ return names; };
    RTPS_DllAPI inline void setNames(std::vector<std::string>& nam){ names = nam; };
private:
	std::vector<std::string> names;
};


/**
 * Class TopicDataQosPolicy, to indicate the Topic Data.
 */
class  TopicDataQosPolicy : private Parameter_t, public QosPolicy
{
	friend class ParameterList;
public:
	RTPS_DllAPI TopicDataQosPolicy() :Parameter_t(PID_TOPIC_DATA, 0), QosPolicy(false){};
	RTPS_DllAPI virtual ~TopicDataQosPolicy(){};
	RTPS_DllAPI bool addToCDRMessage(CDRMessage_t* msg);
	RTPS_DllAPI inline void push_back(octet oc){ value.push_back(oc); };
	RTPS_DllAPI inline void clear(){ value.clear(); };
	RTPS_DllAPI inline void setValue(std::vector<octet> ocv){ value = ocv; };
	RTPS_DllAPI inline std::vector<octet> getValue(){ return value; };
private:
	std::vector<octet> value;
};

/**
 * Class GroupDataQosPolicy, to indicate the Group Data.
 */
class  GroupDataQosPolicy : private Parameter_t, public QosPolicy
{
	friend class ParameterList;
public:
	RTPS_DllAPI GroupDataQosPolicy() :Parameter_t(PID_GROUP_DATA, 0), QosPolicy(false){}
	RTPS_DllAPI virtual ~GroupDataQosPolicy(){};
	RTPS_DllAPI bool addToCDRMessage(CDRMessage_t* msg);
	RTPS_DllAPI inline void push_back(octet oc){ value.push_back(oc); };
	RTPS_DllAPI inline void clear(){ value.clear(); };
	RTPS_DllAPI inline void setValue(std::vector<octet> ocv){ value = ocv; };
	RTPS_DllAPI inline std::vector<octet> getValue(){ return value; };
private:
	std::vector<octet> value;
};

/**
 * Enum HistoryQosPolicyKind, different kinds of History Qos for HistoryQosPolicy.
  */
enum HistoryQosPolicyKind:octet {
	KEEP_LAST_HISTORY_QOS, //!< Keep only a number of samples, default value.
	KEEP_ALL_HISTORY_QOS //!< Keep all samples until the ResourceLimitsQosPolicy are exhausted.
};

/**
 * Class HistoryQosPolicy, defines the HistoryQos of the topic in the Writer or Reader side.
 * kind: Default value KEEP_LAST_HISTORY_QOS.
 * depth: Default value 1000.
 */
class RTPS_DllAPI HistoryQosPolicy : private Parameter_t, public QosPolicy {
public:
	HistoryQosPolicyKind kind;
	int32_t depth;
	HistoryQosPolicy():Parameter_t(PID_HISTORY,PARAMETER_KIND_LENGTH+4),QosPolicy(true),
			kind(KEEP_LAST_HISTORY_QOS),depth(1000){};
	virtual ~HistoryQosPolicy(){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

/**
 * Class ResourceLimitsQosPolicy, defines the ResourceLimits for the Writer or the Reader.
 * max_samples: Default value 5000.
 * max_instances: Default value 10.
 * max_samples_per_instance: Default value 400.
 * allocated_samples: Default value 3000.
 */
class RTPS_DllAPI ResourceLimitsQosPolicy : private Parameter_t, public QosPolicy {
public:
	int32_t max_samples;
	int32_t max_instances;
	int32_t max_samples_per_instance;
	int32_t allocated_samples;
	ResourceLimitsQosPolicy():Parameter_t(PID_RESOURCE_LIMITS,4+4+4),QosPolicy(false),
			max_samples(5000),max_instances(10),max_samples_per_instance(400),allocated_samples(3000){};
	virtual ~ResourceLimitsQosPolicy(){};
	bool addToCDRMessage(CDRMessage_t* msg);
};



/**
 * Class DurabilityServiceQosPolicy, to indicate the Durability Service.
 * This QosPolicy can be defined and is transmitted to the rest of the network but is not implemented in this version.
 * service_cleanup_delay: Default value c_TimeZero.
 * history_kind: Default value KEEP_LAST_HISTORY_QOS.
 * history_depth: Default value 1.
 * max_samples: Default value -1.
 * max_instances: Default value -1.
 * max_samples_per_instance: Default value -1.
 */
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

/**
 * Class LifespanQosPolicy.
 * This QosPolicy can be defined and is transmitted to the rest of the network but is not implemented in this version.
 * duration: Default value c_TimeInfinite.
 */
class RTPS_DllAPI LifespanQosPolicy : private Parameter_t, public QosPolicy {
public:
	LifespanQosPolicy():Parameter_t(PID_LIFESPAN,PARAMETER_TIME_LENGTH),QosPolicy(true),duration(c_TimeInfinite){};
	virtual ~LifespanQosPolicy(){};
	Duration_t duration;
	bool addToCDRMessage(CDRMessage_t* msg);
};

/**
 * Class OwnershipStrengthQosPolicy, to indicate the strength of the ownership.
 * value: Default value 0.
 */
class RTPS_DllAPI OwnershipStrengthQosPolicy : private Parameter_t, public QosPolicy {
public:
	uint32_t value;
	OwnershipStrengthQosPolicy():Parameter_t(PID_OWNERSHIP_STRENGTH,4),QosPolicy(false),value(0){};
	virtual ~OwnershipStrengthQosPolicy(){};
	bool addToCDRMessage(CDRMessage_t* msg);
};



/**
 * Class TransportPriorityQosPolicy.
 * This QosPolicy can be defined and is transmitted to the rest of the network but is not implemented in this version.
 * value: Default value 0.
 */
class RTPS_DllAPI TransportPriorityQosPolicy : private Parameter_t , public QosPolicy{
public:
	uint32_t value;
	TransportPriorityQosPolicy():Parameter_t(PID_TRANSPORT_PRIORITY,4),QosPolicy(false),value(0){};
	virtual ~TransportPriorityQosPolicy(){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

/**
* Enum PublishModeQosPolicyKind, different kinds of publication synchronism
*/
typedef enum PublishModeQosPolicyKind : octet{
	SYNCHRONOUS_PUBLISH_MODE,	//!< Synchronous publication mode (default for writers).
	ASYNCHRONOUS_PUBLISH_MODE	//!< Asynchronous publication mode.
}PublishModeQosPolicyKind_t;

/**
* Class PublishModeQosPolicy, defines the publication mode for a specific writer.
* kind: Default value SYNCHRONOUS_PUBLISH_MODE.
*/
class RTPS_DllAPI PublishModeQosPolicy : public QosPolicy {
public:
	PublishModeQosPolicyKind kind;
	PublishModeQosPolicy() : kind(SYNCHRONOUS_PUBLISH_MODE){};
	virtual ~PublishModeQosPolicy(){};
};


}
}



#endif /* QOS_POLICIES_H_ */
