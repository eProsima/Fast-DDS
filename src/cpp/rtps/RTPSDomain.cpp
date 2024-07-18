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
 * @file RTPSDomain.cpp
 */

#include <fastdds/rtps/RTPSDomain.hpp>

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <regex>
#include <string>
#include <thread>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/LibrarySettings.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>
#include <fastdds/utils/IPFinder.hpp>
#include <fastdds/utils/IPLocator.hpp>
#include <fastdds/utils/md5.hpp>

#include <rtps/attributes/ServerAttributes.hpp>
#include <rtps/common/GuidUtils.hpp>
#include <rtps/network/utils/external_locators.hpp>
#include <rtps/participant/RTPSParticipantImpl.h>
#include <rtps/reader/BaseReader.hpp>
#include <rtps/RTPSDomainImpl.hpp>
#include <rtps/transport/TCPv4Transport.h>
#include <rtps/transport/TCPv6Transport.h>
#include <rtps/transport/test_UDPv4Transport.h>
#include <rtps/transport/UDPv4Transport.h>
#include <rtps/transport/UDPv6Transport.h>
#include <rtps/writer/BaseWriter.hpp>
#include <utils/Host.hpp>
#include <utils/SystemInfo.hpp>
#include <xmlparser/XMLProfileManager.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

template<typename _Descriptor>
bool has_user_transport(
        const RTPSParticipantAttributes& att)
{
    const auto& transports = att.userTransports;
    const auto end_it = transports.end();
    return end_it != std::find_if(transports.begin(), end_it,
                   [](const std::shared_ptr<eprosima::fastdds::rtps::TransportDescriptorInterface>& item)
                   {
                       return nullptr != dynamic_cast<_Descriptor*>(item.get());
                   });
}

static void guid_prefix_create(
        uint32_t ID,
        GuidPrefix_t& guidP)
{
    eprosima::fastdds::rtps::GuidUtils::instance().guid_prefix_create(ID, guidP);
}

std::shared_ptr<RTPSDomainImpl> RTPSDomainImpl::get_instance()
{
    static std::shared_ptr<RTPSDomainImpl> instance = std::make_shared<RTPSDomainImpl>();
    return instance;
}

void RTPSDomain::set_filewatch_thread_config(
        const fastdds::rtps::ThreadSettings& watch_thread,
        const fastdds::rtps::ThreadSettings& callback_thread)
{
    RTPSDomainImpl::set_filewatch_thread_config(watch_thread, callback_thread);
}

void RTPSDomain::stopAll()
{
    RTPSDomainImpl::stopAll();
}

RTPSParticipant* RTPSDomain::createParticipant(
        uint32_t domain_id,
        const RTPSParticipantAttributes& attrs,
        RTPSParticipantListener* listen)
{
    return RTPSDomainImpl::createParticipant(domain_id, true, attrs, listen);
}

RTPSParticipant* RTPSDomain::createParticipant(
        uint32_t domain_id,
        bool enabled,
        const RTPSParticipantAttributes& attrs,
        RTPSParticipantListener* listen)
{
    return RTPSDomainImpl::createParticipant(domain_id, enabled, attrs, listen);
}

bool RTPSDomain::removeRTPSParticipant(
        RTPSParticipant* p)
{
    return RTPSDomainImpl::removeRTPSParticipant(p);
}

