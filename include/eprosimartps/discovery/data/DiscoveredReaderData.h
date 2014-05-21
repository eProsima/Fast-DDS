/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file DiscoveredReaderData.h
 *
 *  Created on: Apr 8, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef DISCOVEREDREADERDATA_H_
#define DISCOVEREDREADERDATA_H_

#include "eprosimartps/qos/DDSQosPolicies.h"
#include "eprosimartps/writer/ReaderProxy.h"
#include "eprosimartps/common/attributes/TopicAttributes.h"

using namespace eprosima::dds;

namespace eprosima {
namespace rtps {

class ReaderQos{
public:
	ReaderQos(){};
	virtual ~ReaderQos(){};
	DurabilityQosPolicy m_durability;
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
		DurabilityServiceQosPolicy m_durabilityService;
		LifespanQosPolicy m_lifespan;
		void setQos(ReaderQos& readerqos, bool first_time);
};


/**
 * Class DiscoveredReaderData used by the SEDP.
 */
class DiscoveredReaderData {
public:
	DiscoveredReaderData():
		userDefinedId(-1),isAlive(false),topicKind(NO_KEY){};
	virtual ~DiscoveredReaderData();
	ReaderProxy_t m_readerProxy;
	InstanceHandle_t m_key;
	InstanceHandle_t m_participantKey;
	std::string m_typeName;
	std::string m_topicName;
	uint16_t userDefinedId;
ReaderQos m_qos;


	bool isAlive;
	TopicKind_t topicKind;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* DISCOVEREDREADERDATA_H_ */
