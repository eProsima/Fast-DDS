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
 * @file PDPSimple.cpp
 *
 */

#include <fastrtps/rtps/builtin/discovery/participant/PDPSimple.h>
#include <fastrtps/rtps/builtin/discovery/participant/PDPListener.h>
#include <fastrtps/rtps/builtin/discovery/endpoint/EDPSimple.h>
#include <fastrtps/rtps/builtin/discovery/endpoint/EDPStatic.h>
#include <fastrtps/rtps/builtin/discovery/participant/timedevent/ResendParticipantProxyDataPeriod.h>
#include <fastrtps/rtps/builtin/discovery/participant/timedevent/RemoteParticipantLeaseDuration.h>
#include <fastrtps/rtps/builtin/BuiltinProtocols.h>
#include <fastrtps/rtps/builtin/liveliness/WLP.h>
#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>
#include <fastrtps/rtps/builtin/data/ReaderProxyData.h>
#include <fastrtps/rtps/builtin/data/WriterProxyData.h>

#include <fastrtps/rtps/participant/RTPSParticipantListener.h>
#include <fastrtps/rtps/writer/StatelessWriter.h>
#include <fastrtps/rtps/resources/AsyncWriterThread.h>

#include <fastrtps/rtps/reader/StatelessReader.h>
#include <fastrtps/rtps/reader/StatefulReader.h>
#include <fastrtps/rtps/reader/WriterProxy.h>

#include <fastrtps/rtps/history/WriterHistory.h>
#include <fastrtps/rtps/history/ReaderHistory.h>

#include <fastrtps/utils/TimeConversion.h>
#include <fastrtps/utils/IPLocator.h>

#include "../../../participant/RTPSParticipantImpl.h"

#include <fastrtps/log/Log.h>

#include <mutex>

using namespace eprosima::fastrtps;

