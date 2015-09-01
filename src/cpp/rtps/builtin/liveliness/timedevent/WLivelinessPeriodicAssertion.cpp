/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WLivelinessPeriodicAssertion.cpp
 *
 */

#include <fastrtps/rtps/builtin/liveliness/timedevent/WLivelinessPeriodicAssertion.h>
#include <fastrtps/rtps/builtin/liveliness/WLP.h>
#include "../../../participant/RTPSParticipantImpl.h"

#include <fastrtps/rtps/writer/StatefulWriter.h>
#include <fastrtps/rtps/history/WriterHistory.h>

#include <fastrtps/utils/RTPSLog.h>
#include <fastrtps/utils/eClock.h>

#include <fastrtps/rtps/builtin/discovery/participant/PDPSimple.h>
#include <fastrtps/rtps/builtin/BuiltinProtocols.h>

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>


namespace eprosima {
namespace fastrtps{
namespace rtps {

static const char* const CLASS_NAME = "WLivelinessPeriodicAssertion";

WLivelinessPeriodicAssertion::WLivelinessPeriodicAssertion(WLP* pwlp,LivelinessQosPolicyKind kind):
												TimedEvent(pwlp->getRTPSParticipant()->getIOService(), 0),
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
	//const char* const METHOD_NAME = "~WLivelinessPeriodicAssertion";
	stop_timer();
}

void WLivelinessPeriodicAssertion::event(EventCode code, const char* msg)
{
	const char* const METHOD_NAME = "event";

    // Unused in release mode.
    (void)msg;

	if(code == EVENT_SUCCESS)
	{
		logInfo(RTPS_LIVELINESS,"Period: "<< this->getIntervalMilliSec(),C_MAGENTA);
		if(this->mp_WLP->mp_builtinWriter->getMatchedReadersSize()>0)
		{
			if(m_livelinessKind == AUTOMATIC_LIVELINESS_QOS)
				AutomaticLivelinessAssertion();
			else if(m_livelinessKind == MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
				ManualByRTPSParticipantLivelinessAssertion();
		}
		this->mp_WLP->getBuiltinProtocols()->mp_PDP->assertLocalWritersLiveliness(m_livelinessKind);
		this->restart_timer();
	}
	else if(code == EVENT_ABORT)
	{
		logInfo(RTPS_LIVELINESS,"Liveliness Periodic Assertion aborted",C_MAGENTA);
	}
	else
	{
		logInfo(RTPS_LIVELINESS,"Boost message: " <<msg,C_MAGENTA);
	}
}

bool WLivelinessPeriodicAssertion::AutomaticLivelinessAssertion()
{
	boost::lock_guard<boost::recursive_mutex> guard(*this->mp_WLP->getMutex());
	if(this->mp_WLP->m_livAutomaticWriters.size()>0)
	{
		boost::lock_guard<boost::recursive_mutex> guard(*this->mp_WLP->mp_builtinWriter->getMutex());
		CacheChange_t* change=this->mp_WLP->mp_builtinWriter->new_change(ALIVE,m_iHandle);
		if(change!=nullptr)
		{
			//change->instanceHandle = m_iHandle;
			change->serializedPayload.encapsulation = (EPROSIMA_ENDIAN == BIGEND) ? PL_CDR_BE: PL_CDR_LE;
			memcpy(change->serializedPayload.data,m_guidP.value,12);
			for(uint8_t i =12;i<24;++i)
				change->serializedPayload.data[i] = 0;
			change->serializedPayload.data[15] = m_livelinessKind+1;
			change->serializedPayload.length = 12+4+4+4;
			if(mp_WLP->mp_builtinWriterHistory->getHistorySize() > 0)
			{
				for(std::vector<CacheChange_t*>::iterator chit = mp_WLP->mp_builtinWriterHistory->changesBegin();
						chit!=mp_WLP->mp_builtinWriterHistory->changesEnd();++chit)
				{
					if((*chit)->instanceHandle == change->instanceHandle)
					{
						mp_WLP->mp_builtinWriterHistory->remove_change(*chit);
						break;
					}
				}
			}
			mp_WLP->mp_builtinWriterHistory->add_change(change);
		}
	}
	return true;
}

bool WLivelinessPeriodicAssertion::ManualByRTPSParticipantLivelinessAssertion()
{
	boost::lock_guard<boost::recursive_mutex> guard(*this->mp_WLP->getMutex());
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
		boost::lock_guard<boost::recursive_mutex> guard(*this->mp_WLP->mp_builtinWriter->getMutex());
		CacheChange_t* change=this->mp_WLP->mp_builtinWriter->new_change(ALIVE);
		if(change!=nullptr)
		{
			change->instanceHandle = m_iHandle;
			change->serializedPayload.encapsulation = (EPROSIMA_ENDIAN == BIGEND) ? PL_CDR_BE: PL_CDR_LE;
			memcpy(change->serializedPayload.data,m_guidP.value,12);

			for(uint8_t i =12;i<24;++i)
				change->serializedPayload.data[i] = 0;
			change->serializedPayload.data[15] = m_livelinessKind+1;
			change->serializedPayload.length = 12+4+4+4;
			for(auto ch = mp_WLP->mp_builtinWriterHistory->changesBegin();
					ch!=mp_WLP->mp_builtinWriterHistory->changesEnd();++ch)
			{
				if((*ch)->instanceHandle == change->instanceHandle)
				{
					mp_WLP->mp_builtinWriterHistory->remove_change(*ch);
				}
			}
			mp_WLP->mp_builtinWriterHistory->add_change(change);
		}
	}
	return false;
}

}
} /* namespace rtps */
} /* namespace eprosima */
