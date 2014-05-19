/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file DiscoveredData.h
 *
 *  Created on: May 19, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef DISCOVEREDDATA_H_
#define DISCOVEREDDATA_H_

#include "eprosimartps/qos/DDSQosPolicies.h"
#include "eprosimartps/writer/ReaderProxy.h"
#include "eprosimartps/reader/WriterProxy.h"

namespace eprosima {
namespace rtps {

class DiscoveredData {
public:
	DiscoveredData();
	virtual ~DiscoveredData();

	GUID_t remoteGuid;
	LocatorList_t unicastLocatorList;
	LocatorList_t multicastLocatorList;
	InstanceHandle_t m_key;
	InstanceHandle_t m_participantKey;
	std::string m_typeName;
	std::string m_topicName;
	uint16_t userDefinedId;

	bool expectsInlineQos;
	TopicKind_t topicKind;

	DurabilityQosPolicy m_durability;
	DurabilityServiceQosPolicy m_durabilityService;
	DeadlineQosPolicy m_deadline;
	LatencyBudgetQosPolicy m_latencyBudget;
	LivelinessQosPolicy m_liveliness;
	ReliabilityQosPolicy m_reliability;
	OwnershipQosPolicy m_ownership;
	DestinationOrderQosPolicy m_destinationOrder;
	UserDataQosPolicy m_userData;
	TimeBasedFilterQosPolicy m_timeBasedFilter;
	PresentationQosPolicy m_presentation;
	PartitionQosPolicy m_partition;
	TopicDataQosPolicy m_topicData;
	GroupDataQosPolicy m_groupData;

	LifespanQosPolicy m_lifespan;
	TransportPriorityQosPolicy m_transportPriority;


	HistoryQosPolicy m_history;
	ResourceLimitsQosPolicy m_resourceLimits;

	OwnershipStrengthQosPolicy m_ownershipStrength;



};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* DISCOVEREDDATA_H_ */
