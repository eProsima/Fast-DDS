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

StatelessReader::StatelessReader(const ReaderParams_t* param,uint32_t payload_size):
		RTPSReader(param->historySize,payload_size)
{
	//reader_cache.changes.reserve(param.historySize);
	m_stateType = STATELESS;
	//locator lists:
	unicastLocatorList = param->unicastLocatorList;
	multicastLocatorList = param->multicastLocatorList;
	expectsInlineQos = param->expectsInlineQos;
	topicKind = param->topicKind;
	m_topicName = param->topicName;
	this->m_userDefinedId = param->userDefinedId;
}



bool StatelessReader::takeNextCacheChange(void* data)
{
	pDebugInfo("Taking Data from Reader"<<endl);
	SequenceNumber_t seq;
	GUID_t gui;
	if(this->m_reader_cache.get_seq_num_min(&seq,&gui))
	{
		CacheChange_t* change;
		if(this->m_reader_cache.get_change(seq,gui,&change))
		{
			if(change->serializedPayload.data !=NULL)
			{
				if(this->mp_type->deserialize(&change->serializedPayload,data))
				{
					return this->m_reader_cache.remove_change(seq,gui);
				}
			}

		}
	}
	return false;
}


bool StatelessReader::readNextCacheChange(void*data)
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
		if((*it)->serializedPayload.data !=NULL)
		{
			if(this->mp_type->deserialize(&(*it)->serializedPayload,data))
			{
				(*it)->isRead = true;
				return true;
			}
		}
		(*it)->isRead = true;
	}
	return false;
}




} /* namespace rtps */
} /* namespace eprosima */
