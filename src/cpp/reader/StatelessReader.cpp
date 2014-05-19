/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * StatelessReader.cpp
 *
 *  Created on: Feb 27, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/reader/StatelessReader.h"

namespace eprosima {
namespace rtps {



StatelessReader::~StatelessReader() {

	pDebugInfo("StatelessReader destructor"<<endl;);
}

StatelessReader::StatelessReader(const SubscriberAttributes* param,uint32_t payload_size):
		RTPSReader(param->historyMaxSize,payload_size)
{
	//reader_cache.changes.reserve(param.historySize);
	m_stateType = STATELESS;
	//locator lists:
	unicastLocatorList = param->unicastLocatorList;
	multicastLocatorList = param->multicastLocatorList;
	expectsInlineQos = param->expectsInlineQos;
	m_topic = param->topic;

	this->m_userDefinedId = param->userDefinedId;
}



bool StatelessReader::takeNextCacheChange(void* data,SampleInfo_t* info)
{
	pDebugInfo("Taking Data from Reader"<<endl);
	SequenceNumber_t seq;
	GUID_t gui;
	if(this->m_reader_cache.get_seq_num_min(&seq,&gui))
	{
		CacheChange_t* change;
		if(this->m_reader_cache.get_change(seq,gui,&change))
		{
			if(change->kind == ALIVE)
			{
				this->mp_type->deserialize(&change->serializedPayload,data);
			}
			info->sampleKind = change->kind;
			return this->m_reader_cache.remove_change(seq,gui);
		}

	}
	return false;
}


bool StatelessReader::readNextCacheChange(void*data,SampleInfo_t* info)
{
	m_reader_cache.sortCacheChangesBySeqNum();
	bool found = false;
	std::vector<CacheChange_t*>::iterator it;
	for(it = m_reader_cache.m_changes.begin();
			it!=m_reader_cache.m_changes.end();++it)
	{
		if(!(*it)->isRead)
		{
			found = true;
			break;
		}
	}
	if(found)
	{
		if((*it)->kind == ALIVE)
		{
			this->mp_type->deserialize(&(*it)->serializedPayload,data);
			info->sampleKind = ALIVE;
		}
		info->sampleKind = (*it)->kind;
		(*it)->isRead = true;
		return true;
	}
	return false;
}

bool StatelessReader::isUnreadCacheChange()
{
	m_reader_cache.sortCacheChangesBySeqNum();
	std::vector<CacheChange_t*>::iterator it;
	for(it = m_reader_cache.m_changes.begin();
			it!=m_reader_cache.m_changes.end();++it)
	{
		if(!(*it)->isRead)
		{
			return true;
		}
	}
	return false;
}




} /* namespace rtps */
} /* namespace eprosima */
