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
 * @file RTPSParticipant.cpp
 *
 */

#include <algorithm>
#include <functional>
#include <memory>
#include <mutex>
#include <sstream>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/BuiltinTransports.hpp>
#include <fastdds/rtps/attributes/ServerAttributes.h>
#include <fastdds/rtps/builtin/BuiltinProtocols.h>
#include <fastdds/rtps/builtin/discovery/endpoint/EDP.h>
#include <fastdds/rtps/builtin/discovery/participant/PDP.h>
#include <fastdds/rtps/builtin/discovery/participant/PDPSimple.h>
#include <fastdds/rtps/builtin/data/ParticipantProxyData.h>
#include <fastdds/rtps/builtin/liveliness/WLP.h>
#include <fastdds/rtps/common/EntityId_t.hpp>
#include <fastdds/rtps/history/WriterHistory.h>
#include <fastdds/rtps/messages/MessageReceiver.h>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.h>
#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/rtps/reader/StatelessReader.h>
#include <fastdds/rtps/reader/StatefulReader.h>
#include <fastdds/rtps/reader/StatelessPersistentReader.h>
#include <fastdds/rtps/reader/StatefulPersistentReader.h>
#include <fastdds/rtps/RTPSDomain.h>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.h>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h>
#include <fastdds/rtps/writer/StatelessWriter.h>
#include <fastdds/rtps/writer/StatefulWriter.h>
#include <fastdds/rtps/writer/StatelessPersistentWriter.h>
#include <fastdds/rtps/writer/StatefulPersistentWriter.h>
#include <fastrtps/utils/UnitsParser.hpp>

#include <fastdds/rtps/common/LocatorList.hpp>

#include <fastrtps/utils/IPFinder.h>
#include <fastrtps/utils/Semaphore.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <rtps/builtin/discovery/participant/PDPClient.h>
#include <rtps/builtin/discovery/participant/PDPServer.hpp>
#include <rtps/history/BasicPayloadPool.hpp>
#include <rtps/network/utils/external_locators.hpp>
#include <rtps/network/utils/netmask_filter.hpp>
#include <rtps/participant/RTPSParticipantImpl.h>
#include <rtps/persistence/PersistenceService.h>
#include <statistics/rtps/GuidUtils.hpp>
#include <utils/string_utilities.hpp>
#include <utils/SystemInfo.hpp>

#ifdef FASTDDS_STATISTICS
#include <statistics/types/monitorservice_types.h>
#include <statistics/rtps/monitor-service/MonitorService.hpp>
#endif // ifdef FASTDDS_STATISTICS

#if HAVE_SECURITY
#include <security/logging/LogTopic.h>
#endif  // HAVE_SECURITY

namespace eprosima {
namespace fastrtps {
namespace rtps {

using UDPv4TransportDescriptor = fastdds::rtps::UDPv4TransportDescriptor;
using TCPTransportDescriptor = fastdds::rtps::TCPTransportDescriptor;
using SharedMemTransportDescriptor = fastdds::rtps::SharedMemTransportDescriptor;
using BuiltinTransports = fastdds::rtps::BuiltinTransports;

/**
 * Parse the environment variable specifying the transports to instantiate and optional configuration options
 * if the transport selected is LARGE_DATA.
 */
static void set_builtin_transports_from_env_var(
        RTPSParticipantAttributes& attr)
{
    static constexpr const char* env_var_name = "FASTDDS_BUILTIN_TRANSPORTS";

    BuiltinTransports ret_val = BuiltinTransports::DEFAULT;
    std::string env_value;
    if (SystemInfo::get_env(env_var_name, env_value) == ReturnCode_t::RETCODE_OK)
    {
        std::regex COMMON_REGEX(R"((\w+))");
        std::regex OPTIONS_REGEX(
            R"((\w+)\?(((max_msg_size|sockets_size)=(\d+)(\w*)&?)|(non_blocking=(\w+)&?)|(tcp_negotiation_timeout=(\d+)&?)){0,4})");
        std::smatch mr;

        if (std::regex_match(env_value, COMMON_REGEX, std::regex_constants::match_not_null))
        {
            // Only transport mode is specified
            if (!get_element_enum_value(env_value.c_str(), ret_val,
                    "NONE", BuiltinTransports::NONE,
                    "DEFAULT", BuiltinTransports::DEFAULT,
                    "DEFAULTv6", BuiltinTransports::DEFAULTv6,
                    "SHM", BuiltinTransports::SHM,
                    "UDPv4", BuiltinTransports::UDPv4,
                    "UDPv6", BuiltinTransports::UDPv6,
                    "LARGE_DATA", BuiltinTransports::LARGE_DATA,
                    "LARGE_DATAv6", BuiltinTransports::LARGE_DATAv6))
            {
                EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "Wrong value '" << env_value << "' for environment variable '" <<
                        env_var_name << "'. Leaving as DEFAULT");
            }
        }
        else if (std::regex_match(env_value, mr, OPTIONS_REGEX, std::regex_constants::match_not_null))
        {
            // Transport mode AND options are specified
            std::regex msg_size_regex(R"((max_msg_size)=(\d+)(\w*))");
            std::regex sockets_size_regex(R"((sockets_size)=(\d+)(\w*))");
            std::regex non_blocking_regex(R"((non_blocking)=(true|false))");
            std::regex tcp_timeout_regex(R"((tcp_negotiation_timeout)=(\d+))");

            fastdds::rtps::BuiltinTransportsOptions options;

            try
            {
                if (!get_element_enum_value(mr[1].str().c_str(), ret_val,
                        "NONE", BuiltinTransports::NONE,
                        "DEFAULT", BuiltinTransports::DEFAULT,
                        "DEFAULTv6", BuiltinTransports::DEFAULTv6,
                        "SHM", BuiltinTransports::SHM,
                        "UDPv4", BuiltinTransports::UDPv4,
                        "UDPv6", BuiltinTransports::UDPv6,
                        "LARGE_DATA", BuiltinTransports::LARGE_DATA,
                        "LARGE_DATAv6", BuiltinTransports::LARGE_DATAv6))
                {
                    EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "Wrong value '" << env_value << "' for environment variable '" <<
                            env_var_name << "'. Leaving as DEFAULT");
                }
                // Max_msg_size parser
                if (std::regex_search(env_value, mr, msg_size_regex, std::regex_constants::match_not_null))
                {
                    std::string value = mr[2];
                    std::string unit = mr[3].str();
                    options.maxMessageSize = eprosima::fastdds::dds::utils::parse_value_and_units(value, unit);
                }
                // Sockets_size parser
                if (std::regex_search(env_value, mr, sockets_size_regex, std::regex_constants::match_not_null))
                {
                    std::string value = mr[2];
                    std::string unit = mr[3].str();
                    options.sockets_buffer_size = eprosima::fastdds::dds::utils::parse_value_and_units(value, unit);
                }
                // Non-blocking-send parser
                if (std::regex_search(env_value, mr, non_blocking_regex, std::regex_constants::match_not_null))
                {
                    options.non_blocking_send = mr[2] == "true";
                }
                // TCP_negotiation_timeout parser
                if (std::regex_search(env_value, mr, tcp_timeout_regex, std::regex_constants::match_not_null))
                {
                    options.tcp_negotiation_timeout = static_cast<uint32_t>(std::stoul(mr[2]));
                }
                attr.setup_transports(ret_val, options);
                return;
            }
            catch (std::exception& e)
            {
                EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT,
                        "Exception parsing environment variable: " << e.what() <<
                        " Leaving LARGE_DATA with default options.");
                attr.setup_transports(ret_val);
                return;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "Wrong value '" << env_value << "' for environment variable '" <<
                    env_var_name << "'. Leaving as DEFAULT");
        }
    }
    attr.setup_transports(ret_val);
}

static EntityId_t TrustedWriter(
        const EntityId_t& reader)
{
    return
        (reader == c_EntityId_SPDPReader) ? c_EntityId_SPDPWriter :
        (reader == c_EntityId_SEDPPubReader) ? c_EntityId_SEDPPubWriter :
        (reader == c_EntityId_SEDPSubReader) ? c_EntityId_SEDPSubWriter :
        (reader == c_EntityId_ReaderLiveliness) ? c_EntityId_WriterLiveliness :
        c_EntityId_Unknown;
}

static bool should_be_intraprocess_only(
        const RTPSParticipantAttributes& att)
{
    return
        xmlparser::XMLProfileManager::library_settings().intraprocess_delivery == INTRAPROCESS_FULL &&
        att.builtin.discovery_config.ignoreParticipantFlags ==
        (ParticipantFilteringFlags::FILTER_DIFFERENT_HOST | ParticipantFilteringFlags::FILTER_DIFFERENT_PROCESS);
}

static bool get_unique_flows_parameters(
        const RTPSParticipantAttributes& part_att,
        const EndpointAttributes& att,
        bool& unique_flows,
        uint16_t& initial_port,
        uint16_t& final_port)
{
    const std::string* value = PropertyPolicyHelper::find_property(att.properties, "fastdds.unique_network_flows");

    unique_flows = (nullptr != value);
    if (unique_flows)
    {
        // TODO (Miguel C): parse value to get port range
        final_port = part_att.port.portBase;
        initial_port = part_att.port.portBase - 400;
    }

    return true;
}

Locator_t& RTPSParticipantImpl::applyLocatorAdaptRule(
        Locator_t& loc)
{
    // This is a completely made up rule
    // It is transport responsibility to interpret this new port.
    uint16_t delta = m_att.port.participantIDGain;
    if (metatraffic_unicast_port_ == loc.port)
    {
        metatraffic_unicast_port_ += delta;
    }
    loc.port += delta;
    return loc;
}

RTPSParticipantImpl::RTPSParticipantImpl(
        uint32_t domain_id,
        const RTPSParticipantAttributes& PParam,
        const GuidPrefix_t& guidP,
        const GuidPrefix_t& persistence_guid,
        RTPSParticipant* par,
        RTPSParticipantListener* plisten)
    : domain_id_(domain_id)
    , m_att(PParam)
    , m_guid(guidP, c_EntityId_RTPSParticipant)
    , mp_builtinProtocols(nullptr)
    , IdCounter(0)
    , m_network_Factory(PParam)
    , type_check_fn_(nullptr)
    , client_override_(false)
    , internal_metatraffic_locators_(false)
    , internal_default_locators_(false)
#if HAVE_SECURITY
    , m_security_manager(this, *this)
#endif // if HAVE_SECURITY
    , mp_participantListener(plisten)
    , mp_userParticipant(par)
    , mp_mutex(new std::recursive_mutex())
    , is_intraprocess_only_(should_be_intraprocess_only(PParam))
#ifdef FASTDDS_STATISTICS
    , monitor_server_(nullptr)
    , conns_observer_(nullptr)
