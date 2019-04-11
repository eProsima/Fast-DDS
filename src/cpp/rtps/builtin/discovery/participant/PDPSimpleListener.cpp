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
 * @file PDPSimpleListener.cpp
 *
 */

#include <fastrtps/rtps/builtin/discovery/participant/PDPSimpleListener.h>

#include <fastrtps/rtps/builtin/discovery/participant/timedevent/RemoteParticipantLeaseDuration.h>

#include <fastrtps/rtps/builtin/discovery/participant/PDPSimple.h>
#include "../../../participant/RTPSParticipantImpl.h"

#include <fastrtps/rtps/builtin/discovery/endpoint/EDP.h>
#include <fastrtps/rtps/reader/RTPSReader.h>

#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastrtps/rtps/participant/ParticipantDiscoveryInfo.h>
#include <fastrtps/rtps/participant/RTPSParticipantListener.h>

#include <fastrtps/utils/TimeConversion.h>


#include <mutex>

#include <fastrtps/log/Log.h>

namespace eprosima {
namespace fastrtps{
namespace rtps {



void PDPSimpleListener::onNewCacheChangeAdded(RTPSReader* reader, const CacheChange_t* const change_in)
{
    CacheChange_t* change = (CacheChange_t*)(change_in);
    logInfo(RTPS_PDP,"SPDP Message received");
    if(change->instanceHandle == c_InstanceHandle_Unknown)
    {
        if(!this->getKey(change))
        {
            logWarning(RTPS_PDP,"Problem getting the key of the change, removing");
            this->mp_SPDP->mp_SPDPReaderHistory->remove_change(change);
            return;
        }
    }
    if(change->kind == ALIVE)
    {
        //LOAD INFORMATION IN TEMPORAL RTPSParticipant PROXY DATA
        ParticipantProxyData participant_data;
        CDRMessage_t msg(change->serializedPayload);
        if(participant_data.readFromCDRMessage(&msg))
        {
            //AFTER CORRECTLY READING IT
            //CHECK IF IS THE SAME RTPSParticipant
            change->instanceHandle = participant_data.m_key;
            if(participant_data.m_guid == mp_SPDP->getRTPSParticipant()->getGuid())
            {
                logInfo(RTPS_PDP,"Message from own RTPSParticipant, removing");
                this->mp_SPDP->mp_SPDPReaderHistory->remove_change(change);
                return;
            }

            // At this point we can release reader lock.
            reader->getMutex().unlock();

            //LOOK IF IS AN UPDATED INFORMATION
            ParticipantProxyData* pdata = nullptr;
            std::unique_lock<std::recursive_mutex> lock(*mp_SPDP->getMutex());
            for (auto it = mp_SPDP->m_participantProxies.begin();
                    it != mp_SPDP->m_participantProxies.end();++it)
            {
                if(participant_data.m_key == (*it)->m_key)
                {
                    pdata = (*it);
                    break;
                }
            }

            auto status = (pdata == nullptr) ? ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT :
                ParticipantDiscoveryInfo::CHANGED_QOS_PARTICIPANT;

            if(pdata == nullptr)
            {
                //IF WE DIDNT FOUND IT WE MUST CREATE A NEW ONE
                pdata = new ParticipantProxyData(participant_data);
                pdata->isAlive = true;
                pdata->mp_leaseDurationTimer = new RemoteParticipantLeaseDuration(mp_SPDP,
                        pdata,
                        TimeConv::Time_t2MilliSecondsDouble(pdata->m_leaseDuration));
                pdata->mp_leaseDurationTimer->restart_timer();
                this->mp_SPDP->m_participantProxies.push_back(pdata);
                lock.unlock();

                mp_SPDP->announceParticipantState(false);
                mp_SPDP->assignRemoteEndpoints(&participant_data);
            }
            else
            {
                pdata->updateData(participant_data);
                pdata->isAlive = true;
                lock.unlock();

                if(mp_SPDP->m_discovery.use_STATIC_EndpointDiscoveryProtocol)
                    mp_SPDP->mp_EDP->assignRemoteEndpoints(participant_data);
            }

            auto listener = this->mp_SPDP->getRTPSParticipant()->getListener();
            if (listener != nullptr)
            {
                ParticipantDiscoveryInfo info;
                info.status = status;
                info.info = participant_data;

                listener->onParticipantDiscovery(this->mp_SPDP->getRTPSParticipant()->getUserRTPSParticipant(), std::move(info));
            }

            // Take again the reader lock
            reader->getMutex().lock();
        }
    }
    else
    {
        GUID_t guid;
        iHandle2GUID(guid, change->instanceHandle);

        ParticipantDiscoveryInfo info;
        info.status = ParticipantDiscoveryInfo::REMOVED_PARTICIPANT;

        this->mp_SPDP->lookupParticipantProxyData(guid, info.info);

        if(this->mp_SPDP->removeRemoteParticipant(guid))
        {
            auto listener = this->mp_SPDP->getRTPSParticipant()->getListener();
            if(listener != nullptr)
            {
                listener->onParticipantDiscovery(this->mp_SPDP->getRTPSParticipant()->getUserRTPSParticipant(), std::move(info));
            }
        }
    }

    //Remove change form history.
    this->mp_SPDP->mp_SPDPReaderHistory->remove_change(change);

    return;
}

bool PDPSimpleListener::getKey(CacheChange_t* change)
{
    return ParameterList::readInstanceHandleFromCDRMsg(change, PID_PARTICIPANT_GUID);
}



}
} /* namespace rtps */
} /* namespace eprosima */