void RTPSDomainImpl::stopAll()
{
    auto instance = get_instance();
    std::unique_lock<std::mutex> lock(instance->m_mutex);
    EPROSIMA_LOG_INFO(RTPS_PARTICIPANT, "DELETING ALL ENDPOINTS IN THIS DOMAIN");

    // Stop monitoring environment file
    SystemInfo::stop_watching_file(instance->file_watch_handle_);

    while (instance->m_RTPSParticipants.size() > 0)
    {
        t_p_RTPSParticipant participant = instance->m_RTPSParticipants.back();
        instance->m_RTPSParticipantIDs.erase(participant.second->getRTPSParticipantID());
        instance->m_RTPSParticipants.pop_back();

        lock.unlock();
        instance->removeRTPSParticipant_nts(participant);
        lock.lock();
    }

    xmlparser::XMLProfileManager::DeleteInstance();

    EPROSIMA_LOG_INFO(RTPS_PARTICIPANT, "RTPSParticipants deleted correctly ");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

RTPSParticipant* RTPSDomainImpl::createParticipant(
        uint32_t domain_id,
        bool enabled,
        const RTPSParticipantAttributes& attrs,
        RTPSParticipantListener* listen)
{
    EPROSIMA_LOG_INFO(RTPS_PARTICIPANT, "");

    RTPSParticipantAttributes PParam = attrs;

    if (PParam.builtin.discovery_config.leaseDuration < dds::c_TimeInfinite &&
            PParam.builtin.discovery_config.leaseDuration <=
            PParam.builtin.discovery_config.leaseDuration_announcementperiod)
    {
        EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT,
                "RTPSParticipant Attributes: LeaseDuration should be >= leaseDuration announcement period");
        return nullptr;
    }

    // Only the first time, initialize environment file watch if the corresponding environment variable is set
    auto instance = get_instance();
    if (!instance->file_watch_handle_)
    {
        std::string filename = SystemInfo::get_environment_file();
        if (!filename.empty() && SystemInfo::file_exists(filename))
        {
            std::lock_guard<std::mutex> guard(instance->m_mutex);
            // Create filewatch
            instance->file_watch_handle_ = SystemInfo::watch_file(filename, RTPSDomainImpl::file_watch_callback,
                            instance->watch_thread_config_, instance->callback_thread_config_);
        }
        else if (!filename.empty())
        {
            EPROSIMA_LOG_WARNING(RTPS_PARTICIPANT, filename + " does not exist. File watching not initialized.");
        }
    }

    uint32_t ID;
    if (!instance->prepare_participant_id(PParam.participantID, ID))
    {
        return nullptr;
    }

    if (!PParam.defaultUnicastLocatorList.isValid())
    {
        EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "Default Unicast Locator List contains invalid Locator");
        return nullptr;
    }
    if (!PParam.defaultMulticastLocatorList.isValid())
    {
        EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "Default Multicast Locator List contains invalid Locator");
        return nullptr;
    }

    PParam.participantID = ID;

    // Generate a new GuidPrefix_t
    GuidPrefix_t guidP;
    guid_prefix_create(instance->get_id_for_prefix(ID), guidP);
    if (!PParam.builtin.metatraffic_external_unicast_locators.empty())
    {
        fastdds::rtps::LocatorList locators;
        fastdds::rtps::IPFinder::getIP4Address(&locators);
        fastdds::rtps::network::external_locators::add_external_locators(locators,
                PParam.builtin.metatraffic_external_unicast_locators);
        uint16_t host_id = Host::compute_id(locators);
        guidP.value[2] = static_cast<octet>(host_id & 0xFF);
        guidP.value[3] = static_cast<octet>((host_id >> 8) & 0xFF);
    }

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
        if (PParam.builtin.discovery_config.discoveryProtocol == DiscoveryProtocol::BACKUP)
        {
            EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "Specifying a GUID prefix is mandatory for BACKUP Discovery Servers.");
            return nullptr;
        }
        pimpl = new RTPSParticipantImpl(domain_id, PParam, guidP, p, listen);
    }

    // Check implementation was correctly initialized
    if (!pimpl->is_initialized())
    {
        EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "Cannot create participant due to initialization error");
        delete pimpl;
        return nullptr;
    }

    // Above constructors create the sender resources. If a given listening port cannot be allocated an iterative
    // mechanism will allocate another by default. Change the default listening port is unacceptable for
    // discovery server Participant.
    if ((PParam.builtin.discovery_config.discoveryProtocol == DiscoveryProtocol::SERVER
            || PParam.builtin.discovery_config.discoveryProtocol == DiscoveryProtocol::BACKUP)
            && pimpl->did_mutation_took_place_on_meta(
                PParam.builtin.metatrafficMulticastLocatorList,
                PParam.builtin.metatrafficUnicastLocatorList))
    {
        if (PParam.builtin.metatrafficMulticastLocatorList.empty() &&
                PParam.builtin.metatrafficUnicastLocatorList.empty())
        {
            EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "Discovery Server requires to specify a listening address.");
        }
        else
        {
            EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT,
                    "Discovery Server wasn't able to allocate the specified listening port.");
        }

        delete pimpl;
        return nullptr;
    }

    // Check there is at least one transport registered.
    if (!pimpl->networkFactoryHasRegisteredTransports())
    {
        EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "Cannot create participant, because there is any transport");
        delete pimpl;
        return nullptr;
    }

    {
        std::lock_guard<std::mutex> guard(instance->m_mutex);
        instance->m_RTPSParticipants.push_back(t_p_RTPSParticipant(p, pimpl));
        instance->m_RTPSParticipantIDs[ID].used = true;
        instance->m_RTPSParticipantIDs[ID].reserved = true;
    }

    // Check the environment file in case it was modified during participant creation leading to a missed callback.
    if ((PParam.builtin.discovery_config.discoveryProtocol != DiscoveryProtocol::CLIENT) &&
            instance->file_watch_handle_)
    {
        pimpl->environment_file_has_changed();
    }

    if (enabled)
    {
        // Start protocols
        pimpl->enable();
    }
    return p;
}

