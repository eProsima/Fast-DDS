/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PeriodicHeartbeat.cpp
 *
 */

#include "fastrtps/rtps/writer/timedevent/PeriodicHeartbeat.h"

#include "fastrtps/rtps/writer/StatefulWriter.h"
#include "fastrtps/rtps/writer/ReaderProxy.h"

#include "fastrtps/rtps/participant/RTPSParticipantImpl.h"

#include "fastrtps/rtps/messages/RTPSMessageCreator.h"

#include "fastrtps/utils/RTPSLog.h"


namespace eprosima {
namespace fastrtps{
namespace rtps{

static const char* const CLASS_NAME = "PeriodicHeartbeat";

PeriodicHeartbeat::~PeriodicHeartbeat()
{

}

PeriodicHeartbeat::PeriodicHeartbeat(StatefulWriter* p_SFW,double interval):
				TimedEvent(p_SFW->getRTPSParticipant()->getIOService(),interval),
				mp_SFW(p_SFW)
{

}

void PeriodicHeartbeat::event(EventCode code, const char* msg)
{
	const char* const METHOD_NAME = "event";
	logInfo(RTPS_WRITER,"TimedEvent: PeriodicHeartbeat";);
	if(code == EVENT_SUCCESS)
	{
		logInfo(RTPS_WRITER,"Sending Heartbeat");
		std::vector<ChangeForReader_t*> unack;
		bool unacked_changes = false;
		for(std::vector<ReaderProxy*>::iterator it = mp_SFW->matchedReadersBegin();
				it!=mp_SFW->matchedReadersEnd();++it)
		{
			unack.clear();
			(*it)->unacked_changes(&unack);
			if(!unack.empty())
			{
				unacked_changes= true;
				break;
			}
		}
		if(unacked_changes)
		{
			SequenceNumber_t first = mp_SFW->get_seq_num_min();
			SequenceNumber_t last = mp_SFW->get_seq_num_max();
			if(first.to64long()>0 && last.to64long() >= first.to64long())
			{
				mp_SFW->incrementHBCount();
				CDRMessage::initCDRMsg(&m_periodic_hb_msg);
				RTPSMessageCreator::addMessageHeartbeat(&m_periodic_hb_msg,mp_SFW->getGuid().guidPrefix,
						mp_SFW->getHBReaderEntityId(),mp_SFW->getGuid().entityId,
						first,last,mp_SFW->getHeartbeatCount(),false,false);
				std::vector<Locator_t>::iterator lit;
				for(std::vector<ReaderProxy*>::iterator rit = mp_SFW->matchedReadersBegin();
						rit!=mp_SFW->matchedReadersEnd();++rit)
				{
					for(lit = (*rit)->m_att.endpoint.unicastLocatorList.begin();
							lit!=(*rit)->m_att.endpoint.unicastLocatorList.end();++lit)
						mp_SFW->getRTPSParticipant()->sendSync(&m_periodic_hb_msg,(*lit));
					for(lit = (*rit)->m_att.endpoint.multicastLocatorList.begin();
							lit!=(*rit)->m_att.endpoint.multicastLocatorList.end();++lit)
						mp_SFW->getRTPSParticipant()->sendSync(&m_periodic_hb_msg,(*lit));
				}
			}
			//Reset TIMER
			this->restart_timer();
		}

	}
	else if(code == EVENT_ABORT)
	{
		logWarning(RTPS_WRITER,"Aborted");
		this->stopSemaphorePost();
	}
	else
	{
		logInfo(RTPS_WRITER,"Boost message: " <<msg);
	}
}

}
}
} /* namespace eprosima */
