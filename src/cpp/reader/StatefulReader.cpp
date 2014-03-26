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

}



StatefulReader::StatefulReader(const ReaderParams_t* param,uint32_t payload_size):
		RTPSReader(param->historySize,payload_size)
{
	//reader_cache.changes.reserve(param.historySize);
	stateType = STATEFUL;
	reader_cache.rtpsreader = (RTPSReader*)this;
	reader_cache.historyKind = READER;

	reliability=param->reliablility;
	//locator lists:
	unicastLocatorList = param->unicastLocatorList;
	multicastLocatorList = param->multicastLocatorList;

	expectsInlineQos = param->expectsInlineQos;
	topicKind = param->topicKind;
	reader_cache.historySize = param->historySize;
}

bool StatefulReader::matched_writer_add(WriterProxy_t* WPparam)
{
	std::vector<WriterProxy*>::iterator it;
	for(it=matched_writers.begin();it!=matched_writers.end();++it)
	{
		if((*it)->param.remoteWriterGuid == WPparam->remoteWriterGuid)
		{
			pWarning("Attempting to add existing writer" << endl);
			return false;
		}
	}
	WriterProxy* wp = new WriterProxy(WPparam,this);
	matched_writers.push_back(wp);
	pDebugInfo("Writer Proxy added" << endl);
	return true;
}

bool StatefulReader::matched_writer_remove(GUID_t& writerGuid)
{
	std::vector<WriterProxy*>::iterator it;
	for(it=matched_writers.begin();it!=matched_writers.end();++it)
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
	std::vector<WriterProxy*>::iterator it;
	for(it=matched_writers.begin();it!=matched_writers.end();++it)
	{
		if((*it)->param.remoteWriterGuid == writerGUID)
		{
			*WP = *it;
			return true;
		}
	}
	return false;
}




} /* namespace rtps */
} /* namespace eprosima */
