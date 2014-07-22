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
#include "eprosimartps/reader/WriterProxyData.h"
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
		info->writerGUID = change->writerGUID;
		info->sourceTimestamp = change->sourceTimestamp;
		if(!change->isRead)
			m_reader_cache.decreaseUnreadCount();
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
		info->writerGUID = (*it)->writerGUID;
		info->sourceTimestamp = (*it)->sourceTimestamp;
		(*it)->isRead = true;
		m_reader_cache.decreaseUnreadCount();
		return true;
	}
	pInfo("No Unread elements left"<<endl);
	return false;
}

bool StatelessReader::isUnreadCacheChange()
{
	return m_reader_cache.isUnreadCache();
}

bool StatelessReader::matched_writer_add(WriterProxyData* wdata)
{
	boost::lock_guard<Endpoint> guard(*this);
	for(std::vector<WriterProxyData*>::iterator it = m_matched_writers.begin();it!=m_matched_writers.end();++it)
		{
			if((*it)->m_guid == wdata->m_guid)
				return false;
		}
		pInfo("Added "<< wdata->m_guid << " to the matched writer list"<<endl);
		m_matched_writers.push_back(wdata);
		return true;
}
bool StatelessReader::matched_writer_remove(WriterProxyData* wdata)
{
	boost::lock_guard<Endpoint> guard(*this);
	for(std::vector<WriterProxyData*>::iterator it = m_matched_writers.begin();it!=m_matched_writers.end();++it)
		{
			if((*it)->m_guid == wdata->m_guid)
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

bool StatelessReader::acceptMsgFrom(GUID_t& writerId)
{
	if(this->m_acceptMessagesFromUnkownWriters)
	{
		for(std::vector<WriterProxyData*>::iterator it = this->m_matched_writers.begin();
				it!=m_matched_writers.end();++it)
		{
			if((*it)->m_guid == writerId)
				return true;
		}
	}
	else
	{
		if(writerId.entityId == this->m_trustedWriterEntityId)
			return true;
	}
	return false;
}


} /* namespace rtps */
} /* namespace eprosima */


