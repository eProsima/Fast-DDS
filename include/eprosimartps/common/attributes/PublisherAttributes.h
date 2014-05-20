/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PublisherAttributes.h
 *	Publisher Attributes
 *  Created on: May 5, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef PUBLISHERPARAMETERS_H_
#define PUBLISHERPARAMETERS_H_

//#include "eprosimartps/qos/DDSQosPolicies.h"

namespace eprosima {
namespace dds {


//
//class PublisherQos{
//public:
//	PublisherQos();
//	virtual ~PublisherQos();
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
//};



/**
 * Class PublisherAttributes, used by the user to define the attributes of a Publisher.
 * @ingroup ATTRIBUTESMODULE
 */
class PublisherAttributes {
public:
	PublisherAttributes()
{
		pushMode = true;
		historyMaxSize = 10;
		userDefinedId = -1;
};
	virtual ~PublisherAttributes(){};
	//! If set to true the Publisher will send the data directly, if set to false it will send
	//! a Heartbeat message and wait for ACKNACK messages (option only available for RELIABLE Publishers.)
	//! to send the data.
	bool pushMode;
	/**
	 * Maximum size of the History.
	 */
	uint16_t historyMaxSize;
	//! Unicast LocatorList where the writer should be listening for responses (RELIABLE only).
	LocatorList_t unicastLocatorList;
	//!MulticastLocatorList where the writer should be listening for responses (RELIABLE only).
	LocatorList_t multicastLocatorList;
	//!Reliability parameters for the Publisher
	PublisherReliability reliability;
	//!Topic Attributes for the Publisher
	TopicAttributes topic;
	//! User defined Id for this Publisher (only needed in STATICEDP)
	int16_t userDefinedId;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* PUBLISHERPARAMETERS_H_ */
