/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file HeartbeatResponseDelay.cpp
 *
 */

#include <fastrtps/rtps/reader/timedevent/HeartbeatResponseDelay.h>
#include <fastrtps/rtps/resources/ResourceEvent.h>
#include <fastrtps/rtps/reader/WriterProxy.h>

#include <fastrtps/rtps/reader/StatefulReader.h>
#include "../../participant/RTPSParticipantImpl.h"

#include <fastrtps/rtps/messages/RTPSMessageCreator.h>
#include <fastrtps/rtps/messages/CDRMessage.h>
#include <fastrtps/utils/RTPSLog.h>

#include <boost/thread/lock_guard.hpp>
#include <boost/thread/recursive_mutex.hpp>

namespace eprosima {
namespace fastrtps{
namespace rtps {

static const char* const CLASS_NAME = "HeartbeatResponseDelay";

HeartbeatResponseDelay::~HeartbeatResponseDelay()
{
    destroy();
}

HeartbeatResponseDelay::HeartbeatResponseDelay(WriterProxy* p_WP,double interval):
TimedEvent(p_WP->mp_SFR->getRTPSParticipant()->getEventResource().getIOService(),
p_WP->mp_SFR->getRTPSParticipant()->getEventResource().getThread(), interval),
mp_WP(p_WP)
{

}

void HeartbeatResponseDelay::event(EventCode code, const char* msg)
{
	const char* const METHOD_NAME = "event";

    // Unused in release mode.
    (void)msg;

	if(code == EVENT_SUCCESS)
	{
		logInfo(RTPS_READER,"");
		const std::vector<ChangeFromWriter_t> ch_vec = mp_WP->missing_changes();

		if(!ch_vec.empty() || !mp_WP->m_heartbeatFinalFlag)
		{
			SequenceNumberSet_t sns;
            sns.base = mp_WP->available_changes_max();
			sns.base++;

			for(auto ch : ch_vec)
			{
				if(!sns.add(ch.getSequenceNumber()))
				{
					logWarning(RTPS_READER,"Error adding seqNum " << ch.getSequenceNumber()
							<< " with SeqNumSet Base: "<< sns.base);
					break;
				}
			}

			mp_WP->m_acknackCount++;
			logInfo(RTPS_READER,"Sending ACKNACK: "<< sns;);

			bool final = false;
			if(sns.isSetEmpty())
				final = true;
			CDRMessage::initCDRMsg(&m_heartbeat_response_msg);
			RTPSMessageCreator::addMessageAcknack(&m_heartbeat_response_msg,
												mp_WP->mp_SFR->getGuid().guidPrefix,
                                                mp_WP->m_att.guid.guidPrefix,
												mp_WP->mp_SFR->getGuid().entityId,
												mp_WP->m_att.guid.entityId,
												sns,
												mp_WP->m_acknackCount,
												final);

			std::vector<Locator_t>::iterator lit;

			for(lit = mp_WP->m_att.endpoint.unicastLocatorList.begin();
					lit!=mp_WP->m_att.endpoint.unicastLocatorList.end();++lit)
				mp_WP->mp_SFR->getRTPSParticipant()->sendSync(&m_heartbeat_response_msg,(*lit));

			for(lit = mp_WP->m_att.endpoint.multicastLocatorList.begin();
					lit!=mp_WP->m_att.endpoint.multicastLocatorList.end();++lit)
				mp_WP->mp_SFR->getRTPSParticipant()->sendSync(&m_heartbeat_response_msg,(*lit));

		}

	}
	else if(code == EVENT_ABORT)
	{
		logInfo(RTPS_READER,"HeartbeatResponseDelay aborted");
	}
	else
	{
		logInfo(RTPS_READER,"HeartbeatResponseDelay boost message: " <<msg);
	}
}



}
} /* namespace rtps */
} /* namespace eprosima */
