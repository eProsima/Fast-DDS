/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * @file StatelessReader.cpp
 *             	
 */

#include "eprosimartps/reader/StatelessReader.h"
#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/dds/SampleInfo.h"
#include "eprosimartps/dds/DDSTopicDataType.h"

using namespace eprosima::dds;

namespace eprosima {
namespace rtps {



StatelessReader::~StatelessReader() {

	pDebugInfo("StatelessReader destructor"<<endl;);
}

StatelessReader::StatelessReader(const SubscriberAttributes& param,
		const GuidPrefix_t&guidP, const EntityId_t& entId,DDSTopicDataType* ptype):
		RTPSReader(guidP,entId,param.topic,ptype,STATELESS,
				param.userDefinedId,param.payloadMaxSize)
{
	//locator lists:
	unicastLocatorList = param.unicastLocatorList;
	multicastLocatorList = param.multicastLocatorList;
	m_expectsInlineQos = param.expectsInlineQos;
}



bool StatelessReader::takeNextCacheChange(void* data,SampleInfo_t* info)
{
	boost::lock_guard<Endpoint> guard(*this);
	pDebugInfo("Taking Data from Reader"<<endl);
	CacheChange_t* change;
	if(this->m_reader_cache.get_min_change(&change))
	{
		if(change->kind == ALIVE)
		{
			this->mp_type->deserialize(&change->serializedPayload,data);
		}
		info->sampleKind = change->kind;
		return this->m_reader_cache.remove_change(change);
	}
	return false;
}


bool StatelessReader::readNextCacheChange(void*data,SampleInfo_t* info)
{
	boost::lock_guard<Endpoint> guard(*this);
	//m_reader_cache.sortCacheChangesBySeqNum();
	bool found = false;
	std::vector<CacheChange_t*>::iterator it;
	for(it = m_reader_cache.changesBegin();
			it!=m_reader_cache.changesEnd();++it)
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
	pInfo("No Unread elements left"<<endl);
	return false;
}

bool StatelessReader::isUnreadCacheChange()
{
	return m_reader_cache.isUnreadCache();
}

bool StatelessReader::matched_writer_add(const GUID_t& guid)
{
	for(std::vector<GUID_t>::iterator it = m_matched_writers.begin();it!=m_matched_writers.end();++it)
	{
		if(*it == guid)
			return false;
	}
	pInfo("Added "<< guid << " to the matched writer list"<<endl);
	m_matched_writers.push_back(guid);
	return true;
}

bool StatelessReader::matched_writer_remove(const GUID_t& guid)
{
	for(std::vector<GUID_t>::iterator it = m_matched_writers.begin();it!=m_matched_writers.end();++it)
	{
		if(*it == guid)
		{
			m_matched_writers.erase(it);
			return true;
		}
	}
	return false;
}


bool StatelessReader::change_removed_by_history(CacheChange_t*ch)
{
	return m_reader_cache.remove_change(ch);
}


} /* namespace rtps */
} /* namespace eprosima */


