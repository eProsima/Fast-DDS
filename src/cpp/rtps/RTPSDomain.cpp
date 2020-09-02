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

/*
 * RTPSDomain.cpp
 *
 */

#include <fastdds/rtps/RTPSDomain.h>

#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <rtps/participant/RTPSParticipantImpl.h>

#include <fastdds/dds/log/Log.hpp>

#include <fastdds/rtps/transport/UDPv4Transport.h>
#include <fastdds/rtps/transport/UDPv6Transport.h>
#include <fastdds/rtps/transport/test_UDPv4Transport.h>

#include <fastrtps/utils/IPFinder.h>
#include <fastrtps/utils/IPLocator.h>
#include <fastrtps/utils/System.h>
#include <fastrtps/utils/md5.h>

#include <fastdds/rtps/writer/RTPSWriter.h>
#include <fastdds/rtps/reader/RTPSReader.h>

#include <fastrtps/xmlparser/XMLProfileManager.h>

#include "RTPSDomainImpl.hpp"
#include <utils/Host.hpp>

#include <chrono>
#include <thread>
#include <cstdlib>
#include <regex>

namespace eprosima {
namespace fastrtps {
namespace rtps {

static void guid_prefix_create(
        uint32_t ID,
        GuidPrefix_t& guidP)
{
    // Make a new participant GuidPrefix_t up
    int pid = System::GetPID();

    guidP.value[0] = c_VendorId_eProsima[0];
    guidP.value[1] = c_VendorId_eProsima[1];

    uint16_t host_id = Host::get().id();
    guidP.value[2] = octet(host_id);
    guidP.value[3] = octet(host_id >> 8);

    guidP.value[4] = octet(pid);
    guidP.value[5] = octet(pid >> 8);
    guidP.value[6] = octet(pid >> 16);
    guidP.value[7] = octet(pid >> 24);
    guidP.value[8] = octet(ID);
    guidP.value[9] = octet(ID >> 8);
    guidP.value[10] = octet(ID >> 16);
    guidP.value[11] = octet(ID >> 24);
}

// environment variables that forces server-client discovery
// it must contain a list of UDPv4 locators separated by ;
// the position in the list defines the default server that listens on the locator
const char* const DEFAULT_ROS2_MASTER_URI = "ROS_DISCOVERY_SERVER";

std::mutex RTPSDomain::m_mutex;
std::atomic<uint32_t> RTPSDomain::m_maxRTPSParticipantID(1);
std::vector<RTPSDomain::t_p_RTPSParticipant> RTPSDomain::m_RTPSParticipants;
std::set<uint32_t> RTPSDomain::m_RTPSParticipantIDs;

void RTPSDomain::stopAll()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    logInfo(RTPS_PARTICIPANT, "DELETING ALL ENDPOINTS IN THIS DOMAIN");

