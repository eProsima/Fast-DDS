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
 *  Created on: May 22, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef WRITERQOS_H_
#define WRITERQOS_H_

#include "eprosimartps/qos/DDSQosPolicies.h"

namespace eprosima {
namespace dds {

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
	void setQos(WriterQos& qos, bool first_time);
};

} /* namespace dds */
} /* namespace eprosima */

#endif /* WRITERQOS_H_ */