bool RTPSDomainImpl::removeRTPSParticipant(
        RTPSParticipant* p)
{
    if (p != nullptr)
    {
        assert((p->mp_impl != nullptr) && "This participant has been previously invalidated");

        auto instance = get_instance();
        std::unique_lock<std::mutex> lock(instance->m_mutex);
        for (auto it = instance->m_RTPSParticipants.begin(); it != instance->m_RTPSParticipants.end(); ++it)
        {
            if (it->second->getGuid().guidPrefix == p->getGuid().guidPrefix)
            {
                RTPSDomainImpl::t_p_RTPSParticipant participant = *it;
                instance->m_RTPSParticipants.erase(it);
                uint32_t participant_id = participant.second->getRTPSParticipantID();
                instance->m_RTPSParticipantIDs[participant_id].used = false;
                instance->m_RTPSParticipantIDs[participant_id].reserved = false;
                lock.unlock();
                instance->removeRTPSParticipant_nts(participant);
                return true;
            }
        }
    }
    EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "RTPSParticipant not valid or not recognized");
    return false;
}

void RTPSDomainImpl::removeRTPSParticipant_nts(
        RTPSDomainImpl::t_p_RTPSParticipant& participant)
{
    participant.second->disable();
    // The destructor of RTPSParticipantImpl already deletes the associated RTPSParticipant and sets
    // its pointer to the RTPSParticipant to nullptr, so there is no need to do it here manually.
    delete(participant.second);
}

RTPSWriter* RTPSDomain::createRTPSWriter(
        RTPSParticipant* p,
        WriterAttributes& watt,
        WriterHistory* hist,
        WriterListener* listen)
{
    RTPSParticipantImpl* impl = RTPSDomainImpl::find_local_participant(p->getGuid());
    if (impl)
    {
        RTPSWriter* ret_val = nullptr;
        if (impl->createWriter(&ret_val, watt, hist, listen))
        {
            return ret_val;
        }
    }

    return nullptr;
}

RTPSWriter* RTPSDomain::createRTPSWriter(
        RTPSParticipant* p,
        const EntityId_t& entity_id,
        WriterAttributes& watt,
        WriterHistory* hist,
        WriterListener* listen)
{
    RTPSParticipantImpl* impl = RTPSDomainImpl::find_local_participant(p->getGuid());
    if (impl)
    {
        RTPSWriter* ret_val = nullptr;
        if (impl->createWriter(&ret_val, watt, hist, listen, entity_id))
        {
            return ret_val;
        }
    }

    return nullptr;
}

RTPSWriter* RTPSDomainImpl::create_rtps_writer(
        RTPSParticipant* p,
        const EntityId_t& entity_id,
        WriterAttributes& watt,
        WriterHistory* hist,
        WriterListener* listen)
{
    RTPSParticipantImpl* impl = RTPSDomainImpl::find_local_participant(p->getGuid());
    if (impl)
    {
        RTPSWriter* ret_val = nullptr;
        if (impl->create_writer(&ret_val, watt, hist, listen, entity_id, false))
        {
            return ret_val;
        }
    }

    return nullptr;
}

bool RTPSDomain::removeRTPSWriter(
        RTPSWriter* writer)
{
    return RTPSDomainImpl::removeRTPSWriter(writer);
}

