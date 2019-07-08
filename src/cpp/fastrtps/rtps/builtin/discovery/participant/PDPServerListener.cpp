// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file PDPServerListener.cpp
 *
 */

#include <fastrtps/rtps/reader/RTPSReader.h>

#include <fastrtps/rtps/history/ReaderHistory.h>

#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>

#include <fastrtps/utils/TimeConversion.h>

#include <fastrtps/rtps/builtin/discovery/participant/timedevent/RemoteParticipantLeaseDuration.h>

#include <fastrtps/rtps/participant/ParticipantDiscoveryInfo.h>
#include <fastrtps/rtps/participant/RTPSParticipantListener.h>

#include "../../../participant/RTPSParticipantImpl.h"

#include <mutex>

#include <fastrtps/log/Log.h>

#include <fastrtps/rtps/builtin/discovery/participant/PDPServerListener.h>
#include <fastrtps/rtps/builtin/discovery/participant/PDPServer.h>


namespace eprosima {
namespace fastrtps{
namespace rtps {



void PDPServerListener::on_new_cache_change_added(
    RTPSReader* reader,
    const CacheChange_t* const change_in)
{
    CacheChange_t* change = (CacheChange_t*)(change_in);
    logInfo(RTPS_PDP,"SPDP Message received");

    if(change->instanceHandle == c_InstanceHandle_Unknown)
    {
        if(!this->getKey(change))
        {
            logWarning(RTPS_PDP,"Problem getting the key of the change, removing");
            mp_PDP->mp_PDPReaderHistory->remove_change(change);
            return;
        }
    }

    // update the PDP Writer with this reader info
    if (!mp_PDP->addRelayedChangeToHistory(*change))
    {
        mp_PDP->mp_PDPReaderHistory->remove_change(change);
        return; // already there
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
            if(participant_data.m_guid == mp_PDP->getRTPSParticipant()->getGuid())
            {
                logInfo(RTPS_PDP,"Message from own RTPSParticipant, removing");
                mp_PDP->mp_PDPReaderHistory->remove_change(change);
                return;
            }

            // At this point we can release reader lock.
            reader->getMutex().unlock();

            //LOOK IF IS AN UPDATED INFORMATION
            ParticipantProxyData* pdata = nullptr;
            std::unique_lock<std::recursive_mutex> lock(*mp_PDP->getMutex());
            for (auto it = mp_PDP->m_participantProxies.begin();
                    it != mp_PDP->m_participantProxies.end();++it)
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
                pdata = mp_PDP->createParticipantProxyData(participant_data, *change);

                lock.unlock();

                // Dismiss any client data relayed by a server
                if (pdata->m_guid.guidPrefix == change->writerGUID.guidPrefix )
                {
                    // This call would be needed again if the clients known not the server prefix
                    //  mp_PDP->announceParticipantState(false);
                    mp_PDP->assignRemoteEndpoints(pdata);
                    mp_PDP->queueParticipantForEDPMatch(pdata);
                }
            }
            else
            {
                pdata->updateData(participant_data);
                pdata->isAlive = true;
                lock.unlock();

                // Included for symmetry with PDPListener to profit from a future updateInfoMatchesEDP override
                // right now servers doesn't need to modify EDP on updates
                if (mp_PDP->updateInfoMatchesEDP())
                {
                    mp_PDP->mp_EDP->assignRemoteEndpoints(*pdata);
                }

            }

            RTPSParticipantListener* listener = mp_PDP->getRTPSParticipant()->getListener();
            if (listener != nullptr)
            {
                ParticipantDiscoveryInfo info;
                info.status = status;
                info.info = participant_data;

                listener->onParticipantDiscovery(mp_PDP->getRTPSParticipant()->getUserRTPSParticipant(),
                    std::move(info));
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

        if (!mp_PDP->lookupParticipantProxyData(guid, info.info))
        {
            logWarning(RTPS_PDP, "PDPServerListener received DATA(p) NOT_ALIVE_DISPOSED from unknown participant");
            mp_PDP->mp_PDPReaderHistory->remove_change(change);
            return;
        }

        std::unique_ptr<PDPServer::InPDPCallback> guard = mp_PDP->signalCallback();

        if(mp_PDP->removeRemoteParticipant(guid))
        {
            auto listener = this->mp_PDP->getRTPSParticipant()->getListener();
            if(listener != nullptr)
            {
                listener->onParticipantDiscovery(mp_PDP->getRTPSParticipant()->getUserRTPSParticipant(),
                    std::move(info));
            }

            return; // all changes related with this participant have been removed from history by removeRemoteParticipant
        }
    }

    //Remove change form history.
    mp_PDP->mp_PDPReaderHistory->remove_change(change);

    return;
}

bool PDPServerListener::getKey(CacheChange_t* change)
{
    return ParameterList::readInstanceHandleFromCDRMsg(change, PID_PARTICIPANT_GUID);
}

}
} /* namespace rtps */
} /* namespace eprosima */