#endif // if FASTDDS_STATISTICS
    , has_shm_transport_(false)
    , match_local_endpoints_(should_match_local_endpoints(PParam))
{
    if (c_GuidPrefix_Unknown != persistence_guid)
    {
        m_persistence_guid = GUID_t(persistence_guid, c_EntityId_RTPSParticipant);
    }

    // Setup builtin transports
    if (m_att.useBuiltinTransports)
    {
        set_builtin_transports_from_env_var(m_att);
    }

    // BACKUP servers guid is its persistence one
    if (m_att.builtin.discovery_config.discoveryProtocol == DiscoveryProtocol::BACKUP)
    {
        m_persistence_guid = m_guid;
    }

    // Store the Guid in string format.
    std::stringstream guid_sstr;
    guid_sstr << m_guid;
    guid_str_ = guid_sstr.str();

    // Client-server discovery protocol requires that every TCP transport has a listening port
    switch (m_att.builtin.discovery_config.discoveryProtocol)
    {
        case DiscoveryProtocol::BACKUP:
        case DiscoveryProtocol::SERVER:
            // Verify if listening ports are provided
            for (auto& transportDescriptor : m_att.userTransports)
            {
                TCPTransportDescriptor* pT = dynamic_cast<TCPTransportDescriptor*>(transportDescriptor.get());
                if (pT)
                {
                    if (pT->listening_ports.empty())
                    {
                        EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT,
                                "Participant " << m_att.getName() << " with GUID " << m_guid <<
                                " tries to create a TCP server for discovery server without providing a proper listening port.");
                        break;
                    }
                    if (!m_att.builtin.metatrafficUnicastLocatorList.empty())
                    {
                        std::for_each(m_att.builtin.metatrafficUnicastLocatorList.begin(),
                                m_att.builtin.metatrafficUnicastLocatorList.end(), [&](Locator_t& locator)
                                {
                                    // TCP DS default logical port is the same as the physical one
                                    if (locator.kind == LOCATOR_KIND_TCPv4 || locator.kind == LOCATOR_KIND_TCPv6)
                                    {
                                        if (IPLocator::getLogicalPort(locator) == 0)
                                        {
                                            IPLocator::setLogicalPort(locator, IPLocator::getPhysicalPort(locator));
                                        }
                                    }
                                });
                    }
                }
            }
            break;
        case DiscoveryProtocol::CLIENT:
        case DiscoveryProtocol::SUPER_CLIENT:
            // Verify if listening ports are provided
            for (auto& transportDescriptor : m_att.userTransports)
            {
                TCPTransportDescriptor* pT = dynamic_cast<TCPTransportDescriptor*>(transportDescriptor.get());
                if (pT)
                {
                    if (pT->listening_ports.empty())
                    {
                        EPROSIMA_LOG_INFO(RTPS_PARTICIPANT,
                                "Participant " << m_att.getName() << " with GUID " << m_guid <<
                                " tries to create a TCP client for discovery server without providing a proper listening port." <<
                                " No TCP participants will be able to connect to this participant, but it will be able make connections.");
                    }
                    for (fastdds::rtps::RemoteServerAttributes& it : m_att.builtin.discovery_config.m_DiscoveryServers)
                    {
                        std::for_each(it.metatrafficUnicastLocatorList.begin(),
                                it.metatrafficUnicastLocatorList.end(), [&](Locator_t& locator)
                                {
                                    // TCP DS default logical port is the same as the physical one
                                    if (locator.kind == LOCATOR_KIND_TCPv4 || locator.kind == LOCATOR_KIND_TCPv6)
                                    {
                                        if (IPLocator::getLogicalPort(locator) == 0)
                                        {
                                            IPLocator::setLogicalPort(locator, IPLocator::getPhysicalPort(locator));
                                        }
                                    }
                                });
                    }
                }
            }
        default:
            break;
    }

    // User defined transports
    for (const auto& transportDescriptor : m_att.userTransports)
    {
        bool register_transport = true;

        // Lock user's transport descriptor since it could be modified during registration
        transportDescriptor->lock();

        auto socket_descriptor =
                std::dynamic_pointer_cast<fastdds::rtps::SocketTransportDescriptor>(transportDescriptor);
        fastdds::rtps::NetmaskFilterKind socket_descriptor_netmask_filter{};
        if (socket_descriptor != nullptr)
        {
            // Copy original netmask filter value to restore it after registration
            socket_descriptor_netmask_filter = socket_descriptor->netmask_filter;
            if (!fastdds::rtps::network::netmask_filter::validate_and_transform(socket_descriptor->netmask_filter,
                    m_att.netmaskFilter))
            {
                EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT,
                        "User transport failed to register. Provided descriptor's netmask filter ("
                        << socket_descriptor->netmask_filter << ") is incompatible with participant's ("
                        << m_att.netmaskFilter << ").");
                register_transport = false;
            }
        }

        bool transport_registered = register_transport && m_network_Factory.RegisterTransport(
            transportDescriptor.get(), &m_att.properties, m_att.max_msg_size_no_frag);

        if (socket_descriptor != nullptr)
        {
            // Restore original netmask filter value prior to unlock
            socket_descriptor->netmask_filter = socket_descriptor_netmask_filter;
        }

        transportDescriptor->unlock();

        if (transport_registered)
        {
            has_shm_transport_ |=
                    (dynamic_cast<fastdds::rtps::SharedMemTransportDescriptor*>(transportDescriptor.get()) != nullptr);
        }
        else
        {
            // SHM transport could be disabled
            if ((dynamic_cast<fastdds::rtps::SharedMemTransportDescriptor*>(transportDescriptor.get()) != nullptr))
            {
                EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT,
                        "Unable to Register SHM Transport. SHM Transport is not supported in"
                        " the current platform.");
            }
            else
            {
                EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT,
                        "User transport failed to register.");
            }

        }
    }

    mp_userParticipant->mp_impl = this;
    uint32_t id_for_thread = static_cast<uint32_t>(m_att.participantID);
    const fastdds::rtps::ThreadSettings& thr_config = m_att.timed_events_thread;
    mp_event_thr.init_thread(thr_config, "dds.ev.%u", id_for_thread);

    if (!networkFactoryHasRegisteredTransports())
    {
        return;
    }

    // Check netmask filtering preconditions
    std::vector<fastdds::rtps::TransportNetmaskFilterInfo> netmask_filter_info =
            m_network_Factory.netmask_filter_info();
    std::string error_msg;
    if (!fastdds::rtps::network::netmask_filter::check_preconditions(netmask_filter_info,
            m_att.ignore_non_matching_locators,
            error_msg) ||
            !fastdds::rtps::network::netmask_filter::check_preconditions(netmask_filter_info,
            m_att.builtin.metatraffic_external_unicast_locators,
            error_msg) ||
            !fastdds::rtps::network::netmask_filter::check_preconditions(netmask_filter_info,
            m_att.default_external_unicast_locators, error_msg))
    {
        EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, error_msg);
        return;
    }

#if HAVE_SECURITY
    // Start security
    if (!m_security_manager.init(
                security_attributes_,
                m_att.properties))
    {
        // Participant will be deleted, no need to allocate buffers or create builtin endpoints
        return;
    }
#endif // if HAVE_SECURITY

    setup_meta_traffic();
    setup_user_traffic();
    setup_initial_peers();
    setup_output_traffic();

#if HAVE_SECURITY
    if (m_security_manager.is_security_active())
    {
        if (!m_security_manager.create_entities())
        {
            return;
        }
    }
#endif // if HAVE_SECURITY

    // Copy NetworkFactory network_configuration to participant attributes prior to proxy creation
    // NOTE: all transports already registered before
    m_att.builtin.network_configuration = m_network_Factory.network_configuration();

    mp_builtinProtocols = new BuiltinProtocols();

    // Initialize builtin protocols
    if (!mp_builtinProtocols->initBuiltinProtocols(this, m_att.builtin))
    {
        EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "The builtin protocols were not correctly initialized");
        return;
    }

    if (c_GuidPrefix_Unknown != persistence_guid)
    {
        EPROSIMA_LOG_INFO(RTPS_PARTICIPANT,
                "RTPSParticipant \"" << m_att.getName() << "\" with guidPrefix: " << m_guid.guidPrefix
                                     << " and persistence guid: " << persistence_guid);
    }
    else
    {
        EPROSIMA_LOG_INFO(RTPS_PARTICIPANT,
                "RTPSParticipant \"" << m_att.getName() << "\" with guidPrefix: " << m_guid.guidPrefix);
    }

    initialized_ = true;
}

RTPSParticipantImpl::RTPSParticipantImpl(
        uint32_t domain_id,
        const RTPSParticipantAttributes& PParam,
        const GuidPrefix_t& guidP,
        RTPSParticipant* par,
        RTPSParticipantListener* plisten)
    : RTPSParticipantImpl(domain_id, PParam, guidP, c_GuidPrefix_Unknown, par, plisten)
{
}

void RTPSParticipantImpl::setup_meta_traffic()
{
    /* If metatrafficMulticastLocatorList is empty, add mandatory default Locators
       Else -> Take them */

    // Creation of metatraffic locator and receiver resources
    uint32_t metatraffic_multicast_port = m_att.port.getMulticastPort(domain_id_);
    metatraffic_unicast_port_ = m_att.port.getUnicastPort(domain_id_, static_cast<uint32_t>(m_att.participantID));
    uint32_t meta_multicast_port_for_check = metatraffic_multicast_port;

    /* INSERT DEFAULT MANDATORY MULTICAST LOCATORS HERE */
    if (m_att.builtin.metatrafficMulticastLocatorList.empty() && m_att.builtin.metatrafficUnicastLocatorList.empty())
    {
        get_default_metatraffic_locators();
        internal_metatraffic_locators_ = true;
    }
    else
    {
        if (0 < m_att.builtin.metatrafficMulticastLocatorList.size() &&
                0 !=  m_att.builtin.metatrafficMulticastLocatorList.begin()->port)
        {
            meta_multicast_port_for_check = m_att.builtin.metatrafficMulticastLocatorList.begin()->port;
        }
        std::for_each(m_att.builtin.metatrafficMulticastLocatorList.begin(),
                m_att.builtin.metatrafficMulticastLocatorList.end(), [&](Locator_t& locator)
                {
                    m_network_Factory.fillMetatrafficMulticastLocator(locator, metatraffic_multicast_port);
                });
        m_network_Factory.NormalizeLocators(m_att.builtin.metatrafficMulticastLocatorList);

        std::for_each(m_att.builtin.metatrafficUnicastLocatorList.begin(),
                m_att.builtin.metatrafficUnicastLocatorList.end(), [&](Locator_t& locator)
                {
                    m_network_Factory.fillMetatrafficUnicastLocator(locator, metatraffic_unicast_port_);
                });
        m_network_Factory.NormalizeLocators(m_att.builtin.metatrafficUnicastLocatorList);
    }

    if (is_intraprocess_only())
    {
        m_att.builtin.metatrafficUnicastLocatorList.clear();
    }

    createReceiverResources(m_att.builtin.metatrafficUnicastLocatorList, true, false, true);
    createReceiverResources(m_att.builtin.metatrafficMulticastLocatorList, false, false, true);

    // Check metatraffic multicast port
    if (0 < m_att.builtin.metatrafficMulticastLocatorList.size() &&
            m_att.builtin.metatrafficMulticastLocatorList.begin()->port != meta_multicast_port_for_check)
    {
        EPROSIMA_LOG_WARNING(RTPS_PARTICIPANT,
                "Metatraffic multicast port " << meta_multicast_port_for_check << " cannot be opened."
                " It may is opened by another application. Discovery may fail.");
    }

    namespace external_locators = fastdds::rtps::network::external_locators;
    external_locators::set_listening_locators(m_att.builtin.metatraffic_external_unicast_locators,
            m_att.builtin.metatrafficUnicastLocatorList);
}

void RTPSParticipantImpl::setup_user_traffic()
{
    // Creation of user locator and receiver resources
    //If no default locators are defined we define some.
    /* The reasoning here is the following.
       If the parameters of the RTPS Participant don't hold default listening locators for the creation
       of Endpoints, we make some for Unicast only.
       If there is at least one listen locator of any kind, we do not create any default ones.
       If there are no sending locators defined, we create default ones for the transports we implement.
     */
    if (m_att.defaultUnicastLocatorList.empty() && m_att.defaultMulticastLocatorList.empty())
    {
        //Default Unicast Locators in case they have not been provided
        /* INSERT DEFAULT UNICAST LOCATORS FOR THE PARTICIPANT */
        get_default_unicast_locators();
        internal_default_locators_ = true;
        EPROSIMA_LOG_INFO(RTPS_PARTICIPANT,
                m_att.getName() << " Created with NO default Unicast Locator List, adding Locators:"
                                << m_att.defaultUnicastLocatorList);
    }
    else
    {
        // Locator with port 0, calculate port.
        uint32_t unicast_port = metatraffic_unicast_port_ + m_att.port.offsetd3 - m_att.port.offsetd1;
        std::for_each(m_att.defaultUnicastLocatorList.begin(), m_att.defaultUnicastLocatorList.end(),
                [&](Locator_t& loc)
                {
                    m_network_Factory.fill_default_locator_port(loc, unicast_port);
                });
        m_network_Factory.NormalizeLocators(m_att.defaultUnicastLocatorList);

        // Locator with port 0, calculate port.
        uint32_t multicast_port = m_network_Factory.calculate_well_known_port(domain_id_, m_att, true);
        std::for_each(m_att.defaultMulticastLocatorList.begin(), m_att.defaultMulticastLocatorList.end(),
                [&](Locator_t& loc)
                {
                    m_network_Factory.fill_default_locator_port(loc, multicast_port);
                });
    }

    if (is_intraprocess_only())
    {
        m_att.defaultUnicastLocatorList.clear();
        m_att.defaultMulticastLocatorList.clear();
    }

    createReceiverResources(m_att.defaultUnicastLocatorList, true, false, true);
    createReceiverResources(m_att.defaultMulticastLocatorList, false, false, true);

    namespace external_locators = fastdds::rtps::network::external_locators;
    external_locators::set_listening_locators(m_att.default_external_unicast_locators,
            m_att.defaultUnicastLocatorList);
}

void RTPSParticipantImpl::setup_initial_peers()
{
    // Initial peers
    if (m_att.builtin.initialPeersList.empty())
    {
        m_att.builtin.initialPeersList = m_att.builtin.metatrafficMulticastLocatorList;
    }
    else
    {
        LocatorList_t initial_peers;
        initial_peers.swap(m_att.builtin.initialPeersList);

        std::for_each(initial_peers.begin(), initial_peers.end(),
                [&](Locator_t& locator)
                {
                    m_network_Factory.configureInitialPeerLocator(domain_id_, locator, m_att);
                });
    }
}

void RTPSParticipantImpl::setup_output_traffic()
{
    bool allow_growing_buffers = m_att.allocation.send_buffers.dynamic;
    size_t num_send_buffers = m_att.allocation.send_buffers.preallocated_number;
    if (num_send_buffers == 0)
    {
        // Two buffers (user, events)
        num_send_buffers = 2;
        // Add one buffer per reception thread
        num_send_buffers += m_receiverResourcelist.size();
    }

    // Create buffer pool
    send_buffers_.reset(new SendBuffersManager(num_send_buffers, allow_growing_buffers));
    send_buffers_->init(this);

    // Initialize flow controller factory.
    // This must be done after initiate network layer.
    flow_controller_factory_.init(this);

    // Support old API
    if (m_att.throughputController.bytesPerPeriod != UINT32_MAX && m_att.throughputController.periodMillisecs != 0)
    {
        fastdds::rtps::FlowControllerDescriptor old_descriptor;
        old_descriptor.name = guid_str_.c_str();
        old_descriptor.max_bytes_per_period = m_att.throughputController.bytesPerPeriod;
        old_descriptor.period_ms = m_att.throughputController.periodMillisecs;
        flow_controller_factory_.register_flow_controller(old_descriptor);
    }

    // Register user's flow controllers.
    for (auto flow_controller_desc : m_att.flow_controllers)
    {
        flow_controller_factory_.register_flow_controller(*flow_controller_desc.get());
    }
}

void RTPSParticipantImpl::enable()
{
    mp_builtinProtocols->enable();

    //Start reception
    for (auto& receiver : m_receiverResourcelist)
    {
        receiver.Receiver->RegisterReceiver(receiver.mp_receiver);
    }
}

