/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file DiscoveredWriterData.h
 *
 *  Created on: Apr 8, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef DISCOVEREDWRITERDATA_H_
#define DISCOVEREDWRITERDATA_H_

#include "eprosimartps/qos/DDSQosPolicies.h"
#include "eprosimartps/reader/WriterProxy.h"
#include "eprosimartps/dds/attributes/TopicAttributes.h"

using namespace eprosima::dds;

namespace eprosima {
namespace rtps {




/**
 * Class DiscoveredWriterData used by the SEDP.
 */
class DiscoveredWriterData {
public:
	DiscoveredWriterData():
		userDefinedId(-1),isAlive(false),topicKind(NO_KEY){};
	virtual ~DiscoveredWriterData(){};
	WriterProxy_t m_writerProxy;
	InstanceHandle_t m_key;
	InstanceHandle_t m_participantKey;
	std::string m_typeName;
	std::string m_topicName;
	uint16_t userDefinedId;
	//FIXME: Check Qos default values in page 96 of DDS implementation
	WriterQos m_qos;
//	DurabilityQosPolicy m_durability;
//	DurabilityServiceQosPolicy m_durabilityService;
//	DeadlineQosPolicy m_deadline;
//	LatencyBudgetQosPolicy m_latencyBudget;
//	LivelinessQosPolicy m_liveliness;
//	ReliabilityQosPolicy m_reliability;
//	LifespanQosPolicy m_lifespan;
//	UserDataQosPolicy m_userData;
//	TimeBasedFilterQosPolicy m_timeBasedFilter;
//	OwnershipQosPolicy m_ownership;
//	OwnershipStrengthQosPolicy m_ownershipStrength;
//	DestinationOrderQosPolicy m_destinationOrder;
//	PresentationQosPolicy m_presentation;
//	PartitionQosPolicy m_partition;
//	TopicDataQosPolicy m_topicData;
//	GroupDataQosPolicy m_groupData;

	bool isAlive;
	TopicKind_t topicKind;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* DISCOVEREDWRITERDATA_H_ */
