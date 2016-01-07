/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * @file RTPSWriter.cpp
 *
 */

#include <fastrtps/rtps/writer/RTPSWriter.h>
#include <fastrtps/rtps/history/WriterHistory.h>
#include <fastrtps/rtps/writer/timedevent/UnsentChangesNotEmptyEvent.h>
#include <fastrtps/rtps/messages/RTPSMessageCreator.h>
#include <fastrtps/utils/RTPSLog.h>

namespace eprosima {
namespace fastrtps{
namespace rtps {

static const char* const CLASS_NAME = "RTPSWriter";

RTPSWriter::RTPSWriter(RTPSParticipantImpl* impl,GUID_t& guid,WriterAttributes& att,WriterHistory* hist,WriterListener* listen):
				Endpoint(impl,guid,att.endpoint),
				m_pushMode(true),
				m_cdrmessages(hist->m_att.payloadMaxSize),
				m_livelinessAsserted(false),
				mp_unsetChangesNotEmpty(nullptr),
				mp_history(hist),
				mp_listener(listen)
{
	const char* const METHOD_NAME = "RTPSWriter";
	mp_history->mp_writer = this;
    mp_history->mp_mutex = mp_mutex;
	this->init_header();
	logInfo(RTPS_WRITER,"RTPSWriter created");
}

void RTPSWriter::init_header()
{
	CDRMessage::initCDRMsg(&m_cdrmessages.m_rtpsmsg_header);
	RTPSMessageCreator::addHeader(&m_cdrmessages.m_rtpsmsg_header,m_guid.guidPrefix);
}


RTPSWriter::~RTPSWriter()
{
	const char* const METHOD_NAME = "~RTPSWriter";
	logInfo(RTPS_WRITER,"RTPSWriter destructor");

    if(mp_unsetChangesNotEmpty != nullptr)
    {
        mp_unsetChangesNotEmpty->stop_timer();
        delete mp_unsetChangesNotEmpty;
    }

    mp_history->mp_writer = nullptr;
    mp_history->mp_mutex = nullptr;
}

CacheChange_t* RTPSWriter::new_change(ChangeKind_t changeKind,InstanceHandle_t handle)
{
	const char* const METHOD_NAME = "new_change";
	logInfo(RTPS_WRITER,"Creating new change");
	CacheChange_t* ch = nullptr;

	if(!mp_history->reserve_Cache(&ch))
	{
		logWarning(RTPS_WRITER,"Problem reserving Cache from the History");
		return nullptr;
	}

	ch->kind = changeKind;
	if(m_att.topicKind == WITH_KEY && !handle.isDefined())
	{
		logWarning(RTPS_WRITER,"Changes in KEYED Writers need a valid instanceHandle");
		//		if(mp_type->m_isGetKeyDefined)
		//		{
		//			mp_type->getKey(data,&ch->instanceHandle);
		//		}
		//		else
		//		{
		//			logWarning(RTPS_WRITER,"Get key function not defined";);
		//		}
	}
	ch->instanceHandle = handle;
	ch->writerGUID = m_guid;
	return ch;
}

SequenceNumber_t RTPSWriter::get_seq_num_min()
{
	CacheChange_t* change;
	if(mp_history->get_min_change(&change) && change!= nullptr)
		return change->sequenceNumber;
	else
		return c_SequenceNumber_Unknown;
}

SequenceNumber_t RTPSWriter::get_seq_num_max()
{
	CacheChange_t* change;
	if(mp_history->get_max_change(&change) && change!=nullptr)
		return change->sequenceNumber;
	else
		return c_SequenceNumber_Unknown;
}

uint32_t RTPSWriter::getTypeMaxSerialized()
{
	return mp_history->getTypeMaxSerialized();
}


//bool RTPSWriter::add_new_change(ChangeKind_t kind,void*Data)
//{
//	const char* const METHOD_NAME = "add_new_change";
//	if(kind != ALIVE && getTopic().getTopicKind() == NO_KEY)
//	{
//		logWarning(RTPS_WRITER,"NOT ALIVE change in NO KEY Topic ";)
//		return false;
//	}
//
//	CacheChange_t* change;
//	if(new_change(kind,Data,&change))
//	{
//		logInfo(RTPS_WRITER,"New change created";);
//		if(!m_writer_cache.add_change(change))
//		{
//			m_writer_cache.release_Cache(change);
//			logInfo(RTPS_WRITER,"Change not added";);
//			return false;
//		}
//		this->setLivelinessAsserted(true);
//		//DO SOMETHING ONCE THE NEW CHANGE HAS BEEN ADDED.
//		unsent_change_add(change);
//		if(m_writer_cache.isFull() && mp_listener !=NULL)
//			mp_listener->onHistoryFull();
//		return true;
//	}
//	else
//		return false;
//}

//bool RTPSWriter::get_seq_num_min(SequenceNumber_t* seqNum,GUID_t* writerGuid)
//{
//	CacheChange_t* change;
//	if(m_writer_cache.get_min_change(&change))
//	{
//
//		*seqNum = change->sequenceNumber;
//		if(writerGuid!=NULL)
//			*writerGuid = change->writerGUID;
//		return true;
//	}
//	else
//	{
//		*seqNum = SequenceNumber_t(0,0);
//		return false;
//	}
//}
//
//bool RTPSWriter::get_seq_num_max(SequenceNumber_t* seqNum,GUID_t* writerGuid)
//{
//	CacheChange_t* change;
//	if(m_writer_cache.get_max_change(&change))
//	{
//		*seqNum = change->sequenceNumber;
//		if(writerGuid!=NULL)
//			*writerGuid = change->writerGUID;
//		return true;
//	}
//	else
//	{
//		*seqNum = SequenceNumber_t(0,0);
//		return false;
//	}
//}

//bool RTPSWriter::add_change(CacheChange_t*change)
//{
//	if(m_writer_cache.add_change(change))
//	{
//		m_livelinessAsserted = true;
//		return true;
//	}
//	return false;
//}



} /* namespace rtps */
} /* namespace eprosima */


}
