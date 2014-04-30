/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file DiscoveredTopicData.h
 *
 *  Created on: Apr 8, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef DISCOVEREDTOPICDATA_H_
#define DISCOVEREDTOPICDATA_H_

namespace eprosima {
namespace rtps {

class DiscoveredTopicData {
public:
	DiscoveredTopicData();
	virtual ~DiscoveredTopicData();
	InstanceHandle_t m_key;
	std::string m_typeName;
	std::string m_topicName;

	DurabilityQosPolicy m_durability;
	DeadlineQosPolicy m_deadline;
	LatencyBudgetQosPolicy m_latencyBudget;
	LivelinessQosPolicy m_liveliness;
	ReliabilityQosPolicy m_reliability;
	TransportPriorityQosPolicy m_transportPriority;
	LifespanQosPolicy m_lifespan;
	DestinationOrderQosPolicy m_destinationOrder;
	PresentationQosPolicy m_presentation;
	HistoryQosPolicy m_history;
	ResourceLimitsQosPolicy m_resourceLimits;
	OwnershipQosPolicy m_ownership;
	TopicDataQosPolicy m_topicData;



};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* DISCOVEREDTOPICDATA_H_ */
