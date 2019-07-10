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
 * @file BuiltinProtocols.cpp
 *
 */

#include <fastrtps/rtps/builtin/BuiltinProtocols.h>
#include <fastrtps/rtps/common/Locator.h>

#include <fastrtps/rtps/builtin/discovery/participant/PDPSimple.h>
#include <fastrtps/rtps/builtin/discovery/participant/PDPClient.h>
#include <fastrtps/rtps/builtin/discovery/participant/PDPServer.h>
#include <fastrtps/rtps/builtin/discovery/endpoint/EDP.h>
#include <fastrtps/rtps/builtin/discovery/endpoint/EDPStatic.h>

#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>

#include <fastrtps/rtps/builtin/liveliness/WLP.h>

#include "../participant/RTPSParticipantImpl.h"

#include <fastrtps/log/Log.h>
#include <fastrtps/utils/IPFinder.h>

#include <algorithm>



using namespace eprosima::fastrtps;

namespace eprosima {
namespace fastrtps{
namespace rtps {


BuiltinProtocols::BuiltinProtocols()
    : mp_participantImpl(nullptr)
    , mp_PDP(nullptr)
    , mp_WLP(nullptr)
    {
    }

BuiltinProtocols::~BuiltinProtocols()
{
    // Send participant is disposed
    if(mp_PDP != nullptr)
    {
        mp_PDP->announceParticipantState(true, true);
    }

    // TODO Auto-generated destructor stub
    delete(mp_WLP);
    delete(mp_PDP);

}


bool BuiltinProtocols::initBuiltinProtocols(
    RTPSParticipantImpl* p_part,
    BuiltinAttributes& attributes)
{
    mp_participantImpl = p_part;
    m_att = attributes;
    m_metatrafficUnicastLocatorList = m_att.metatrafficUnicastLocatorList;
    m_metatrafficMulticastLocatorList = m_att.metatrafficMulticastLocatorList;
    m_initialPeersList = m_att.initialPeersList;
    m_DiscoveryServers = m_att.discovery_config.m_DiscoveryServers;

    // PDP
    switch (m_att.discovery_config.discoveryProtocol)
    {
        case DiscoveryProtocol_t::NONE:
            logWarning(RTPS_PDP, "No participant discovery protocol specified");
            return false;

        case DiscoveryProtocol_t::SIMPLE:
            mp_PDP = new PDPSimple(this);
            break;

        case DiscoveryProtocol_t::EXTERNAL:
            logError(RTPS_PDP, "Flag only present for debugging purposes");
            break;

        case DiscoveryProtocol_t::CLIENT:
            mp_PDP = new PDPClient(this);
            break;

        case DiscoveryProtocol_t::SERVER:
            mp_PDP = new PDPServer(this, DurabilityKind_t::TRANSIENT_LOCAL);
            break;

        case DiscoveryProtocol_t::BACKUP:
            mp_PDP = new PDPServer(this, DurabilityKind_t::TRANSIENT);
            break;
    }

    if (!mp_PDP->initPDP(mp_participantImpl)) {
        logError(RTPS_PDP, "Participant discovery configuration failed");
        return false;
    }

    // WLP
    if (m_att.use_WriterLivelinessProtocol)
    {
        mp_WLP = new WLP(this);
        mp_WLP->initWL(mp_participantImpl);
    }

    mp_PDP->announceParticipantState(true);
    mp_PDP->resetParticipantAnnouncement();

    return true;
}

bool BuiltinProtocols::updateMetatrafficLocators(LocatorList_t& loclist)
{
    m_metatrafficUnicastLocatorList = loclist;
    return true;
}

bool BuiltinProtocols::addLocalWriter(
    RTPSWriter* w,
    const fastrtps::TopicAttributes& topicAtt,
    const fastrtps::WriterQos& wqos)
{
    bool ok = false;
    if(mp_PDP!=nullptr)
    {
        ok |= mp_PDP->getEDP()->newLocalWriterProxyData(w,topicAtt,wqos);
    }
    else
    {
        logWarning(RTPS_EDP, "EDP is not used in this Participant, register a Writer is impossible");
    }
    if(mp_WLP !=nullptr)
    {
        ok|= mp_WLP->add_local_writer(w,wqos);
    }
    else
    {
        logWarning(RTPS_LIVELINESS, "LIVELINESS is not used in this Participant, register a Writer is impossible");
    }
    return ok;
}

bool BuiltinProtocols::addLocalReader(
    RTPSReader* R,
    const fastrtps::TopicAttributes& topicAtt,
    const fastrtps::ReaderQos& rqos)
{
    bool ok = false;
    if(mp_PDP!=nullptr)
    {
        ok |= mp_PDP->getEDP()->newLocalReaderProxyData(R, topicAtt, rqos);
    }
    else
    {
        logWarning(RTPS_EDP, "EDP is not used in this Participant, register a Reader is impossible");
    }
    if (mp_WLP != nullptr)
    {
        ok|= mp_WLP->add_local_reader(R, rqos);
    }
    return ok;
}

bool BuiltinProtocols::updateLocalWriter(
    RTPSWriter* W,
    const TopicAttributes& topicAtt,
    const WriterQos& wqos)
{
    bool ok = false;
    if(mp_PDP!=nullptr && mp_PDP->getEDP()!=nullptr)
    {
        ok |= mp_PDP->getEDP()->updatedLocalWriter(W, topicAtt, wqos);
    }
    return ok;
}

bool BuiltinProtocols::updateLocalReader(
    RTPSReader* R,
    const TopicAttributes& topicAtt,
    const ReaderQos& rqos)
{
    bool ok = false;
    if(mp_PDP!=nullptr && mp_PDP->getEDP()!=nullptr)
    {
        ok |= mp_PDP->getEDP()->updatedLocalReader(R, topicAtt, rqos);
    }
    return ok;
}

bool BuiltinProtocols::removeLocalWriter(RTPSWriter* W)
{
    bool ok = false;
    if(mp_WLP !=nullptr)
    {
        ok |= mp_WLP->remove_local_writer(W);
    }
    if(mp_PDP!=nullptr && mp_PDP->getEDP() != nullptr)
    {
        ok |= mp_PDP->getEDP()->removeLocalWriter(W);
    }
    return ok;
}

bool BuiltinProtocols::removeLocalReader(RTPSReader* R)
{
    bool ok = false;
    if (mp_WLP != nullptr)
    {
        ok |= mp_WLP->remove_local_reader(R);
    }
    if(mp_PDP!=nullptr && mp_PDP->getEDP() != nullptr)
    {
        ok |= mp_PDP->getEDP()->removeLocalReader(R);
    }
    return ok;
}

void BuiltinProtocols::announceRTPSParticipantState()
{
    assert(mp_PDP);

    if (mp_PDP)
    {
        mp_PDP->announceParticipantState(false);
    }
    else
    {
        logError(RTPS_EDP, "Trying to use BuiltinProtocols interfaces before initBuiltinProtocols call");
    }
}

void BuiltinProtocols::stopRTPSParticipantAnnouncement()
{
    assert(mp_PDP);

    if (mp_PDP)
    {
        mp_PDP->stopParticipantAnnouncement();
    }
    else
    {
        logError(RTPS_EDP, "Trying to use BuiltinProtocols interfaces before initBuiltinProtocols call");
    }
}

void BuiltinProtocols::resetRTPSParticipantAnnouncement()
{
    assert(mp_PDP);

    if (mp_PDP)
    {
        mp_PDP->resetParticipantAnnouncement();
    }
    else
    {
        logError(RTPS_EDP, "Trying to use BuiltinProtocols interfaces before initBuiltinProtocols call");
    }
}

}
} /* namespace rtps */
} /* namespace eprosima */
