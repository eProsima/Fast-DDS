/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WLivelinessPeriodicAssertion.cpp
 *
 */

#include "eprosimartps/builtin/liveliness/timedevent/WLivelinessPeriodicAssertion.h"
#include "eprosimartps/builtin/liveliness/WLP.h"
#include "eprosimartps/Participant.h"

#include "eprosimartps/reader/StatefulReader.h"
#include "eprosimartps/writer/StatefulWriter.h"

#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/utils/eClock.h"

#include "eprosimartps/builtin/discovery/participant/PDPSimple.h"

namespace eprosima {
namespace rtps {

WLivelinessPeriodicAssertion::WLivelinessPeriodicAssertion(WLP* pwlp,LivelinessQosPolicyKind kind):
								TimedEvent(&pwlp->mp_participant->getEventResource()->io_service, boost::posix_time::milliseconds(0)),
								m_livelinessKind(kind),
								mp_WLP(pwlp)
{
	m_guidP = this->mp_WLP->mp_participant->getGuid().guidPrefix;
	for(uint8_t i =0;i<12;++i)
	{
		m_iHandle.value[i] = m_guidP.value[i];
	}
	m_iHandle.value[15] = m_livelinessKind+0x01;
}

WLivelinessPeriodicAssertion::~WLivelinessPeriodicAssertion()
{
	pDebugInfo(RTPS_MAGENTA<<"LivelinessPeriodicAssertion TimedEvent destructor " <<RTPS_DEF<<endl;);
	stop_timer();
	delete(timer);
}

void WLivelinessPeriodicAssertion::event(const boost::system::error_code& ec)
{
	this->m_isWaiting = false;
	if(ec == boost::system::errc::success)
	{
		pDebugInfo(RTPS_MAGENTA<<"Writer LivelinessPeriodic Assertion (period: "<< this->m_interval_msec<< ")"<<RTPS_DEF<<endl;);
		if(this->mp_WLP->mp_builtinParticipantMessageWriter->matchedReadersSize()>0)
		{
			if(m_livelinessKind == AUTOMATIC_LIVELINESS_QOS)
				AutomaticLivelinessAssertion();
			else if(m_livelinessKind == MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
				ManualByParticipantLivelinessAssertion();
		}
		this->mp_WLP->getBuiltinProtocols()->mp_PDP->assertLocalWritersLiveliness(m_livelinessKind);
		this->restart_timer();
	}
	else if(ec==boost::asio::error::operation_aborted)
	{
		pWarning("Liveliness Periodic Assertion aborted"<<endl);
		this->mp_stopSemaphore->post();
	}
	else
	{
		pInfo("Liveliness Periodic Assertion boost message: " <<ec.message()<<endl);
	}
}

bool WLivelinessPeriodicAssertion::AutomaticLivelinessAssertion()
{
	boost::lock_guard<WLP> guard(*this->mp_WLP);
	if(this->mp_WLP->m_livAutomaticWriters.size()>0)
	{
		CacheChange_t* change=NULL;
		boost::lock_guard<Endpoint> guard(*(Endpoint*)this->mp_WLP->mp_builtinParticipantMessageWriter);
		if(this->mp_WLP->mp_builtinParticipantMessageWriter->new_change(ALIVE,NULL,&change))
		{
			change->instanceHandle = m_iHandle;
			change->serializedPayload.encapsulation = (EPROSIMA_ENDIAN == BIGEND) ? PL_CDR_BE: PL_CDR_LE;
			memcpy(change->serializedPayload.data,m_guidP.value,12);

			for(uint8_t i =12;i<24;++i)
				change->serializedPayload.data[i] = 0;
			change->serializedPayload.data[15] = m_livelinessKind+1;
			change->serializedPayload.length = 12+4+4+4;
			mp_WLP->mp_builtinParticipantMessageWriter->add_change(change);
			mp_WLP->mp_builtinParticipantMessageWriter->unsent_change_add(change);
		}

	}
	return true;
}

bool WLivelinessPeriodicAssertion::ManualByParticipantLivelinessAssertion()
{
	boost::lock_guard<WLP> guard(*this->mp_WLP);
	bool livelinessAsserted = false;
	for(std::vector<RTPSWriter*>::iterator wit=this->mp_WLP->m_livManParticipantWriters.begin();
			wit!=this->mp_WLP->m_livManParticipantWriters.end();++wit)
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
		boost::lock_guard<Endpoint> guard(*(Endpoint*)this->mp_WLP->mp_builtinParticipantMessageWriter);
		if(this->mp_WLP->mp_builtinParticipantMessageWriter->new_change(ALIVE,NULL,&change))
		{
			change->instanceHandle = m_iHandle;
			change->serializedPayload.encapsulation = (EPROSIMA_ENDIAN == BIGEND) ? PL_CDR_BE: PL_CDR_LE;
			memcpy(change->serializedPayload.data,m_guidP.value,12);

			for(uint8_t i =12;i<24;++i)
				change->serializedPayload.data[i] = 0;
			change->serializedPayload.data[15] = m_livelinessKind+1;
			change->serializedPayload.length = 12+4+4+4;

			mp_WLP->mp_builtinParticipantMessageWriter->add_change(change);
			mp_WLP->mp_builtinParticipantMessageWriter->unsent_change_add(change);
		}
	}
	return false;
}


} /* namespace rtps */
} /* namespace eprosima */
