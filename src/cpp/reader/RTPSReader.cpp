/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * RTPSReader.cpp
 *
*/

#include "eprosimartps/reader/RTPSReader.h"

#include "eprosimartps/utils/RTPSLog.h"

namespace eprosima {
namespace rtps {

RTPSReader::RTPSReader(GuidPrefix_t guidP,EntityId_t entId,TopicAttributes topic,TopicDataType* ptype,
		StateKind_t state,
		int16_t userDefinedId,uint32_t payload_size):
		Endpoint(guidP,entId,topic,ptype,state,READER,userDefinedId),
		//mp_Sub(NULL),
		mp_listener(NULL),
		#pragma warning(disable: 4355)
		m_reader_cache((Endpoint*)this,payload_size),
		m_expectsInlineQos(true),
		m_acceptMessagesToUnknownReaders(true),
		m_acceptMessagesFromUnkownWriters(true)

{
	pDebugInfo("RTPSReader created correctly"<<endl);
}

RTPSReader::~RTPSReader() {

	pDebugInfo("RTPSReader destructor"<<endl;);
}

bool RTPSReader::acceptMsgDirectedTo(EntityId_t& entityId)
{
	if(entityId == m_guid.entityId)
		return true;
	if(m_acceptMessagesToUnknownReaders && entityId == c_EntityId_Unknown)
		return true;
	else
		return false;
}

bool RTPSReader::removeCacheChangesByKey(InstanceHandle_t& key)
{
	return m_reader_cache.removeCacheChangesByKey(key);
}




} /* namespace rtps */
} /* namespace eprosima */


