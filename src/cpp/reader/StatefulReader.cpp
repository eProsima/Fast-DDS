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



StatefulReader::StatefulReader(const ReaderParams_t* param,uint32_t payload_size):
		RTPSReader(param->historySize,payload_size)
{
	m_stateType = STATEFUL;
	reliability=param->reliablility;
	//locator lists:
	unicastLocatorList = param->unicastLocatorList;
	multicastLocatorList = param->multicastLocatorList;
	expectsInlineQos = param->expectsInlineQos;
	topicKind = param->topicKind;
	m_topicName = param->topicName;
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

bool StatefulReader::removeMinSeqCacheChange()
{
	SequenceNumber_t seq;
	GUID_t gui;
	m_reader_cache.get_seq_num_min(&seq,&gui);
	WriterProxy* wp;
	if(matched_writer_lookup(gui,&wp))
	{
		SequenceNumber_t seq2;
		wp->available_changes_max(&seq2);
		if(seq.to64long()<=seq2.to64long())
		{
			wp->removeChangeFromWriter(seq);
			m_reader_cache.remove_change(seq,gui);
			return true;
		}
	}
	return false;
}

bool StatefulReader::removeAllCacheChange(int32_t* removed)
{
	int32_t n_count = 0;
	while(this->removeMinSeqCacheChange())
	{
		n_count++;
	}
	*removed = n_count;
	if(this->m_reader_cache.getHistorySize()==0)
		return true;
	else
		return false;
}


} /* namespace rtps */
} /* namespace eprosima */
