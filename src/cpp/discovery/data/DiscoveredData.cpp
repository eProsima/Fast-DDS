/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file DiscoveredData.cpp
 *
 */


#include "eprosimartps/discovery/data/DiscoveredData.h"
#include "eprosimartps/common/types/InstanceHandle.h"
#include "eprosimartps/qos/DDSQosPolicies.h"

#include "eprosimartps/utils/RTPSLog.h"


namespace eprosima{

namespace rtps{

bool DiscoveredData::ParameterList2DiscoveredWriterData(ParameterList_t& param, DiscoveredWriterData* wdata)
{
	for(std::vector<Parameter_t*>::iterator it = param.m_parameters.begin();
			it!=param.m_parameters.end();++it)
	{
		switch((*it)->Pid)
		{
		case PID_DURABILITY:
		{
			DurabilityQosPolicy * p = (DurabilityQosPolicy*)(*it);
			wdata->m_qos.m_durability = *p;
			break;
		}
		case PID_DURABILITY_SERVICE:
		{
			DurabilityServiceQosPolicy * p = (DurabilityServiceQosPolicy*)(*it);
			wdata->m_qos.m_durabilityService = *p;
			break;
		}
		case PID_DEADLINE:
		{
			DeadlineQosPolicy * p = (DeadlineQosPolicy*)(*it);
			wdata->m_qos.m_deadline = *p;
			break;
		}
		case PID_LATENCY_BUDGET:
		{
			LatencyBudgetQosPolicy * p = (LatencyBudgetQosPolicy*)(*it);
			wdata->m_qos.m_latencyBudget = *p;
			break;
		}
		case PID_LIVELINESS:
		{
			LivelinessQosPolicy * p = (LivelinessQosPolicy*)(*it);
			wdata->m_qos.m_liveliness = *p;
			wdata->m_writerProxy.leaseDuration = p->lease_duration;
			wdata->m_writerProxy.livelinessKind = p->kind;
			break;
		}
		case PID_RELIABILITY:
		{
			ReliabilityQosPolicy * p = (ReliabilityQosPolicy*)(*it);
			wdata->m_qos.m_reliability = *p;
			break;
		}
		case PID_LIFESPAN:
		{

			LifespanQosPolicy * p = (LifespanQosPolicy*)(*it);
			wdata->m_qos.m_lifespan = *p;
			break;
		}
		case PID_USER_DATA:
		{
			UserDataQosPolicy * p = (UserDataQosPolicy*)(*it);
			wdata->m_qos.m_userData = *p;
			break;
		}
		case PID_TIME_BASED_FILTER:
		{
			TimeBasedFilterQosPolicy * p = (TimeBasedFilterQosPolicy*)(*it);
			wdata->m_qos.m_timeBasedFilter = *p;
			break;
		}
		case PID_OWNERSHIP:
		{
			OwnershipQosPolicy * p = (OwnershipQosPolicy*)(*it);
			wdata->m_qos.m_ownership = *p;
			break;
		}
		case PID_OWNERSHIP_STRENGTH:
		{
			OwnershipStrengthQosPolicy * p = (OwnershipStrengthQosPolicy*)(*it);
			wdata->m_qos.m_ownershipStrength = *p;
			break;
		}
		case PID_DESTINATION_ORDER:
		{
			DestinationOrderQosPolicy * p = (DestinationOrderQosPolicy*)(*it);
			wdata->m_qos.m_destinationOrder = *p;
			break;
		}

		case PID_PRESENTATION:
		{
			PresentationQosPolicy * p = (PresentationQosPolicy*)(*it);
			wdata->m_qos.m_presentation = *p;
			break;
		}
		case PID_PARTITION:
		{
			PartitionQosPolicy * p = (PartitionQosPolicy*)(*it);
			wdata->m_qos.m_partition = *p;
			break;
		}
		case PID_TOPIC_DATA:
		{
			TopicDataQosPolicy * p = (TopicDataQosPolicy*)(*it);
			wdata->m_qos.m_topicData = *p;
			break;
		}
		case PID_GROUP_DATA:
		{

			GroupDataQosPolicy * p = (GroupDataQosPolicy*)(*it);
			wdata->m_qos.m_groupData = *p;
			break;
		}
		case PID_TOPIC_NAME:
		{
			ParameterString_t*p = (ParameterString_t*)(*it);
			wdata->m_topicName = p->m_string;
			break;
		}
		case PID_TYPE_NAME:
		{
			ParameterString_t*p = (ParameterString_t*)(*it);
			wdata->m_typeName = p->m_string;
			break;
		}
		case PID_PARTICIPANT_GUID:
		{
			ParameterGuid_t * p = (ParameterGuid_t*)(*it);
			for(uint8_t i =0;i<16;++i)
			{
				if(i<12)
					wdata->m_participantKey.value[i] = p->guid.guidPrefix.value[i];
				else
					wdata->m_participantKey.value[i] = p->guid.entityId.value[i];
			}
			break;
		}
		case PID_ENDPOINT_GUID:
		{
			ParameterGuid_t * p = (ParameterGuid_t*)(*it);
			wdata->m_writerProxy.remoteWriterGuid = p->guid;
			for(uint8_t i=0;i<16;++i)
			{
				if(i<12)
					wdata->m_key.value[i] = p->guid.guidPrefix.value[i];
				else
					wdata->m_key.value[i] = p->guid.entityId.value[i-12];
			}
			break;
		}
		case PID_UNICAST_LOCATOR:
		{
			ParameterLocator_t* p = (ParameterLocator_t*)(*it);
			wdata->m_writerProxy.unicastLocatorList.push_back(p->locator);
			break;
		}
		case PID_MULTICAST_LOCATOR:
		{
			ParameterLocator_t* p = (ParameterLocator_t*)(*it);
			wdata->m_writerProxy.multicastLocatorList.push_back(p->locator);
			break;
		}
		case PID_KEY_HASH:
		{
			ParameterKey_t*p=(ParameterKey_t*)(*it);
			wdata->m_key = p->key;
			iHandle2GUID(wdata->m_writerProxy.remoteWriterGuid,wdata->m_key);
			break;
		}
		default:
		{
			pWarning("Parameter with ID: " << std::hex <<(*it)->Pid <<std::dec << " NOT CONSIDERED"<< endl);
			break;
		}
		}
	}
	return true;
}

bool DiscoveredData::ParameterList2DiscoveredReaderData(ParameterList_t& param, DiscoveredReaderData* rdata)
{
	for(std::vector<Parameter_t*>::iterator it = param.m_parameters.begin();
			it!=param.m_parameters.end();++it)
	{
		switch((*it)->Pid)
		{
		case PID_DURABILITY:
		{
			DurabilityQosPolicy * p = (DurabilityQosPolicy*)(*it);
			rdata->m_qos.m_durability = *p;
			break;
		}
		case PID_DURABILITY_SERVICE:
		{
			DurabilityServiceQosPolicy * p = (DurabilityServiceQosPolicy*)(*it);
			rdata->m_qos.m_durabilityService = *p;
			break;
		}
		case PID_DEADLINE:
		{
			DeadlineQosPolicy * p = (DeadlineQosPolicy*)(*it);
			rdata->m_qos.m_deadline = *p;
			break;
		}
		case PID_LATENCY_BUDGET:
		{
			LatencyBudgetQosPolicy * p = (LatencyBudgetQosPolicy*)(*it);
			rdata->m_qos.m_latencyBudget = *p;
			break;
		}
		case PID_LIVELINESS:
		{
			LivelinessQosPolicy * p = (LivelinessQosPolicy*)(*it);
			rdata->m_qos.m_liveliness = *p;
			break;
		}
		case PID_RELIABILITY:
		{
			ReliabilityQosPolicy * p = (ReliabilityQosPolicy*)(*it);
			rdata->m_qos.m_reliability = *p;
			break;
		}
		case PID_LIFESPAN:
		{

			LifespanQosPolicy * p = (LifespanQosPolicy*)(*it);
			rdata->m_qos.m_lifespan = *p;
			break;
		}
		case PID_USER_DATA:
		{
			UserDataQosPolicy * p = (UserDataQosPolicy*)(*it);
			rdata->m_qos.m_userData = *p;
			break;
		}
		case PID_TIME_BASED_FILTER:
		{
			TimeBasedFilterQosPolicy * p = (TimeBasedFilterQosPolicy*)(*it);
			rdata->m_qos.m_timeBasedFilter = *p;
			break;
		}
		case PID_OWNERSHIP:
		{
			OwnershipQosPolicy * p = (OwnershipQosPolicy*)(*it);
			rdata->m_qos.m_ownership = *p;
			break;
		}
		case PID_DESTINATION_ORDER:
		{
			DestinationOrderQosPolicy * p = (DestinationOrderQosPolicy*)(*it);
			rdata->m_qos.m_destinationOrder = *p;
			break;
		}

		case PID_PRESENTATION:
		{
			PresentationQosPolicy * p = (PresentationQosPolicy*)(*it);
			rdata->m_qos.m_presentation = *p;
			break;
		}
		case PID_PARTITION:
		{
			PartitionQosPolicy * p = (PartitionQosPolicy*)(*it);
			rdata->m_qos.m_partition = *p;
			break;
		}
		case PID_TOPIC_DATA:
		{
			TopicDataQosPolicy * p = (TopicDataQosPolicy*)(*it);
			rdata->m_qos.m_topicData = *p;
			break;
		}
		case PID_GROUP_DATA:
		{

			GroupDataQosPolicy * p = (GroupDataQosPolicy*)(*it);
			rdata->m_qos.m_groupData = *p;
			break;
		}
		case PID_TOPIC_NAME:
		{
			ParameterString_t*p = (ParameterString_t*)(*it);
			rdata->m_topicName = p->m_string;
			break;
		}
		case PID_TYPE_NAME:
		{
			ParameterString_t*p = (ParameterString_t*)(*it);
			rdata->m_typeName = p->m_string;
			break;
		}
		case PID_PARTICIPANT_GUID:
		{
			ParameterGuid_t * p = (ParameterGuid_t*)(*it);
			for(uint8_t i =0;i<16;++i)
			{
				if(i<12)
					rdata->m_participantKey.value[i] = p->guid.guidPrefix.value[i];
				else
					rdata->m_participantKey.value[i] = p->guid.entityId.value[i];
			}
			break;
		}
		case PID_ENDPOINT_GUID:
		{
			ParameterGuid_t * p = (ParameterGuid_t*)(*it);
			rdata->m_readerProxy.remoteReaderGuid = p->guid;
			for(uint8_t i=0;i<16;++i)
			{
				if(i<12)
					rdata->m_key.value[i] = p->guid.guidPrefix.value[i];
				else
					rdata->m_key.value[i] = p->guid.entityId.value[i-12];
			}
			break;
		}
		case PID_UNICAST_LOCATOR:
		{
			ParameterLocator_t* p = (ParameterLocator_t*)(*it);
			rdata->m_readerProxy.unicastLocatorList.push_back(p->locator);
			break;
		}
		case PID_MULTICAST_LOCATOR:
		{
			ParameterLocator_t* p = (ParameterLocator_t*)(*it);
			rdata->m_readerProxy.multicastLocatorList.push_back(p->locator);
			break;
		}
		case PID_EXPECTS_INLINE_QOS:
		{
			ParameterBool_t*p =(ParameterBool_t*)(*it);
			rdata->m_readerProxy.expectsInlineQos = p->value;
			break;
		}
		case PID_KEY_HASH:
		{
			ParameterKey_t*p=(ParameterKey_t*)(*it);
			rdata->m_key = p->key;
			iHandle2GUID(rdata->m_readerProxy.remoteReaderGuid,rdata->m_key);
			break;
		}
		default:
		{
			pWarning("Parameter with ID: " << std::hex <<(*it)->Pid <<std::dec << " NOT CONSIDERED"<< endl);
			break;
		}
		}
	}
	return true;

}

bool DiscoveredData::DiscoveredWriterData2ParameterList(DiscoveredWriterData& wdata, ParameterList_t* param)
{
	for(LocatorListIterator lit = wdata.m_writerProxy.unicastLocatorList.begin();
			lit!=wdata.m_writerProxy.unicastLocatorList.end();++lit)
	{
		ParameterLocator_t* p = new ParameterLocator_t(PID_UNICAST_LOCATOR,PARAMETER_LOCATOR_LENGTH,*lit);
		param->m_parameters.push_back((Parameter_t*)p);
	}
	for(LocatorListIterator lit = wdata.m_writerProxy.multicastLocatorList.begin();
			lit!=wdata.m_writerProxy.multicastLocatorList.end();++lit)
	{
		ParameterLocator_t* p = new ParameterLocator_t(PID_MULTICAST_LOCATOR,PARAMETER_LOCATOR_LENGTH,*lit);
		param->m_parameters.push_back((Parameter_t*)p);
	}
	{
		ParameterGuid_t* p = new ParameterGuid_t(PID_PARTICIPANT_GUID,PARAMETER_GUID_LENGTH,wdata.m_participantKey);
		param->m_parameters.push_back((Parameter_t*)p);
	}
	{
		ParameterString_t * p = new ParameterString_t(PID_TOPIC_NAME,0,wdata.m_topicName);
		param->m_parameters.push_back((Parameter_t*)p);
	}
	{
		ParameterString_t * p = new ParameterString_t(PID_TYPE_NAME,0,wdata.m_typeName);
		param->m_parameters.push_back((Parameter_t*)p);
	}
	{
		ParameterKey_t * p = new ParameterKey_t(PID_KEY_HASH,16,wdata.m_key);
		param->m_parameters.push_back((Parameter_t*)p);
	}
	{
		ParameterGuid_t * p = new ParameterGuid_t(PID_ENDPOINT_GUID,16,wdata.m_writerProxy.remoteWriterGuid);
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if( wdata.m_qos.m_durability.sendAlways() || wdata.m_qos.m_durability.hasChanged)
	{
		DurabilityQosPolicy*p = new DurabilityQosPolicy();
		*p = wdata.m_qos.m_durability;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if(wdata.m_qos.m_durabilityService.sendAlways() || wdata.m_qos.m_durabilityService.hasChanged)
	{
		DurabilityServiceQosPolicy*p = new DurabilityServiceQosPolicy();
		*p = wdata.m_qos.m_durabilityService;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if(wdata.m_qos.m_deadline.sendAlways() ||  wdata.m_qos.m_deadline.hasChanged)
	{
		DeadlineQosPolicy*p = new DeadlineQosPolicy();
		*p = wdata.m_qos.m_deadline;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if(wdata.m_qos.m_latencyBudget.sendAlways() ||  wdata.m_qos.m_latencyBudget.hasChanged)
	{
		LatencyBudgetQosPolicy*p = new LatencyBudgetQosPolicy();
		*p = wdata.m_qos.m_latencyBudget;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if(wdata.m_qos.m_durability.sendAlways() ||  wdata.m_qos.m_liveliness.hasChanged)
	{
		LivelinessQosPolicy*p = new LivelinessQosPolicy();
		*p = wdata.m_qos.m_liveliness;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if(wdata.m_qos.m_reliability.sendAlways() ||  wdata.m_qos.m_reliability.hasChanged)
	{
		ReliabilityQosPolicy*p = new ReliabilityQosPolicy();
		*p = wdata.m_qos.m_reliability;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if(wdata.m_qos.m_lifespan.sendAlways() ||  wdata.m_qos.m_lifespan.hasChanged)
	{
		LifespanQosPolicy*p = new LifespanQosPolicy();
		*p = wdata.m_qos.m_lifespan;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if( wdata.m_qos.m_userData.sendAlways() || wdata.m_qos.m_userData.hasChanged)
	{
		UserDataQosPolicy*p = new UserDataQosPolicy();
		*p = wdata.m_qos.m_userData;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if(wdata.m_qos.m_timeBasedFilter.sendAlways() ||  wdata.m_qos.m_timeBasedFilter.hasChanged)
	{
		TimeBasedFilterQosPolicy*p = new TimeBasedFilterQosPolicy();
		*p = wdata.m_qos.m_timeBasedFilter;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if(wdata.m_qos.m_ownership.sendAlways() ||  wdata.m_qos.m_ownership.hasChanged)
	{
		OwnershipQosPolicy*p = new OwnershipQosPolicy();
		*p = wdata.m_qos.m_ownership;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if(wdata.m_qos.m_durability.sendAlways() ||  wdata.m_qos.m_ownershipStrength.hasChanged)
	{
		OwnershipStrengthQosPolicy*p = new OwnershipStrengthQosPolicy();
		*p = wdata.m_qos.m_ownershipStrength;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if(wdata.m_qos.m_destinationOrder.sendAlways() ||  wdata.m_qos.m_destinationOrder.hasChanged)
	{
		DestinationOrderQosPolicy*p = new DestinationOrderQosPolicy();
		*p = wdata.m_qos.m_destinationOrder;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if(wdata.m_qos.m_presentation.sendAlways() ||  wdata.m_qos.m_presentation.hasChanged)
	{
		PresentationQosPolicy*p = new PresentationQosPolicy();
		*p = wdata.m_qos.m_presentation;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if(wdata.m_qos.m_partition.sendAlways() ||  wdata.m_qos.m_partition.hasChanged)
	{
		PartitionQosPolicy*p = new PartitionQosPolicy();
		*p = wdata.m_qos.m_partition;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if(wdata.m_qos.m_topicData.sendAlways() || wdata.m_qos.m_topicData.hasChanged)
	{
		TopicDataQosPolicy*p = new TopicDataQosPolicy();
		*p = wdata.m_qos.m_topicData;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if(wdata.m_qos.m_groupData.sendAlways() ||  wdata.m_qos.m_groupData.hasChanged)
	{
		GroupDataQosPolicy*p = new GroupDataQosPolicy();
		*p = wdata.m_qos.m_groupData;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	pDebugInfo(RTPS_CYAN << "DiscoveredWriterData converted to ParameterList with " << param->m_parameters.size()<< " parameters"<<endl);
	return true;
}

bool DiscoveredData::DiscoveredReaderData2ParameterList(DiscoveredReaderData& rdata, ParameterList_t* param)
{
	for(LocatorListIterator lit = rdata.m_readerProxy.unicastLocatorList.begin();
			lit!=rdata.m_readerProxy.unicastLocatorList.end();++lit)
	{
		ParameterLocator_t* p = new ParameterLocator_t(PID_UNICAST_LOCATOR,PARAMETER_LOCATOR_LENGTH,*lit);
		param->m_parameters.push_back((Parameter_t*)p);
	}
	for(LocatorListIterator lit = rdata.m_readerProxy.multicastLocatorList.begin();
			lit!=rdata.m_readerProxy.multicastLocatorList.end();++lit)
	{
		ParameterLocator_t* p = new ParameterLocator_t(PID_MULTICAST_LOCATOR,PARAMETER_LOCATOR_LENGTH,*lit);
		param->m_parameters.push_back((Parameter_t*)p);
	}
	{
		ParameterBool_t * p = new ParameterBool_t(PID_EXPECTS_INLINE_QOS,PARAMETER_BOOL_LENGTH,rdata.m_readerProxy.expectsInlineQos);
		param->m_parameters.push_back((Parameter_t*)p);
	}
	{
		ParameterGuid_t* p = new ParameterGuid_t(PID_PARTICIPANT_GUID,PARAMETER_GUID_LENGTH,rdata.m_participantKey);
		param->m_parameters.push_back((Parameter_t*)p);
	}
	{
		ParameterString_t * p = new ParameterString_t(PID_TOPIC_NAME,0,rdata.m_topicName);
		param->m_parameters.push_back((Parameter_t*)p);
	}
	{
		ParameterString_t * p = new ParameterString_t(PID_TYPE_NAME,0,rdata.m_typeName);
		param->m_parameters.push_back((Parameter_t*)p);
	}
	{
		ParameterKey_t * p = new ParameterKey_t(PID_KEY_HASH,16,rdata.m_key);
		param->m_parameters.push_back((Parameter_t*)p);
	}
	{
		ParameterGuid_t * p = new ParameterGuid_t(PID_ENDPOINT_GUID,16,rdata.m_readerProxy.remoteReaderGuid);
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if(rdata.m_qos.m_durability.sendAlways() || rdata.m_qos.m_durability.hasChanged)
	{
		DurabilityQosPolicy*p = new DurabilityQosPolicy();
		*p = rdata.m_qos.m_durability;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if(rdata.m_qos.m_durabilityService.sendAlways() || rdata.m_qos.m_durabilityService.hasChanged)
	{
		DurabilityServiceQosPolicy*p = new DurabilityServiceQosPolicy();
		*p = rdata.m_qos.m_durabilityService;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if(rdata.m_qos.m_deadline.sendAlways() || rdata.m_qos.m_deadline.hasChanged)
	{
		DeadlineQosPolicy*p = new DeadlineQosPolicy();
		*p = rdata.m_qos.m_deadline;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if(rdata.m_qos.m_latencyBudget.sendAlways() || rdata.m_qos.m_latencyBudget.hasChanged)
	{
		LatencyBudgetQosPolicy*p = new LatencyBudgetQosPolicy();
		*p = rdata.m_qos.m_latencyBudget;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if(rdata.m_qos.m_liveliness.sendAlways() || rdata.m_qos.m_liveliness.hasChanged)
	{
		LivelinessQosPolicy*p = new LivelinessQosPolicy();
		*p = rdata.m_qos.m_liveliness;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if(rdata.m_qos.m_reliability.sendAlways() || rdata.m_qos.m_reliability.hasChanged)
	{
		ReliabilityQosPolicy*p = new ReliabilityQosPolicy();
		*p = rdata.m_qos.m_reliability;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if(rdata.m_qos.m_lifespan.sendAlways() || rdata.m_qos.m_lifespan.hasChanged)
	{
		LifespanQosPolicy*p = new LifespanQosPolicy();
		*p = rdata.m_qos.m_lifespan;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if(rdata.m_qos.m_userData.sendAlways() || rdata.m_qos.m_userData.hasChanged)
	{
		UserDataQosPolicy*p = new UserDataQosPolicy();
		*p = rdata.m_qos.m_userData;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if(rdata.m_qos.m_timeBasedFilter.sendAlways() || rdata.m_qos.m_timeBasedFilter.hasChanged)
	{
		TimeBasedFilterQosPolicy*p = new TimeBasedFilterQosPolicy();
		*p = rdata.m_qos.m_timeBasedFilter;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if(rdata.m_qos.m_ownership.sendAlways() || rdata.m_qos.m_ownership.hasChanged)
	{
		OwnershipQosPolicy*p = new OwnershipQosPolicy();
		*p = rdata.m_qos.m_ownership;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if(rdata.m_qos.m_destinationOrder.sendAlways() || rdata.m_qos.m_destinationOrder.hasChanged)
	{
		DestinationOrderQosPolicy*p = new DestinationOrderQosPolicy();
		*p = rdata.m_qos.m_destinationOrder;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if(rdata.m_qos.m_presentation.sendAlways() || rdata.m_qos.m_presentation.hasChanged)
	{
		PresentationQosPolicy*p = new PresentationQosPolicy();
		*p = rdata.m_qos.m_presentation;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if(rdata.m_qos.m_partition.sendAlways() || rdata.m_qos.m_partition.hasChanged)
	{
		PartitionQosPolicy*p = new PartitionQosPolicy();
		*p = rdata.m_qos.m_partition;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if(rdata.m_qos.m_topicData.sendAlways() || rdata.m_qos.m_topicData.hasChanged)
	{
		TopicDataQosPolicy*p = new TopicDataQosPolicy();
		*p = rdata.m_qos.m_topicData;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if(rdata.m_qos.m_groupData.sendAlways() || rdata.m_qos.m_groupData.hasChanged)
	{
		GroupDataQosPolicy*p = new GroupDataQosPolicy();
		*p = rdata.m_qos.m_groupData;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if(rdata.m_qos.m_timeBasedFilter.sendAlways() || rdata.m_qos.m_timeBasedFilter.hasChanged)
	{
		TimeBasedFilterQosPolicy*p = new TimeBasedFilterQosPolicy();
		*p = rdata.m_qos.m_timeBasedFilter;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	if(rdata.m_qos.m_durabilityService.sendAlways() || rdata.m_qos.m_durabilityService.hasChanged)
	{
		DurabilityServiceQosPolicy * p = new DurabilityServiceQosPolicy();
		*p = rdata.m_qos.m_durabilityService;
		param->m_parameters.push_back((Parameter_t*)p);
	}
	pDebugInfo(RTPS_CYAN << "DiscoveredReaderData converted to ParameterList with " << param->m_parameters.size()<< " parameters"<<endl);
	return true;
}

//bool DiscoveredData::DiscoveredTopicData2ParameterList(DiscoveredTopicData& tdata, ParameterList_t* param)
//{
//	pError("DiscoveredTopicData2ParameterList NOT YET IMPLEMENTED"<<endl);
//	return true;
//}

}
}


