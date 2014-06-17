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
						param.userDefinedId,param.historyMaxSize,param.payloadMaxSize),
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
			delete(*it);
			matched_writers.erase(it);
			pDebugInfo("Writer Proxy removed" << endl);
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
	std::vector<SequenceNumber_t> seq_vec;
	SequenceNumber_t seq, seqmin;
	WriterProxy* wpmin = NULL;
	for(std::vector<WriterProxy*>::iterator it = this->matched_writers.begin();
			it!=this->matched_writers.end();++it)
	{
		if((*it)->available_changes_min(&seq))
		{
			if(seqmin.to64long() == 0 || seqmin > seq)
			{
				wpmin = *it;
				seqmin = seq;
			}
		}
	}
	if(seqmin.to64long() == 0)
	{
		pDebugInfo("StatefulReader: takeNextCacheChange: seqMin = 0"<<endl);
		return false;
	}
	CacheChange_t* change;
	pDebugInfo("StatefulReader: trying takeNextCacheChange: "<< seqmin.to64long()<<endl);
	if(this->m_reader_cache.get_change(seqmin,wpmin->param.remoteWriterGuid,&change))
	{
		if(change->kind == ALIVE)
			this->mp_type->deserialize(&change->serializedPayload,data);
		if(wpmin->removeChangeFromWriter(seqmin))
		{

			info->sampleKind = change->kind;
			return m_reader_cache.remove_change(seq,wpmin->param.remoteWriterGuid);
		}
	}
	pDebugInfo("StatefulReader: takeNextCacheChange: FALSE"<<endl);
	return false;
}

bool StatefulReader::readNextCacheChange(void*data,SampleInfo_t* info)
{
	boost::lock_guard<Endpoint> guard(*this);
	m_reader_cache.sortCacheChangesBySeqNum();
	for(std::vector<CacheChange_t*>::iterator it = m_reader_cache.m_changes.begin();
			it!=m_reader_cache.m_changes.end();++it)
	{
		if((*it)->isRead)
			continue;
		WriterProxy* wp;
		if(this->matched_writer_lookup((*it)->writerGUID,&wp))
		{
			SequenceNumber_t seq;
			wp->available_changes_max(&seq);
			if(seq.to64long()>=(*it)->sequenceNumber.to64long())
			{
				if((*it)->kind == ALIVE)
				{
					this->mp_type->deserialize(&(*it)->serializedPayload,data);
				}
				(*it)->isRead = true;
				info->sampleKind = (*it)->kind;
				return true;
			}
		}
	}
	return false;
}


bool StatefulReader::isUnreadCacheChange()
{
	m_reader_cache.sortCacheChangesBySeqNum();
	for(std::vector<CacheChange_t*>::iterator it = m_reader_cache.m_changes.begin();
			it!=m_reader_cache.m_changes.end();++it)
	{
		if((*it)->isRead)
			continue;
		WriterProxy* wp;
		if(this->matched_writer_lookup((*it)->writerGUID,&wp))
		{
			SequenceNumber_t seq;
			wp->available_changes_max(&seq);
			if(seq.to64long()>=(*it)->sequenceNumber.to64long())
			{
				pDebugInfo("StatefulReader, isUnreadCacheChange: TRUE : "<< (*it)->sequenceNumber.to64long()<<endl);
				return true;
			}
		}
	}
	pDebugInfo("StatefulReader, isUnreadCacheChange: FALSE"<<endl);
	return false;
}


} /* namespace rtps */
} /* namespace eprosima */