void RTPSParticipantImpl::disable()
{
    // Disabling event thread also disables participant announcement, so there is no need to call
    // stopRTPSParticipantAnnouncement()
    mp_event_thr.stop_thread();

    // Disable Retries on Transports
    m_network_Factory.Shutdown();

    // Safely abort threads.
    for (auto& block : m_receiverResourcelist)
    {
        block.Receiver->UnregisterReceiver(block.mp_receiver);
        block.disable();
    }

    deleteAllUserEndpoints();

    if (nullptr != mp_builtinProtocols)
    {
        delete(mp_builtinProtocols);
        mp_builtinProtocols = nullptr;
    }
}

const std::vector<RTPSWriter*>& RTPSParticipantImpl::getAllWriters() const
{
    return m_allWriterList;
}

const std::vector<RTPSReader*>& RTPSParticipantImpl::getAllReaders() const
{
    return m_allReaderList;
}

RTPSParticipantImpl::~RTPSParticipantImpl()
{
    disable();

#if HAVE_SECURITY
    m_security_manager.destroy();
#endif // if HAVE_SECURITY

    // Destruct message receivers
    for (auto& block : m_receiverResourcelist)
    {
        delete block.mp_receiver;
    }
    m_receiverResourcelist.clear();

    delete mp_userParticipant;
    mp_userParticipant = nullptr;
    send_resource_list_.clear();

    delete mp_mutex;
}

template <EndpointKind_t kind, octet no_key, octet with_key>
bool RTPSParticipantImpl::preprocess_endpoint_attributes(
        const EntityId_t& entity_id,
        std::atomic<uint32_t>& id_counter,
        EndpointAttributes& att,
        EntityId_t& entId)
{
    const char* debug_label = (att.endpointKind == WRITER ? "writer" : "reader");

    if (!att.unicastLocatorList.isValid())
    {
        EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "Unicast Locator List for " << debug_label << " contains invalid Locator");
        return false;
    }
    if (!att.multicastLocatorList.isValid())
    {
        EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT,
                "Multicast Locator List for " << debug_label << " contains invalid Locator");
        return false;
    }
    if (!att.remoteLocatorList.isValid())
    {
        EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "Remote Locator List for " << debug_label << " contains invalid Locator");
        return false;
    }

    if (entity_id == c_EntityId_Unknown)
    {
        if (att.topicKind == NO_KEY)
        {
            entId.value[3] = (-2 == att.getUserDefinedID() && 0 < att.getEntityID()) ? (0x60) | no_key : no_key;
        }
        else if (att.topicKind == WITH_KEY)
        {
            entId.value[3] = (-2 == att.getUserDefinedID() && 0 < att.getEntityID()) ? (0x60) | with_key : with_key;
        }
        uint32_t idnum;
        if (att.getEntityID() > 0)
        {
            idnum = static_cast<uint32_t>(att.getEntityID());
        }
        else
        {
            idnum = ++id_counter;
        }

        entId.value[2] = octet(idnum);
        entId.value[1] = octet(idnum >> 8);
        entId.value[0] = octet(idnum >> 16);
    }
    else
    {
        entId = entity_id;
    }

    if (att.persistence_guid == c_Guid_Unknown)
    {
        // Try to load persistence_guid from property
        const std::string* persistence_guid_property = PropertyPolicyHelper::find_property(
            att.properties, "dds.persistence.guid");
        if (persistence_guid_property != nullptr)
        {
            // Load persistence_guid from property
            std::istringstream(persistence_guid_property->c_str()) >> att.persistence_guid;
            if (att.persistence_guid == c_Guid_Unknown)
            {
                // Wrongly configured property
                EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "Cannot configure " << debug_label << "'s persistence GUID from '"
                                                                         << persistence_guid_property->c_str()
                                                                         << "'. Wrong input");
                return false;
            }
        }
    }

    // Error log level can be disable. Avoid unused warning
    static_cast<void>(debug_label);

    return true;
}

template<typename Functor>
bool RTPSParticipantImpl::create_writer(
        RTPSWriter** writer_out,
        WriterAttributes& param,
        const EntityId_t& entity_id,
        bool is_builtin,
        const Functor& callback)
{
    std::string type = (param.endpoint.reliabilityKind == RELIABLE) ? "RELIABLE" : "BEST_EFFORT";
    EPROSIMA_LOG_INFO(RTPS_PARTICIPANT, "Creating writer of type " << type);
    EntityId_t entId;
    if (!preprocess_endpoint_attributes<WRITER, 0x03, 0x02>(entity_id, IdCounter, param.endpoint, entId))
    {
        return false;
    }

    if (existsEntityId(entId, WRITER))
    {
        EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT,
                "A writer with the same entityId already exists in this RTPSParticipant");
        return false;
    }

    GUID_t guid(m_guid.guidPrefix, entId);
    fastdds::rtps::FlowController* flow_controller = nullptr;
    const char* flow_controller_name = param.flow_controller_name;

    // Support of old flow controller style.
    if (param.throughputController.bytesPerPeriod != UINT32_MAX && param.throughputController.periodMillisecs != 0)
    {
        flow_controller_name = guid_str_.c_str();
        if (ASYNCHRONOUS_WRITER == param.mode)
        {
            fastdds::rtps::FlowControllerDescriptor old_descriptor;
            old_descriptor.name = guid_str_.c_str();
            old_descriptor.max_bytes_per_period = param.throughputController.bytesPerPeriod;
            old_descriptor.period_ms = param.throughputController.periodMillisecs;
            flow_controller_factory_.register_flow_controller(old_descriptor);
            flow_controller =  flow_controller_factory_.retrieve_flow_controller(guid_str_.c_str(), param);
        }
        else
        {
            EPROSIMA_LOG_WARNING(RTPS_PARTICIPANT,
                    "Throughput flow controller was configured while writer's publish mode is configured as synchronous." \
                    "Throughput flow controller configuration is not taken into account.");

        }
    }
    if (m_att.throughputController.bytesPerPeriod != UINT32_MAX && m_att.throughputController.periodMillisecs != 0)
    {
        if (ASYNCHRONOUS_WRITER == param.mode && nullptr == flow_controller)
        {
            flow_controller_name = guid_str_.c_str();
            flow_controller = flow_controller_factory_.retrieve_flow_controller(guid_str_, param);
        }
        else
        {
            EPROSIMA_LOG_WARNING(RTPS_PARTICIPANT,
                    "Throughput flow controller was configured while writer's publish mode is configured as synchronous." \
                    "Throughput flow controller configuration is not taken into account.");
        }
    }

    // Retrieve flow controller.
    // If not default flow controller, publish_mode must be asynchronously.
    if (nullptr == flow_controller &&
            (fastdds::rtps::FASTDDS_FLOW_CONTROLLER_DEFAULT == flow_controller_name ||
            ASYNCHRONOUS_WRITER == param.mode))
    {
        flow_controller = flow_controller_factory_.retrieve_flow_controller(flow_controller_name, param);
    }

    if (nullptr == flow_controller)
    {
        if (fastdds::rtps::FASTDDS_FLOW_CONTROLLER_DEFAULT != flow_controller_name &&
                SYNCHRONOUS_WRITER == param.mode)
        {
            EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "Cannot use a flow controller in synchronously publication mode.");
        }
        else
        {
            EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "Cannot create the writer. Couldn't find flow controller "
                    << flow_controller_name << " for writer.");
        }
        return false;
    }

    // Check for unique_network_flows feature
    if (nullptr != PropertyPolicyHelper::find_property(param.endpoint.properties, "fastdds.unique_network_flows"))
    {
        EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "Unique network flows not supported on writers");
        return false;
    }

    // Special case for DiscoveryProtocol::BACKUP, which abuses persistence guid
    GUID_t former_persistence_guid = param.endpoint.persistence_guid;
    if (param.endpoint.persistence_guid == c_Guid_Unknown)
    {
        if (m_persistence_guid != c_Guid_Unknown)
        {
            // Generate persistence guid from participant persistence guid
            param.endpoint.persistence_guid = GUID_t(
                m_persistence_guid.guidPrefix,
                entity_id);
        }
    }

    // Get persistence service
    IPersistenceService* persistence = nullptr;
    if (!get_persistence_service(is_builtin, param.endpoint, persistence))
    {
        return false;
    }

    normalize_endpoint_locators(param.endpoint);

    RTPSWriter* SWriter = nullptr;
    SWriter = callback(guid, param, flow_controller, persistence, param.endpoint.reliabilityKind == RELIABLE);

    // restore attributes
    param.endpoint.persistence_guid = former_persistence_guid;

    if (SWriter == nullptr)
    {
        return false;
    }

    if (!SWriter->is_pool_initialized())
    {
        delete(SWriter);
        return false;
    }

    // Use participant's external locators if writer has none
    // WARNING: call before createAndAssociateReceiverswithEndpoint, as the latter intentionally clears external
    // locators list when using unique_flows feature
    setup_external_locators(SWriter);

#if HAVE_SECURITY
    if (!is_builtin)
    {
        if (!m_security_manager.register_local_writer(SWriter->getGuid(),
                param.endpoint.properties, SWriter->getAttributes().security_attributes()))
        {
            delete(SWriter);
            return false;
        }
    }
    else
    {
        if (!m_security_manager.register_local_builtin_writer(SWriter->getGuid(),
                SWriter->getAttributes().security_attributes()))
        {
            delete(SWriter);
            return false;
        }
    }
#endif // if HAVE_SECURITY

    createSendResources(SWriter);
    if (param.endpoint.reliabilityKind == RELIABLE)
    {
        if (!createAndAssociateReceiverswithEndpoint(SWriter))
        {
            delete(SWriter);
            return false;
        }
    }

    {
        std::lock_guard<shared_mutex> _(endpoints_list_mutex);
        m_allWriterList.push_back(SWriter);

        if (!is_builtin)
        {
            m_userWriterList.push_back(SWriter);
        }
    }
    *writer_out = SWriter;

#ifdef FASTDDS_STATISTICS

    if (!is_builtin)
    {
        // Register all compatible statistical listeners
        for_each_listener([this, &guid](Key listener)
                {
                    if (are_writers_involved(listener->mask()))
                    {
                        register_in_writer(listener->get_shared_ptr(), guid);
                    }
                });

        SWriter->set_enabled_statistics_writers_mask(StatisticsParticipantImpl::get_enabled_statistics_writers_mask());
    }

#endif // FASTDDS_STATISTICS

    return true;
}

template <typename Functor>
bool RTPSParticipantImpl::create_reader(
        RTPSReader** reader_out,
        ReaderAttributes& param,
        const EntityId_t& entity_id,
        bool is_builtin,
        bool enable,
        const Functor& callback)
{
    std::string type = (param.endpoint.reliabilityKind == RELIABLE) ? "RELIABLE" : "BEST_EFFORT";
    EPROSIMA_LOG_INFO(RTPS_PARTICIPANT, "Creating reader of type " << type);
    EntityId_t entId;
    if (!preprocess_endpoint_attributes<READER, 0x04, 0x07>(entity_id, IdCounter, param.endpoint, entId))
    {
        return false;
    }

    if (existsEntityId(entId, READER))
    {
        EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT,
                "A reader with the same entityId already exists in this RTPSParticipant");
        return false;
    }

    // Special case for DiscoveryProtocol::BACKUP, which abuses persistence guid
    GUID_t former_persistence_guid = param.endpoint.persistence_guid;
    if (param.endpoint.persistence_guid == c_Guid_Unknown)
    {
        if (m_persistence_guid != c_Guid_Unknown)
        {
            // Generate persistence guid from participant persistence guid
            param.endpoint.persistence_guid = GUID_t(
                m_persistence_guid.guidPrefix,
                entity_id);
        }
    }

    // Get persistence service
    IPersistenceService* persistence = nullptr;
    if (!get_persistence_service(is_builtin, param.endpoint, persistence))
    {
        return false;
    }

    // Check for unique_network_flows feature
    bool request_unique_flows = false;
    uint16_t initial_port = 0;
    uint16_t final_port = 0;
    if (!get_unique_flows_parameters(m_att, param.endpoint, request_unique_flows, initial_port, final_port))
    {
        return false;
    }

    normalize_endpoint_locators(param.endpoint);

    RTPSReader* SReader = nullptr;
    GUID_t guid(m_guid.guidPrefix, entId);
    SReader = callback(guid, param, persistence, param.endpoint.reliabilityKind == RELIABLE);

    // restore attributes
    param.endpoint.persistence_guid = former_persistence_guid;

    if (SReader == nullptr)
    {
        return false;
    }

    // Use participant's external locators if reader has none
    // WARNING: call before createAndAssociateReceiverswithEndpoint, as the latter intentionally clears external
    // locators list when using unique_flows feature
    setup_external_locators(SReader);

#if HAVE_SECURITY

    if (!is_builtin)
    {
        if (!m_security_manager.register_local_reader(SReader->getGuid(),
                param.endpoint.properties, SReader->getAttributes().security_attributes()))
        {
            delete(SReader);
            return false;
        }
    }
    else
    {
        if (!m_security_manager.register_local_builtin_reader(SReader->getGuid(),
                SReader->getAttributes().security_attributes()))
        {
            delete(SReader);
            return false;
        }
    }
