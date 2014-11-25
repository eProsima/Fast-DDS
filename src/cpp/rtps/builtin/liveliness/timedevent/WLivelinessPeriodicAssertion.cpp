/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WLivelinessPeriodicAssertion.cpp
 *
 */

#include "fastrtps/rtps/builtin/liveliness/timedevent/WLivelinessPeriodicAssertion.h"
#include "fastrtps/rtps/builtin/liveliness/WLP.h"
#include "fastrtps/rtps/participant/RTPSParticipantImpl.h"
//
//#include "fastrtps/reader/StatefulReader.h"
//#include "fastrtps/writer/StatefulWriter.h"

#include "fastrtps/utils/RTPSLog.h"
#include "fastrtps/utils/eClock.h"

//#include "fastrtps/builtin/discovery/RTPSParticipant/PDPSimple.h"

namespace eprosima {
namespace fastrtps{
namespace rtps {

static const char* const CLASS_NAME = "WLivelinessPeriodicAssertion";

WLivelinessPeriodicAssertion::WLivelinessPeriodicAssertion(WLP* pwlp,LivelinessQosPolicyKind kind):
								TimedEvent(&pwlp->getRTPSParticipant()->getIOService(), 0),
								m_livelinessKind(kind),
								mp_WLP(pwlp)
{
	m_guidP = this->mp_WLP->getRTPSParticipant()->getGuid().guidPrefix;
	for(uint8_t i =0;i<12;++i)
	{
		m_iHandle.value[i] = m_guidP.value[i];
	}
	m_iHandle.value[15] = m_livelinessKind+0x01;
}

WLivelinessPeriodicAssertion::~WLivelinessPeriodicAssertion()
{
	const char* const METHOD_NAME = "WLivelinessPeriodicAssertion";
	logInfo(RTPS_LIVELINESS,"TimedEvent destructor.",C_MAGENTA);
}

void WLivelinessPeriodicAssertion::event(EventCode code, const char* msg= nullptr)
{
	const char* const METHOD_NAME = "event";
	if(code == EVENT_SUCCESS)
	{
		logInfo(RTPS_LIVELINESS,"Period: "<< this->m_interval_msec,C_MAGENTA);
		if(this->mp_WLP->mp_builtinRTPSParticipantMessageWriter->matchedReadersSize()>0)
		{
			if(m_livelinessKind == AUTOMATIC_LIVELINESS_QOS)
				AutomaticLivelinessAssertion();
			else if(m_livelinessKind == MANUAL_BY_RTPSParticipant_LIVELINESS_QOS)
				ManualByRTPSParticipantLivelinessAssertion();
		}
		this->mp_WLP->getBuiltinProtocols()->mp_PDP->assertLocalWritersLiveliness(m_livelinessKind);
		this->restart_timer();
	}
	else if(code == EVENT_ABORT)
	{
		logWarning(RTPS_LIVELINESS,"Liveliness Periodic Assertion aborted",C_MAGENTA);
		this->stopSemaphorePost();
	}
	else
	{
		logInfo(RTPS_LIVELINESS,"Boost message: " <<ec.message(),C_MAGENTA);
	}
}

bool WLivelinessPeriodicAssertion::AutomaticLivelinessAssertion()
{
	boost::lock_guard<WLP> guard(*this->mp_WLP);
	if(this->mp_WLP->m_livAutomaticWriters.size()>0)
	{
		CacheChange_t* change=NULL;
		boost::lock_guard<Endpoint> guard(*(Endpoint*)this->mp_WLP->mp_builtinRTPSParticipantMessageWriter);
		if(this->mp_WLP->mp_builtinRTPSParticipantMessageWriter->new_change(ALIVE,NULL,&change))
		{
			change->instanceHandle = m_iHandle;
			change->serializedPayload.encapsulation = (EPROSIMA_ENDIAN == BIGEND) ? PL_CDR_BE: PL_CDR_LE;
			memcpy(change->serializedPayload.data,m_guidP.value,12);

			for(uint8_t i =12;i<24;++i)
				change->serializedPayload.data[i] = 0;
			change->serializedPayload.data[15] = m_livelinessKind+1;
			change->serializedPayload.length = 12+4+4+4;
			mp_WLP->mp_builtinRTPSParticipantMessageWriter->add_change(change);
			mp_WLP->mp_builtinRTPSParticipantMessageWriter->unsent_change_add(change);
		}

	}
	return true;
}

bool WLivelinessPeriodicAssertion::ManualByRTPSParticipantLivelinessAssertion()
{
	boost::lock_guard<WLP> guard(*this->mp_WLP);
	bool livelinessAsserted = false;
	for(std::vector<RTPSWriter*>::iterator wit=this->mp_WLP->m_livManRTPSParticipantWriters.begin();
			wit!=this->mp_WLP->m_livManRTPSParticipantWriters.end();++wit)
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
		boost::lock_guard<Endpoint> guard(*(Endpoint*)this->mp_WLP->mp_builtinRTPSParticipantMessageWriter);
		if(this->mp_WLP->mp_builtinRTPSParticipantMessageWriter->new_change(ALIVE,NULL,&change))
		{
			change->instanceHandle = m_iHandle;
			change->serializedPayload.encapsulation = (EPROSIMA_ENDIAN == BIGEND) ? PL_CDR_BE: PL_CDR_LE;
			memcpy(change->serializedPayload.data,m_guidP.value,12);

			for(uint8_t i =12;i<24;++i)
				change->serializedPayload.data[i] = 0;
			change->serializedPayload.data[15] = m_livelinessKind+1;
			change->serializedPayload.length = 12+4+4+4;

			mp_WLP->mp_builtinRTPSParticipantMessageWriter->add_change(change);
			mp_WLP->mp_builtinRTPSParticipantMessageWriter->unsent_change_add(change);
		}
	}
	return false;
}

}
} /* namespace rtps */
} /* namespace eprosima */
