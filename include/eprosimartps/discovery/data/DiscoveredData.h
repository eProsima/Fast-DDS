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

class DiscoveredData_t {
public:
	DiscoveredData_t();
	virtual ~DiscoveredData_t();

	GUID_t remoteGuid;
	LocatorList_t m_unicastLocatorList;
	LocatorList_t m_multicastLocatorList;
	LocatorList_t m_metatrafficUnicastLocatorList;
	LocatorList_t m_metatrafficMulticastLocatorList;
	LocatorList_t m_defaultUnicastLocatorList;
	LocatorList_t m_defaultMulticastLocatorList;
	InstanceHandle_t m_key;
	InstanceHandle_t m_participantKey;
	std::string m_typeName;
	std::string m_topicName;
	uint16_t userDefinedId;
	GUID_t m_participantGuid;
	bool m_expectsInlineQos;
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
	Duration_t m_leaseDuration;
	BuiltinEndpointSet_t m_availableBuiltinEndpoints;
	std::string m_participantName;

	LifespanQosPolicy m_lifespan;
	TransportPriorityQosPolicy m_transportPriority;


	HistoryQosPolicy m_history;
	ResourceLimitsQosPolicy m_resourceLimits;

	OwnershipStrengthQosPolicy m_ownershipStrength;
	ParameterPropertyList_t m_propertyList;
	ProtocolVersion_t m_protocolVersion;
	VendorId_t m_VendorId;

};

class DiscoveredData
{
public:
	DiscoveredData(){};
	virtual ~DiscoveredData(){};
	bool processParameterList(ParameterList_t& param,DiscoveredData_t* data);
	bool DiscoveredData2WriterData(DiscoveredData_t* ddata,DiscoveredWriterData* wdata);
	bool DiscoveredData2ReaderData(DiscoveredData_t* ddata,DiscoveredReaderData* wdata);
	bool DiscoveredData2TopicData(DiscoveredData_t* ddata,DiscoveredTopicData* wdata);
	bool DiscoveredData2ParticipantData(DiscoveredData_t* ddata,DiscoveredParticipantData* wdata);
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* DISCOVEREDDATA_H_ */
