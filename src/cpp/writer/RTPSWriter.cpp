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
#include "eprosimartps/HistoryCache.h"
#include "eprosimartps/CDRMessage.h"
#include "eprosimartps/dds/Publisher.h"
#include "eprosimartps/dds/PublisherListener.h"
#include "eprosimartps/dds/DDSTopicDataType.h"
#include "eprosimartps/qos/ParameterList.h"

#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/RTPSMessageCreator.h"

namespace eprosima {
namespace rtps {


RTPSWriter::RTPSWriter(GuidPrefix_t guidP,EntityId_t entId,const PublisherAttributes& param,DDSTopicDataType* ptype,
		StateKind_t state,
		int16_t userDefinedId, uint32_t payload_size):
					Endpoint(guidP,entId,param.topic,ptype,state,WRITER,userDefinedId),
					m_writer_cache((Endpoint*)this,payload_size),
					m_pushMode(true),
					m_cdrmessages(payload_size),
					mp_listener(NULL),
					m_livelinessAsserted(false)
{
	init_header();
	pDebugInfo("RTPSWriter created"<<endl)

}

void RTPSWriter::init_header()
{
	CDRMessage::initCDRMsg(&m_cdrmessages.m_rtpsmsg_header);
	RTPSMessageCreator::addHeader(&m_cdrmessages.m_rtpsmsg_header,m_guid.guidPrefix);
}



RTPSWriter::~RTPSWriter()
{
	pDebugInfo("RTPSWriter destructor"<<endl;);
}

bool RTPSWriter::new_change(ChangeKind_t changeKind,void* data,CacheChange_t** change_out)
{
	pDebugInfo("Creating new change"<<endl);
	CacheChange_t* ch = m_writer_cache.reserve_Cache();
	if(ch == NULL)
	{
		pWarning("Problem reserving Cache"<<endl);
		return false;
	}
	if(changeKind == ALIVE && data !=NULL && mp_type !=NULL)
	{
		if(!mp_type->serialize(data,&ch->serializedPayload))
		{
			pWarning("RTPSWriter:Serialization returns false"<<endl);
			return false;
		}
	}
	ch->kind = changeKind;

	if(getTopic().getTopicKind() == WITH_KEY && mp_type !=NULL)
	{
		if(mp_type->m_isGetKeyDefined)
		{
			mp_type->getKey(data,&ch->instanceHandle);
		}
		else
		{
			pWarning("Get key function not defined"<<endl);
		}
	}
	//change->sequenceNumber = lastChangeSequenceNumber;
	ch->writerGUID = m_guid;
	*change_out = ch;
	return true;
}

bool RTPSWriter::add_new_change(ChangeKind_t kind,void*Data)
{
	if(kind != ALIVE && getTopic().getTopicKind() == NO_KEY)
	{
		pWarning("NOT ALIVE change in NO KEY Topic "<<endl)
		return false;
	}

	CacheChange_t* change;
	if(new_change(kind,Data,&change))
	{
		pDebugInfo("New change created"<<endl);
		if(!m_writer_cache.add_change(change))
		{
			m_writer_cache.release_Cache(change);
			return false;
		}
		m_livelinessAsserted = true;
		//DO SOMETHING ONCE THE NEW CHANGE HAS BEEN ADDED.
		unsent_change_add(change);
		if(m_writer_cache.isFull() && mp_listener !=NULL)
			mp_listener->onHistoryFull();
		return true;
	}
	else
		return false;
}




} /* namespace rtps */
} /* namespace eprosima */