bool RTPSDomainImpl::removeRTPSWriter(
        RTPSWriter* writer)
{
    if (writer != nullptr)
    {
        auto instance = get_instance();
        std::unique_lock<std::mutex> lock(instance->m_mutex);
        for (auto it = instance->m_RTPSParticipants.begin(); it != instance->m_RTPSParticipants.end(); ++it)
        {
            if (it->first->getGuid().guidPrefix == writer->getGuid().guidPrefix)
            {
                t_p_RTPSParticipant participant = *it;
                lock.unlock();
                return participant.second->deleteUserEndpoint(writer->getGuid());
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
    RTPSParticipantImpl* impl = RTPSDomainImpl::find_local_participant(p->getGuid());
    if (impl)
    {
        RTPSReader* reader;
        if (impl->createReader(&reader, ratt, rhist, rlisten))
        {
            return reader;
        }
    }
    return nullptr;
}

RTPSReader* RTPSDomain::createRTPSReader(
        RTPSParticipant* p,
        ReaderAttributes& ratt,
        const std::shared_ptr<IPayloadPool>& payload_pool,
        ReaderHistory* rhist,
        ReaderListener* rlisten)
{
    RTPSParticipantImpl* impl = RTPSDomainImpl::find_local_participant(p->getGuid());
    if (impl)
    {
        RTPSReader* reader;
        if (impl->createReader(&reader, ratt, payload_pool, rhist, rlisten))
        {
            return reader;
        }
    }
    return nullptr;
}

RTPSReader* RTPSDomain::createRTPSReader(
        RTPSParticipant* p,
        const EntityId_t& entity_id,
        ReaderAttributes& ratt,
        const std::shared_ptr<IPayloadPool>& payload_pool,
        ReaderHistory* rhist,
        ReaderListener* rlisten)
{
    RTPSParticipantImpl* impl = p->mp_impl;
    if (impl)
    {
        RTPSReader* reader;
        if (impl->createReader(&reader, ratt, payload_pool, rhist, rlisten, entity_id))
        {
            return reader;
        }
    }
    return nullptr;
}

bool RTPSDomain::removeRTPSReader(
        RTPSReader* reader)
{
    return RTPSDomainImpl::removeRTPSReader(reader);
}

bool RTPSDomainImpl::removeRTPSReader(
        RTPSReader* reader)
{
    if (reader !=  nullptr)
    {
        auto instance = get_instance();
        std::unique_lock<std::mutex> lock(instance->m_mutex);
        for (auto it = instance->m_RTPSParticipants.begin(); it != instance->m_RTPSParticipants.end(); ++it)
        {
            if (it->first->getGuid().guidPrefix == reader->getGuid().guidPrefix)
            {
                t_p_RTPSParticipant participant = *it;
                lock.unlock();
                return participant.second->deleteUserEndpoint(reader->getGuid());
            }
        }
    }
    return false;
}

RTPSParticipant* RTPSDomainImpl::clientServerEnvironmentCreationOverride(
        uint32_t domain_id,
        bool enabled,
        const RTPSParticipantAttributes& att,
        RTPSParticipantListener* listen)
{
    // Check the specified discovery protocol: if other than simple it has priority over ros environment variable
    if (att.builtin.discovery_config.discoveryProtocol != DiscoveryProtocol::SIMPLE)
    {
        EPROSIMA_LOG_INFO(DOMAIN, "Detected non simple discovery protocol attributes."
                << " Ignoring auto default client-server setup.");
        return nullptr;
    }

    // We only make the attributes copy when we are sure is worth
    // Is up to the caller guarantee the att argument is not modified during the call
    RTPSParticipantAttributes client_att(att);

    // Retrieve the info from the environment variable
    LocatorList_t& server_list = client_att.builtin.discovery_config.m_DiscoveryServers;
    if (load_environment_server_info(server_list) && server_list.empty())
    {
        // It's not an error, the environment variable may not be set. Any issue with environment
        // variable syntax is EPROSIMA_LOG_ERROR already
        return nullptr;
    }

    // Check if some address requires the UDPv6, TCPv4 or TCPv6 transport
    if (server_list.has_kind<LOCATOR_KIND_UDPv6>() &&
            !has_user_transport<fastdds::rtps::UDPv6TransportDescriptor>(client_att))
    {
        // Extend builtin transports with the UDPv6 transport
        auto descriptor = std::make_shared<fastdds::rtps::UDPv6TransportDescriptor>();
        descriptor->sendBufferSize = client_att.sendSocketBufferSize;
        descriptor->receiveBufferSize = client_att.listenSocketBufferSize;
        client_att.userTransports.push_back(std::move(descriptor));
    }
    if (server_list.has_kind<LOCATOR_KIND_TCPv4>() &&
            !has_user_transport<fastdds::rtps::TCPv4TransportDescriptor>(client_att))
    {
        // Extend builtin transports with the TCPv4 transport
        auto descriptor = std::make_shared<fastdds::rtps::TCPv4TransportDescriptor>();
        // Add automatic port
        descriptor->add_listener_port(0);
        descriptor->sendBufferSize = client_att.sendSocketBufferSize;
        descriptor->receiveBufferSize = client_att.listenSocketBufferSize;
        client_att.userTransports.push_back(std::move(descriptor));
    }
    if (server_list.has_kind<LOCATOR_KIND_TCPv6>() &&
            !has_user_transport<fastdds::rtps::TCPv6TransportDescriptor>(client_att))
    {
        // Extend builtin transports with the TCPv6 transport
        auto descriptor = std::make_shared<fastdds::rtps::TCPv6TransportDescriptor>();
        // Add automatic port
        descriptor->add_listener_port(0);
        descriptor->sendBufferSize = client_att.sendSocketBufferSize;
        descriptor->receiveBufferSize = client_att.listenSocketBufferSize;
        client_att.userTransports.push_back(std::move(descriptor));
    }

    EPROSIMA_LOG_INFO(DOMAIN, "Detected auto client-server environment variable."
            << "Trying to create client with the default server setup: "
            << client_att.builtin.discovery_config.m_DiscoveryServers);

    client_att.builtin.discovery_config.discoveryProtocol = DiscoveryProtocol::CLIENT;
    // RemoteServerAttributes already fill in above

    // Check if the client must become a super client
    if (ros_super_client_env())
    {
        client_att.builtin.discovery_config.discoveryProtocol = DiscoveryProtocol::SUPER_CLIENT;
    }

    RTPSParticipant* part = createParticipant(domain_id, enabled, client_att, listen);
    if (nullptr != part)
    {
        // Client successfully created
        EPROSIMA_LOG_INFO(DOMAIN, "Auto default server-client setup. Default client created.");
        part->mp_impl->client_override(true);
        return part;
    }

    // Unable to create auto server-client default participants
    EPROSIMA_LOG_ERROR(DOMAIN, "Auto default server-client setup. Unable to create the client.");
    return nullptr;
}

uint32_t RTPSDomainImpl::getNewId()
{
    // Get the smallest available participant ID.
    // Settings like maxInitialPeersRange control how many participants a peer
    // will look for on this host.
    // Choosing the smallest value ensures peers using unicast discovery will
    // find this participant as long as the total number of participants has
    // not exceeded the number of peers they will look for.
    uint32_t i = 0;
    while (m_RTPSParticipantIDs[i].reserved || m_RTPSParticipantIDs[i].used)
    {
        ++i;
    }
    m_RTPSParticipantIDs[i].reserved = true;
    return i;
}

bool RTPSDomainImpl::prepare_participant_id(
        int32_t input_id,
        uint32_t& participant_id)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    if (input_id < 0)
    {
        participant_id = getNewId();
    }
    else
    {
        participant_id = input_id;
        if (m_RTPSParticipantIDs[participant_id].used == true)
        {
            EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "RTPSParticipant with the same ID already exists");
            return false;
        }
    }
    return true;
}

uint32_t RTPSDomainImpl::get_id_for_prefix(
        uint32_t participant_id)
{
    uint32_t ret = participant_id;
    if (ret < 0x10000)
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        ret |= m_RTPSParticipantIDs[participant_id].counter;
        m_RTPSParticipantIDs[participant_id].counter += 0x10000;
    }

    return ret;
}