    while (m_RTPSParticipants.size() > 0)
    {
        RTPSDomain::t_p_RTPSParticipant participant = m_RTPSParticipants.back();
        m_RTPSParticipantIDs.erase(m_RTPSParticipantIDs.find(participant.second->getRTPSParticipantID()));
        m_RTPSParticipants.pop_back();

        lock.unlock();
        RTPSDomain::removeRTPSParticipant_nts(participant);
        lock.lock();
    }
    logInfo(RTPS_PARTICIPANT, "RTPSParticipants deleted correctly ");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

RTPSParticipant* RTPSDomain::createParticipant(
        uint32_t domain_id,
        const RTPSParticipantAttributes& attrs,
        RTPSParticipantListener* listen)
{
    return createParticipant(domain_id, true, attrs, listen);
}

RTPSParticipant* RTPSDomain::createParticipant(
        uint32_t domain_id,
        bool enabled,
        const RTPSParticipantAttributes& attrs,
        RTPSParticipantListener* listen)
{
    logInfo(RTPS_PARTICIPANT, "");

    RTPSParticipantAttributes PParam = attrs;

    if (PParam.builtin.discovery_config.leaseDuration < c_TimeInfinite &&
            PParam.builtin.discovery_config.leaseDuration <=
            PParam.builtin.discovery_config.leaseDuration_announcementperiod)
    {
        logError(RTPS_PARTICIPANT,
                "RTPSParticipant Attributes: LeaseDuration should be >= leaseDuration announcement period");
        return nullptr;
    }

    uint32_t ID;
    {
        std::lock_guard<std::mutex> guard(m_mutex);

        if (PParam.participantID < 0)
        {
            ID = getNewId();
            while (m_RTPSParticipantIDs.insert(ID).second == false)
            {
                ID = getNewId();
            }
        }
        else
        {
            ID = PParam.participantID;
            if (m_RTPSParticipantIDs.insert(ID).second == false)
            {
                logError(RTPS_PARTICIPANT, "RTPSParticipant with the same ID already exists");
                return nullptr;
            }
        }
    }

    if (!PParam.defaultUnicastLocatorList.isValid())
    {
        logError(RTPS_PARTICIPANT, "Default Unicast Locator List contains invalid Locator");
        return nullptr;
    }
    if (!PParam.defaultMulticastLocatorList.isValid())
    {
        logError(RTPS_PARTICIPANT, "Default Multicast Locator List contains invalid Locator");
        return nullptr;
    }

    PParam.participantID = ID;
    LocatorList_t loc;
    IPFinder::getIP4Address(&loc);

    if (loc.empty() && PParam.builtin.initialPeersList.empty())
    {
        Locator_t local;
        IPLocator::setIPv4(local, 127, 0, 0, 1);
        PParam.builtin.initialPeersList.push_back(local);
    }

    // Generate a new GuidPrefix_t
    GuidPrefix_t guidP;
    guid_prefix_create(ID, guidP);

    RTPSParticipant* p = new RTPSParticipant(nullptr);
    RTPSParticipantImpl* pimpl = nullptr;

    // If we force the participant to have a specific prefix we must define a different persistence GuidPrefix_t that
    // would ensure builtin endpoints are able to differentiate between a communication loss and a participant recovery
    if (PParam.prefix != c_GuidPrefix_Unknown)
    {
        pimpl = new RTPSParticipantImpl(domain_id, PParam, PParam.prefix, guidP, p, listen);
    }
    else
    {
        pimpl = new RTPSParticipantImpl(domain_id, PParam, guidP, p, listen);
    }

    // Above constructors create the sender resources. If a given listening port cannot be allocated an iterative
    // mechanism will allocate another by default. Change the default listening port is unacceptable for server
    // discovery.
    if ((PParam.builtin.discovery_config.discoveryProtocol == DiscoveryProtocol_t::SERVER
            || PParam.builtin.discovery_config.discoveryProtocol == DiscoveryProtocol_t::BACKUP)
            && pimpl->did_mutation_took_place_on_meta(
                PParam.builtin.metatrafficMulticastLocatorList,
                PParam.builtin.metatrafficUnicastLocatorList))
    {
        // we do not log an error because the library may use participant creation as a trial for server existence
        logInfo(RTPS_PARTICIPANT, "Server wasn't able to allocate the specified listening port");
        delete pimpl;
        return nullptr;
    }

    // Check there is at least one transport registered.
    if (!pimpl->networkFactoryHasRegisteredTransports())
    {
        logError(RTPS_PARTICIPANT, "Cannot create participant, because there is any transport");
        delete pimpl;
        return nullptr;
    }

#if HAVE_SECURITY
    // Check security was correctly initialized
    if (!pimpl->is_security_initialized())
    {
        logError(RTPS_PARTICIPANT, "Cannot create participant due to security initialization error");
        delete pimpl;
        return nullptr;
    }
#endif // if HAVE_SECURITY

    {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_RTPSParticipants.push_back(t_p_RTPSParticipant(p, pimpl));
    }

    if (enabled)
    {
        // Start protocols
        pimpl->enable();
    }
    return p;
}

bool RTPSDomain::removeRTPSParticipant(
        RTPSParticipant* p)
{
    if (p != nullptr)
    {
        p->mp_impl->disable();

        std::unique_lock<std::mutex> lock(m_mutex);
        for (auto it = m_RTPSParticipants.begin(); it != m_RTPSParticipants.end(); ++it)
        {
            if (it->second->getGuid().guidPrefix == p->getGuid().guidPrefix)
            {
                RTPSDomain::t_p_RTPSParticipant participant = *it;
                m_RTPSParticipants.erase(it);
                m_RTPSParticipantIDs.erase(m_RTPSParticipantIDs.find(participant.second->getRTPSParticipantID()));
                lock.unlock();
                removeRTPSParticipant_nts(participant);
                return true;
            }
        }
    }
    logError(RTPS_PARTICIPANT, "RTPSParticipant not valid or not recognized");
    return false;
}

void RTPSDomain::removeRTPSParticipant_nts(
        RTPSDomain::t_p_RTPSParticipant& participant)
{
    delete(participant.second);
}

RTPSWriter* RTPSDomain::createRTPSWriter(
        RTPSParticipant* p,
        WriterAttributes& watt,
        WriterHistory* hist,
        WriterListener* listen)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    for (auto it = m_RTPSParticipants.begin(); it != m_RTPSParticipants.end(); ++it)
    {
        if (it->first->getGuid().guidPrefix == p->getGuid().guidPrefix)
        {
            t_p_RTPSParticipant participant = *it;
            lock.unlock();
            RTPSWriter* writ;
            if (participant.second->createWriter(&writ, watt, hist, listen))
            {
                return writ;
            }
            return nullptr;
        }
    }
    return nullptr;
}

bool RTPSDomain::removeRTPSWriter(
        RTPSWriter* writer)
{
    if (writer != nullptr)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        for (auto it = m_RTPSParticipants.begin(); it != m_RTPSParticipants.end(); ++it)
        {
            if (it->first->getGuid().guidPrefix == writer->getGuid().guidPrefix)
            {
                t_p_RTPSParticipant participant = *it;
                lock.unlock();
                return participant.second->deleteUserEndpoint((Endpoint*)writer);
            }
        }
    }
    return false;
}

RTPSReader* RTPSDomain::createRTPSReader(
        RTPSParticipant* p,
        ReaderAttributes& ratt,
        ReaderHistory* rhist,
        ReaderListener* rlisten)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    for (auto it = m_RTPSParticipants.begin(); it != m_RTPSParticipants.end(); ++it)
    {
        if (it->first->getGuid().guidPrefix == p->getGuid().guidPrefix)
        {
            t_p_RTPSParticipant participant = *it;
            lock.unlock();
            RTPSReader* reader;
            if (participant.second->createReader(&reader, ratt, rhist, rlisten))
            {
                return reader;
            }

            return nullptr;
        }
    }
    return nullptr;
}

