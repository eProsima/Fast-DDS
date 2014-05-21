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
#include "eprosimartps/discovery/SimpleEDP.h"
#include "eprosimartps/discovery/data/DiscoveredData.h"

namespace eprosima {
namespace rtps {

void SEDPPubListener::onNewDataMessage()
{
	CacheChange_t* change;
	if(this->mp_SEDP->mp_PubReader->m_reader_cache.get_last_added_cache(&change))
	{
		bool from_myself = true;
		GuidPrefix_t guidPrefix;
		for(uint8_t i =0;i<12;++i)
		{
			guidPrefix.value[i] = change->instanceHandle.value[i];
			if(change->instanceHandle.value[i] != this->mp_SEDP->mp_PDP->mp_localDPData->m_guidPrefix.value[i])
			{
				from_myself = false;
			}
		}
		if(from_myself)
		{
			pInfo("Message from own participant, removing"<<endl);
			this->mp_SEDP->mp_PubReader->m_reader_cache.remove_change(change->sequenceNumber,change->writerGUID);
			return;
		}
		DiscoveredParticipantData* pdata = NULL;
		for(std::vector<DiscoveredParticipantData>::iterator pit = this->mp_SEDP->mp_PDP->m_discoveredParticipants.begin();
				pit!=this->mp_SEDP->mp_PDP->m_discoveredParticipants.begin();++pit)
		{
			if(pit->m_guidPrefix == guidPrefix)
			{
				pdata = &(*pit);
				break;
			}
		}
		if(pdata == NULL)
		{
			pWarning("PubReader received message from unkown participant, ignoring"<<endl);
			return;
		}
		bool already_in_history = false;
		//Check if CacheChange_t with same Key is already in History:
		for(std::vector<CacheChange_t*>::iterator it = this->mp_SEDP->mp_PubReader->m_reader_cache.m_changes.begin();
				it!=this->mp_SEDP->mp_PubReader->m_reader_cache.m_changes.begin();++it)
		{
			if((*it)->instanceHandle == change->instanceHandle)
			{
				this->mp_SEDP->mp_PubReader->m_reader_cache.remove_change((*it)->sequenceNumber,(*it)->writerGUID);
				already_in_history = true;
				break;
			}
		}
		DiscoveredWriterData* wdata;
		if(already_in_history)
		{
			for(std::vector<DiscoveredWriterData>::iterator it = pdata->m_writers.begin();
					it!=pdata->m_writers.end();++it)
			{
				if(it->m_key == change->instanceHandle)
				{
					wdata = &(*it);
					break;
				}
			}
		}
		else
		{
			wdata = new DiscoveredWriterData();
		}
		ParameterList_t param;
		CDRMessage_t msg;
		msg.msg_endian = change->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND:LITTLEEND;
		msg.length = change->serializedPayload.length;
		memcpy(msg.buffer,change->serializedPayload.data,msg.length);
		ParameterList::readParameterListfromCDRMsg(&msg,&param,NULL,NULL);
		DiscoveredData::ParameterList2DiscoveredWriterData(param,wdata);
		if(!already_in_history)
		{
			pdata->m_writers.push_back(*wdata);
		}
		for(std::vector<RTPSReader*>::iterator rit = this->mp_SEDP->mp_PDP->mp_participant->m_readerList.begin();
				rit!=this->mp_SEDP->mp_PDP->mp_participant->m_readerList.end();++rit)
		{
			if(already_in_history)
			{
				this->mp_SEDP->updateReaderMatching(*rit,wdata);
			}
			else
			{
				this->mp_SEDP->localReaderMatching(*rit,wdata);
			}
		}
		if(!already_in_history)
		{
			delete(wdata);
		}
		param.deleteParams();
	}
}

void SEDPSubListener::onNewDataMessage()
{
	CacheChange_t* change;
	if(this->mp_SEDP->mp_SubReader->m_reader_cache.get_last_added_cache(&change))
	{
		bool from_myself = true;
		GuidPrefix_t guidPrefix;
		for(uint8_t i =0;i<12;++i)
		{
			guidPrefix.value[i] = change->instanceHandle.value[i];
			if(change->instanceHandle.value[i] != this->mp_SEDP->mp_PDP->mp_localDPData->m_guidPrefix.value[i])
			{
				from_myself = false;
			}
		}
		if(from_myself)
		{
			pInfo("Message from own participant, removing"<<endl);
			this->mp_SEDP->mp_SubReader->m_reader_cache.remove_change(change->sequenceNumber,change->writerGUID);
			return;
		}
		DiscoveredParticipantData* pdata = NULL;
		for(std::vector<DiscoveredParticipantData>::iterator pit = this->mp_SEDP->mp_PDP->m_discoveredParticipants.begin();
				pit!=this->mp_SEDP->mp_PDP->m_discoveredParticipants.begin();++pit)
		{
			if(pit->m_guidPrefix == guidPrefix)
			{
				pdata = &(*pit);
				break;
			}
		}
		if(pdata == NULL)
		{
			pWarning("SubReader received message from unkown participant, ignoring"<<endl);
			return;
		}
		bool already_in_history = false;
		//Check if CacheChange_t with same Key is already in History:
		for(std::vector<CacheChange_t*>::iterator it = this->mp_SEDP->mp_SubReader->m_reader_cache.m_changes.begin();
				it!=this->mp_SEDP->mp_SubReader->m_reader_cache.m_changes.begin();++it)
		{
			if((*it)->instanceHandle == change->instanceHandle)
			{
				this->mp_SEDP->mp_SubReader->m_reader_cache.remove_change((*it)->sequenceNumber,(*it)->writerGUID);
				already_in_history = true;
				break;
			}
		}
		DiscoveredReaderData* rdata;
		if(already_in_history)
		{
			for(std::vector<DiscoveredReaderData>::iterator it = pdata->m_readers.begin();
					it!=pdata->m_readers.end();++it)
			{
				if(it->m_key == change->instanceHandle)
				{
					rdata = &(*it);
					break;
				}
			}
		}
		else
		{
			rdata = new DiscoveredReaderData();
		}
		ParameterList_t param;
		CDRMessage_t msg;
		msg.msg_endian = change->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND:LITTLEEND;
		msg.length = change->serializedPayload.length;
		memcpy(msg.buffer,change->serializedPayload.data,msg.length);
		ParameterList::readParameterListfromCDRMsg(&msg,&param,NULL,NULL);
		DiscoveredData::ParameterList2DiscoveredReaderData(param,rdata);
		if(!already_in_history)
		{
			pdata->m_readers.push_back(*rdata);
		}
		for(std::vector<RTPSWriter*>::iterator wit = this->mp_SEDP->mp_PDP->mp_participant->m_writerList.begin();
				wit!=this->mp_SEDP->mp_PDP->mp_participant->m_writerList.end();++wit)
		{
			if(already_in_history)
			{
				this->mp_SEDP->updateWriterMatching(*wit,rdata);
			}
			else
			{
				this->mp_SEDP->localWriterMatching(*wit,rdata);
			}
		}
		if(!already_in_history)
		{
			delete(rdata);
		}
		param.deleteParams();
	}
}




} /* namespace rtps */
} /* namespace eprosima */
