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

#include <rtps/builtin/BuiltinProtocols.h>

#include <algorithm>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/utils/IPFinder.hpp>

#include <fastdds/builtin/type_lookup_service/TypeLookupManager.hpp>
#include <fastdds/utils/TypePropagation.hpp>
#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/builtin/discovery/endpoint/EDP.h>
#include <rtps/builtin/discovery/endpoint/EDPStatic.h>
#include <rtps/builtin/discovery/participant/PDPClient.h>
#include <rtps/builtin/discovery/participant/PDPServer.hpp>
#include <rtps/builtin/discovery/participant/PDPSimple.h>
#include <rtps/builtin/liveliness/WLP.hpp>
#include <rtps/participant/RTPSParticipantImpl.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {


BuiltinProtocols::BuiltinProtocols()
    : mp_participantImpl(nullptr)
    , mp_PDP(nullptr)
    , mp_WLP(nullptr)
    , typelookup_manager_(nullptr)
{
}

BuiltinProtocols::~BuiltinProtocols()
{
    if (nullptr != mp_PDP)
    {
        // Send participant is disposed
        mp_PDP->announceParticipantState(true, true);
        // Consider all discovered participants as disposed
        mp_PDP->disable();
    }

    // The type lookup manager should be deleted first, since it will access the PDP database
    if (nullptr != typelookup_manager_)
    {
        delete typelookup_manager_;
    }

    delete mp_WLP;
    delete mp_PDP;
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

    {
        std::unique_lock<eprosima::shared_mutex> disc_lock(getDiscoveryMutex());
        m_DiscoveryServers = m_att.discovery_config.m_DiscoveryServers;
    }

    filter_server_remote_locators(p_part->network_factory());

    const RTPSParticipantAllocationAttributes& allocation = p_part->get_attributes().allocation;

    // PDP
    switch (m_att.discovery_config.discoveryProtocol)
    {
        case DiscoveryProtocol::NONE:
            EPROSIMA_LOG_WARNING(RTPS_PDP, "No participant discovery protocol specified");
            return true;

        case DiscoveryProtocol::SIMPLE:
            mp_PDP = new PDPSimple(this, allocation);
            break;

        case DiscoveryProtocol::EXTERNAL:
            EPROSIMA_LOG_ERROR(RTPS_PDP, "Flag only present for debugging purposes");
            return false;

        case DiscoveryProtocol::CLIENT:
            mp_PDP = new fastdds::rtps::PDPClient(this, allocation);
            break;

        case DiscoveryProtocol::SERVER:
            mp_PDP = new fastdds::rtps::PDPServer(this, allocation, DurabilityKind_t::TRANSIENT_LOCAL);
            break;

#if HAVE_SQLITE3
        case DiscoveryProtocol::BACKUP:
            EPROSIMA_LOG_WARNING(RTPS_PDP, "BACKUP discovery protocol is not yet supported with XTypes.");
            mp_PDP = new fastdds::rtps::PDPServer(this, allocation, DurabilityKind_t::TRANSIENT);
            break;
#endif // if HAVE_SQLITE3

        case DiscoveryProtocol::SUPER_CLIENT:
            mp_PDP = new fastdds::rtps::PDPClient(this, allocation, true);
            break;

        default:
            EPROSIMA_LOG_ERROR(RTPS_PDP, "Unknown DiscoveryProtocol specified.");
            return false;
    }

    if (!mp_PDP->init(mp_participantImpl))
    {
        EPROSIMA_LOG_ERROR(RTPS_PDP, "Participant discovery configuration failed");
        delete mp_PDP;
        mp_PDP = nullptr;
        return false;
    }

    // WLP
    if (m_att.use_WriterLivelinessProtocol)
    {
        mp_WLP = new WLP(this);
        mp_WLP->initWL(mp_participantImpl);
    }

    // TypeLookupManager
    auto type_propagation = p_part->type_propagation();
    bool should_create_typelookup =
            (dds::utils::TypePropagation::TYPEPROPAGATION_ENABLED == type_propagation) ||
            (dds::utils::TypePropagation::TYPEPROPAGATION_MINIMAL_BANDWIDTH == type_propagation);

    if (should_create_typelookup)
    {
        typelookup_manager_ = new fastdds::dds::builtin::TypeLookupManager();
        typelookup_manager_->init(this);
    }

    return true;
}

void BuiltinProtocols::enable()
{
    if (nullptr != mp_PDP)
    {
        mp_PDP->enable();
        mp_PDP->announceParticipantState(true);
        mp_PDP->resetParticipantAnnouncement();
    }
}

bool BuiltinProtocols::updateMetatrafficLocators(
        LocatorList_t& loclist)
{
    m_metatrafficUnicastLocatorList = loclist;
    return true;
}

void BuiltinProtocols::filter_server_remote_locators(
        NetworkFactory& nf)
{
    eprosima::shared_lock<eprosima::shared_mutex> disc_lock(getDiscoveryMutex());

    LocatorList_t allowed_locators;

    for (auto loc : m_DiscoveryServers)
    {
        if (nf.is_locator_remote_or_allowed(loc))
        {
            allowed_locators.push_back(loc);
        }
        else
        {
            EPROSIMA_LOG_WARNING(RTPS_PDP, "Ignoring remote server locator " << loc << " : not allowed.");
        }
    }
    m_DiscoveryServers.swap(allowed_locators);
}

