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

}

void LivelinessPeriodicAssertion::event(const boost::system::error_code& ec)
{
	this->m_isWaiting = false;
	if(ec == boost::system::errc::success)
	{
		if(first) //FIRST TIME, WE CREATE IT
		{
			first = false;
			for(uint8_t i =0;i<12;++i)
			{
				m_iHandle.value[i] = m_guidP.value[i];
			}
			m_iHandle.value[15] = m_livelinessKind;
		}
		if(m_livelinessKind == AUTOMATIC_LIVELINESS_QOS)
			AutomaticLivelinessAssertion();
		else if(m_livelinessKind == MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
			ManualByParticipantLivelinessAssertion();

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
			change->serializedPayload.length = 0;
			//FIXME: PREPARE HISTORYCACHE TO SUPPORT DIFFERENT HISTORYKIND.
			mp_writerLiveliness->mp_builtinParticipantMessageWriter->removeMinSeqCacheChangeByKey(m_iHandle);
			mp_writerLiveliness->mp_builtinParticipantMessageWriter->add_change(change);
			mp_writerLiveliness->mp_builtinParticipantMessageWriter->unsent_change_add(change);
		}
	}
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

			//FIXME: PREPARE HISTORYCACHE TO SUPPORT DIFFERENT HISTORYKIND.
			mp_writerLiveliness->mp_builtinParticipantMessageWriter->removeMinSeqCacheChangeByKey(m_iHandle);
			mp_writerLiveliness->mp_builtinParticipantMessageWriter->add_change(change);
			mp_writerLiveliness->mp_builtinParticipantMessageWriter->unsent_change_add(change);
		}
	}
	return false;
}




} /* namespace rtps */
} /* namespace eprosima */


