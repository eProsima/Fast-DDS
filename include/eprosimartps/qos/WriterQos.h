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

#include "eprosimartps/qos/DDSQosPolicies.h"

namespace eprosima {
namespace dds {


/**
 * WriterQos class contains all the possible Qos that can be set for a determined Publisher/Writer.
 * Although this values can be set and are transmitted in the Discovery the behaviour associated with them is yet to be implemented.
 */
class WriterQos{
public:
	WriterQos(){};
	virtual ~ WriterQos(){};
	DurabilityQosPolicy m_durability;
	DurabilityServiceQosPolicy m_durabilityService;
	DeadlineQosPolicy m_deadline;
	LatencyBudgetQosPolicy m_latencyBudget;
	LivelinessQosPolicy m_liveliness;
	ReliabilityQosPolicy m_reliability;
	LifespanQosPolicy m_lifespan;
	UserDataQosPolicy m_userData;
	TimeBasedFilterQosPolicy m_timeBasedFilter;
	OwnershipQosPolicy m_ownership;
	OwnershipStrengthQosPolicy m_ownershipStrength;
	DestinationOrderQosPolicy m_destinationOrder;
	PresentationQosPolicy m_presentation;
	PartitionQosPolicy m_partition;
	TopicDataQosPolicy m_topicData;
	GroupDataQosPolicy m_groupData;
	void setQos( WriterQos& qos, bool first_time);
	bool checkQos();
};



} /* namespace dds */
} /* namespace eprosima */

#endif /* WRITERQOS_H_ */
