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
#include <fastrtps/rtps/builtin/discovery/endpoint/EDP.h>

#include <fastrtps/rtps/builtin/liveliness/WLP.h>

#include "../participant/RTPSParticipantImpl.h"

#include <fastrtps/log/Log.h>
#include <fastrtps/utils/IPFinder.h>

#include <algorithm>



using namespace eprosima::fastrtps;

namespace eprosima {
namespace fastrtps{
namespace rtps {


BuiltinProtocols::BuiltinProtocols():
    mp_participantImpl(nullptr),
    mp_PDP(nullptr),
    mp_WLP(nullptr)
    {
    }

BuiltinProtocols::~BuiltinProtocols() {
    // Send participant is disposed
    if(mp_PDP != nullptr)
        mp_PDP->announceParticipantState(true, true);
    // TODO Auto-generated destructor stub
    if(mp_WLP!=nullptr)
        delete(mp_WLP);
    if(mp_PDP!=nullptr)
        delete(mp_PDP);
}


bool BuiltinProtocols::initBuiltinProtocols(RTPSParticipantImpl* p_part, BuiltinAttributes& attributes)
{
    mp_participantImpl = p_part;
    m_att = attributes;

    /// Creation of metatraffic locator and receiver resources
    uint32_t metatraffic_multicast_port = p_part->getAttributes().port.getMulticastPort(m_att.domainId);
    uint32_t metatraffic_unicast_port = p_part->getAttributes().port.getUnicastPort(m_att.domainId, p_part->getAttributes().participantID);

    /* If metatrafficMulticastLocatorList is empty, add mandatory default Locators
       Else -> Take them */

    /* INSERT DEFAULT MANDATORY MULTICAST LOCATORS HERE */

    if(m_att.metatrafficMulticastLocatorList.empty() &&
            m_att.metatrafficUnicastLocatorList.empty())
    {
        //UDPv4
        Locator_t mandatoryMulticastLocator;
        mandatoryMulticastLocator.kind = LOCATOR_KIND_UDPv4;
        mandatoryMulticastLocator.port = metatraffic_multicast_port;
        mandatoryMulticastLocator.set_IP4_address(239,255,0,1);
        m_att.metatrafficMulticastLocatorList.push_back(mandatoryMulticastLocator);

        Locator_t default_metatraffic_unicast_locator;
        default_metatraffic_unicast_locator.port = metatraffic_unicast_port;
        m_att.metatrafficUnicastLocatorList.push_back(default_metatraffic_unicast_locator);

    }
    else
    {
        std::for_each(m_att.metatrafficMulticastLocatorList.begin(), m_att.metatrafficMulticastLocatorList.end(),
                [&](Locator_t& locator) {
                    if(locator.port == 0)
                        locator.port = metatraffic_multicast_port;
                });

        std::for_each(m_att.metatrafficUnicastLocatorList.begin(), m_att.metatrafficUnicastLocatorList.end(),
                [&](Locator_t& locator) {
                    if(locator.port == 0)
                        locator.port = metatraffic_unicast_port;
                });
    }

    // Initial peers
    if(m_att.initialPeersList.empty())
    {
        m_att.initialPeersList = m_att.metatrafficMulticastLocatorList;
        /*
        Locator_t loopbackLocator;
        loopbackLocator.set_IP4_address(127, 0, 0, 1);
        loopbackLocator.port = metatraffic_unicast_port;
        m_att.builtin.initialPeersList.push_back(loopbackLocator);
        */
    }
    else
    {
        LocatorList_t initial_peers;
        initial_peers.swap(m_att.initialPeersList);

        std::for_each(initial_peers.begin(), initial_peers.end(),
                [&](Locator_t& locator) {
                    if(locator.port == 0)
                    {
                        // TODO(Ricardo) Make configurable.
                        for(int32_t i = 0; i < 4; ++i)
                        {
                            Locator_t auxloc(locator);
                            auxloc.port = p_part->getAttributes().port.getUnicastPort(m_att.domainId, i);

                            m_att.initialPeersList.push_back(auxloc);
                        }
                    }
                    else
                    {
                        m_att.initialPeersList.push_back(locator);
                    }
                });

        p_part->network_factory().NormalizeLocators(m_att.initialPeersList);
    }


    if(m_att.use_SIMPLE_RTPSParticipantDiscoveryProtocol)
    {
        mp_PDP = new PDPSimple(this);
        if(!mp_PDP->initPDP(mp_participantImpl, m_att))
        {
            logError(RTPS_PDP,"Participant discovery configuration failed");
            return false;
        }
        if(m_att.use_WriterLivelinessProtocol)
        {
            mp_WLP = new WLP(this);
            mp_WLP->initWL(mp_participantImpl);
        }
        mp_PDP->announceParticipantState(true);
        mp_PDP->resetParticipantAnnouncement();
    }

    return true;
}

bool BuiltinProtocols::addLocalWriter(RTPSWriter* w,fastrtps::TopicAttributes& topicAtt,fastrtps::WriterQos& wqos)
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
        ok|= mp_WLP->addLocalWriter(w,wqos);
    }
    else
    {
        logWarning(RTPS_LIVELINESS, "LIVELINESS is not used in this Participant, register a Writer is impossible");
    }
    return ok;
}

bool BuiltinProtocols::addLocalReader(RTPSReader* R,fastrtps::TopicAttributes& topicAtt, fastrtps::ReaderQos& rqos)
{
    bool ok = false;
    if(mp_PDP!=nullptr)
    {
        ok |= mp_PDP->getEDP()->newLocalReaderProxyData(R,topicAtt, rqos);
    }
    else
    {
        logWarning(RTPS_EDP, "EDP is not used in this Participant, register a Reader is impossible");
    }
    return ok;
}

bool BuiltinProtocols::updateLocalWriter(RTPSWriter* W,WriterQos& wqos)
{
    bool ok = false;
    if(mp_PDP!=nullptr && mp_PDP->getEDP()!=nullptr)
    {
        ok |= mp_PDP->getEDP()->updatedLocalWriter(W,wqos);
    }
    if(mp_WLP!=nullptr)
    {
        ok |= mp_WLP->updateLocalWriter(W,wqos);
    }
    return ok;
}

bool BuiltinProtocols::updateLocalReader(RTPSReader* R,ReaderQos& rqos)
{
    bool ok = false;
    if(mp_PDP!=nullptr && mp_PDP->getEDP()!=nullptr)
    {
        ok |= mp_PDP->getEDP()->updatedLocalReader(R,rqos);
    }
    return ok;
}

bool BuiltinProtocols::removeLocalWriter(RTPSWriter* W)
{
    bool ok = false;
    if(mp_WLP !=nullptr)
    {
        ok|= mp_WLP->removeLocalWriter(W);
    }
    if(mp_PDP!=nullptr && mp_PDP->getEDP() != nullptr)
    {
        ok|= mp_PDP->getEDP()->removeLocalWriter(W);
    }
    return ok;
}

bool BuiltinProtocols::removeLocalReader(RTPSReader* R)
{
    bool ok = false;
    if(mp_PDP!=nullptr && mp_PDP->getEDP() != nullptr)
    {
        ok|= mp_PDP->getEDP()->removeLocalReader(R);
    }
    return ok;
}

void BuiltinProtocols::announceRTPSParticipantState()
{
    mp_PDP->announceParticipantState(false);
}

void BuiltinProtocols::stopRTPSParticipantAnnouncement()
{
    mp_PDP->stopParticipantAnnouncement();
}

void BuiltinProtocols::resetRTPSParticipantAnnouncement()
{
    mp_PDP->resetParticipantAnnouncement();
}

}
} /* namespace rtps */
} /* namespace eprosima */
