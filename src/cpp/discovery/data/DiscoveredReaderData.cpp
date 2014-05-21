/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file DiscoveredReaderData.cpp
 *
 *  Created on: Apr 8, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/discovery/data/DiscoveredReaderData.h"

namespace eprosima {
namespace rtps {

void ReaderQos::setQos(ReaderQos& qos, bool first_time)
{
	if(m_durability.kind != qos.m_durability.kind)
	{
		m_durability.hasChanged = true;
		m_durability.kind = qos.m_durability.kind;
	}
	if(m_deadline.period != qos.m_deadline.period)
	{
		m_deadline.hasChanged = true;
		m_deadline.period = qos.m_deadline.period;
	}
	if(m_latencyBudget.duration)
}

} /* namespace rtps */
} /* namespace eprosima */
