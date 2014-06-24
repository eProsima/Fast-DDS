/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file StatefulReader.cpp
 *
 */

#include "eprosimartps/reader/StatefulReader.h"
#include "eprosimartps/utils/RTPSLog.h"

#include "eprosimartps/dds/SampleInfo.h"
#include "eprosimartps/dds/DDSTopicDataType.h"

using namespace eprosima::dds;

namespace eprosima {
namespace rtps {


StatefulReader::~StatefulReader()
{
	pDebugInfo("StatefulReader destructor"<<endl;);
	for(std::vector<WriterProxy*>::iterator it = matched_writers.begin();
			it!=matched_writers.end();++it)
	{
		delete(*it);
	}
}



StatefulReader::StatefulReader(const SubscriberAttributes& param,
		const GuidPrefix_t&guidP, const EntityId_t& entId,DDSTopicDataType* ptype):
		RTPSReader(guidP,entId,param.topic,ptype,STATEFUL,
						param.userDefinedId,param.payloadMaxSize),
						m_SubTimes(param.times)
{
	//locator lists:
	unicastLocatorList = param.unicastLocatorList;
	multicastLocatorList = param.multicastLocatorList;
	m_expectsInlineQos = param.expectsInlineQos;
}

bool StatefulReader::matched_writer_add(WriterProxy_t& WPparam)
{
	boost::lock_guard<Endpoint> guard(*this);
	for(std::vector<WriterProxy*>::iterator it=matched_writers.begin();
			it!=matched_writers.end();++it)
	{
		if((*it)->param.remoteWriterGuid == WPparam.remoteWriterGuid)
		{
			pWarning("Attempting to add existing writer" << endl);
			return false;
		}
	}
	WriterProxy* wp = new WriterProxy(WPparam,m_SubTimes.heartbeatResponseDelay,this);
	matched_writers.push_back(wp);
	pDebugInfo("new Writer Proxy added to StatefulReader" << endl);
	return true;
}

bool StatefulReader::matched_writer_remove(GUID_t& writerGuid)
{
	boost::lock_guard<Endpoint> guard(*this);
	for(std::vector<WriterProxy*>::iterator it=matched_writers.begin();it!=matched_writers.end();++it)
	{
		if((*it)->param.remoteWriterGuid == writerGuid)
		{
			pWarning("Writer Proxy " << (*it)->param.remoteWriterGuid << " removed" << endl);
			delete(*it);
			matched_writers.erase(it);
			
			return true;
		}
	}
	pInfo("Writer Proxy doesn't exist in this reader" << endl)
	return false;
}


bool StatefulReader::matched_writer_remove(WriterProxy_t& Wp)
{
	return matched_writer_remove(Wp.remoteWriterGuid);
}

bool StatefulReader::matched_writer_lookup(GUID_t& writerGUID,WriterProxy** WP)
{
	boost::lock_guard<Endpoint> guard(*this);
	for(std::vector<WriterProxy*>::iterator it=matched_writers.begin();it!=matched_writers.end();++it)
	{
		if((*it)->param.remoteWriterGuid == writerGUID)
		{
			*WP = *it;
			pDebugInfo("StatefulReader looking for matched writerProxy, FOUND"<<endl);
			return true;
		}
	}
	pDebugInfo("StatefulReader looking for matched writerProxy, NOT FOUND"<<endl);
	return false;
}

bool StatefulReader::takeNextCacheChange(void* data,SampleInfo_t* info)
{
	boost::lock_guard<Endpoint> guard(*this);
	CacheChange_t* min_change;
	if(m_reader_cache.get_min_change(&min_change))
	{
		pDebugInfo("StatefulReader: trying takeNextCacheChange: "<< min_change->sequenceNumber.to64long()<<endl);
		WriterProxy* wp;
		if(matched_writer_lookup(min_change->writerGUID,&wp))
		{
			if(min_change->kind == ALIVE)
				this->mp_type->deserialize(&min_change->serializedPayload,data);
			if(wp->removeChangeFromWriter(min_change->sequenceNumber))
			{
				info->sampleKind = min_change->kind;
				return m_reader_cache.remove_change(min_change);
			}
		}
	}
	return false;
}

bool StatefulReader::readNextCacheChange(void*data,SampleInfo_t* info)
{
	boost::lock_guard<Endpoint> guard(*this);
	for(std::vector<CacheChange_t*>::iterator it = m_reader_cache.changesBegin();
			it!=m_reader_cache.changesEnd();++it)
	{
		if((*it)->isRead)
			continue;
		WriterProxy* wp;
		if(this->matched_writer_lookup((*it)->writerGUID,&wp))
		{
			SequenceNumber_t seq;
			wp->available_changes_max(&seq);
			if(seq >= (*it)->sequenceNumber)
			{
				if((*it)->kind == ALIVE)
				{
					this->mp_type->deserialize(&(*it)->serializedPayload,data);
				}
				(*it)->isRead = true;
				info->sampleKind = (*it)->kind;
				m_reader_cache.decreaseUnreadCount();
				return true;
			}
		}
	}
	return false;
}


bool StatefulReader::isUnreadCacheChange()
{
	return m_reader_cache.isUnreadCache();
}

bool StatefulReader::change_removed_by_history(CacheChange_t* a_change)
{
	boost::lock_guard<Endpoint> guard(*this);
	WriterProxy* wp;
	if(matched_writer_lookup(a_change->writerGUID,&wp))
	{
		std::vector<ChangeFromWriter_t>::iterator chit;
		for(chit = wp->m_changesFromW.begin();
				chit!=wp->m_changesFromW.end();++chit)
		{
			if(a_change->sequenceNumber == chit->seqNum)
			{
				break;
			}
		}
		chit->notValid();
		return true;
	}
	return false;
}



} /* namespace rtps */
} /* namespace eprosima */