namespace eprosima {
namespace fastrtps{
namespace rtps {


PDPSimple::PDPSimple(BuiltinProtocols* built):
    PDP(built),
    mp_resendParticipantTimer(nullptr)
    {

    }

PDPSimple::~PDPSimple()
{
    if(mp_resendParticipantTimer != nullptr)
        delete(mp_resendParticipantTimer);
}

void PDPSimple::initializeParticipantProxyData(ParticipantProxyData* participant_data)
{
    PDP::initializeParticipantProxyData(participant_data);

    if(!getRTPSParticipant()->getAttributes().builtin.use_SIMPLE_EndpointDiscoveryProtocol)
    {
        logError(RTPS_PDP, "Using a PDP Simple object with another user's settings");
    }

    if (getRTPSParticipant()->getAttributes().builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader)
    {
        participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER;
        participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR;
    }

    if (getRTPSParticipant()->getAttributes().builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter)
    {
        participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR;
        participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER;
    }

#if HAVE_SECURITY
    if (getRTPSParticipant()->getAttributes().builtin.m_simpleEDP.enable_builtin_secure_publications_writer_and_subscriptions_reader)
    {
        participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_ANNOUNCER;
        participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_DETECTOR;
    }

    if (getRTPSParticipant()->getAttributes().builtin.m_simpleEDP.enable_builtin_secure_subscriptions_writer_and_publications_reader)
    {
        participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_ANNOUNCER;
        participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_DETECTOR;
    }
#endif

}

bool PDPSimple::initPDP(RTPSParticipantImpl* part)
{
    if (!PDP::initPDP(part))
    {
        return false;
    }

    //INIT EDP
    if(m_discovery.use_STATIC_EndpointDiscoveryProtocol)
    {
        mp_EDP = (EDP*)(new EDPStatic(this,mp_RTPSParticipant));
        if(!mp_EDP->initEDP(m_discovery))
        {
            logError(RTPS_PDP,"Endpoint discovery configuration failed");
            return false;
        }

    }
    else if(m_discovery.use_SIMPLE_EndpointDiscoveryProtocol)
    {
        mp_EDP = (EDP*)(new EDPSimple(this,mp_RTPSParticipant));
        if(!mp_EDP->initEDP(m_discovery))
        {
            logError(RTPS_PDP,"Endpoint discovery configuration failed");
            return false;
        }
    }
    else
    {
        logWarning(RTPS_PDP,"No EndpointDiscoveryProtocol defined");
        return false;
    }

    mp_resendParticipantTimer = new ResendParticipantProxyDataPeriod(this,TimeConv::Time_t2MilliSecondsDouble(m_discovery.leaseDuration_announcementperiod));

    return true;
}

// EDPStatic requires matching on ParticipantProxyData property updates
bool PDPSimple::updateInfoMatchesEDP()
{
    return dynamic_cast<EDPStatic*>(mp_EDP) != nullptr;
}

void PDPSimple::announceParticipantState(bool new_change, bool dispose)
{
    PDP::announceParticipantState(new_change, dispose);

    if (!(dispose || new_change))
    {
        StatelessWriter * pW = dynamic_cast<StatelessWriter*>(mp_PDPWriter);

        if (pW != nullptr)
        {
            pW->unsent_changes_reset();
        }
        else
        {
            logError(RTPS_PDP, "Using PDPSimple protocol with a reliable writer");
        }
    }
}

void PDPSimple::stopParticipantAnnouncement()
{
    mp_resendParticipantTimer->cancel_timer();
}

void PDPSimple::resetParticipantAnnouncement()
{
    mp_resendParticipantTimer->restart_timer();
}


bool PDPSimple::createPDPEndpoints()
{
    logInfo(RTPS_PDP,"Beginning");
    //SPDP BUILTIN RTPSParticipant WRITER
    HistoryAttributes hatt;
    hatt.payloadMaxSize = DISCOVERY_PARTICIPANT_DATA_MAX_SIZE;
    hatt.initialReservedCaches = 25;
    hatt.memoryPolicy = mp_builtin->m_att.readerHistoryMemoryPolicy;
    mp_PDPReaderHistory = new ReaderHistory(hatt);
    ReaderAttributes ratt;
    ratt.endpoint.multicastLocatorList = mp_builtin->m_metatrafficMulticastLocatorList;
    ratt.endpoint.unicastLocatorList = mp_builtin->m_metatrafficUnicastLocatorList;
    ratt.endpoint.topicKind = WITH_KEY;
    ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
    ratt.endpoint.reliabilityKind = BEST_EFFORT;
    mp_listener = new PDPListener(this);
    StatelessReader* rout;
    if (mp_RTPSParticipant->createReader(&mp_PDPReader, ratt, mp_PDPReaderHistory, mp_listener, c_EntityId_SPDPReader, true, false))
    {
#if HAVE_SECURITY
        mp_RTPSParticipant->set_endpoint_rtps_protection_supports(rout, false);
#endif
        rout = dynamic_cast<StatelessReader*>(mp_PDPReader);
        if (rout != nullptr)
        {
            RemoteWriterAttributes rwatt;
            rwatt.endpoint.remoteLocatorList = mp_builtin->m_initialPeersList;
            rwatt.endpoint.topicKind = WITH_KEY;
            rwatt.endpoint.durabilityKind = TRANSIENT_LOCAL;
            rwatt.endpoint.reliabilityKind = BEST_EFFORT;
            rout->matched_writer_add(rwatt);
        }
    }
    else
    {
        logError(RTPS_PDP, "SimplePDP Reader creation failed");
        delete(mp_PDPReaderHistory);
        mp_PDPReaderHistory = nullptr;
        delete(mp_listener);
        mp_listener = nullptr;
        return false;
    }

    hatt.payloadMaxSize = DISCOVERY_PARTICIPANT_DATA_MAX_SIZE;
    hatt.initialReservedCaches = 20;
    hatt.memoryPolicy = mp_builtin->m_att.writerHistoryMemoryPolicy;
    mp_PDPWriterHistory = new WriterHistory(hatt);
    WriterAttributes watt;
    watt.endpoint.endpointKind = WRITER;
    watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
    watt.endpoint.reliabilityKind = BEST_EFFORT;
    watt.endpoint.topicKind = WITH_KEY;
    watt.endpoint.remoteLocatorList = m_discovery.initialPeersList;

    if (mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.bytesPerPeriod != UINT32_MAX &&
        mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.periodMillisecs != 0)
    {
        watt.mode = ASYNCHRONOUS_WRITER;
    }

    StatelessWriter* wout;
    if (mp_RTPSParticipant->createWriter(&mp_PDPWriter, watt, mp_PDPWriterHistory, nullptr, c_EntityId_SPDPWriter, true))
    {
#if HAVE_SECURITY
        mp_RTPSParticipant->set_endpoint_rtps_protection_supports(wout, false);
#endif
        wout = dynamic_cast<StatelessWriter*>(mp_PDPWriter);
        if (wout != nullptr)
        {
            for (LocatorListIterator lit = mp_builtin->m_initialPeersList.begin();
                lit != mp_builtin->m_initialPeersList.end(); ++lit)
            {
                wout->add_locator(*lit);
            }

            RemoteReaderAttributes rratt;
            for (auto it = mp_builtin->m_initialPeersList.begin(); it != mp_builtin->m_initialPeersList.end(); ++it)
            {
                if (IPLocator::isMulticast(*it))
                {
                    rratt.endpoint.multicastLocatorList.push_back(*it);
                }
                else
                {
                    rratt.endpoint.unicastLocatorList.push_back(*it);
                }
            }
            rratt.endpoint.topicKind = WITH_KEY;
            rratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
            rratt.endpoint.reliabilityKind = BEST_EFFORT;
            wout->matched_reader_add(rratt);
        }
    }
    else
    {
        logError(RTPS_PDP, "SimplePDP Writer creation failed");
        delete(mp_PDPWriterHistory);
        mp_PDPWriterHistory = nullptr;
        return false;
    }
    logInfo(RTPS_PDP,"SPDP Endpoints creation finished");
    return true;
}

void PDPSimple::assignRemoteEndpoints(ParticipantProxyData* pdata)
{
    logInfo(RTPS_PDP,"For RTPSParticipant: "<<pdata->m_guid.guidPrefix);
    uint32_t endp = pdata->m_availableBuiltinEndpoints;
    uint32_t auxendp = endp;
    auxendp &=DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER;
    if(auxendp!=0)
    {
        RemoteWriterAttributes watt(pdata->m_VendorId);
        watt.guid.guidPrefix = pdata->m_guid.guidPrefix;
        watt.guid.entityId = c_EntityId_SPDPWriter;
        watt.endpoint.persistence_guid = watt.guid;
        watt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
        watt.endpoint.reliabilityKind = BEST_EFFORT;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        mp_PDPReader->matched_writer_add(watt);
    }
    auxendp = endp;
    auxendp &=DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;
    if(auxendp!=0)
    {
        RemoteReaderAttributes ratt(pdata->m_VendorId);
        ratt.expectsInlineQos = false;
        ratt.guid.guidPrefix = pdata->m_guid.guidPrefix;
        ratt.guid.entityId = c_EntityId_SPDPReader;
        ratt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
        ratt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
        ratt.endpoint.reliabilityKind = BEST_EFFORT;
        ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        mp_PDPWriter->matched_reader_add(ratt);
    }

#if HAVE_SECURITY
    // Validate remote participant
    mp_RTPSParticipant->security_manager().discovered_participant(*pdata);
#else
    //Inform EDP of new RTPSParticipant data:
    notifyAboveRemoteEndpoints(*pdata);
#endif
}


void PDPSimple::removeRemoteEndpoints(ParticipantProxyData* pdata)
{
    logInfo(RTPS_PDP,"For RTPSParticipant: "<<pdata->m_guid);
    uint32_t endp = pdata->m_availableBuiltinEndpoints;
    uint32_t auxendp = endp;
    auxendp &=DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER;
    if(auxendp!=0)
    {
        RemoteWriterAttributes watt;
        watt.guid.guidPrefix = pdata->m_guid.guidPrefix;
        watt.guid.entityId = c_EntityId_SPDPWriter;
        watt.endpoint.persistence_guid = watt.guid;
        watt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
        watt.endpoint.reliabilityKind = BEST_EFFORT;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        mp_PDPReader->matched_writer_remove(watt);
    }
    auxendp = endp;
    auxendp &=DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;
    if(auxendp!=0)
    {
        RemoteReaderAttributes ratt;
        ratt.expectsInlineQos = false;
        ratt.guid.guidPrefix = pdata->m_guid.guidPrefix;
        ratt.guid.entityId = c_EntityId_SPDPReader;
        ratt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
        ratt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
        ratt.endpoint.reliabilityKind = BEST_EFFORT;
        ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        mp_PDPWriter->matched_reader_remove(ratt);
    }
}

void PDPSimple::notifyAboveRemoteEndpoints(const ParticipantProxyData& pdata)
{
    //Inform EDP of new RTPSParticipant data:
    if (mp_EDP != nullptr)
        mp_EDP->assignRemoteEndpoints(pdata);
    if (mp_builtin->mp_WLP != nullptr)
        mp_builtin->mp_WLP->assignRemoteEndpoints(pdata);
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