bool BuiltinProtocols::add_writer(
        RTPSWriter* rtps_writer,
        const TopicDescription& topic,
        const fastdds::dds::WriterQos& qos)
{
    bool ok = true;

    if (nullptr != mp_PDP)
    {
        ok = mp_PDP->get_edp()->new_writer_proxy_data(rtps_writer, topic, qos);

        if (!ok)
        {
            EPROSIMA_LOG_WARNING(RTPS_EDP, "Failed register WriterProxyData in EDP");
            return false;
        }
    }
    else
    {
        EPROSIMA_LOG_WARNING(RTPS_EDP, "EDP is not used in this Participant, register a Writer is impossible");
    }

    if (nullptr != mp_WLP)
    {
        ok &= mp_WLP->add_local_writer(rtps_writer, qos.m_liveliness);
    }
    else
    {
        EPROSIMA_LOG_WARNING(RTPS_LIVELINESS,
                "LIVELINESS is not used in this Participant, register a Writer is impossible");
    }
    return ok;
}

bool BuiltinProtocols::add_reader(
        RTPSReader* rtps_reader,
        const TopicDescription& topic,
        const fastdds::dds::ReaderQos& qos,
        const fastdds::rtps::ContentFilterProperty* content_filter)
{
    bool ok = true;

    if (nullptr != mp_PDP)
    {
        ok = mp_PDP->get_edp()->new_reader_proxy_data(rtps_reader, topic, qos, content_filter);

        if (!ok)
        {
            EPROSIMA_LOG_WARNING(RTPS_EDP, "Failed register ReaderProxyData in EDP");
            return false;
        }
    }
    else
    {
        EPROSIMA_LOG_WARNING(RTPS_EDP, "EDP is not used in this Participant, register a Reader is impossible");
    }

    if (nullptr != mp_WLP)
    {
        ok &= mp_WLP->add_local_reader(rtps_reader, qos.m_liveliness);
    }

    return ok;
}

bool BuiltinProtocols::update_writer(
        RTPSWriter* rtps_writer,
        const fastdds::dds::WriterQos& wqos)
{
    bool ok = false;
    if ((nullptr != mp_PDP) && (nullptr != mp_PDP->get_edp()))
    {
        ok = mp_PDP->get_edp()->update_writer(rtps_writer, wqos);
    }
    return ok;
}

bool BuiltinProtocols::update_reader(
        RTPSReader* rtps_reader,
        const fastdds::dds::ReaderQos& rqos,
        const fastdds::rtps::ContentFilterProperty* content_filter)
{
    bool ok = false;
    if ((nullptr != mp_PDP) && (nullptr != mp_PDP->get_edp()))
    {
        ok = mp_PDP->get_edp()->update_reader(rtps_reader, rqos, content_filter);
    }
    return ok;
}

bool BuiltinProtocols::remove_writer(
        RTPSWriter* rtps_writer)
{
    bool ok = false;
    if (nullptr != mp_WLP)
    {
        ok |= mp_WLP->remove_local_writer(rtps_writer);
    }
    if ((nullptr != mp_PDP) && (nullptr != mp_PDP->get_edp()))
    {
        ok |= mp_PDP->get_edp()->remove_writer(rtps_writer);
    }
    return ok;
}

bool BuiltinProtocols::remove_reader(
        RTPSReader* rtps_reader)
{
    bool ok = false;
    if (nullptr != mp_WLP)
    {
        ok |= mp_WLP->remove_local_reader(rtps_reader);
    }
    if ((nullptr != mp_PDP) && (nullptr != mp_PDP->get_edp()))
    {
        ok |= mp_PDP->get_edp()->remove_reader(rtps_reader);
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
    else if (m_att.discovery_config.discoveryProtocol != DiscoveryProtocol::NONE)
    {
        EPROSIMA_LOG_ERROR(RTPS_EDP, "Trying to use BuiltinProtocols interfaces before initBuiltinProtocols call");
    }
}

void BuiltinProtocols::stopRTPSParticipantAnnouncement()
{
    // note that participants created with DiscoveryProtocol::NONE
    // may not have mp_PDP available

    if (mp_PDP)
    {
        mp_PDP->stopParticipantAnnouncement();
    }
    else if (m_att.discovery_config.discoveryProtocol != DiscoveryProtocol::NONE)
    {
        EPROSIMA_LOG_ERROR(RTPS_EDP, "Trying to use BuiltinProtocols interfaces before initBuiltinProtocols call");
    }
}

void BuiltinProtocols::resetRTPSParticipantAnnouncement()
{
    assert(mp_PDP);

    if (mp_PDP)
    {
        mp_PDP->resetParticipantAnnouncement();
    }
    else if (m_att.discovery_config.discoveryProtocol != DiscoveryProtocol::NONE)
    {
        EPROSIMA_LOG_ERROR(RTPS_EDP, "Trying to use BuiltinProtocols interfaces before initBuiltinProtocols call");
    }
}

} // namespace rtps
} /* namespace rtps */
} /* namespace eprosima */
