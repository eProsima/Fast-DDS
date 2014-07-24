/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ReaderQos.cpp
 *
 */

#include "eprosimartps/qos/ReaderQos.h"
#include "eprosimartps/utils/RTPSLog.h"

namespace eprosima {
namespace dds {


void ReaderQos::setQos( ReaderQos& qos, bool first_time)
{
	if(first_time)
	{
		m_durability = qos.m_durability;
		m_durability.hasChanged = true;
	}
	if(m_deadline.period != qos.m_deadline.period)
	{
		m_deadline = qos.m_deadline;
		m_deadline.hasChanged = true;
	}
	if(m_latencyBudget.duration != qos.m_latencyBudget.duration)
	{
		m_latencyBudget = qos.m_latencyBudget;
		m_latencyBudget.hasChanged = true;
	}
	if(m_liveliness.lease_duration != qos.m_liveliness.lease_duration)
	{
		m_liveliness.lease_duration = qos.m_liveliness.lease_duration;
		m_liveliness.hasChanged = true;
	}
	if(first_time)
	{
		m_liveliness = qos.m_liveliness;
		m_liveliness.hasChanged = true;
	}
	if(first_time)
	{
		m_reliability = qos.m_reliability;
		m_reliability.hasChanged = true;
	}
	if(first_time)
	{
		m_ownership = qos.m_ownership;
		m_ownership.hasChanged = true;
	}
	if(m_destinationOrder.kind != qos.m_destinationOrder.kind)
	{
		m_destinationOrder = qos.m_destinationOrder;
		m_destinationOrder.hasChanged = true;
	}
	if(m_userData.data != qos.m_userData.data )
		{
		m_userData = qos.m_userData;
		m_userData.hasChanged = true;
		}
	if(m_timeBasedFilter.minimum_separation != qos.m_timeBasedFilter.minimum_separation )
	{
		m_timeBasedFilter = qos.m_timeBasedFilter;
		m_timeBasedFilter.hasChanged = true;
	}
	if(m_presentation.access_scope != qos.m_presentation.access_scope ||
			m_presentation.coherent_access != qos.m_presentation.coherent_access ||
			m_presentation.ordered_access != qos.m_presentation.ordered_access)
	{
		m_presentation = qos.m_presentation;
		m_presentation.hasChanged = true;
	}
	if(first_time)
	{
		m_partition = qos.m_partition;
		m_partition.hasChanged = true;
	}
	if(m_topicData.value != qos.m_topicData.value )
	{
		m_topicData = qos.m_topicData;
		m_topicData.hasChanged = true;
	}
	if(m_groupData.value != qos.m_groupData.value )
	{
		m_groupData = qos.m_groupData;
		m_groupData.hasChanged = true;
	}
	if(m_durabilityService.history_kind != qos.m_durabilityService.history_kind ||
			m_durabilityService.history_depth != qos.m_durabilityService.history_depth ||
			m_durabilityService.max_instances != qos.m_durabilityService.max_instances ||
			m_durabilityService.max_samples != qos.m_durabilityService.max_samples||
			m_durabilityService.max_samples_per_instance != qos.m_durabilityService.max_samples_per_instance ||
			m_durabilityService.service_cleanup_delay != qos.m_durabilityService.service_cleanup_delay
	)
	{
		m_durabilityService = qos.m_durabilityService;
		m_durabilityService.hasChanged = true;
	}
	if(m_lifespan.duration != qos.m_lifespan.duration )
	{
		m_lifespan = qos.m_lifespan;
		m_lifespan.hasChanged = true;
	}
}


bool ReaderQos::checkQos()
{
	if(m_durability.kind == TRANSIENT_DURABILITY_QOS)
	{
		pError("TRANSIENT Durability not supported"<<endl);
		return false;
	}
	if(m_durability.kind == PERSISTENT_DURABILITY_QOS)
	{
		pError("PERSISTENT Durability not supported"<<endl);
		return false;
	}
	if(m_destinationOrder.kind == BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS)
	{
		pError("BY SOURCE TIMESTAMP DestinationOrder not supported"<<endl);
		return false;
	}
	if(m_reliability.kind == BEST_EFFORT_RELIABILITY_QOS && m_ownership.kind == EXCLUSIVE_OWNERSHIP_QOS)
	{
		pError("BEST_EFFORT incompatible with EXCLUSIVE ownership"<<endl);
		return false;
	}
	return true;
}

} /* namespace dds */
} /* namespace eprosima */
