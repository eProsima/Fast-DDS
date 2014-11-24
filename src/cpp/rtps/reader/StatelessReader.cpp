/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * @file StatelessReader.cpp
 *             	
 */

#include "fastrtps/reader/StatelessReader.h"
#include "fastrtps/reader/WriterProxyData.h"
#include "fastrtps/utils/RTPSLog.h"
#include "fastrtps/pubsub/SampleInfo.h"
#include "fastrtps/pubsub/TopicDataType.h"

#include <boost/thread/recursive_mutex.hpp>

using namespace eprosima::pubsub;

namespace eprosima {
namespace rtps {

static const char* const CLASS_NAME = "StatelessReader";

StatelessReader::~StatelessReader()
{
	const char* const METHOD_NAME = "~StatelessReader";
	logInfo(RTPS_READER,"Removing reader "<<this->getGuid());
}

StatelessReader::StatelessReader(const SubscriberAttributes& param,
		const GuidPrefix_t&guidP, const EntityId_t& entId,TopicDataType* ptype):
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
	const char* const METHOD_NAME = "takeNextCacheChange";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	CacheChange_t* change;
	if(this->m_reader_cache.get_min_change(&change))
	{
		logInfo(RTPS_READER,this->getGuid().entityId<<" taking next data.");
		if(change->kind == ALIVE)
		{
			this->mp_type->deserialize(&change->serializedPayload,data);
		}
		if(info!=NULL)
		{
			info->sampleKind = change->kind;
			info->writerGUID = change->writerGUID;
			info->sourceTimestamp = change->sourceTimestamp;
			info->iHandle = change->instanceHandle;
		}
		if(!change->isRead)
			m_reader_cache.decreaseUnreadCount();
		if(!m_reader_cache.remove_change(change))
			logWarning(RTPS_READER,"Problem removing change from ReaderHistory");
		return true;
	}
	return false;
}


bool StatelessReader::readNextCacheChange(void*data,SampleInfo_t* info)
{
	const char* const METHOD_NAME = "readNextCacheChange";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
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
		if(info!=NULL)
		{
			info->sampleKind = (*it)->kind;
			info->writerGUID = (*it)->writerGUID;
			info->sourceTimestamp = (*it)->sourceTimestamp;
			info->iHandle = (*it)->instanceHandle;
		}
		(*it)->isRead = true;
		m_reader_cache.decreaseUnreadCount();
		logInfo(RTPS_READER,this->getGuid().entityId<<" reads "<<(*it)->sequenceNumber);
		return true;
	}
	logInfo(RTPS_READER,"No Unread elements left");
	return false;
}

bool StatelessReader::isUnreadCacheChange()
{
	return m_reader_cache.isUnreadCache();
}

bool StatelessReader::matched_writer_add(WriterProxyData* wdata)
{
	const char* const METHOD_NAME = "matched_writer_add";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	for(std::vector<WriterProxyData*>::iterator it = m_matched_writers.begin();it!=m_matched_writers.end();++it)
	{
		if((*it)->m_guid == wdata->m_guid)
			return false;
	}
	logInfo(RTPS_READER,wdata->m_guid << " added to the matched writer list");
	m_matched_writers.push_back(wdata);
	return true;
}
bool StatelessReader::matched_writer_remove(WriterProxyData* wdata)
{
	const char* const METHOD_NAME = "matched_writer_remove";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	for(std::vector<WriterProxyData*>::iterator it = m_matched_writers.begin();it!=m_matched_writers.end();++it)
	{
		if((*it)->m_guid == wdata->m_guid)
		{
			logInfo(RTPS_READER,"Writer Proxy removed: " <<wdata->m_guid);
			m_matched_writers.erase(it);
			return true;
		}
	}
	return false;
}

bool StatelessReader::matched_writer_is_matched(WriterProxyData* wdata)
{
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	for(std::vector<WriterProxyData*>::iterator it = m_matched_writers.begin();it!=m_matched_writers.end();++it)
	{
		if((*it)->m_guid == wdata->m_guid)
		{
			return true;
		}
	}
	return false;
}




bool StatelessReader::change_removed_by_history(CacheChange_t*ch,WriterProxy*prox)
{
	return m_reader_cache.remove_change(ch);
}

bool StatelessReader::acceptMsgFrom(GUID_t& writerId,WriterProxy** wp)
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


