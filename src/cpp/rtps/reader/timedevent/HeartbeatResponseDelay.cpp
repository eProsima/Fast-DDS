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
        boost::lock_guard<boost::recursive_mutex> reader_guard(*mp_WP->mp_SFR->getMutex());
        boost::lock_guard<boost::recursive_mutex> guard(*mp_WP->getMutex());

        // Stores missing changes;
		std::vector<ChangeFromWriter_t*> missing_changes;
        // Stores missing changes but there is some fragments received.
        std::vector<CacheChange_t*> uncompleted_changes;

        mp_WP->missing_changes(&missing_changes);

		if(!missing_changes.empty() || !mp_WP->m_heartbeatFinalFlag)
		{
			SequenceNumberSet_t sns;
			if(!mp_WP->available_changes_max(&sns.base)) //if no changes are available
			{
				logError(RTPS_READER,"No available changes max"<<endl;);
			}

			sns.base++;

			for(auto cit = missing_changes.begin();cit!=missing_changes.end();++cit)
			{
                // Check if the CacheChange_t is uncompleted.
                CacheChange_t* uncomplete_change = mp_WP->mp_SFR->findCacheInFragmentedCachePitStop((*cit)->seqNum, mp_WP->m_att.guid);

                if(uncomplete_change == nullptr)
                {
                    if(!sns.add((*cit)->seqNum))
                    {
                        logWarning(RTPS_READER,"Error adding seqNum " << (*cit)->seqNum
                                << " with SeqNumSet Base: "<< sns.base);
                        break;
                    }
                }
                else
                {
                    uncompleted_changes.push_back(uncomplete_change);
                }
			}
			++mp_WP->m_acknackCount;
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

			for(auto lit = mp_WP->m_att.endpoint.unicastLocatorList.begin();
					lit!=mp_WP->m_att.endpoint.unicastLocatorList.end();++lit)
				mp_WP->mp_SFR->getRTPSParticipant()->sendSync(&m_heartbeat_response_msg,(*lit));

			for(auto lit = mp_WP->m_att.endpoint.multicastLocatorList.begin();
					lit!=mp_WP->m_att.endpoint.multicastLocatorList.end();++lit)
				mp_WP->mp_SFR->getRTPSParticipant()->sendSync(&m_heartbeat_response_msg,(*lit));
		}

        // Now generage NACK_FRAGS
        if(!uncompleted_changes.empty())
        {
            for(auto cit : uncompleted_changes)
            {
                FragmentNumberSet_t frag_sns;

                //  Search first fragment not present.
                uint32_t frag_num = 0;
                auto fit = cit->getDataFragments()->begin();
                for(; fit != cit->getDataFragments()->end(); ++fit)
                {
                    ++frag_num;
                    if(*fit == ChangeFragmentStatus_t::NOT_PRESENT)
                        break;
                }

                // Never should happend.
                assert(frag_num != 0);
                assert(fit != cit->getDataFragments()->end());

                // Store FragmentNumberSet_t base.
                frag_sns.base = frag_num;

                // Fill the FragmentNumberSet_t bitmap.
                for(; fit != cit->getDataFragments()->end(); ++fit)
                {
                    ++frag_num;
                    if(*fit == ChangeFragmentStatus_t::NOT_PRESENT)
                        frag_sns.add(frag_num);
                }

                ++mp_WP->m_nackfragCount;
                logInfo(RTPS_READER,"Sending NACKFRAG for sample" << cit->sequenceNumber << ": "<< frag_sns;);
                CDRMessage::initCDRMsg(&m_heartbeat_response_msg);
                RTPSMessageCreator::addMessageNackFrag(&m_heartbeat_response_msg, mp_WP->mp_SFR->getGuid().guidPrefix,
                        mp_WP->m_att.guid.guidPrefix, mp_WP->mp_SFR->getGuid().entityId, mp_WP->m_att.guid.entityId,
                        cit->sequenceNumber, frag_sns, mp_WP->m_nackfragCount);

                std::vector<Locator_t>::iterator lit;

                for(auto lit = mp_WP->m_att.endpoint.unicastLocatorList.begin();
                        lit!=mp_WP->m_att.endpoint.unicastLocatorList.end();++lit)
                    mp_WP->mp_SFR->getRTPSParticipant()->sendSync(&m_heartbeat_response_msg,(*lit));

                for(auto lit = mp_WP->m_att.endpoint.multicastLocatorList.begin();
                        lit!=mp_WP->m_att.endpoint.multicastLocatorList.end();++lit)
                    mp_WP->mp_SFR->getRTPSParticipant()->sendSync(&m_heartbeat_response_msg,(*lit));
            }
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
