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
#include <fastrtps/rtps/writer/timedevent/PeriodicHeartbeat.h>
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
    mp_RP(p_RP),
    //TODO Put in a macro
    m_cdrmessages(p_RP->mp_SFW->getRTPSParticipant()->getAttributes().sendSocketBufferSize > 65504 ? 65504 : p_RP->mp_SFW->getRTPSParticipant()->getAttributes().sendSocketBufferSize)
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
        mp_RP->convert_status_on_all_changes(REQUESTED,UNSENT);
    }
}
