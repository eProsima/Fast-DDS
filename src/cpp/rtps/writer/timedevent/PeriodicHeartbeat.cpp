/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PeriodicHeartbeat.cpp
 *
 */

#include <fastrtps/rtps/writer/timedevent/PeriodicHeartbeat.h>
#include <fastrtps/rtps/resources/ResourceEvent.h>

#include <fastrtps/rtps/writer/StatefulWriter.h>
#include <fastrtps/rtps/writer/ReaderProxy.h>

#include "../../participant/RTPSParticipantImpl.h"

#include <fastrtps/rtps/messages/RTPSMessageCreator.h>

#include <fastrtps/utils/RTPSLog.h>

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

namespace eprosima {
namespace fastrtps{
namespace rtps{

static const char* const CLASS_NAME = "PeriodicHeartbeat";

PeriodicHeartbeat::~PeriodicHeartbeat()
{
	const char* const METHOD_NAME = "~PeriodicHeartbeat";
	logInfo(RTPS_WRITER,"Destroying PeriodicHB");
    destroy();
}

PeriodicHeartbeat::PeriodicHeartbeat(StatefulWriter* p_SFW,double interval):
TimedEvent(p_SFW->getRTPSParticipant()->getEventResource().getIOService(),
p_SFW->getRTPSParticipant()->getEventResource().getThread(), interval), mp_SFW(p_SFW)
{

}

void PeriodicHeartbeat::event(EventCode code, const char* msg)
{
	const char* const METHOD_NAME = "event";

    // Unused in release mode.
    (void)msg;

	if(code == EVENT_SUCCESS)
	{
		SequenceNumber_t firstSeq, lastSeq;
		LocatorList_t locList;
		bool unacked_changes = false;
		{//BEGIN PROTECTION
			boost::lock_guard<boost::recursive_mutex> guardW(*mp_SFW->getMutex());
			for(std::vector<ReaderProxy*>::iterator it = mp_SFW->matchedReadersBegin();
					it != mp_SFW->matchedReadersEnd(); ++it)
			{
				if(!unacked_changes)
				{
                    if((*it)->thereIsUnacknowledged())
					{
						unacked_changes= true;
					}
				}
				locList.push_back((*it)->m_att.endpoint.unicastLocatorList);
				locList.push_back((*it)->m_att.endpoint.multicastLocatorList);
			}
			firstSeq = mp_SFW->get_seq_num_min();
			lastSeq = mp_SFW->get_seq_num_max();
		}
		if(unacked_changes)
		{
			if(firstSeq != c_SequenceNumber_Unknown && lastSeq != c_SequenceNumber_Unknown && lastSeq >= firstSeq)
			{
				mp_SFW->incrementHBCount();
				CDRMessage::initCDRMsg(&m_periodic_hb_msg);
				RTPSMessageCreator::addMessageHeartbeat(&m_periodic_hb_msg,mp_SFW->getGuid().guidPrefix,
						mp_SFW->getHBReaderEntityId(),mp_SFW->getGuid().entityId,
						firstSeq,lastSeq,mp_SFW->getHeartbeatCount(),false,false);
				logInfo(RTPS_WRITER,mp_SFW->getGuid().entityId << " Sending Heartbeat ("<<firstSeq<< " - " << lastSeq<<")" );
				for (std::vector<Locator_t>::iterator lit = locList.begin(); lit != locList.end(); ++lit)
					mp_SFW->getRTPSParticipant()->sendSync(&m_periodic_hb_msg, (*lit));
			}
			//Reset TIMER
			this->restart_timer();
		}

	}
	else if(code == EVENT_ABORT)
	{
		logInfo(RTPS_WRITER,"Aborted");
	}
	else
	{
		logInfo(RTPS_WRITER,"Boost message: " <<msg);
	}
}

}
}
} /* namespace eprosima */
