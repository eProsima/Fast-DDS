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
 *  Created on: Mar 24, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/reader/StatefulReader.h"

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



StatefulReader::StatefulReader(const SubscriberAttributes* param,uint32_t payload_size):
				RTPSReader(param->historyMaxSize,payload_size)
{
	m_stateType = STATEFUL;
	m_reliability=param->reliability;
	//locator lists:
	unicastLocatorList = param->unicastLocatorList;
	multicastLocatorList = param->multicastLocatorList;
	expectsInlineQos = param->expectsInlineQos;
	topicKind = param->topic.topicKind;
	m_topicName = param->topic.topicName;
	this->m_userDefinedId = param->userDefinedId;
}

bool StatefulReader::matched_writer_add(WriterProxy_t* WPparam)
{
	for(std::vector<WriterProxy*>::iterator it=matched_writers.begin();
			it!=matched_writers.end();++it)
	{
		if((*it)->param.remoteWriterGuid == WPparam->remoteWriterGuid)
		{
			pWarning("Attempting to add existing writer" << endl);
			return false;
		}
	}
	WriterProxy* wp = new WriterProxy(WPparam,this);
	matched_writers.push_back(wp);
	pDebugInfo("new Writer Proxy added to StatefulReader" << endl);
	return true;
}

bool StatefulReader::matched_writer_remove(GUID_t& writerGuid)
{
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
	pDebugInfo("StatefulReader looking for matched writerProxy"<<endl);
	for(std::vector<WriterProxy*>::iterator it=matched_writers.begin();it!=matched_writers.end();++it)
	{
		if((*it)->param.remoteWriterGuid == writerGUID)
		{
			*WP = *it;
			return true;
		}
	}
	return false;
}

bool StatefulReader::takeNextCacheChange(void* data,SampleInfo_t* info)
{
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
		return false;
	CacheChange_t* change;
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
	return false;
}

bool StatefulReader::readNextCacheChange(void*data,SampleInfo_t* info)
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
				return true;
		}
	}
	return false;
}


} /* namespace rtps */
} /* namespace eprosima */
