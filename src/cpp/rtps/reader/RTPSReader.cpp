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
static const char* const CLASS_NAME = "RTPSReader";

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
	const char* const METHOD_NAME = "RTPSReader";
	logInfo(RTPS_READER,"RTPSReader created correctly in topic: "<<this->getTopic().topicName;);
}

RTPSReader::~RTPSReader()
{
	const char* const METHOD_NAME = "~RTPSReader";
	logInfo(RTPS_READER,"Removing reader "<<this->getGuid().entityId;);
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