#endif // if HAVE_SECURITY

    if (param.endpoint.reliabilityKind == RELIABLE)
    {
        createSendResources(SReader);
    }

    if (is_builtin)
    {
        SReader->setTrustedWriter(TrustedWriter(SReader->getGuid().entityId));
    }

    if (enable)
    {
        if (!createAndAssociateReceiverswithEndpoint(SReader, request_unique_flows, initial_port, final_port))
        {
            delete(SReader);
            return false;
        }
    }

    {
        std::lock_guard<shared_mutex> _(endpoints_list_mutex);

        m_allReaderList.push_back(SReader);

        if (!is_builtin)
        {
            m_userReaderList.push_back(SReader);
        }
    }
    *reader_out = SReader;

#ifdef FASTDDS_STATISTICS

    if (!is_builtin)
    {
        // Register all compatible statistical listeners
        for_each_listener([this, &guid](Key listener)
                {
                    if (are_readers_involved(listener->mask()))
                    {
                        register_in_reader(listener->get_shared_ptr(), guid);
                    }
                });

        SReader->set_enabled_statistics_writers_mask(StatisticsParticipantImpl::get_enabled_statistics_writers_mask());
    }

#endif // FASTDDS_STATISTICS

    return true;
}

/*
 *
 * MAIN RTPSParticipant IMPL API
 *
 */
bool RTPSParticipantImpl::createWriter(
        RTPSWriter** WriterOut,
        WriterAttributes& param,
        WriterHistory* hist,
        WriterListener* listen,
        const EntityId_t& entityId,
        bool isBuiltin)
{
    auto callback = [hist, listen, this]
                (const GUID_t& guid, WriterAttributes& param, fastdds::rtps::FlowController* flow_controller,
                    IPersistenceService* persistence, bool is_reliable) -> RTPSWriter*
            {
                if (is_reliable)
                {
                    if (persistence != nullptr)
                    {
                        return new StatefulPersistentWriter(this, guid, param, flow_controller,
                                       hist, listen, persistence);
                    }
                    else
                    {
                        return new StatefulWriter(this, guid, param, flow_controller,
                                       hist, listen);
                    }
                }
                else
                {
                    if (persistence != nullptr)
                    {
                        return new StatelessPersistentWriter(this, guid, param, flow_controller,
                                       hist, listen, persistence);
                    }
                    else
                    {
                        return new StatelessWriter(this, guid, param, flow_controller,
                                       hist, listen);
                    }
                }
            };
    return create_writer(WriterOut, param, entityId, isBuiltin, callback);
}

bool RTPSParticipantImpl::createWriter(
        RTPSWriter** WriterOut,
        WriterAttributes& param,
        const std::shared_ptr<IPayloadPool>& payload_pool,
        WriterHistory* hist,
        WriterListener* listen,
        const EntityId_t& entityId,
        bool isBuiltin)
{
    if (!payload_pool)
    {
        EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "Trying to create writer with null payload pool");
        return false;
    }

    auto callback = [hist, listen, &payload_pool, this]
                (const GUID_t& guid, WriterAttributes& param, fastdds::rtps::FlowController* flow_controller,
                    IPersistenceService* persistence, bool is_reliable) -> RTPSWriter*
            {
                if (is_reliable)
                {
                    if (persistence != nullptr)
                    {
                        return new StatefulPersistentWriter(this, guid, param, payload_pool, flow_controller,
                                       hist, listen, persistence);
                    }
                    else
                    {
                        return new StatefulWriter(this, guid, param, payload_pool, flow_controller,
                                       hist, listen);
                    }
                }
                else
                {
                    if (persistence != nullptr)
                    {
                        return new StatelessPersistentWriter(this, guid, param, payload_pool, flow_controller,
                                       hist, listen, persistence);
                    }
                    else
                    {
                        return new StatelessWriter(this, guid, param, payload_pool, flow_controller,
                                       hist, listen);
                    }
                }
            };
    return create_writer(WriterOut, param, entityId, isBuiltin, callback);
}

bool RTPSParticipantImpl::create_writer(
        RTPSWriter** WriterOut,
        WriterAttributes& watt,
        const std::shared_ptr<IPayloadPool>& payload_pool,
        const std::shared_ptr<IChangePool>& change_pool,
        WriterHistory* hist,
        WriterListener* listen,
        const EntityId_t& entityId,
        bool isBuiltin)
{
    if (!payload_pool)
    {
        EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "Trying to create writer with null payload pool");
        return false;
    }

    auto callback = [hist, listen, &payload_pool, &change_pool, this]
                (const GUID_t& guid, WriterAttributes& watt, fastdds::rtps::FlowController* flow_controller,
                    IPersistenceService* persistence, bool is_reliable) -> RTPSWriter*
            {
                if (is_reliable)
                {
                    if (persistence != nullptr)
                    {
                        return new StatefulPersistentWriter(this, guid, watt, payload_pool, change_pool,
                                       flow_controller, hist, listen, persistence);
                    }
                    else
                    {
                        return new StatefulWriter(this, guid, watt, payload_pool, change_pool,
                                       flow_controller, hist, listen);
                    }
                }
                else
                {
                    if (persistence != nullptr)
                    {
                        return new StatelessPersistentWriter(this, guid, watt, payload_pool, change_pool,
                                       flow_controller, hist, listen, persistence);
                    }
                    else
                    {
                        return new StatelessWriter(this, guid, watt, payload_pool, change_pool,
                                       flow_controller, hist, listen);
                    }
                }
            };
    return create_writer(WriterOut, watt, entityId, isBuiltin, callback);
}

bool RTPSParticipantImpl::createReader(
        RTPSReader** ReaderOut,
        ReaderAttributes& param,
        ReaderHistory* hist,
        ReaderListener* listen,
        const EntityId_t& entityId,
        bool isBuiltin,
        bool enable)
{
    auto callback = [hist, listen, this]
                (const GUID_t& guid, ReaderAttributes& param, IPersistenceService* persistence,
                    bool is_reliable) -> RTPSReader*
            {
                if (is_reliable)
                {
                    if (persistence != nullptr)
                    {
                        return new StatefulPersistentReader(this, guid, param, hist, listen, persistence);
                    }
                    else
                    {
                        return new StatefulReader(this, guid, param, hist, listen);
                    }
                }
                else
                {
                    if (persistence != nullptr)
                    {
                        return new StatelessPersistentReader(this, guid, param, hist, listen, persistence);
                    }
                    else
                    {
                        return new StatelessReader(this, guid, param, hist, listen);
                    }
                }
            };
    return create_reader(ReaderOut, param, entityId, isBuiltin, enable, callback);
}

bool RTPSParticipantImpl::createReader(
        RTPSReader** ReaderOut,
        ReaderAttributes& param,
        const std::shared_ptr<IPayloadPool>& payload_pool,
        ReaderHistory* hist,
        ReaderListener* listen,
        const EntityId_t& entityId,
        bool isBuiltin,
        bool enable)
{
    if (!payload_pool)
    {
        EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "Trying to create reader with null payload pool");
        return false;
    }

    auto callback = [hist, listen, &payload_pool, this]
                (const GUID_t& guid, ReaderAttributes& param, IPersistenceService* persistence,
                    bool is_reliable) -> RTPSReader*
            {
                if (is_reliable)
                {
                    if (persistence != nullptr)
                    {
                        return new StatefulPersistentReader(this, guid, param, payload_pool, hist, listen, persistence);
                    }
                    else
                    {
                        return new StatefulReader(this, guid, param, payload_pool, hist, listen);
                    }
                }
                else
                {
                    if (persistence != nullptr)
                    {
                        return new StatelessPersistentReader(this, guid, param, payload_pool, hist, listen,
                                       persistence);
                    }
                    else
                    {
                        return new StatelessReader(this, guid, param, payload_pool, hist, listen);
                    }
                }
            };
    return create_reader(ReaderOut, param, entityId, isBuiltin, enable, callback);
}

RTPSReader* RTPSParticipantImpl::find_local_reader(
        const GUID_t& reader_guid)
{
    shared_lock<shared_mutex> _(endpoints_list_mutex);

    for (auto reader : m_allReaderList)
    {
        if (reader->getGuid() == reader_guid)
        {
            return reader;
        }
    }

    return nullptr;
}

RTPSWriter* RTPSParticipantImpl::find_local_writer(
        const GUID_t& writer_guid)
{
    shared_lock<shared_mutex> _(endpoints_list_mutex);

    for (auto writer : m_allWriterList)
    {
        if (writer->getGuid() == writer_guid)
        {
            return writer;
        }
    }

    return nullptr;
}

bool RTPSParticipantImpl::enableReader(
        RTPSReader* reader)
{
    if (!assignEndpointListenResources(reader))
    {
        return false;
    }
    return true;
}

// Avoid to receive PDPSimple reader a DATA while calling ~PDPSimple and EDP was destroy already.
void RTPSParticipantImpl::disableReader(
        RTPSReader* reader)
{
    m_receiverResourcelistMutex.lock();
    for (auto it = m_receiverResourcelist.begin(); it != m_receiverResourcelist.end(); ++it)
    {
        it->mp_receiver->removeEndpoint(reader);
    }
    m_receiverResourcelistMutex.unlock();
}

bool RTPSParticipantImpl::registerWriter(
        RTPSWriter* Writer,
        const TopicAttributes& topicAtt,
        const WriterQos& wqos)
{
    return this->mp_builtinProtocols->addLocalWriter(Writer, topicAtt, wqos);
}

bool RTPSParticipantImpl::registerReader(
        RTPSReader* reader,
        const TopicAttributes& topicAtt,
        const ReaderQos& rqos,
        const fastdds::rtps::ContentFilterProperty* content_filter)
{
    return this->mp_builtinProtocols->addLocalReader(reader, topicAtt, rqos, content_filter);
}

