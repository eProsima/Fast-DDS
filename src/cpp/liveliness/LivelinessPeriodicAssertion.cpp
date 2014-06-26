/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file LivelinessPeriodicAssertion.cpp
 */

#include "eprosimartps/liveliness/LivelinessPeriodicAssertion.h"
#include "eprosimartps/liveliness/WriterLiveliness.h"
#include "eprosimartps/Participant.h"
#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/utils/eClock.h"

#include "eprosimartps/writer/StatefulWriter.h"

namespace eprosima {
namespace rtps {

LivelinessPeriodicAssertion::LivelinessPeriodicAssertion(WriterLiveliness* wLiveliness,LivelinessQosPolicyKind kind):
		TimedEvent(&wLiveliness->mp_participant->getEventResource()->io_service, boost::posix_time::milliseconds(0)),
		m_livelinessKind(kind),
		mp_writerLiveliness(wLiveliness),
		first(false)
{
	m_guidP = this->mp_writerLiveliness->mp_participant->getGuid().guidPrefix;
}

LivelinessPeriodicAssertion::~LivelinessPeriodicAssertion()
{
	timer->cancel();
		delete(timer);
}

void LivelinessPeriodicAssertion::event(const boost::system::error_code& ec)
{
	this->m_isWaiting = false;
	if(ec == boost::system::errc::success)
	{
		pDebugInfo(RTPS_MAGENTA<<"LivelinessPeriodic Assertion (period: "<< this->m_interval_msec<< ")"<<RTPS_DEF<<endl;)
		if(first) //FIRST TIME, WE CREATE IT
		{
			first = false;
			for(uint8_t i =0;i<12;++i)
			{
				m_iHandle.value[i] = m_guidP.value[i];
			}
			m_iHandle.value[15] = m_livelinessKind;
		}
		if(this->mp_writerLiveliness->mp_builtinParticipantMessageWriter->matchedReadersSize()>0)
		{
			if(m_livelinessKind == AUTOMATIC_LIVELINESS_QOS)
				AutomaticLivelinessAssertion();
			else if(m_livelinessKind == MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
				ManualByParticipantLivelinessAssertion();
		}
		//eClock::my_sleep(1000);
		//RESTART TIMER
		this->restart_timer();
	}
	else if(ec==boost::asio::error::operation_aborted)
	{
		pWarning("Liveliness Periodic Assertion aborted"<<endl);
	}
	else
	{
		pInfo("Liveliness Periodic Assertion boost message: " <<ec.message()<<endl);
	}
}

bool LivelinessPeriodicAssertion::AutomaticLivelinessAssertion()
{
	if(this->mp_writerLiveliness->m_AutomaticLivelinessWriters.size()>0)
	{
		CacheChange_t* change=NULL;
		boost::lock_guard<Endpoint> guard(*(Endpoint*)this->mp_writerLiveliness->mp_builtinParticipantMessageWriter);
		if(this->mp_writerLiveliness->mp_builtinParticipantMessageWriter->new_change(ALIVE,NULL,&change))
		{
			change->instanceHandle = m_iHandle;
			change->serializedPayload.encapsulation = (EPROSIMA_ENDIAN == BIGEND) ? PL_CDR_BE: PL_CDR_LE;
			memcpy(change->serializedPayload.data,m_guidP.value,12);

			for(uint8_t i =12;i<24;++i)
				change->serializedPayload.data[i] = 0;
			change->serializedPayload.data[15] = m_livelinessKind+1;
			change->serializedPayload.length = 12+4+4+4;
//			//FIXME: PREPARE HISTORYCACHE TO SUPPORT DIFFERENT HISTORYKIND.
//			removeMinSeqNumByKey();
			mp_writerLiveliness->mp_builtinParticipantMessageWriter->add_change(change);
			mp_writerLiveliness->mp_builtinParticipantMessageWriter->unsent_change_add(change);
		}
	}
	return true;
}

bool LivelinessPeriodicAssertion::removeMinSeqNumByKey()
{
//	bool found = false;
//	std::vector<CacheChange_t*>::iterator chit;
//	for(chit= mp_writerLiveliness->mp_builtinParticipantMessageWriter->m_writer_cache.m_changes.begin();
//			chit!=mp_writerLiveliness->mp_builtinParticipantMessageWriter->m_writer_cache.m_changes.end();++chit)
//	{
//		if((*chit)->instanceHandle == m_iHandle)
//		{
//			found = true;
//			break;
//		}
//
//	}
//	if(!found)
//		return false;
//	mp_writerLiveliness->mp_builtinParticipantMessageWriter->m_writer_cache.remove_change(*chit);
//	for(std::vector<ReaderProxy*>::iterator it = mp_writerLiveliness->mp_builtinParticipantMessageWriter->matchedReadersBegin();
//			it!=mp_writerLiveliness->mp_builtinParticipantMessageWriter->matchedReadersEnd();++it)
//	{
//		for(std::vector<ChangeForReader_t>::iterator it2 = (*it)->m_changesForReader.begin();
//				it2!=(*it)->m_changesForReader.end();++it2)
//		{
//			if(it2->seqNum == (*chit)->sequenceNumber)
//			{
//				it2->is_relevant = false;
//				break;
//			}
//		}
//	}
	return true;
}

bool LivelinessPeriodicAssertion::ManualByParticipantLivelinessAssertion()
{
	bool livelinessAsserted = false;
	for(std::vector<RTPSWriter*>::iterator wit=this->mp_writerLiveliness->m_ManualByParticipantLivelinessWriters.begin();
			wit!=this->mp_writerLiveliness->m_ManualByParticipantLivelinessWriters.end();++wit)
	{
		if((*wit)->getLivelinessAsserted())
		{
			livelinessAsserted = true;
		}
		(*wit)->setLivelinessAsserted(false);
	}
	if(livelinessAsserted)
	{
		CacheChange_t* change=NULL;
		boost::lock_guard<Endpoint> guard(*(Endpoint*)this->mp_writerLiveliness->mp_builtinParticipantMessageWriter);
		if(this->mp_writerLiveliness->mp_builtinParticipantMessageWriter->new_change(ALIVE,NULL,&change))
		{
			change->instanceHandle = m_iHandle;
			change->serializedPayload.encapsulation = (EPROSIMA_ENDIAN == BIGEND) ? PL_CDR_BE: PL_CDR_LE;
			change->serializedPayload.length = 0;

//			//FIXME: PREPARE HISTORYCACHE TO SUPPORT DIFFERENT HISTORYKIND.
//			removeMinSeqNumByKey();
			mp_writerLiveliness->mp_builtinParticipantMessageWriter->add_change(change);
			mp_writerLiveliness->mp_builtinParticipantMessageWriter->unsent_change_add(change);
		}
	}
	return false;
}




} /* namespace rtps */
} /* namespace eprosima */


