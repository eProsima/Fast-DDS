/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SEDPListeners.cpp
 *
 *  Created on: May 19, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/discovery/SEDPListeners.h"
#include "eprosimartps/discovery/data/DiscoveredWriterData.h"
#include "eprosimartps/discovery/data/DiscoveredReaderData.h"
#include "eprosimartps/discovery/data/DiscoveredTopicData.h"
#include "eprosimartps/reader/StatefulReader.h"
#include "eprosimartps/discovery/ParticipantDiscoveryProtocol.h"

namespace eprosima {
namespace rtps {

void SEDPPubListener::onNewDataMessage()
{
	CacheChange_t* change;
	if(this->mp_SEDP->mp_PubReader->m_reader_cache.get_last_added_cache(&change))
	{
		bool from_myself = true;
		for(uint8_t i =0;i<12;++i)
		{
			if(change->instanceHandle.value[i] != this->mp_SEDP->mp_DPDP->mp_localPDP->m_guidPrefix.value[i])
			{
				from_myself = false;
				break;
			}
		}
		if(from_myself)
		{
			pInfo("Message from own participant, removing"<<endl)
				this->mp_SEDP->mp_PubReader->m_reader_cache.remove_change(change->sequenceNumber,change->writerGUID);
			return;
		}
		else
		{
			DiscoveredParticipantData* pdata;
		}
	}
}

void SEDPSubListener::onNewDataMessage()
{

}

void SEDPTopListener::onNewDataMessage()
{

}


bool SEDPListeners::findParticipant(GuidPrefix_t& guidP,DiscoveredParticipantData** pdata)
{
return true;
}

bool SEDPListeners::processParameterList(CacheChange_t* change,DiscoveredData* ddata)
{
	CDRMessage_t msg(change->serializedPayload.length);
	msg.msg_endian = change->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND:LITTLEEND;
	msg.length = change->serializedPayload.length;
	memcpy(msg.buffer,change->serializedPayload.data,msg.length);
	bool is_sentinel = false;
	bool valid = true;
	ParameterId_t pid;
	uint16_t plength;
	while(!is_sentinel)
	{
		valid = true;
		valid&=CDRMessage::readUInt16(&msg,(uint16_t*)&pid);
		valid&=CDRMessage::readUInt16(&msg,&plength);
		if(valid)
		{
			switch(pid)
			{
			case PID_UNICAST_LOCATOR:
			{
				Locator_t loc;
				valid &= CDRMessage::readLocator(&msg,&loc);
				ddata->unicastLocatorList.push_back(loc);
				break;
			}
			case PID_MULTICAST_LOCATOR:
			{
				Locator_t loc;
				valid &= CDRMessage::readLocator(&msg,&loc);
				ddata->unicastLocatorList.push_back(loc);
				break;
			}
			case PID_EXPECTS_INLINE_QOS:
			{
				octet expects;
				valid &= CDRMessage::readOctet(&msg,&expects);
				ddata->expectsInlineQos = (bool)expects;
				msg.pos+=3;
			}
			case PID_
			}
		}
	}
}

} /* namespace rtps */
} /* namespace eprosima */