void RTPSParticipantImpl::update_attributes(
        const RTPSParticipantAttributes& patt)
{
    bool local_interfaces_changed = false;

    // Update cached network interfaces
    if (!SystemInfo::update_interfaces())
    {
        EPROSIMA_LOG_WARNING(RTPS_PARTICIPANT,
                "Failed to update cached network interfaces during " << m_att.getName() <<
                " attributes update");
    }

    // Check if new interfaces have been added
    if (internal_metatraffic_locators_)
    {
        LocatorList_t metatraffic_unicast_locator_list = m_att.builtin.metatrafficUnicastLocatorList;
        get_default_metatraffic_locators();
        if (!(metatraffic_unicast_locator_list == m_att.builtin.metatrafficUnicastLocatorList))
        {
            local_interfaces_changed = true;
            EPROSIMA_LOG_INFO(RTPS_PARTICIPANT, m_att.getName() << " updated its metatraffic locators");
        }
    }
    if (internal_default_locators_)
    {
        LocatorList_t default_unicast_locator_list = m_att.defaultUnicastLocatorList;
        get_default_unicast_locators();
        if (!(default_unicast_locator_list == m_att.defaultUnicastLocatorList))
        {
            local_interfaces_changed = true;
            EPROSIMA_LOG_INFO(RTPS_PARTICIPANT,
                    m_att.getName() << " updated default unicast locator list, current locators: "
                                    << m_att.defaultUnicastLocatorList);
        }
    }

    if (local_interfaces_changed)
    {
        m_network_Factory.update_network_interfaces();
    }

    auto pdp = mp_builtinProtocols->mp_PDP;
    bool update_pdp = false;

    // Check if discovery servers need to be updated
    eprosima::fastdds::rtps::RemoteServerList_t converted_discovery_servers =
            patt.builtin.discovery_config.m_DiscoveryServers;
    if (patt.builtin.discovery_config.m_DiscoveryServers != m_att.builtin.discovery_config.m_DiscoveryServers)
    {
        for (auto& transportDescriptor : m_att.userTransports)
        {
            TCPTransportDescriptor* pT = dynamic_cast<TCPTransportDescriptor*>(transportDescriptor.get());
            if (pT)
            {
                for (fastdds::rtps::RemoteServerAttributes& it : converted_discovery_servers)
                {
                    std::for_each(it.metatrafficUnicastLocatorList.begin(),
                            it.metatrafficUnicastLocatorList.end(), [&](Locator_t& locator)
                            {
                                // TCP DS default logical port is the same as the physical one
                                if (locator.kind == LOCATOR_KIND_TCPv4 || locator.kind == LOCATOR_KIND_TCPv6)
                                {
                                    if (IPLocator::getLogicalPort(locator) == 0)
                                    {
                                        IPLocator::setLogicalPort(locator, IPLocator::getPhysicalPort(locator));
                                    }
                                }
                            });
                }
            }
        }
    }

    // Check if there are changes
    if (converted_discovery_servers != m_att.builtin.discovery_config.m_DiscoveryServers
            || patt.userData != m_att.userData
            || local_interfaces_changed)
    {
        update_pdp = true;
        std::vector<GUID_t> modified_servers;
        LocatorList_t modified_locators;

        // Update RTPSParticipantAttributes members
        m_att.userData = patt.userData;

        // If there's no PDP don't process Discovery-related attributes.
        if (!pdp)
        {
            return;
        }

        // Update listening locators on external locators
        {
            using namespace fastdds::rtps::network::external_locators;
            if (local_interfaces_changed && internal_metatraffic_locators_)
            {
                set_listening_locators(m_att.builtin.metatraffic_external_unicast_locators,
                        m_att.builtin.metatrafficUnicastLocatorList);
            }
            if (local_interfaces_changed && internal_default_locators_)
            {
                set_listening_locators(m_att.default_external_unicast_locators,
                        m_att.defaultUnicastLocatorList);
            }
        }

        // Check that the remote servers list is consistent: all the already known remote servers must be included in
        // the list and either new remote servers are added or remote server listening locator is modified.
        for (auto existing_server : m_att.builtin.discovery_config.m_DiscoveryServers)
        {
            bool contained = false;
            for (auto incoming_server : converted_discovery_servers)
            {
                if (existing_server.guidPrefix == incoming_server.guidPrefix)
                {
                    for (auto incoming_locator : incoming_server.metatrafficUnicastLocatorList)
                    {
                        bool locator_contained = false;
                        for (auto existing_locator : existing_server.metatrafficUnicastLocatorList)
                        {
                            if (incoming_locator == existing_locator)
                            {
                                locator_contained = true;
                                break;
                            }
                        }
                        if (!locator_contained)
                        {
                            modified_servers.emplace_back(incoming_server.GetParticipant());
                            modified_locators.push_back(incoming_locator);
                            EPROSIMA_LOG_INFO(RTPS_QOS_CHECK,
                                    "DS Server: " << incoming_server.guidPrefix << " has modified its locators: "
                                                  << incoming_locator << " being added");
                        }
                    }
                    contained = true;
                    break;
                }
            }
            if (!contained)
            {
                EPROSIMA_LOG_ERROR(RTPS_QOS_CHECK,
                        "Discovery Servers cannot be removed from the list; they can only be added");
                return;
            }
        }

        {
            std::lock_guard<std::recursive_mutex> lock(*pdp->getMutex());

            // Update user data
            auto local_participant_proxy_data = pdp->getLocalParticipantProxyData();
            local_participant_proxy_data->m_userData.data_vec(m_att.userData);

            // Update metatraffic locators
            for (auto locator : m_att.builtin.metatrafficMulticastLocatorList)
            {
                local_participant_proxy_data->metatraffic_locators.add_multicast_locator(locator);
            }
            for (auto locator : m_att.builtin.metatrafficUnicastLocatorList)
            {
                local_participant_proxy_data->metatraffic_locators.add_unicast_locator(locator);
            }

            // Update default locators
            for (auto locator : m_att.defaultUnicastLocatorList)
            {
                local_participant_proxy_data->default_locators.add_unicast_locator(locator);
            }

            if (local_interfaces_changed)
            {
                createSenderResources(m_att.builtin.metatrafficMulticastLocatorList);
                createSenderResources(m_att.builtin.metatrafficUnicastLocatorList);
                createSenderResources(m_att.defaultUnicastLocatorList);
            }
            if (!modified_locators.empty())
            {
                createSenderResources(modified_locators);
            }

            // Update remote servers list
            if (m_att.builtin.discovery_config.discoveryProtocol == DiscoveryProtocol::CLIENT ||
                    m_att.builtin.discovery_config.discoveryProtocol == DiscoveryProtocol::SUPER_CLIENT ||
                    m_att.builtin.discovery_config.discoveryProtocol == DiscoveryProtocol::SERVER ||
                    m_att.builtin.discovery_config.discoveryProtocol == DiscoveryProtocol::BACKUP)
            {
                // Add incoming servers if we don't know about them already or the listening locator has been modified
                for (auto incoming_server : converted_discovery_servers)
                {
                    eprosima::fastdds::rtps::RemoteServerList_t::iterator server_it;
                    for (server_it = m_att.builtin.discovery_config.m_DiscoveryServers.begin();
                            server_it != m_att.builtin.discovery_config.m_DiscoveryServers.end(); server_it++)
                    {
                        if (server_it->guidPrefix == incoming_server.guidPrefix)
                        {
                            // Check if the listening locators have been modified
                            for (auto guid : modified_servers)
                            {
                                if (guid == incoming_server.GetParticipant())
                                {
                                    server_it->metatrafficUnicastLocatorList =
                                            incoming_server.metatrafficUnicastLocatorList;
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    if (server_it == m_att.builtin.discovery_config.m_DiscoveryServers.end())
                    {
                        m_att.builtin.discovery_config.m_DiscoveryServers.push_back(incoming_server);
                    }
                }

                // Update the servers list in builtin protocols
                {
                    std::unique_lock<eprosima::shared_mutex> disc_lock(mp_builtinProtocols->getDiscoveryMutex());
                    mp_builtinProtocols->m_DiscoveryServers = m_att.builtin.discovery_config.m_DiscoveryServers;
                }

                // Notify PDPServer
                if (m_att.builtin.discovery_config.discoveryProtocol == DiscoveryProtocol::SERVER ||
                        m_att.builtin.discovery_config.discoveryProtocol == DiscoveryProtocol::BACKUP)
                {
                    fastdds::rtps::PDPServer* pdp_server = static_cast<fastdds::rtps::PDPServer*>(pdp);
                    pdp_server->update_remote_servers_list();
                    for (auto remote_server : modified_servers)
                    {
                        pdp_server->remove_remote_participant(remote_server,
                                ParticipantDiscoveryInfo::DISCOVERY_STATUS::DROPPED_PARTICIPANT);
                    }
                }
                // Notify PDPClient
                else if (m_att.builtin.discovery_config.discoveryProtocol == DiscoveryProtocol::CLIENT ||
                        m_att.builtin.discovery_config.discoveryProtocol == DiscoveryProtocol::SUPER_CLIENT)
                {
                    fastdds::rtps::PDPClient* pdp_client = static_cast<fastdds::rtps::PDPClient*>(pdp);
                    pdp_client->update_remote_servers_list();
                    for (auto remote_server : modified_servers)
                    {
                        pdp_client->remove_remote_participant(remote_server,
                                ParticipantDiscoveryInfo::DISCOVERY_STATUS::DROPPED_PARTICIPANT);
                    }
                }
            }
        }
    }

    if (update_pdp)
    {
        // Send DATA(P)
        pdp->announceParticipantState(true);
    }
}

bool RTPSParticipantImpl::updateLocalWriter(
        RTPSWriter* Writer,
        const TopicAttributes& topicAtt,
        const WriterQos& wqos)
{
    return this->mp_builtinProtocols->updateLocalWriter(Writer, topicAtt, wqos);
}

bool RTPSParticipantImpl::updateLocalReader(
        RTPSReader* reader,
        const TopicAttributes& topicAtt,
        const ReaderQos& rqos,
        const fastdds::rtps::ContentFilterProperty* content_filter)
{
    return this->mp_builtinProtocols->updateLocalReader(reader, topicAtt, rqos, content_filter);
}

/*
 *
 * AUXILIARY METHODS
 *
 *
 */

bool RTPSParticipantImpl::existsEntityId(
        const EntityId_t& ent,
        EndpointKind_t kind) const
{

    auto check = [&ent](Endpoint* e)
            {
                return ent == e->getGuid().entityId;
            };

    shared_lock<shared_mutex> _(endpoints_list_mutex);

    if (kind == WRITER)
    {
        return std::any_of(m_userWriterList.begin(), m_userWriterList.end(), check);
    }
    else
    {
        return std::any_of(m_userReaderList.begin(), m_userReaderList.end(), check);
    }

    return false;
}

/*
 *
 * RECEIVER RESOURCE METHODS
 *
 */
bool RTPSParticipantImpl::assignEndpointListenResources(
        Endpoint* endp)
{
    //Tag the endpoint with the ReceiverResources
    bool valid = true;

    /* No need to check for emptiness on the lists, as it was already done on part function
       In case are using the default list of Locators they have already been embedded to the parameters
     */

    //UNICAST
    assignEndpoint2LocatorList(endp, endp->getAttributes().unicastLocatorList);
    //MULTICAST
    assignEndpoint2LocatorList(endp, endp->getAttributes().multicastLocatorList);
    return valid;
}

bool RTPSParticipantImpl::createAndAssociateReceiverswithEndpoint(
        Endpoint* pend,
        bool unique_flows,
        uint16_t initial_unique_port,
        uint16_t final_unique_port)
{
    /*	This function...
        - Asks the network factory for new resources
        - Encapsulates the new resources within the ReceiverControlBlock list
        - Associated the endpoint to the new elements in the list
        - Launches the listener thread
     */

    auto& attributes = pend->getAttributes();
    if (unique_flows)
    {
        attributes.multicastLocatorList.clear();
        attributes.unicastLocatorList = m_att.defaultUnicastLocatorList;
        attributes.external_unicast_locators.clear();

        uint16_t port = initial_unique_port;
        while (port < final_unique_port)
        {
            // Set port on unicast locators
            for (Locator_t& loc : attributes.unicastLocatorList)
            {
                loc.port = port;
            }

            // Try creating receiver resources
            LocatorList_t aux_locator_list = attributes.unicastLocatorList;
            if (createReceiverResources(aux_locator_list, false, true, false))
            {
                break;
            }

            // Try with next port
            ++port;
        }

        // Fail when unique ports are exhausted
        if (port >= final_unique_port)
        {
            EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "Unique flows requested but exhausted. Port range: "
                    << initial_unique_port << "-" << final_unique_port);
            return false;
        }
    }
    else
    {
        // 1 - Ask the network factory to generate the elements that do still not exist
        //Iterate through the list of unicast and multicast locators the endpoint has... unless its empty
        //In that case, just use the standard
        if (attributes.unicastLocatorList.empty() && attributes.multicastLocatorList.empty())
        {
            // Take default locators from the participant.
            attributes.unicastLocatorList = m_att.defaultUnicastLocatorList;
            attributes.multicastLocatorList = m_att.defaultMulticastLocatorList;
            attributes.external_unicast_locators = m_att.default_external_unicast_locators;
        }
        createReceiverResources(attributes.unicastLocatorList, false, true, true);
        createReceiverResources(attributes.multicastLocatorList, false, true, true);
    }

    fastdds::rtps::network::external_locators::set_listening_locators(attributes.external_unicast_locators,
            attributes.unicastLocatorList);

    // Associate the Endpoint with ReceiverControlBlock
    assignEndpointListenResources(pend);
    return true;
}

bool RTPSParticipantImpl::assignEndpoint2LocatorList(
        Endpoint* endp,
        LocatorList_t& list)
{
    /* Note:
       The previous version of this function associated (or created) ListenResources and added the endpoint to them.
       It then requested the list of Locators the Listener is listening to and appended to the LocatorList_t from the parameters.

       This has been removed because it is considered redundant. For ReceiveResources that listen on multiple interfaces, only
       one of the supported Locators is needed to make the match, and the case of new ListenResources being created has been removed
       since its the NetworkFactory the one that takes care of Resource creation.
     */
    for (auto lit = list.begin(); lit != list.end(); ++lit)
    {
        //Iteration of all Locators within the Locator list passed down as argument
        std::lock_guard<std::mutex> guard(m_receiverResourcelistMutex);
        //Check among ReceiverResources whether the locator is supported or not
        for (auto it = m_receiverResourcelist.begin(); it != m_receiverResourcelist.end(); ++it)
        {
            //Take mutex for the resource since we are going to interact with shared resources
            //std::lock_guard<std::mutex> guard((*it).mtx);
            if (it->Receiver->SupportsLocator(*lit))
            {
                //Supported! Take mutex and update lists - We maintain reader/writer discrimination just in case
                it->mp_receiver->associateEndpoint(endp);
                // end association between reader/writer and the receive resources
            }

        }
        //Finished iteratig through all ListenResources for a single Locator (from the parameter list).
        //Since this function is called after checking with NetFactory we do not have to create any more resource.
    }
    return true;
}

bool RTPSParticipantImpl::createSendResources(
        Endpoint* pend)
{
    if (pend->m_att.remoteLocatorList.empty())
    {
        // Adds the default locators of every registered transport.
        m_network_Factory.GetDefaultOutputLocators(pend->m_att.remoteLocatorList);
    }

    std::lock_guard<std::timed_mutex> guard(m_send_resources_mutex_);

    //Output locators have been specified, create them
    for (auto it = pend->m_att.remoteLocatorList.begin(); it != pend->m_att.remoteLocatorList.end(); ++it)
    {
        if (!m_network_Factory.build_send_resources(send_resource_list_, (*it)))
        {
            EPROSIMA_LOG_WARNING(RTPS_PARTICIPANT, "Cannot create send resource for endpoint remote locator (" <<
                    pend->getGuid() << ", " << (*it) << ")");
        }
    }

    return true;
}

void RTPSParticipantImpl::setup_external_locators(
        Endpoint* endpoint)
{
    auto& attributes = endpoint->getAttributes();
    if (attributes.external_unicast_locators.empty())
    {
        // Take external locators from the participant.
        attributes.external_unicast_locators = m_att.default_external_unicast_locators;
    }
}

bool RTPSParticipantImpl::createReceiverResources(
        LocatorList_t& Locator_list,
        bool ApplyMutation,
        bool RegisterReceiver,
        bool log_when_creation_fails)
{
    auto input_list = Locator_list;
    Locator_list.clear();

    std::vector<std::shared_ptr<ReceiverResource>> newItemsBuffer;
    bool ret_val = input_list.empty();

#if HAVE_SECURITY
    // An auxilary buffer is needed in the ReceiverResource to to decrypt the message,
    // that imposes a limit in the received messages size even if the transport allows (uint32_t) messages size.
    uint32_t max_receiver_buffer_size =
            is_secure() ? std::numeric_limits<uint16_t>::max() : (std::numeric_limits<uint32_t>::max)();
#else
    uint32_t max_receiver_buffer_size = (std::numeric_limits<uint32_t>::max)();
#endif // if HAVE_SECURITY

    for (auto it_loc = input_list.begin(); it_loc != input_list.end(); ++it_loc)
    {
        Locator_t loc = *it_loc;
        bool ret = m_network_Factory.BuildReceiverResources(loc, newItemsBuffer, max_receiver_buffer_size);
        if (!ret && ApplyMutation)
        {
            uint32_t tries = 0;
            while (!ret && (tries < m_att.builtin.mutation_tries))
            {
                tries++;
                applyLocatorAdaptRule(loc);
                ret = m_network_Factory.BuildReceiverResources(loc, newItemsBuffer, max_receiver_buffer_size);
            }
        }

        if (ret)
        {
            Locator_list.push_back(loc);
        }
        else if (log_when_creation_fails)
        {
            std::string postfix = ApplyMutation ? ". Applied mutation until: " + IPLocator::to_string(loc) : "";
            static_cast<void>(postfix); // Might be unused if log is disabled
            EPROSIMA_LOG_WARNING(RTPS_PARTICIPANT,
                    "Could not create the specified receiver resource for '" << *it_loc << "'" << postfix);
        }

        ret_val |= !newItemsBuffer.empty();

        for (auto it_buffer = newItemsBuffer.begin(); it_buffer != newItemsBuffer.end(); ++it_buffer)
        {
            std::lock_guard<std::mutex> lock(m_receiverResourcelistMutex);
            //Push the new items into the ReceiverResource buffer
            m_receiverResourcelist.emplace_back(*it_buffer);
            //Create and init the MessageReceiver
            auto mr = new MessageReceiver(this, (*it_buffer)->max_message_size());
            m_receiverResourcelist.back().mp_receiver = mr;
            //Start reception
            if (RegisterReceiver)
            {
                m_receiverResourcelist.back().Receiver->RegisterReceiver(mr);
            }
        }
        newItemsBuffer.clear();
    }

    return ret_val;
}

void RTPSParticipantImpl::createSenderResources(
        const LocatorList_t& locator_list)
{
    std::lock_guard<std::timed_mutex> lock(m_send_resources_mutex_);

    for (auto it_loc = locator_list.begin(); it_loc != locator_list.end(); ++it_loc)
    {
        m_network_Factory.build_send_resources(send_resource_list_, *it_loc);
    }
}

void RTPSParticipantImpl::createSenderResources(
        const Locator_t& locator)
{
    std::lock_guard<std::timed_mutex> lock(m_send_resources_mutex_);

    m_network_Factory.build_send_resources(send_resource_list_, locator);
}

void RTPSParticipantImpl::createSenderResources(
        const LocatorSelectorEntry& locator_selector_entry)
{
    std::lock_guard<std::timed_mutex> lock(m_send_resources_mutex_);

    m_network_Factory.build_send_resources(send_resource_list_, locator_selector_entry);
}

bool RTPSParticipantImpl::deleteUserEndpoint(
        const GUID_t& endpoint)
{
    if ( getGuid().guidPrefix != endpoint.guidPrefix)
    {
        return false;
    }

    bool found = false, found_in_users = false;
    Endpoint* p_endpoint = nullptr;

    if (endpoint.entityId.is_writer())
    {
        std::lock_guard<shared_mutex> _(endpoints_list_mutex);

        for (auto wit = m_userWriterList.begin(); wit != m_userWriterList.end(); ++wit)
        {
            if ((*wit)->getGuid().entityId == endpoint.entityId) //Found it
            {
                m_userWriterList.erase(wit);
                found_in_users = true;
                break;
            }
        }

        for (auto wit = m_allWriterList.begin(); wit != m_allWriterList.end(); ++wit)
        {
            if ((*wit)->getGuid().entityId == endpoint.entityId) //Found it
            {
                p_endpoint = *wit;
                m_allWriterList.erase(wit);
                found = true;
                break;
            }
        }
    }
    else
    {
        std::lock_guard<shared_mutex> _(endpoints_list_mutex);

        for (auto rit = m_userReaderList.begin(); rit != m_userReaderList.end(); ++rit)
        {
            if ((*rit)->getGuid().entityId == endpoint.entityId) //Found it
            {
                m_userReaderList.erase(rit);
                found_in_users = true;
                break;
            }
        }

        for (auto rit = m_allReaderList.begin(); rit != m_allReaderList.end(); ++rit)
        {
            if ((*rit)->getGuid().entityId == endpoint.entityId) //Found it
            {
                p_endpoint = *rit;
                m_allReaderList.erase(rit);
                found = true;
                break;
            }
        }
    }

    if (!found)
    {
        return false;
    }

    {
        std::lock_guard<std::mutex> _(m_receiverResourcelistMutex);

        for (auto& rb : m_receiverResourcelist)
        {
            auto receiver = rb.mp_receiver;
            if (receiver)
            {
                receiver->removeEndpoint(p_endpoint);
            }
        }
    }

    //REMOVE FROM BUILTIN PROTOCOLS
    if (p_endpoint->getAttributes().endpointKind == WRITER)
    {
        if (found_in_users)
        {
            mp_builtinProtocols->removeLocalWriter(static_cast<RTPSWriter*>(p_endpoint));
        }

#if HAVE_SECURITY
        if (p_endpoint->getAttributes().security_attributes().is_submessage_protected ||
                p_endpoint->getAttributes().security_attributes().is_payload_protected)
        {
            m_security_manager.unregister_local_writer(p_endpoint->getGuid());
        }
#endif // if HAVE_SECURITY
    }
    else
    {
        if (found_in_users)
        {
            mp_builtinProtocols->removeLocalReader(static_cast<RTPSReader*>(p_endpoint));
        }

#if HAVE_SECURITY
        if (p_endpoint->getAttributes().security_attributes().is_submessage_protected ||
                p_endpoint->getAttributes().security_attributes().is_payload_protected)
        {
            m_security_manager.unregister_local_reader(p_endpoint->getGuid());
        }
#endif // if HAVE_SECURITY
    }

    delete(p_endpoint);
    return true;
}

void RTPSParticipantImpl::deleteAllUserEndpoints()
{
    std::vector<Endpoint*> tmp(0);

    {
        using namespace std;

        lock_guard<shared_mutex> _(endpoints_list_mutex);

        // move the collections to a local
        tmp.resize(m_userWriterList.size() + m_userReaderList.size());
        auto it = move(m_userWriterList.begin(), m_userWriterList.end(), tmp.begin());
        it = move(m_userReaderList.begin(), m_userReaderList.end(), it);

        // check we have copied all elements
        assert(tmp.end() == it);

        // now update the all collections by removing the user elements
        sort(m_userWriterList.begin(), m_userWriterList.end());
        sort(m_userReaderList.begin(), m_userReaderList.end());
        sort(m_allWriterList.begin(), m_allWriterList.end());
        sort(m_allReaderList.begin(), m_allReaderList.end());

        vector<RTPSWriter*> writers;
        set_difference(m_allWriterList.begin(), m_allWriterList.end(),
                m_userWriterList.begin(), m_userWriterList.end(),
                back_inserter(writers));
        swap(writers, m_allWriterList);

        vector<RTPSReader*> readers;
        set_difference(m_allReaderList.begin(), m_allReaderList.end(),
                m_userReaderList.begin(), m_userReaderList.end(),
                back_inserter(readers));
        swap(readers, m_allReaderList);

        // remove dangling references
        m_userWriterList.clear();
        m_userReaderList.clear();
    }

    // unlink the transport receiver blocks from the endpoints
    for ( auto endpoint : tmp)
    {
        std::lock_guard<std::mutex> _(m_receiverResourcelistMutex);

        for (auto& rb : m_receiverResourcelist)
        {
            auto receiver = rb.mp_receiver;
            if (receiver)
            {
                receiver->removeEndpoint(endpoint);
            }
        }
    }

    // Remove from builtin protocols
    auto removeEndpoint = [this](EndpointKind_t kind, Endpoint* p)
            {
                return kind == WRITER
                       ? mp_builtinProtocols->removeLocalWriter((RTPSWriter*)p)
                       : mp_builtinProtocols->removeLocalReader((RTPSReader*)p);
            };

#if HAVE_SECURITY
    bool (eprosima::fastrtps::rtps::security::SecurityManager::* unregister_endpoint[2])(
            const GUID_t& writer_guid);
    unregister_endpoint[WRITER] = &security::SecurityManager::unregister_local_writer;
    unregister_endpoint[READER] = &security::SecurityManager::unregister_local_reader;
#endif // if HAVE_SECURITY

    for ( auto endpoint : tmp)
    {
        auto kind = endpoint->getAttributes().endpointKind;
        removeEndpoint(kind, endpoint);

#if HAVE_SECURITY
        if (endpoint->getAttributes().security_attributes().is_submessage_protected ||
                endpoint->getAttributes().security_attributes().is_payload_protected)
        {
            (m_security_manager.*unregister_endpoint[kind])(endpoint->getGuid());
        }
#endif // if HAVE_SECURITY

        // remove the endpoints
        delete(endpoint);
    }
}

void RTPSParticipantImpl::normalize_endpoint_locators(
        EndpointAttributes& endpoint_att)
{
    // Locators with port 0, calculate port.
    uint32_t unicast_port = metatraffic_unicast_port_ + m_att.port.offsetd3 - m_att.port.offsetd1;
    for (Locator_t& loc : endpoint_att.unicastLocatorList)
    {
        m_network_Factory.fill_default_locator_port(loc, unicast_port);
    }
    uint32_t multicast_port = m_network_Factory.calculate_well_known_port(domain_id_, m_att, true);
    for (Locator_t& loc : endpoint_att.multicastLocatorList)
    {
        m_network_Factory.fill_default_locator_port(loc, multicast_port);
    }

    // Normalize unicast locators
    if (!endpoint_att.unicastLocatorList.empty())
    {
        m_network_Factory.NormalizeLocators(endpoint_att.unicastLocatorList);
    }
}

std::vector<std::string> RTPSParticipantImpl::getParticipantNames() const
{
    std::vector<std::string> participant_names;
    auto pdp = mp_builtinProtocols->mp_PDP;
    for (auto it = pdp->ParticipantProxiesBegin(); it != pdp->ParticipantProxiesEnd(); ++it)
    {
        participant_names.emplace_back((*it)->m_participantName.to_string());
    }
    return participant_names;
}

void RTPSParticipantImpl::setGuid(
        GUID_t& guid)
{
    m_guid = guid;
}

void RTPSParticipantImpl::announceRTPSParticipantState()
{
    return mp_builtinProtocols->announceRTPSParticipantState();
}

void RTPSParticipantImpl::stopRTPSParticipantAnnouncement()
{
    return mp_builtinProtocols->stopRTPSParticipantAnnouncement();
}

void RTPSParticipantImpl::resetRTPSParticipantAnnouncement()
{
    return mp_builtinProtocols->resetRTPSParticipantAnnouncement();
}

void RTPSParticipantImpl::loose_next_change()
{
    //NOTE: This is replaced by the test transport
    //this->mp_send_thr->loose_next_change();
}

bool RTPSParticipantImpl::newRemoteEndpointDiscovered(
        const GUID_t& pguid,
        int16_t userDefinedId,
        EndpointKind_t kind)
{
    if (m_att.builtin.discovery_config.discoveryProtocol != DiscoveryProtocol::SIMPLE ||
            m_att.builtin.discovery_config.use_STATIC_EndpointDiscoveryProtocol == false)
    {
        EPROSIMA_LOG_WARNING(RTPS_PARTICIPANT,
                "Remote Endpoints can only be activated with static discovery protocol over PDP simple protocol");
        return false;
    }

    if (PDPSimple* pS = dynamic_cast<PDPSimple*>(mp_builtinProtocols->mp_PDP))
    {
        return pS->newRemoteEndpointStaticallyDiscovered(pguid, userDefinedId, kind);
    }

    return false;
}

void RTPSParticipantImpl::assert_remote_participant_liveliness(
        const GuidPrefix_t& remote_guid)
{
    if (mp_builtinProtocols && mp_builtinProtocols->mp_PDP)
    {
        mp_builtinProtocols->mp_PDP->assert_remote_participant_liveliness(remote_guid);
    }
}

/**
 * Get the list of locators from which this publisher may send data.
 *
 * @param [out] locators  LocatorList_t where the list of locators will be stored.
 */
void RTPSParticipantImpl::get_sending_locators(
        rtps::LocatorList_t& locators) const
{
    locators.clear();

    // Traverse the sender list and query
    for (const auto& send_resource : send_resource_list_)
    {
        send_resource->add_locators_to_list(locators);
    }
}

uint32_t RTPSParticipantImpl::getMaxMessageSize() const
{
#if HAVE_SECURITY
    // An auxilary buffer is needed in the ReceiverResource to to decrypt the message,
    // that imposes a limit in the received messages size even if the transport allows (uint32_t) messages size.
    // So the sender limits also its size.
    uint32_t max_receiver_buffer_size =
            is_secure() ? std::numeric_limits<uint16_t>::max() : (std::numeric_limits<uint32_t>::max)();
#else
    uint32_t max_receiver_buffer_size = (std::numeric_limits<uint32_t>::max)();
#endif // if HAVE_SECURITY

    return (std::min)(
        m_network_Factory.get_max_message_size_between_transports(),
        max_receiver_buffer_size);
}

uint32_t RTPSParticipantImpl::getMaxDataSize()
{
    return calculateMaxDataSize(getMaxMessageSize());
}

uint32_t RTPSParticipantImpl::calculateMaxDataSize(
        uint32_t length)
{
    uint32_t maxDataSize = length;

#if HAVE_SECURITY
    // If there is rtps messsage protection, reduce max size for messages,
    // because extra data is added on encryption.
    if (security_attributes_.is_rtps_protected)
    {
        maxDataSize -= m_security_manager.calculate_extra_size_for_rtps_message();
    }
#endif // if HAVE_SECURITY

    // RTPS header
    maxDataSize -= RTPSMESSAGE_HEADER_SIZE;
    return maxDataSize;
}

bool RTPSParticipantImpl::networkFactoryHasRegisteredTransports() const
{
    return m_network_Factory.numberOfRegisteredTransports() > 0;
}

#if HAVE_SECURITY
bool RTPSParticipantImpl::pairing_remote_reader_with_local_writer_after_security(
        const GUID_t& local_writer,
        const ReaderProxyData& remote_reader_data)
{
    bool return_value;

    return_value = mp_builtinProtocols->mp_PDP->pairing_remote_reader_with_local_writer_after_security(
        local_writer, remote_reader_data);
    if (!return_value && mp_builtinProtocols->mp_WLP != nullptr)
    {
        return_value = mp_builtinProtocols->mp_WLP->pairing_remote_reader_with_local_writer_after_security(
            local_writer, remote_reader_data);
    }

    return return_value;
}

bool RTPSParticipantImpl::pairing_remote_writer_with_local_reader_after_security(
        const GUID_t& local_reader,
        const WriterProxyData& remote_writer_data)
{
    bool return_value;

    return_value = mp_builtinProtocols->mp_PDP->pairing_remote_writer_with_local_reader_after_security(
        local_reader, remote_writer_data);
    if (!return_value && mp_builtinProtocols->mp_WLP != nullptr)
    {
        return_value = mp_builtinProtocols->mp_WLP->pairing_remote_writer_with_local_reader_after_security(
            local_reader, remote_writer_data);
    }

    return return_value;
}

bool RTPSParticipantImpl::is_security_enabled_for_writer(
        const WriterAttributes& writer_attributes)
{
    if (!is_initialized() || !is_secure())
    {
        return false;
    }

    if (security_attributes().is_rtps_protected)
    {
        return true;
    }

    security::EndpointSecurityAttributes security_attributes;
    if (security_manager().get_datawriter_sec_attributes(writer_attributes.endpoint.properties, security_attributes))
    {
        return (security_attributes.is_payload_protected == true ||
               security_attributes.is_submessage_protected == true);
    }

    return false;
}

bool RTPSParticipantImpl::is_security_enabled_for_reader(
        const ReaderAttributes& reader_attributes)
{
    if (!is_initialized() || !is_secure())
    {
        return false;
    }

    if (security_attributes().is_rtps_protected)
    {
        return true;
    }

    security::EndpointSecurityAttributes security_attributes;
    if (security_manager().get_datareader_sec_attributes(reader_attributes.endpoint.properties, security_attributes))
    {
        return (security_attributes.is_payload_protected == true ||
               security_attributes.is_submessage_protected == true);
    }

    return false;
}

security::Logging* RTPSParticipantImpl::create_builtin_logging_plugin()
{
    uint32_t participant_id = static_cast<uint32_t>(m_att.participantID);
    return new security::LogTopic(participant_id, m_att.security_log_thread);
}

#endif // if HAVE_SECURITY

PDP* RTPSParticipantImpl::pdp()
{
    return mp_builtinProtocols->mp_PDP;
}

PDPSimple* RTPSParticipantImpl::pdpsimple()
{
    return dynamic_cast<PDPSimple*>(mp_builtinProtocols->mp_PDP);
}

WLP* RTPSParticipantImpl::wlp()
{
    return mp_builtinProtocols->mp_WLP;
}

fastdds::dds::builtin::TypeLookupManager* RTPSParticipantImpl::typelookup_manager() const
{
    return mp_builtinProtocols->tlm_;
}

IPersistenceService* RTPSParticipantImpl::get_persistence_service(
        const EndpointAttributes& param)
{
    IPersistenceService* ret_val;

    ret_val = PersistenceFactory::create_persistence_service(param.properties);
    return ret_val != nullptr ?
           ret_val :
           PersistenceFactory::create_persistence_service(m_att.properties);
}

bool RTPSParticipantImpl::get_persistence_service(
        bool is_builtin,
        const EndpointAttributes& param,
        IPersistenceService*& service)
{
    service = nullptr;

    const char* debug_label = (param.endpointKind == WRITER ? "writer" : "reader");

    // Check if also support persistence with TRANSIENT_LOCAL.
    DurabilityKind_t durability_red_line = get_persistence_durability_red_line(is_builtin);
    if (param.durabilityKind >= durability_red_line)
    {
        if (param.persistence_guid == c_Guid_Unknown)
        {
            EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "Cannot create persistence service. Persistence GUID not specified");
            return false;
        }
        service = get_persistence_service(param);
        if (service == nullptr)
        {
            EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT,
                    "Couldn't create writer persistence service for transient/persistent " << debug_label);
            return false;
        }
    }

    // Error log level can be disable. Avoid unused warning
    static_cast<void>(debug_label);

    return true;
}

bool RTPSParticipantImpl::get_new_entity_id(
        EntityId_t& entityId)
{
    if (entityId == c_EntityId_Unknown)
    {
        uint32_t idnum = ++IdCounter;
        octet* c = reinterpret_cast<octet*>(&idnum);
        entityId.value[2] = c[0];
        entityId.value[1] = c[1];
        entityId.value[0] = c[2];
        entityId.value[3] = 0x01; // Vendor specific
    }
    else
    {
        return !existsEntityId(entityId, READER) && !existsEntityId(entityId, WRITER);
    }

    return true;
}

void RTPSParticipantImpl::set_check_type_function(
        std::function<bool(const std::string&)>&& check_type)
{
    type_check_fn_ = std::move(check_type);
}

std::unique_ptr<RTPSMessageGroup_t> RTPSParticipantImpl::get_send_buffer(
        const std::chrono::steady_clock::time_point& max_blocking_time)
{
    return send_buffers_->get_buffer(this, max_blocking_time);
}

void RTPSParticipantImpl::return_send_buffer(
        std::unique_ptr <RTPSMessageGroup_t>&& buffer)
{
    send_buffers_->return_buffer(std::move(buffer));
}

uint32_t RTPSParticipantImpl::get_domain_id() const
{
    return domain_id_;
}

//!Compare metatraffic locators list searching for mutations
bool RTPSParticipantImpl::did_mutation_took_place_on_meta(
        const LocatorList_t& MulticastLocatorList,
        const LocatorList_t& UnicastLocatorList) const
{
    using namespace std;
    using namespace eprosima::fastdds::rtps;

    if (m_att.builtin.metatrafficMulticastLocatorList == MulticastLocatorList
            && m_att.builtin.metatrafficUnicastLocatorList == UnicastLocatorList)
    {
        // no mutation
        return false;
    }

    // If one of the locators is 0.0.0.0 we must replace it by all local interfaces like the framework does
    list<Locator_t> unicast_real_locators;
    LocatorListConstIterator it = UnicastLocatorList.begin(), old_it;
    LocatorList_t locals;

    do
    {
        // copy ordinary locators till the first ANY
        old_it = it;
        it = find_if(it, UnicastLocatorList.end(), IPLocator::isAny);

        // copy ordinary locators
        copy(old_it, it, back_inserter(unicast_real_locators));

        // transform new ones if needed
        if (it != UnicastLocatorList.end())
        {
            const Locator_t& an_any = *it;

            // load interfaces if needed
            if (locals.empty())
            {
                IPFinder::getIP4Address(&locals);
            }

            // add a locator for each local
            transform(locals.begin(),
                    locals.end(),
                    back_inserter(unicast_real_locators),
                    [&an_any](const Locator_t& loc) -> Locator_t
                    {
                        Locator_t specific(loc);
                        specific.port = an_any.port;
                        specific.kind = an_any.kind;
                        return specific;
                    });

            // search for the next if any
            ++it;
        }
    } while (it != UnicastLocatorList.end());

    // TCP is a special case because physical ports are taken from the TransportDescriptors
    // besides WAN address may be added by the transport
    struct ResetLogical
    {
        // use of unary_function to introduce the following aliases is deprecated
        // using argument_type = Locator_t;
        // using result_type   = Locator_t&;

        using Transports = vector<shared_ptr<TransportDescriptorInterface>>;

        ResetLogical(
                const Transports& tp)
            : Transports_(tp)
        {
            for (auto desc : Transports_)
            {
                if (nullptr == tcp4)
                {
                    tcp4 = dynamic_pointer_cast<TCPv4TransportDescriptor>(desc);
                }

                if (nullptr == tcp6)
                {
                    tcp6 = dynamic_pointer_cast<TCPv6TransportDescriptor>(desc);
                }
            }
        }

        uint16_t Tcp4ListeningPort() const
        {
            return tcp4 ? ( tcp4->listening_ports.empty() ? 0 : tcp4->listening_ports[0]) : 0;
        }

        uint16_t Tcp6ListeningPort() const
        {
            return tcp6 ? ( tcp6->listening_ports.empty() ? 0 : tcp6->listening_ports[0]) : 0;
        }

        void set_wan_address(
                Locator_t& loc) const
        {
            if (tcp4)
            {
                assert(LOCATOR_KIND_TCPv4 == loc.kind);
                auto& ip = tcp4->wan_addr;
                IPLocator::setWan(loc, ip[0], ip[1], ip[2], ip[3]);
            }
        }

        Locator_t operator ()(
                const Locator_t& loc) const
        {
            Locator_t ret(loc);
            switch (loc.kind)
            {
                case LOCATOR_KIND_TCPv4:
                    set_wan_address(ret);
                    IPLocator::setPhysicalPort(ret, Tcp4ListeningPort());
                    if (IPLocator::getLogicalPort(ret) == 0)
                    {
                        IPLocator::setLogicalPort(ret, IPLocator::getPhysicalPort(ret));
                    }
                    break;
                case LOCATOR_KIND_TCPv6:
                    IPLocator::setPhysicalPort(ret, Tcp6ListeningPort());
                    if (IPLocator::getLogicalPort(ret) == 0)
                    {
                        IPLocator::setLogicalPort(ret, IPLocator::getPhysicalPort(ret));
                    }
                    break;
            }
            return ret;
        }

        // reference to the transports
        const Transports& Transports_;
        shared_ptr<TCPv4TransportDescriptor> tcp4;
        shared_ptr<TCPv6TransportDescriptor> tcp6;

    }
    transform_functor(m_att.userTransports);

    // transform-copy
    set<Locator_t> update_attributes;

    transform(m_att.builtin.metatrafficMulticastLocatorList.begin(),
            m_att.builtin.metatrafficMulticastLocatorList.end(),
            inserter(update_attributes, update_attributes.begin()),
            transform_functor);

    transform(m_att.builtin.metatrafficUnicastLocatorList.begin(),
            m_att.builtin.metatrafficUnicastLocatorList.end(),
            inserter(update_attributes, update_attributes.begin()),
            transform_functor);

    set<Locator_t> original_ones;

    transform(MulticastLocatorList.begin(),
            MulticastLocatorList.end(),
            inserter(original_ones, original_ones.begin()),
            transform_functor);

    transform(unicast_real_locators.begin(),
            unicast_real_locators.end(),
            inserter(original_ones, original_ones.begin()),
            transform_functor);

    // if equal then no mutation took place on physical ports
    return !(update_attributes == original_ones);
}

DurabilityKind_t RTPSParticipantImpl::get_persistence_durability_red_line(
        bool is_builtin_endpoint)
{
    DurabilityKind_t durability_red_line = TRANSIENT;
    if (!is_builtin_endpoint)
    {
        std::string* persistence_support_transient_local_property = PropertyPolicyHelper::find_property(
            m_att.properties, "dds.persistence.also-support-transient-local");
        if (nullptr != persistence_support_transient_local_property &&
                0 == persistence_support_transient_local_property->compare("true"))
        {
            durability_red_line = TRANSIENT_LOCAL;
        }
    }

    return durability_red_line;
}

void RTPSParticipantImpl::environment_file_has_changed()
{
    RTPSParticipantAttributes patt = m_att;
    // Only if it is a server/backup or a client override
    if (DiscoveryProtocol_t::SERVER == m_att.builtin.discovery_config.discoveryProtocol ||
            DiscoveryProtocol_t::BACKUP == m_att.builtin.discovery_config.discoveryProtocol ||
            client_override_)
    {
        if (load_environment_server_info(patt.builtin.discovery_config.m_DiscoveryServers))
        {
            update_attributes(patt);
        }
    }
    else
    {
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK, "Trying to add Discovery Servers to a participant which is not a SERVER, BACKUP " <<
                "or an overriden CLIENT (SIMPLE participant transformed into CLIENT with the environment variable)");
    }
}

