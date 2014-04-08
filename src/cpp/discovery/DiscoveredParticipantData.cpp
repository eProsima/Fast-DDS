/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file DiscoveredParticipantData.cpp
 *
 *  Created on: Apr 8, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/discovery/DiscoveredParticipantData.h"

namespace eprosima {
namespace rtps {

DiscoveredParticipantData::DiscoveredParticipantData() {
	// TODO Auto-generated constructor stub

}

DiscoveredParticipantData::~DiscoveredParticipantData() {
	// TODO Auto-generated destructor stub
}


bool DiscoveredParticipantData::updateMsg(CacheChange_t* change)
{
	CDRMessage::initCDRMsg(&m_cdrmsg);
	RTPSMessageCreator::addHeader(&m_cdrmsg,m_proxy.m_guidPrefix);
	RTPSMessageCreator::addSubmessageInfoTS_Now(&m_cdrmsg,false);
	RTPSMessageCreator::addSubmessageData(&m_cdrmsg,change,
											WITH_KEY,c_EntityId_Unknown,NULL);
}



} /* namespace rtps */
} /* namespace eprosima */