bool RTPSDomainImpl::reserve_participant_id(
        int32_t& participant_id)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    if (participant_id < 0)
    {
        participant_id = getNewId();
    }
    else
    {
        if (m_RTPSParticipantIDs[participant_id].reserved == true)
        {
            return false;
        }
        m_RTPSParticipantIDs[participant_id].reserved = true;
    }

    return true;
}

bool RTPSDomainImpl::create_participant_guid(
        int32_t& participant_id,
        GUID_t& guid)
{
    bool ret_value = get_instance()->reserve_participant_id(participant_id);

    if (ret_value)
    {
        guid_prefix_create(participant_id, guid.guidPrefix);
        guid.entityId = c_EntityId_RTPSParticipant;
    }

    return ret_value;
}

RTPSParticipantImpl* RTPSDomainImpl::find_local_participant(
        const GUID_t& guid)
{
    auto instance = get_instance();
    std::lock_guard<std::mutex> guard(instance->m_mutex);
    for (const t_p_RTPSParticipant& participant : instance->m_RTPSParticipants)
    {
        if (participant.second->getGuid().guidPrefix == guid.guidPrefix)
        {
            // Participant found, forward the query
            return participant.second;
        }
    }

    return nullptr;
}

BaseReader* RTPSDomainImpl::find_local_reader(
        const GUID_t& reader_guid)
{
    auto instance = get_instance();
    std::lock_guard<std::mutex> guard(instance->m_mutex);
    for (const t_p_RTPSParticipant& participant : instance->m_RTPSParticipants)
    {
        if (participant.second->getGuid().guidPrefix == reader_guid.guidPrefix)
        {
            // Participant found, forward the query
            return participant.second->find_local_reader(reader_guid);
        }
    }

    return nullptr;
}