void RTPSParticipantImpl::get_default_metatraffic_locators()
{
    uint32_t metatraffic_multicast_port = m_att.port.getMulticastPort(domain_id_);

    m_network_Factory.getDefaultMetatrafficMulticastLocators(m_att.builtin.metatrafficMulticastLocatorList,
            metatraffic_multicast_port);
    m_network_Factory.NormalizeLocators(m_att.builtin.metatrafficMulticastLocatorList);

    m_network_Factory.getDefaultMetatrafficUnicastLocators(m_att.builtin.metatrafficUnicastLocatorList,
            metatraffic_unicast_port_);
    m_network_Factory.NormalizeLocators(m_att.builtin.metatrafficUnicastLocatorList);
}

void RTPSParticipantImpl::get_default_unicast_locators()
{
    uint32_t unicast_port = metatraffic_unicast_port_ + m_att.port.offsetd3 - m_att.port.offsetd1;
    m_network_Factory.getDefaultUnicastLocators(m_att.defaultUnicastLocatorList, unicast_port);
    m_network_Factory.NormalizeLocators(m_att.defaultUnicastLocatorList);
}

bool RTPSParticipantImpl::is_participant_ignored(
        const GuidPrefix_t& participant_guid)
{
    shared_lock<shared_mutex> _(ignored_mtx_);
    return ignored_participants_.find(participant_guid) != ignored_participants_.end();
}

