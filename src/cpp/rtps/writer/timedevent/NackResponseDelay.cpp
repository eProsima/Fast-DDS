/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file NackResponseDelay.cpp
 *
 */

#include <fastrtps/rtps/writer/timedevent/NackResponseDelay.h>
#include <fastrtps/rtps/writer/timedevent/NackSupressionDuration.h>
#include <fastrtps/rtps/resources/ResourceEvent.h>

#include <fastrtps/rtps/writer/StatefulWriter.h>
#include <fastrtps/rtps/writer/ReaderProxy.h>
#include "../../participant/RTPSParticipantImpl.h"

#include <fastrtps/utils/RTPSLog.h>

#include <fastrtps/rtps/messages/RTPSMessageCreator.h>

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

using namespace eprosima::fastrtps::rtps;


static const char* const CLASS_NAME = "NackResponseDelay";

NackResponseDelay::~NackResponseDelay()
{
    destroy();
}

NackResponseDelay::NackResponseDelay(ReaderProxy* p_RP,double millisec):
    TimedEvent(p_RP->mp_SFW->getRTPSParticipant()->getEventResource().getIOService(),
            p_RP->mp_SFW->getRTPSParticipant()->getEventResource().getThread(), millisec),
    mp_RP(p_RP), m_cdrmessages(p_RP->mp_SFW->getTypeMaxSerialized())
{
    CDRMessage::initCDRMsg(&m_cdrmessages.m_rtpsmsg_header);
    RTPSMessageCreator::addHeader(&m_cdrmessages.m_rtpsmsg_header,mp_RP->mp_SFW->getGuid().guidPrefix);
}

void NackResponseDelay::event(EventCode code, const char* msg)
{
    const char* const METHOD_NAME = "event";

    // Unused in release mode.
    (void)msg;

    if(code == EVENT_SUCCESS)
    {
        logInfo(RTPS_WRITER,"Responding to Acknack msg";);
        boost::lock_guard<boost::recursive_mutex> guardW(*mp_RP->mp_SFW->getMutex());
        boost::lock_guard<boost::recursive_mutex> guard(*mp_RP->mp_mutex);
        std::vector<const ChangeForReader_t*> ch_vec = mp_RP->requested_changes_to_underway();

        logInfo(RTPS_WRITER,"Requested "<<ch_vec.size() << " changes";);
        //Get relevant data cache changes
        std::vector<const CacheChange_t*> relevant_changes;
        std::vector<SequenceNumber_t> not_relevant_changes;
        for(auto cit = ch_vec.begin(); cit != ch_vec.end(); ++cit)
        {
            if((*cit)->isRelevant() && (*cit)->isValid())
            {
                relevant_changes.push_back((*cit)->getChange());
            }
            else
            {
                not_relevant_changes.push_back((*cit)->getSequenceNumber());
            }
        }
        mp_RP->m_isRequestedChangesEmpty = true;
        if(!relevant_changes.empty())
            RTPSMessageGroup::send_Changes_AsData(&m_cdrmessages,(RTPSWriter*)mp_RP->mp_SFW,
                    &relevant_changes,
                    mp_RP->m_att.guid.guidPrefix,
                    mp_RP->m_att.guid.entityId,
                    mp_RP->m_att.endpoint.unicastLocatorList,
                    mp_RP->m_att.endpoint.multicastLocatorList,
                    mp_RP->m_att.expectsInlineQos);
        if(!not_relevant_changes.empty())
            RTPSMessageGroup::send_Changes_AsGap(&m_cdrmessages,(RTPSWriter*)mp_RP->mp_SFW,
                    &not_relevant_changes,
                    mp_RP->m_att.guid.guidPrefix,
                    mp_RP->m_att.guid.entityId,
                    &mp_RP->m_att.endpoint.unicastLocatorList,
                    &mp_RP->m_att.endpoint.multicastLocatorList);
        if(relevant_changes.empty() && not_relevant_changes.empty())
        {
            CDRMessage::initCDRMsg(&m_cdrmessages.m_rtpsmsg_fullmsg);
            SequenceNumber_t first = mp_RP->mp_SFW->get_seq_num_min();
            SequenceNumber_t last = mp_RP->mp_SFW->get_seq_num_min();
            if(first != c_SequenceNumber_Unknown && last != c_SequenceNumber_Unknown && last >= first)
            {
                mp_RP->mp_SFW->incrementHBCount();
                RTPSMessageCreator::addMessageHeartbeat(&m_cdrmessages.m_rtpsmsg_fullmsg,mp_RP->mp_SFW->getGuid().guidPrefix,
                        mp_RP->m_att.guid.entityId,mp_RP->mp_SFW->getGuid().entityId,
                        first,last,mp_RP->mp_SFW->getHeartbeatCount(),true,false);
                std::vector<Locator_t>::iterator lit;
                for(lit = mp_RP->m_att.endpoint.unicastLocatorList.begin();lit!=mp_RP->m_att.endpoint.unicastLocatorList.end();++lit)
                    mp_RP->mp_SFW->getRTPSParticipant()->sendSync(&m_cdrmessages.m_rtpsmsg_fullmsg,(*lit));
                for (lit = mp_RP->m_att.endpoint.multicastLocatorList.begin(); lit != mp_RP->m_att.endpoint.multicastLocatorList.end(); ++lit)
                    mp_RP->mp_SFW->getRTPSParticipant()->sendSync(&m_cdrmessages.m_rtpsmsg_fullmsg, (*lit));
            }
        }
        else
            mp_RP->mp_nackSupression->restart_timer();
    }
    else if(code == EVENT_ABORT)
    {
        logInfo(RTPS_WRITER,"Nack response aborted");
    }
    else
    {
        logInfo(RTPS_WRITER,"Nack response boost message: " <<msg);
    }
}