BaseWriter* RTPSDomainImpl::find_local_writer(
        const GUID_t& writer_guid)
{
    auto instance = get_instance();
    std::lock_guard<std::mutex> guard(instance->m_mutex);
    for (const t_p_RTPSParticipant& participant : instance->m_RTPSParticipants)
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
        case fastdds::IntraprocessDeliveryType::INTRAPROCESS_FULL:
            return true;

        case fastdds::IntraprocessDeliveryType::INTRAPROCESS_USER_DATA_ONLY:
            return !matched_guid.is_builtin();

        case fastdds::IntraprocessDeliveryType::INTRAPROCESS_OFF:
        default:
            break;
    }

    return false;
}

void RTPSDomainImpl::file_watch_callback()
{
    auto _1s = std::chrono::seconds(1);

    // Ensure that all changes have been saved by the OS
    SystemInfo::wait_for_file_closure(SystemInfo::get_environment_file(), _1s);

    // For all RTPSParticipantImpl registered in the RTPSDomain, call RTPSParticipantImpl::environment_file_has_changed
    auto instance = get_instance();
    std::lock_guard<std::mutex> guard(instance->m_mutex);
    for (auto participant : instance->m_RTPSParticipants)
    {
        participant.second->environment_file_has_changed();
    }
}

void RTPSDomainImpl::set_filewatch_thread_config(
        const fastdds::rtps::ThreadSettings& watch_thread,
        const fastdds::rtps::ThreadSettings& callback_thread)
{
    auto instance = get_instance();
    std::lock_guard<std::mutex> guard(instance->m_mutex);
    instance->watch_thread_config_ = watch_thread;
    instance->callback_thread_config_ = callback_thread;
}

bool RTPSDomain::get_library_settings(
        fastdds::LibrarySettings& library_settings)
{
    return RTPSDomainImpl::get_library_settings(library_settings);
}

bool RTPSDomainImpl::get_library_settings(
        fastdds::LibrarySettings& library_settings)
{
    library_settings = xmlparser::XMLProfileManager::library_settings();
    return true;
}

bool RTPSDomain::set_library_settings(
        const fastdds::LibrarySettings& library_settings)
{
    return RTPSDomainImpl::set_library_settings(library_settings);
}

bool RTPSDomainImpl::set_library_settings(
        const fastdds::LibrarySettings& library_settings)
{
    if (!get_instance()->m_RTPSParticipants.empty())
    {
        return false;
    }
    xmlparser::XMLProfileManager::library_settings(library_settings);
    return true;
}

fastdds::dds::xtypes::ITypeObjectRegistry& RTPSDomainImpl::type_object_registry()
{
    return get_instance()->type_object_registry_;
}

fastdds::dds::xtypes::TypeObjectRegistry& RTPSDomainImpl::type_object_registry_observer()
{
    return get_instance()->type_object_registry_;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