bool RTPSDomain::removeRTPSReader(
        RTPSReader* reader)
{
    if (reader !=  nullptr)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        for (auto it = m_RTPSParticipants.begin(); it != m_RTPSParticipants.end(); ++it)
        {
            if (it->first->getGuid().guidPrefix == reader->getGuid().guidPrefix)
            {
                t_p_RTPSParticipant participant = *it;
                lock.unlock();
                return participant.second->deleteUserEndpoint((Endpoint*)reader);
            }
        }
    }
    return false;
}

RTPSParticipant* RTPSDomain::clientServerEnvironmentCreationOverride(
        uint32_t domain_id,
        bool enabled,
        const RTPSParticipantAttributes& att,
        RTPSParticipantListener* listen /*= nullptr*/)
{
    // retrieve the environment variable value
    std::string list;
    {
#pragma warning(suppress:4996)
        const char* data = std::getenv(DEFAULT_ROS2_MASTER_URI);

        if (nullptr != data)
        {
            list = data;
        }
        else
        {
            // if the variable is not set abort the server-client default setup
            return nullptr;
        }
    }

    // Check the specified discovery protocol: if other than simple it has priority over ros environment variable
    if (att.builtin.discovery_config.discoveryProtocol != DiscoveryProtocol_t::SIMPLE)
    {
        logInfo(DOMAIN, "Detected non simple discovery protocol attributes."
                << " Ignoring auto default client-server setup.");
        return nullptr;
    }

    // we only make the attributes copy when we are sure is worth
    RTPSParticipantAttributes client_att(att);

    // Retrieve the info from the environment variable
    if (!load_environment_server_info(
                list,
                client_att.builtin.discovery_config.m_DiscoveryServers))
    {
        // it's not an error, the environment variable may not be set. Any issue with environment
        // variable syntax is logError already
        return nullptr;
    }

    logInfo(DOMAIN, "Detected auto client-server environment variable."
            "Trying to create client with the default server setup.");

    client_att.builtin.discovery_config.discoveryProtocol = DiscoveryProtocol_t::CLIENT;
    // RemoteServerAttributes already fill in above

    RTPSParticipant* part = RTPSDomain::createParticipant(domain_id, enabled, client_att, listen);
    if (nullptr != part)
    {
        // client successfully created
        logInfo(DOMAIN, "Auto default server-client setup. Default client created.");
        return part;
    }

    // unable to create auto server-client default participants
    logError(DOMAIN, "Auto default server-client setup. Unable to create the client.");
    return nullptr;
}

