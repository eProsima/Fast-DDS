/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * @file RTPSWriter.cpp
 *
 */

#include "eprosimartps/writer/RTPSWriter.h"

#include "eprosimartps/CDRMessage.h"
#include "eprosimartps/dds/Publisher.h"
#include "eprosimartps/dds/PublisherListener.h"
#include "eprosimartps/dds/DDSTopicDataType.h"
#include "eprosimartps/qos/ParameterList.h"

#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/RTPSMessageCreator.h"

namespace eprosima {
namespace rtps {

static const char* const CLASS_NAME = "RTPSWriter";

RTPSWriter::RTPSWriter(GuidPrefix_t guidP,EntityId_t entId,const PublisherAttributes& param,DDSTopicDataType* ptype,
		StateKind_t state,
		int16_t userDefinedId, uint32_t payload_size):
									Endpoint(guidP,entId,param.topic,ptype,state,WRITER,userDefinedId),
									#pragma warning(disable: 4355)
									m_writer_cache((Endpoint*)this,payload_size),
									m_pushMode(true),
									//FIXME: Select a better size, not the payload but maybe more?
									m_cdrmessages(payload_size),
									mp_listener(NULL),
									m_livelinessAsserted(false),
									mp_unsetChangesNotEmpty(NULL)
{
	const char* const METHOD_NAME = "RTPSWriter";
	init_header();
	logInfo(LOG_CATEGORY::RTPS_WRITER,"RTPSWriter created";);

}

void RTPSWriter::init_header()
{
	CDRMessage::initCDRMsg(&m_cdrmessages.m_rtpsmsg_header);
	RTPSMessageCreator::addHeader(&m_cdrmessages.m_rtpsmsg_header,m_guid.guidPrefix);
}



RTPSWriter::~RTPSWriter()
{
	const char* const METHOD_NAME = "~RTPSWriter";
	logInfo(LOG_CATEGORY::RTPS_WRITER,"RTPSWriter destructor";;);
}

bool RTPSWriter::new_change(ChangeKind_t changeKind,void* data,CacheChange_t** change_out)
{
	const char* const METHOD_NAME = "new_change";
	logInfo(LOG_CATEGORY::RTPS_WRITER,"Creating new change";);
	CacheChange_t* ch = m_writer_cache.reserve_Cache();
	if(ch == NULL)
	{
		logWarning(LOG_CATEGORY::RTPS_WRITER,"Problem reserving Cache";);
		return false;
	}
	if(changeKind == ALIVE && data !=NULL && mp_type !=NULL)
	{
		if(!mp_type->serialize(data,&ch->serializedPayload))
		{
			logWarning(LOG_CATEGORY::RTPS_WRITER,"RTPSWriter:Serialization returns false";);
			m_writer_cache.release_Cache(ch);
			return false;
		}
		else if(ch->serializedPayload.length > mp_type->m_typeSize)
		{
			logWarning(LOG_CATEGORY::RTPS_WRITER,"Serialized Payload length larger than maximum type size ("<<ch->serializedPayload.length<<"/"<< mp_type->m_typeSize<<")";);
			m_writer_cache.release_Cache(ch);
			return false;
		}
		else if(ch->serializedPayload.length == 0)
		{
			logWarning(LOG_CATEGORY::RTPS_WRITER,"Serialized Payload length must be set to >0 ";);
			m_writer_cache.release_Cache(ch);
			return false;
		}
	}
	ch->kind = changeKind;

	if(getTopic().getTopicKind() == WITH_KEY && mp_type !=NULL && data!=NULL)
	{
		if(mp_type->m_isGetKeyDefined)
		{
			mp_type->getKey(data,&ch->instanceHandle);
		}
		else
		{
			logWarning(LOG_CATEGORY::RTPS_WRITER,"Get key function not defined";);
		}
	}
	//change->sequenceNumber = lastChangeSequenceNumber;
	ch->writerGUID = m_guid;
	*change_out = ch;
	return true;
}

bool RTPSWriter::add_new_change(ChangeKind_t kind,void*Data)
{
	const char* const METHOD_NAME = "add_new_change";
	if(kind != ALIVE && getTopic().getTopicKind() == NO_KEY)
	{
		logWarning(LOG_CATEGORY::RTPS_WRITER,"NOT ALIVE change in NO KEY Topic ";)
		return false;
	}

	CacheChange_t* change;
	if(new_change(kind,Data,&change))
	{
		logInfo(LOG_CATEGORY::RTPS_WRITER,"New change created";);
		if(!m_writer_cache.add_change(change))
		{
			m_writer_cache.release_Cache(change);
			logInfo(LOG_CATEGORY::RTPS_WRITER,"Change not added";);
			return false;
		}
		this->setLivelinessAsserted(true);
		//DO SOMETHING ONCE THE NEW CHANGE HAS BEEN ADDED.
		unsent_change_add(change);
		if(m_writer_cache.isFull() && mp_listener !=NULL)
			mp_listener->onHistoryFull();
		return true;
	}
	else
		return false;
}

bool RTPSWriter::get_seq_num_min(SequenceNumber_t* seqNum,GUID_t* writerGuid)
{
	CacheChange_t* change;
	if(m_writer_cache.get_min_change(&change))
	{

		*seqNum = change->sequenceNumber;
		if(writerGuid!=NULL)
			*writerGuid = change->writerGUID;
		return true;
	}
	else
	{
		*seqNum = SequenceNumber_t(0,0);
		return false;
	}
}

bool RTPSWriter::get_seq_num_max(SequenceNumber_t* seqNum,GUID_t* writerGuid)
{
	CacheChange_t* change;
	if(m_writer_cache.get_max_change(&change))
	{
		*seqNum = change->sequenceNumber;
		if(writerGuid!=NULL)
			*writerGuid = change->writerGUID;
		return true;
	}
	else
	{
		*seqNum = SequenceNumber_t(0,0);
		return false;
	}
}

bool RTPSWriter::add_change(CacheChange_t*change)
{
	if(m_writer_cache.add_change(change))
	{
		m_livelinessAsserted = true;
		return true;
	}
	return false;
}



} /* namespace rtps */
} /* namespace eprosima */


