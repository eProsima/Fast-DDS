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

#include <mutex>


namespace eprosima {
namespace fastrtps{
namespace rtps {


WLivelinessPeriodicAssertion::WLivelinessPeriodicAssertion(WLP* pwlp,LivelinessQosPolicyKind kind):
    TimedEvent(pwlp->getRTPSParticipant()->getEventResource().getIOService(),
            pwlp->getRTPSParticipant()->getEventResource().getThread(), 0),
    m_livelinessKind(kind), mp_WLP(pwlp)
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

void WLivelinessPeriodicAssertion::event(EventCode code, const char* msg)
{

    // Unused in release mode.
    (void)msg;

    if(code == EVENT_SUCCESS)
    {
        logInfo(RTPS_LIVELINESS,"Period: "<< this->getIntervalMilliSec());
        if(this->mp_WLP->getBuiltinWriter()->getMatchedReadersSize()>0)
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
        logInfo(RTPS_LIVELINESS,"Liveliness Periodic Assertion aborted");
    }
    else
    {
        logInfo(RTPS_LIVELINESS,"Message: " <<msg);
    }
}

bool WLivelinessPeriodicAssertion::AutomaticLivelinessAssertion()
{
    std::lock_guard<std::recursive_mutex> guard(*this->mp_WLP->getBuiltinProtocols()->mp_PDP->getMutex());
    if(this->mp_WLP->m_livAutomaticWriters.size()>0)
    {
        auto writer = this->mp_WLP->getBuiltinWriter();
        auto history = this->mp_WLP->getBuiltinWriterHistory();
        std::lock_guard<std::recursive_timed_mutex> wguard(writer->getMutex());
        CacheChange_t* change=writer->new_change([]() -> uint32_t {return BUILTIN_PARTICIPANT_DATA_MAX_SIZE;}, ALIVE,m_iHandle);
        if(change!=nullptr)
        {
            //change->instanceHandle = m_iHandle;
#if __BIG_ENDIAN__
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_BE;
#else
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_LE;
#endif
            memcpy(change->serializedPayload.data,m_guidP.value,12);
            for(uint8_t i =12;i<24;++i)
                change->serializedPayload.data[i] = 0;
            change->serializedPayload.data[15] = m_livelinessKind+1;
            change->serializedPayload.length = 12+4+4+4;
            if(history->getHistorySize() > 0)
            {
                for(std::vector<CacheChange_t*>::iterator chit = history->changesBegin();
                        chit!=history->changesEnd();++chit)
                {
                    if((*chit)->instanceHandle == change->instanceHandle)
                    {
                        history->remove_change(*chit);
                        break;
                    }
                }
            }
            history->add_change(change);
        }
    }
    return true;
}

bool WLivelinessPeriodicAssertion::ManualByRTPSParticipantLivelinessAssertion()
{
    std::lock_guard<std::recursive_mutex> guard(*this->mp_WLP->getBuiltinProtocols()->mp_PDP->getMutex());
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
        auto writer = this->mp_WLP->getBuiltinWriter();
        auto history = this->mp_WLP->getBuiltinWriterHistory();
        std::lock_guard<std::recursive_timed_mutex> wguard(writer->getMutex());
        CacheChange_t* change=writer->new_change([]() -> uint32_t {return BUILTIN_PARTICIPANT_DATA_MAX_SIZE;}, ALIVE);
        if(change!=nullptr)
        {
            change->instanceHandle = m_iHandle;
#if __BIG_ENDIAN__
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_BE;
#else
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_LE;
#endif
            memcpy(change->serializedPayload.data,m_guidP.value,12);

            for(uint8_t i =12;i<24;++i)
                change->serializedPayload.data[i] = 0;
            change->serializedPayload.data[15] = m_livelinessKind+1;
            change->serializedPayload.length = 12+4+4+4;
            for(auto ch = history->changesBegin();
                    ch!=history->changesEnd();++ch)
            {
                if((*ch)->instanceHandle == change->instanceHandle)
                {
                    history->remove_change(*ch);
                }
            }
            history->add_change(change);
        }
    }
    return false;
}

}
} /* namespace rtps */
} /* namespace eprosima */
