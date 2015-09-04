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
	stop_timer();
}

HeartbeatResponseDelay::HeartbeatResponseDelay(WriterProxy* p_WP,double interval):
		TimedEvent(p_WP->mp_SFR->getRTPSParticipant()->getIOService(),interval),
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
		std::vector<ChangeFromWriter_t*> ch_vec;
		{
		boost::lock_guard<boost::recursive_mutex> guard(*mp_WP->getMutex());
		mp_WP->missing_changes(&ch_vec);
		}
		if(!ch_vec.empty() || !mp_WP->m_heartbeatFinalFlag)
		{
			SequenceNumberSet_t sns;
			if(!mp_WP->available_changes_max(&sns.base)) //if no changes are available
			{
				logError(RTPS_READER,"No available changes max"<<endl;);
			}
			sns.base++;
			std::vector<ChangeFromWriter_t*>::iterator cit;
			for(cit = ch_vec.begin();cit!=ch_vec.end();++cit)
			{
				if(!sns.add((*cit)->seqNum))
				{
					logWarning(RTPS_READER,"Error adding seqNum "<<(*cit)->seqNum.to64long()
							<< " with SeqNumSet Base: "<< sns.base.to64long());
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