bool RTPSParticipantImpl::is_writer_ignored(
        const GUID_t& /*writer_guid*/)
{
    return false;
}

bool RTPSParticipantImpl::is_reader_ignored(
        const GUID_t& /*reader_guid*/)
{
    return false;
}

bool RTPSParticipantImpl::ignore_participant(
        const GuidPrefix_t& participant_guid)
{
    if (participant_guid == m_guid.guidPrefix)
    {
        EPROSIMA_LOG_WARNING(RTPS_PARTICIPANT, "A participant is unable to ignore itself");
        return false;
    }
    {
        shared_lock<eprosima::shared_mutex> _(mp_builtinProtocols->getDiscoveryMutex());

        for (auto server_it = m_att.builtin.discovery_config.m_DiscoveryServers.begin();
                server_it != m_att.builtin.discovery_config.m_DiscoveryServers.end(); server_it++)
        {
            if (server_it->guidPrefix == participant_guid)
            {
                EPROSIMA_LOG_WARNING(RTPS_PARTICIPANT, "Cannot ignore one of this participant Discovery Servers");
                return false;
            }
        }
    }
    {
        std::unique_lock<shared_mutex> _(ignored_mtx_);
        ignored_participants_.insert(participant_guid);
    }
    pdp()->remove_remote_participant(GUID_t(participant_guid, c_EntityId_RTPSParticipant),
            ParticipantDiscoveryInfo::DISCOVERY_STATUS::IGNORED_PARTICIPANT);

    return true;

}