void RTPSDomainImpl::create_participant_guid(
        int32_t& participant_id,
        GUID_t& guid)
{
    if (participant_id < 0)
    {
        std::lock_guard<std::mutex> guard(RTPSDomain::m_mutex);
        do
        {
            participant_id = RTPSDomain::getNewId();
        } while (RTPSDomain::m_RTPSParticipantIDs.find(participant_id) != RTPSDomain::m_RTPSParticipantIDs.end());
    }

    guid_prefix_create(participant_id, guid.guidPrefix);
    guid.entityId = c_EntityId_RTPSParticipant;
}

RTPSReader* RTPSDomainImpl::find_local_reader(
        const GUID_t& reader_guid)
{
    std::lock_guard<std::mutex> guard(RTPSDomain::m_mutex);
    for (const RTPSDomain::t_p_RTPSParticipant& participant : RTPSDomain::m_RTPSParticipants)
    {
        if (participant.second->getGuid().guidPrefix == reader_guid.guidPrefix)
        {
            // Participant found, forward the query
            return participant.second->find_local_reader(reader_guid);
        }
    }

    return nullptr;
}

RTPSWriter* RTPSDomainImpl::find_local_writer(
        const GUID_t& writer_guid)
{
    std::lock_guard<std::mutex> guard(RTPSDomain::m_mutex);
    for (const RTPSDomain::t_p_RTPSParticipant& participant : RTPSDomain::m_RTPSParticipants)
    {
        if (participant.second->getGuid().guidPrefix == writer_guid.guidPrefix)
        {
            // Participant found, forward the query
            return participant.second->find_local_writer(writer_guid);
        }
    }

    return nullptr;
}

/**
 * Check whether intraprocess delivery should be used between two GUIDs.
 *
 * @param local_guid    GUID of the local endpoint performing the query.
 * @param matched_guid  GUID being queried about.
 *
 * @returns true when intraprocess delivery is enabled, false otherwise.
 */
bool RTPSDomainImpl::should_intraprocess_between(
        const GUID_t& local_guid,
        const GUID_t& matched_guid)
{
    if (!local_guid.is_on_same_process_as(matched_guid))
    {
        // Not on the same process, should not use intraprocess mechanism.
        return false;
    }

    if (local_guid.entityId == c_EntityId_SPDPWriter || local_guid.entityId == c_EntityId_SPDPReader)
    {
        // Always disabled for PDP, to avoid inter-domain communications.
        return false;
    }

    switch (xmlparser::XMLProfileManager::library_settings().intraprocess_delivery)
    {
        case IntraprocessDeliveryType::INTRAPROCESS_FULL:
            return true;

        case IntraprocessDeliveryType::INTRAPROCESS_USER_DATA_ONLY:
            return !matched_guid.is_builtin();

        case IntraprocessDeliveryType::INTRAPROCESS_OFF:
        default:
            break;
    }

    return false;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
