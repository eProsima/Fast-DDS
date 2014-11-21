/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WriterQos.h
 *
 */

#ifndef WRITERQOS_H_
#define WRITERQOS_H_

#include "eprosimartps/pubsub/qos/QosPolicies.h"

namespace eprosima {
namespace pubsub {


/**
 * WriterQos class contains all the possible Qos that can be set for a determined Publisher. Although these values can be set and are transmitted
 * during the EDP, not all of the behaviour associated with them has been implemented in the library.
 * The supported Qos in this version are: Durability, Liveliness, Reliability and Partition.
 */
class RTPS_DllAPI WriterQos{
public:
	WriterQos();
	virtual ~ WriterQos();
	//!Durability Qos, implemented in the library.
	DurabilityQosPolicy m_durability;
	//!Durability Service Qos, NOT implemented in the library.
	DurabilityServiceQosPolicy m_durabilityService;
	//!Deadline Qos, NOT implemented in the library.
	DeadlineQosPolicy m_deadline;
	//!Latency Budget Qos, NOT implemented in the library.
	LatencyBudgetQosPolicy m_latencyBudget;
	//!Liveliness Qos, implemented in the library.
	LivelinessQosPolicy m_liveliness;
	//!Reliability Qos, implemented in the library.
	ReliabilityQosPolicy m_reliability;
	//!Lifespan Qos, NOT implemented in the library.
	LifespanQosPolicy m_lifespan;
	//!UserData Qos, NOT implemented in the library.
	UserDataQosPolicy m_userData;
	//!Time Based Filter Qos, NOT implemented in the library.
	TimeBasedFilterQosPolicy m_timeBasedFilter;
	//!Ownership Qos, NOT implemented in the library.
	OwnershipQosPolicy m_ownership;
	//!Owenership Strength Qos, NOT implemented in the library.
	OwnershipStrengthQosPolicy m_ownershipStrength;
	//!Destination Order Qos, NOT implemented in the library.
	DestinationOrderQosPolicy m_destinationOrder;
	//!Presentation Qos, NOT implemented in the library.
	PresentationQosPolicy m_presentation;
	//!Partition Qos, implemented in the library.
	PartitionQosPolicy m_partition;
	//!Topic Data Qos, NOT implemented in the library.
	TopicDataQosPolicy m_topicData;
	//!Group Data Qos, NOT implemented in the library.
	GroupDataQosPolicy m_groupData;
	/**
	 * Set Qos from another class
	 * @param qos Reference from a WriterQos object.
	 * @param first_time Boolean indicating whether is the first time (If not some parameters cannot be set).
	 */
	void setQos(const WriterQos& qos, bool first_time);
	/**
	 * Check if the Qos values are compatible between each other.
	 * @return True if correct.
	 */
	bool checkQos();

	bool canQosBeUpdated(WriterQos& qos);
};



} /* namespace pubsub */
} /* namespace eprosima */

#endif /* WRITERQOS_H_ */
