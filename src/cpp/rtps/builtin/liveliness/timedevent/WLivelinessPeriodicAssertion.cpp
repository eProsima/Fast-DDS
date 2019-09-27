// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file WLivelinessPeriodicAssertion.cpp
 *
 */

#include <fastrtps/rtps/builtin/liveliness/timedevent/WLivelinessPeriodicAssertion.h>
#include <fastrtps/rtps/resources/ResourceEvent.h>
#include <fastrtps/rtps/builtin/liveliness/WLP.h>
#include "../../../participant/RTPSParticipantImpl.h"

#include <fastrtps/rtps/writer/StatefulWriter.h>
#include <fastrtps/rtps/history/WriterHistory.h>
#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>

#include <fastrtps/log/Log.h>
#include <fastrtps/utils/eClock.h>

#include <fastrtps/rtps/builtin/discovery/participant/PDPSimple.h>
#include <fastrtps/rtps/builtin/BuiltinProtocols.h>
#include <fastrtps/rtps/writer/LivelinessManager.h>

#include <mutex>


namespace eprosima {
namespace fastrtps{
namespace rtps {


WLivelinessPeriodicAssertion::WLivelinessPeriodicAssertion(
        WLP* pwlp,
        LivelinessQosPolicyKind kind)
    : TimedEvent(
        pwlp->getRTPSParticipant()->getEventResource().getIOService(),
        pwlp->getRTPSParticipant()->getEventResource().getThread(),
        0)
    , m_livelinessKind(kind)
    , mp_WLP(pwlp)
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
    destroy();
}

void WLivelinessPeriodicAssertion::event(
        EventCode code,
        const char* msg)
{

    // Unused in release mode.
    (void)msg;

    if(code == EVENT_SUCCESS)
    {
        logInfo(RTPS_LIVELINESS,"Period: "<< this->getIntervalMilliSec());
        if(this->mp_WLP->getBuiltinWriter()->getMatchedReadersSize()>0)
        {
            if(m_livelinessKind == AUTOMATIC_LIVELINESS_QOS)
            {
                automatic_liveliness_assertion();
            }
            else if(m_livelinessKind == MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            {
                manual_by_participant_liveliness_assertion();
            }
        }
        this->restart_timer();
    }
    else if(code == EVENT_ABORT)
    {
        logInfo(RTPS_LIVELINESS,"Liveliness Periodic Assertion aborted");
    }
    else
    {
        logInfo(RTPS_LIVELINESS,"Message: " <<msg);
    }
}

bool WLivelinessPeriodicAssertion::automatic_liveliness_assertion()
{
    std::lock_guard<std::recursive_mutex> guard(*this->mp_WLP->getBuiltinProtocols()->mp_PDP->getMutex());

    if(this->mp_WLP->automatic_writers_.size() > 0)
    {
        return add_cache_change();
    }
    return true;
}

bool WLivelinessPeriodicAssertion::manual_by_participant_liveliness_assertion()
{
    std::lock_guard<std::recursive_mutex> guard(*this->mp_WLP->getBuiltinProtocols()->mp_PDP->getMutex());

    if (mp_WLP->manual_by_participant_writers_.size() > 0)
    {
        // Liveliness was asserted for at least one of the writers using MANUAL_BY_PARTICIPANT
        if(mp_WLP->pub_liveliness_manager_->is_any_alive(MANUAL_BY_PARTICIPANT_LIVELINESS_QOS))
        {
            return add_cache_change();
        }
    }
    return false;
}

bool WLivelinessPeriodicAssertion::add_cache_change()
{
    StatefulWriter* writer = mp_WLP->getBuiltinWriter();
    WriterHistory* history = mp_WLP->getBuiltinWriterHistory();

    std::lock_guard<std::recursive_timed_mutex> wguard(writer->getMutex());

    CacheChange_t* change = writer->new_change(
            []() -> uint32_t { return BUILTIN_PARTICIPANT_DATA_MAX_SIZE; },
            ALIVE,
            m_iHandle);

    if (change != nullptr)
    {
        change->serializedPayload.data[0] = 0;
#if __BIG_ENDIAN__
        change->serializedPayload.encapsulation = (uint16_t)PL_CDR_BE;
        change->serializedPayload.data[1] = PL_CDR_BE;
#else
        change->serializedPayload.encapsulation = (uint16_t)PL_CDR_LE;
        change->serializedPayload.data[1] = PL_CDR_LE;
#endif
        change->serializedPayload.data[2] = 0;
        change->serializedPayload.data[3] = 0;

        memcpy(change->serializedPayload.data + 4, m_iHandle.value, 16);

        for (size_t i = 20; i < 28; ++i)
        {
            change->serializedPayload.data[i] = 0;
        }
        change->serializedPayload.length = 4 + 12 + 4 + 4 + 4;

        if (history->getHistorySize() > 0)
        {
            for (auto chit = history->changesBegin(); chit != history->changesEnd(); ++chit)
            {
                if ((*chit)->instanceHandle == change->instanceHandle)
                {
                    history->remove_change(*chit);
                    break;
                }
            }
        }
        history->add_change(change);
        return true;
    }
    return false;
}

}
} /* namespace rtps */
} /* namespace eprosima */
