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
    {
        mp_PDP->announceParticipantState(true, true);
    }

    if(mp_WLP!=nullptr)
    {
        delete(mp_WLP);
    }

    if(mp_PDP!=nullptr)
    {
        delete(mp_PDP);
    }
}


bool BuiltinProtocols::initBuiltinProtocols(RTPSParticipantImpl* p_part, BuiltinAttributes& attributes)
{
    mp_participantImpl = p_part;
    m_att = attributes;
    m_metatrafficUnicastLocatorList = m_att.metatrafficUnicastLocatorList;
    m_metatrafficMulticastLocatorList = m_att.metatrafficMulticastLocatorList;
    m_initialPeersList = m_att.initialPeersList;

    if(m_att.use_SIMPLE_RTPSParticipantDiscoveryProtocol)
    {
        mp_PDP = new PDPSimple(this);
        if(!mp_PDP->initPDP(mp_participantImpl)){
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

bool BuiltinProtocols::updateMetatrafficLocators(LocatorList_t& loclist)
{
    m_metatrafficUnicastLocatorList = loclist;
    return true;
}

bool BuiltinProtocols::addLocalWriter(RTPSWriter* w, const fastrtps::TopicAttributes& topicAtt, const fastrtps::WriterQos& wqos)
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

bool BuiltinProtocols::addLocalReader(RTPSReader* R, const fastrtps::TopicAttributes& topicAtt, const fastrtps::ReaderQos& rqos)
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
    return ok;
}

bool BuiltinProtocols::updateLocalWriter(RTPSWriter* W, const TopicAttributes& topicAtt, const WriterQos& wqos)
{
    bool ok = false;
    if(mp_PDP!=nullptr && mp_PDP->getEDP()!=nullptr)
    {
        ok |= mp_PDP->getEDP()->updatedLocalWriter(W, topicAtt, wqos);
    }
    if(mp_WLP!=nullptr)
    {
        ok |= mp_WLP->updateLocalWriter(W, wqos);
    }
    return ok;
}

bool BuiltinProtocols::updateLocalReader(RTPSReader* R, const TopicAttributes& topicAtt, const ReaderQos& rqos)
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