bool RTPSParticipantImpl::ignore_writer(
        const GUID_t& /*writer_guid*/)
{
    return false;
}

bool RTPSParticipantImpl::ignore_reader(
        const GUID_t& /*reader_guid*/)
{
    return false;
}

std::vector<fastdds::rtps::TransportNetmaskFilterInfo> RTPSParticipantImpl::get_netmask_filter_info() const
{
    return m_network_Factory.netmask_filter_info();
}

#ifdef FASTDDS_STATISTICS

bool RTPSParticipantImpl::register_in_writer(
        std::shared_ptr<fastdds::statistics::IListener> listener,
        GUID_t writer_guid)
{
    bool res = false;

    if ( GUID_t::unknown() == writer_guid )
    {
        shared_lock<shared_mutex> _(endpoints_list_mutex);
        res = true;
        for ( auto writer : m_userWriterList)
        {
            if (!fastdds::statistics::is_statistics_builtin(writer->getGuid().entityId))
            {
                res &= writer->add_statistics_listener(listener);
            }
        }
    }
    else if (!fastdds::statistics::is_statistics_builtin(writer_guid.entityId))
    {
        RTPSWriter* writer = find_local_writer(writer_guid);
        res = writer->add_statistics_listener(listener);
    }

    return res;
}

bool RTPSParticipantImpl::register_in_reader(
        std::shared_ptr<fastdds::statistics::IListener> listener,
        GUID_t reader_guid)
{
    bool res = false;

    if ( GUID_t::unknown() == reader_guid )
    {
        shared_lock<shared_mutex> _(endpoints_list_mutex);
        res = true;
        for ( auto reader : m_userReaderList)
        {
            if (!fastdds::statistics::is_statistics_builtin(reader->getGuid().entityId))
            {
                res &= reader->add_statistics_listener(listener);
            }
        }
    }
    else if (!fastdds::statistics::is_statistics_builtin(reader_guid.entityId))
    {
        RTPSReader* reader = find_local_reader(reader_guid);
        res = reader->add_statistics_listener(listener);
    }

    return res;
}

bool RTPSParticipantImpl::unregister_in_writer(
        std::shared_ptr<fastdds::statistics::IListener> listener)
{
    shared_lock<shared_mutex> _(endpoints_list_mutex);
    bool res = true;

    for ( auto writer : m_userWriterList)
    {
        if (!fastdds::statistics::is_statistics_builtin(writer->getGuid().entityId))
        {
            res &= writer->remove_statistics_listener(listener);
        }
    }

    return res;
}

bool RTPSParticipantImpl::unregister_in_reader(
        std::shared_ptr<fastdds::statistics::IListener> listener)
{
    shared_lock<shared_mutex> _(endpoints_list_mutex);
    bool res = true;

    for ( auto reader : m_userReaderList)
    {
        if (!fastdds::statistics::is_statistics_builtin(reader->getGuid().entityId))
        {
            res &= reader->remove_statistics_listener(listener);
        }
    }

    return res;
}

void RTPSParticipantImpl::set_enabled_statistics_writers_mask(
        uint32_t enabled_writers)
{
    StatisticsParticipantImpl::set_enabled_statistics_writers_mask(enabled_writers);

    // Propagate mask to all readers and writers
    shared_lock<shared_mutex> _(endpoints_list_mutex);

    for (auto reader : m_userReaderList)
    {
        reader->set_enabled_statistics_writers_mask(enabled_writers);
    }

    for (auto writer : m_userWriterList)
    {
        writer->set_enabled_statistics_writers_mask(enabled_writers);
    }
}

const fastdds::statistics::rtps::IStatusObserver* RTPSParticipantImpl::create_monitor_service(
        fastdds::statistics::rtps::IStatusQueryable& status_queryable)
{
    monitor_server_.reset(new fastdds::statistics::rtps::MonitorService(
                m_guid,
                pdp(),
                this,
                status_queryable,
                [&](RTPSWriter** WriterOut,
                WriterAttributes& param,
                const std::shared_ptr<IPayloadPool>& payload_pool,
                WriterHistory* hist,
                WriterListener* listen,
                const EntityId_t& entityId,
                bool isBuiltin) -> bool
                {
                    return this->createWriter(WriterOut, param, payload_pool, hist, listen, entityId, isBuiltin);
                },
                [&](RTPSWriter* w,
                const fastrtps::TopicAttributes& topicAtt,
                const fastrtps::WriterQos& wqos) -> bool
                {
                    return this->registerWriter(w, topicAtt, wqos);
                },
                getEventResource()
                ));

    if (nullptr != monitor_server_)
    {
        auto monitor_listener = monitor_server_->get_listener();
        conns_observer_.store(monitor_listener);
        pdp()->set_proxy_observer(monitor_listener);

        return monitor_listener;
    }
    else
    {
        EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "Could not create monitor service");
        return nullptr;
    }
}

bool RTPSParticipantImpl::create_monitor_service()
{
    bool ret = false;

    simple_queryable_.reset(new fastdds::statistics::rtps::SimpleQueryable);
    create_monitor_service(*simple_queryable_);

    if (nullptr != monitor_server_)
    {
        ret = true;
    }

    return ret;
}

bool RTPSParticipantImpl::is_monitor_service_created() const
{
    return (nullptr != monitor_server_);
}

bool RTPSParticipantImpl::enable_monitor_service() const
{
    bool ret = false;
    if (nullptr != monitor_server_)
    {
        ret = monitor_server_->enable_monitor_service();
    }
    return ret;
}

bool RTPSParticipantImpl::disable_monitor_service() const
{
    bool ret = false;
    if (nullptr != monitor_server_ && monitor_server_->is_enabled())
    {
        ret = monitor_server_->disable_monitor_service();
    }
    return ret;
}

bool RTPSParticipantImpl::fill_discovery_data_from_cdr_message(
        fastrtps::rtps::ParticipantProxyData& data,
        fastdds::statistics::MonitorServiceStatusData& msg)
{
    bool ret = true;
    CDRMessage_t serialized_msg{0};
    serialized_msg.wraps = true;

    serialized_msg.buffer = msg.value().entity_proxy().data();
    serialized_msg.length = static_cast<uint32_t>(msg.value().entity_proxy().size());

    ret = data.readFromCDRMessage(
        &serialized_msg,
        true,
        network_factory(),
        has_shm_transport(),
        false,
        c_VendorId_eProsima);

    return ret && (data.m_guid.entityId == c_EntityId_RTPSParticipant);
}

bool RTPSParticipantImpl::fill_discovery_data_from_cdr_message(
        fastrtps::rtps::WriterProxyData& data,
        fastdds::statistics::MonitorServiceStatusData& msg)
{
    bool ret = true;
    CDRMessage_t serialized_msg{0};
    serialized_msg.wraps = true;

    serialized_msg.buffer = msg.value().entity_proxy().data();
    serialized_msg.length = static_cast<uint32_t>(msg.value().entity_proxy().size());

    ret = data.readFromCDRMessage(
        &serialized_msg,
        network_factory(),
        has_shm_transport(),
        false,
        c_VendorId_eProsima);

    return ret && (data.guid().entityId.is_writer());
}

bool RTPSParticipantImpl::fill_discovery_data_from_cdr_message(
        fastrtps::rtps::ReaderProxyData& data,
        fastdds::statistics::MonitorServiceStatusData& msg)
{
    bool ret = true;
    CDRMessage_t serialized_msg{0};
    serialized_msg.wraps = true;

    serialized_msg.buffer = msg.value().entity_proxy().data();
    serialized_msg.length = static_cast<uint32_t>(msg.value().entity_proxy().size());

    ret = data.readFromCDRMessage(
        &serialized_msg,
        network_factory(),
        has_shm_transport(),
        false,
        c_VendorId_eProsima);

    return ret && (data.guid().entityId.is_reader());
}

bool
RTPSParticipantImpl::get_entity_connections(
        const GUID_t& guid,
        fastdds::statistics::rtps::ConnectionList& conn_list)
{
    bool ret = true;
    if (guid.entityId == c_EntityId_RTPSParticipant)
    {

        //! Avoid getting the local participant
        conn_list.reserve(pdp()->participant_proxies_number());
        std::lock_guard<std::recursive_mutex> lock(*pdp()->getMutex());

        auto pit = pdp()->ParticipantProxiesBegin();
        ++pit;
        for (; pit != pdp()->ParticipantProxiesEnd(); ++pit)
        {
            fastdds::statistics::Connection connection;
            connection.guid(fastdds::statistics::to_statistics_type((*pit)->m_guid));
            connection.mode(fastdds::statistics::TRANSPORT);

            std::vector<fastdds::statistics::detail::Locator_s> statistic_locators;
            statistic_locators.reserve((*pit)->metatraffic_locators.multicast.size() +
                    (*pit)->metatraffic_locators.unicast.size());

            std::for_each((*pit)->metatraffic_locators.multicast.begin(), (*pit)->metatraffic_locators.multicast.end(),
                    [&statistic_locators](const Locator_t& locator)
                    {
                        statistic_locators.push_back(fastdds::statistics::to_statistics_type(locator));
                    });

            std::for_each((*pit)->metatraffic_locators.unicast.begin(), (*pit)->metatraffic_locators.unicast.end(),
                    [&statistic_locators](const Locator_t& locator)
                    {
                        statistic_locators.push_back(fastdds::statistics::to_statistics_type(locator));
                    });

            std::for_each((*pit)->default_locators.multicast.begin(), (*pit)->default_locators.multicast.end(),
                    [&statistic_locators](const Locator_t& locator)
                    {
                        statistic_locators.push_back(fastdds::statistics::to_statistics_type(locator));
                    });

            std::for_each((*pit)->default_locators.unicast.begin(), (*pit)->default_locators.unicast.end(),
                    [&statistic_locators](const Locator_t& locator)
                    {
                        statistic_locators.push_back(fastdds::statistics::to_statistics_type(locator));
                    });

            connection.announced_locators(statistic_locators);
            connection.used_locators(statistic_locators);
            conn_list.push_back(connection);
        }
    }
    else if (guid.entityId.is_reader())
    {
        for (auto& reader : m_userReaderList)
        {
            if (reader->m_guid == guid)
            {
                reader->get_connections(conn_list);
            }
        }
    }
    else if (guid.entityId.is_writer())
    {
        for (auto& writer : m_userWriterList)
        {
            if (writer->m_guid == guid)
            {
                writer->get_connections(conn_list);
            }
        }
    }
    else
    {
        ret = false;
        EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "Unknown entitiy kind to get connections: " << guid);
    }
    return ret;
}

#endif // FASTDDS_STATISTICS

bool RTPSParticipantImpl::should_match_local_endpoints(
        const RTPSParticipantAttributes& att)
{
    bool should_match_local_endpoints = true;

    const std::string* ignore_local_endpoints = PropertyPolicyHelper::find_property(att.properties,
                    "fastdds.ignore_local_endpoints");
    if (nullptr != ignore_local_endpoints)
    {
        if (0 == ignore_local_endpoints->compare("true"))
        {
            should_match_local_endpoints = false;
        }
        else if (0 == ignore_local_endpoints->compare("false"))
        {
            should_match_local_endpoints = true;
        }
        else
        {
            should_match_local_endpoints = true;
            EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT,
                    "Unkown value '" << *ignore_local_endpoints <<
                    "' for property 'fastdds.ignore_local_endpoints'. Setting value to 'true'");
        }
    }
    return should_match_local_endpoints;
}

void RTPSParticipantImpl::update_removed_participant(
        const LocatorList_t& remote_participant_locators)
{
    if (!remote_participant_locators.empty())
    {
        std::lock_guard<std::timed_mutex> guard(m_send_resources_mutex_);
        m_network_Factory.remove_participant_associated_send_resources(
            send_resource_list_,
            remote_participant_locators,
            m_att.builtin.initialPeersList);
    }
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
