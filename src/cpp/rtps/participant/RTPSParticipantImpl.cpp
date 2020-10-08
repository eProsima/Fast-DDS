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

#include <rtps/participant/RTPSParticipantImpl.h>

#include <rtps/flowcontrol/ThroughputController.h>
#include <rtps/persistence/PersistenceService.h>
#include <rtps/history/BasicPayloadPool.hpp>

#include <fastdds/rtps/messages/MessageReceiver.h>

#include <fastdds/rtps/history/WriterHistory.h>

#include <fastdds/rtps/writer/StatelessWriter.h>
#include <fastdds/rtps/writer/StatefulWriter.h>
#include <fastdds/rtps/writer/StatelessPersistentWriter.h>
#include <fastdds/rtps/writer/StatefulPersistentWriter.h>

#include <fastdds/rtps/reader/StatelessReader.h>
#include <fastdds/rtps/reader/StatefulReader.h>
#include <fastdds/rtps/reader/StatelessPersistentReader.h>
#include <fastdds/rtps/reader/StatefulPersistentReader.h>

#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.h>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h>

#include <fastdds/rtps/RTPSDomain.h>

#include <fastdds/rtps/builtin/BuiltinProtocols.h>
#include <fastdds/rtps/builtin/discovery/participant/PDPSimple.h>
#include <fastdds/rtps/builtin/data/ParticipantProxyData.h>
#include <fastdds/rtps/builtin/liveliness/WLP.h>

#include <fastrtps/utils/IPFinder.h>

#include <fastrtps/utils/Semaphore.h>
#include <fastrtps/utils/System.h>

#include <mutex>
#include <functional>
#include <algorithm>

#include <fastdds/dds/log/Log.hpp>
#include <fastrtps/xmlparser/XMLProfileManager.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

using UDPv4TransportDescriptor = fastdds::rtps::UDPv4TransportDescriptor;
using TCPTransportDescriptor = fastdds::rtps::TCPTransportDescriptor;
using SharedMemTransportDescriptor = fastdds::rtps::SharedMemTransportDescriptor;

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

Locator_t& RTPSParticipantImpl::applyLocatorAdaptRule(
        Locator_t& loc)
{
    // This is a completely made up rule
    // It is transport responsability to interpret this new port.
    loc.port += m_att.port.participantIDGain;
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
    , m_persistence_guid(persistence_guid, c_EntityId_RTPSParticipant)
    , mp_builtinProtocols(nullptr)
    , mp_ResourceSemaphore(new Semaphore(0))
    , IdCounter(0)
    , type_check_fn_(nullptr)
#if HAVE_SECURITY
    , m_security_manager(this)
#endif // if HAVE_SECURITY
    , mp_participantListener(plisten)
    , mp_userParticipant(par)
    , mp_mutex(new std::recursive_mutex())
    , is_intraprocess_only_(should_be_intraprocess_only(PParam))
    , has_shm_transport_(false)
{
    // Builtin transports by default
    if (PParam.useBuiltinTransports)
    {
        UDPv4TransportDescriptor descriptor;
        descriptor.sendBufferSize = m_att.sendSocketBufferSize;
        descriptor.receiveBufferSize = m_att.listenSocketBufferSize;
        m_network_Factory.RegisterTransport(&descriptor);

#ifdef SHM_TRANSPORT_BUILTIN
        SharedMemTransportDescriptor shm_transport;
        // We assume (Linux) UDP doubles the user socket buffer size in kernel, so
        // the equivalent segment size in SHM would be socket buffer size x 2
        auto segment_size_udp_equivalent =
                std::max(m_att.sendSocketBufferSize, m_att.listenSocketBufferSize) * 2;
        shm_transport.segment_size(segment_size_udp_equivalent);
        // Use same default max_message_size on both UDP and SHM
        shm_transport.max_message_size(descriptor.max_message_size());
        has_shm_transport_ |= m_network_Factory.RegisterTransport(&shm_transport);
#endif // ifdef SHM_TRANSPORT_BUILTIN
    }

    // BACKUP servers guid is its persistence one
    if (PParam.builtin.discovery_config.discoveryProtocol == DiscoveryProtocol::BACKUP)
    {
        m_persistence_guid = m_guid;
    }

    // Client-server discovery protocol requires that every TCP transport has a listening port
    switch (PParam.builtin.discovery_config.discoveryProtocol)
    {
        case DiscoveryProtocol::BACKUP:
        case DiscoveryProtocol::CLIENT:
        case DiscoveryProtocol::SERVER:
            // Verify if listening ports are provided
            for (auto& transportDescriptor : PParam.userTransports)
            {
                TCPTransportDescriptor* pT = dynamic_cast<TCPTransportDescriptor*>(transportDescriptor.get());
                if (pT && pT->listening_ports.empty())
                {
                    logError(RTPS_PARTICIPANT,
                            "Participant " << m_att.getName() << " with GUID " << m_guid
                                           << " tries to use discovery server over TCP without providing a proper listening port");
                }
            }
        default:
            break;
    }


    // User defined transports
    for (const auto& transportDescriptor : PParam.userTransports)
    {
        if (m_network_Factory.RegisterTransport(transportDescriptor.get()))
        {
            has_shm_transport_ |=
                    (dynamic_cast<fastdds::rtps::SharedMemTransportDescriptor*>(transportDescriptor.get()) != nullptr);
        }
        else
        {
            // SHM transport could be disabled
            if ((dynamic_cast<fastdds::rtps::SharedMemTransportDescriptor*>(transportDescriptor.get()) != nullptr))
            {
                logError(RTPS_PARTICIPANT,
                        "Unable to Register SHM Transport. SHM Transport is not supported in"
                        " the current platform.");
            }
            else
            {
                logError(RTPS_PARTICIPANT,
                        "User transport failed to register.");
            }

        }
    }

    mp_userParticipant->mp_impl = this;
    mp_event_thr.init_thread();

    if (!networkFactoryHasRegisteredTransports())
    {
        return;
    }

    // Throughput controller, if the descriptor has valid values
    if (PParam.throughputController.bytesPerPeriod != UINT32_MAX && PParam.throughputController.periodMillisecs != 0)
    {
        std::unique_ptr<FlowController> controller(new ThroughputController(PParam.throughputController, this));
        m_controllers.push_back(std::move(controller));
    }

    /* If metatrafficMulticastLocatorList is empty, add mandatory default Locators
       Else -> Take them */

    // Creation of metatraffic locator and receiver resources
    uint32_t metatraffic_multicast_port = m_att.port.getMulticastPort(domain_id_);
    uint32_t metatraffic_unicast_port = m_att.port.getUnicastPort(domain_id_,
                    static_cast<uint32_t>(m_att.participantID));

    /* INSERT DEFAULT MANDATORY MULTICAST LOCATORS HERE */
    if (m_att.builtin.metatrafficMulticastLocatorList.empty() && m_att.builtin.metatrafficUnicastLocatorList.empty())
    {
        m_network_Factory.getDefaultMetatrafficMulticastLocators(m_att.builtin.metatrafficMulticastLocatorList,
                metatraffic_multicast_port);
        m_network_Factory.NormalizeLocators(m_att.builtin.metatrafficMulticastLocatorList);

        m_network_Factory.getDefaultMetatrafficUnicastLocators(m_att.builtin.metatrafficUnicastLocatorList,
                metatraffic_unicast_port);
        m_network_Factory.NormalizeLocators(m_att.builtin.metatrafficUnicastLocatorList);
    }
    else
    {
        std::for_each(m_att.builtin.metatrafficMulticastLocatorList.begin(),
                m_att.builtin.metatrafficMulticastLocatorList.end(), [&](Locator_t& locator)
                {
                    m_network_Factory.fillMetatrafficMulticastLocator(locator, metatraffic_multicast_port);
                });
        m_network_Factory.NormalizeLocators(m_att.builtin.metatrafficMulticastLocatorList);

        std::for_each(m_att.builtin.metatrafficUnicastLocatorList.begin(),
                m_att.builtin.metatrafficUnicastLocatorList.end(), [&](Locator_t& locator)
                {
                    m_network_Factory.fillMetatrafficUnicastLocator(locator, metatraffic_unicast_port);
                });
        m_network_Factory.NormalizeLocators(m_att.builtin.metatrafficUnicastLocatorList);
    }

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

    // Creation of user locator and receiver resources
    bool hasLocatorsDefined = true;
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
        hasLocatorsDefined = false;

        m_network_Factory.getDefaultUnicastLocators(domain_id_, m_att.defaultUnicastLocatorList, m_att);
    }
    else
    {
        // Locator with port 0, calculate port.
        std::for_each(m_att.defaultUnicastLocatorList.begin(), m_att.defaultUnicastLocatorList.end(),
                [&](Locator_t& loc)
                {
                    m_network_Factory.fillDefaultUnicastLocator(domain_id_, loc, m_att);
                });

    }

    // Normalize unicast locators.
    m_network_Factory.NormalizeLocators(m_att.defaultUnicastLocatorList);

    if (!hasLocatorsDefined)
    {
        logInfo(RTPS_PARTICIPANT, m_att.getName() << " Created with NO default Unicast Locator List, adding Locators:"
                                                  << m_att.defaultUnicastLocatorList);
    }

#if HAVE_SECURITY
    // Start security
    // TODO(Ricardo) Get returned value in future.
    m_security_manager_initialized = m_security_manager.init(security_attributes_, PParam.properties,
                    m_is_security_active);
    if (!m_security_manager_initialized)
    {
        // Participant will be deleted, no need to allocate buffers or create builtin endpoints
        return;
    }
#endif // if HAVE_SECURITY

    if (is_intraprocess_only())
    {
        m_att.builtin.metatrafficUnicastLocatorList.clear();
        m_att.defaultUnicastLocatorList.clear();
        m_att.defaultMulticastLocatorList.clear();
    }

    createReceiverResources(m_att.builtin.metatrafficMulticastLocatorList, true, false);
    createReceiverResources(m_att.builtin.metatrafficUnicastLocatorList, true, false);
    createReceiverResources(m_att.defaultUnicastLocatorList, true, false);
    createReceiverResources(m_att.defaultMulticastLocatorList, true, false);

    bool allow_growing_buffers = m_att.allocation.send_buffers.dynamic;
    size_t num_send_buffers = m_att.allocation.send_buffers.preallocated_number;
    if (num_send_buffers == 0)
    {
        // Three buffers (user, events and async writer threads)
        num_send_buffers = 3;
        // Add one buffer per reception thread
        num_send_buffers += m_receiverResourcelist.size();
    }

    // Create buffer pool
    send_buffers_.reset(new SendBuffersManager(num_send_buffers, allow_growing_buffers));
    send_buffers_->init(this);

#if HAVE_SECURITY
    if (m_is_security_active)
    {
        m_is_security_active = m_security_manager.create_entities();
        if (!m_is_security_active)
        {
            // Participant will be deleted, no need to create builtin endpoints
            m_security_manager_initialized = false;
            return;
        }
    }
#endif // if HAVE_SECURITY

    mp_builtinProtocols = new BuiltinProtocols();

    logInfo(RTPS_PARTICIPANT, "RTPSParticipant \"" << m_att.getName() << "\" with guidPrefix: " << m_guid.guidPrefix);
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

void RTPSParticipantImpl::enable()
{
    // Start builtin protocols
    if (!mp_builtinProtocols->initBuiltinProtocols(this, m_att.builtin))
    {
        logError(RTPS_PARTICIPANT, "The builtin protocols were not correctly initialized");
    }

    //Start reception
    for (auto& receiver : m_receiverResourcelist)
    {
        receiver.Receiver->RegisterReceiver(receiver.mp_receiver);
    }
}

void RTPSParticipantImpl::disable()
{
    // Ensure that other participants will not accidentally discover this one
    if (mp_builtinProtocols && mp_builtinProtocols->mp_PDP)
    {
        mp_builtinProtocols->stopRTPSParticipantAnnouncement();
    }

    // Disable Retries on Transports
    m_network_Factory.Shutdown();

    // Safely abort threads.
    for (auto& block : m_receiverResourcelist)
    {
        block.Receiver->UnregisterReceiver(block.mp_receiver);
        block.disable();
    }

    while (m_userReaderList.size() > 0)
    {
        deleteUserEndpoint(static_cast<Endpoint*>(*m_userReaderList.begin()));
    }

    while (m_userWriterList.size() > 0)
    {
        deleteUserEndpoint(static_cast<Endpoint*>(*m_userWriterList.begin()));
    }

    delete(mp_builtinProtocols);
    mp_builtinProtocols = nullptr;
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

    delete mp_ResourceSemaphore;
    delete mp_userParticipant;
    send_resource_list_.clear();

    delete mp_mutex;
}

template<typename Functor>
bool RTPSParticipantImpl::create_writer(
        RTPSWriter** WriterOut,
        WriterAttributes& param,
        const EntityId_t& entityId,
        bool isBuiltin,
        const Functor& callback)
{
    std::string type = (param.endpoint.reliabilityKind == RELIABLE) ? "RELIABLE" : "BEST_EFFORT";
    logInfo(RTPS_PARTICIPANT, " of type " << type);
    EntityId_t entId;
    if (entityId == c_EntityId_Unknown)
    {
        if (param.endpoint.topicKind == NO_KEY)
        {
            entId.value[3] = 0x03;
        }
        else if (param.endpoint.topicKind == WITH_KEY)
        {
            entId.value[3] = 0x02;
        }
        uint32_t idnum;
        if (param.endpoint.getEntityID() > 0)
        {
            idnum = static_cast<uint32_t>(param.endpoint.getEntityID());
        }
        else
        {
            IdCounter++;
            idnum = IdCounter;
        }

        octet* c = reinterpret_cast<octet*>(&idnum);
        entId.value[2] = c[0];
        entId.value[1] = c[1];
        entId.value[0] = c[2];
        if (this->existsEntityId(entId, WRITER))
        {
            logError(RTPS_PARTICIPANT, "A writer with the same entityId already exists in this RTPSParticipant");
            return false;
        }
    }
    else
    {
        entId = entityId;
    }
    if (!param.endpoint.unicastLocatorList.isValid())
    {
        logError(RTPS_PARTICIPANT, "Unicast Locator List for Writer contains invalid Locator");
        return false;
    }
    if (!param.endpoint.multicastLocatorList.isValid())
    {
        logError(RTPS_PARTICIPANT, "Multicast Locator List for Writer contains invalid Locator");
        return false;
    }
    if (!param.endpoint.remoteLocatorList.isValid())
    {
        logError(RTPS_PARTICIPANT, "Remote Locator List for Writer contains invalid Locator");
        return false;
    }
    if (((param.throughputController.bytesPerPeriod != UINT32_MAX && param.throughputController.periodMillisecs != 0) ||
            (m_att.throughputController.bytesPerPeriod != UINT32_MAX &&
            m_att.throughputController.periodMillisecs != 0))
            && param.mode != ASYNCHRONOUS_WRITER)
    {
        logError(RTPS_PARTICIPANT,
                "Writer has to be configured to publish asynchronously, because a flowcontroller was configured");
        return false;
    }

    // Update persistence guidPrefix, restore this change later to keep param unblemished
    GUID_t former_persistence_guid = param.endpoint.persistence_guid;
    if (param.endpoint.persistence_guid == c_Guid_Unknown)
    {
        // Try to load persistence_guid from property
        std::string* persistence_guid_property = PropertyPolicyHelper::find_property(
            param.endpoint.properties, "dds.persistence.guid");
        if (persistence_guid_property != nullptr)
        {
            // Load persistence_guid from property
            std::istringstream(persistence_guid_property->c_str()) >> param.endpoint.persistence_guid;
            if (param.endpoint.persistence_guid == c_Guid_Unknown)
            {
                // Wrongly configured property
                logError(RTPS_PARTICIPANT, "Cannot configure writer's persistence GUID from '"
                        << persistence_guid_property->c_str() << "'. Wrong input");
                return false;
            }
            // Make sure the persistence guid sticks
            former_persistence_guid = param.endpoint.persistence_guid;
        }
        else if (m_persistence_guid != c_Guid_Unknown)
        {
            // Generate persistence guid from participant persistence guid
            param.endpoint.persistence_guid = GUID_t(
                m_persistence_guid.guidPrefix,
                entityId);
        }
    }

    // Get persistence service
    IPersistenceService* persistence = nullptr;
    if (param.endpoint.durabilityKind >= TRANSIENT)
    {
        if (param.endpoint.persistence_guid == c_Guid_Unknown)
        {
            logError(RTPS_PARTICIPANT, "Cannot create persistence service. Persistence GUID not specified");
            return false;
        }
        persistence = get_persistence_service(param.endpoint);
        if (persistence == nullptr)
        {
            logError(RTPS_PARTICIPANT, "Couldn't create writer persistence service for transient/persistent writer");
            return false;
        }
    }

    normalize_endpoint_locators(param.endpoint);

    RTPSWriter* SWriter = nullptr;
    GUID_t guid(m_guid.guidPrefix, entId);
    SWriter = callback(guid, param, persistence, param.endpoint.reliabilityKind == RELIABLE);

    // restore attributes
    param.endpoint.persistence_guid = former_persistence_guid;

    if (SWriter == nullptr)
    {
        return false;
    }

#if HAVE_SECURITY
    if (!isBuiltin)
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

    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);
    m_allWriterList.push_back(SWriter);
    if (!isBuiltin)
    {
        m_userWriterList.push_back(SWriter);
    }
    *WriterOut = SWriter;

    // If the terminal throughput controller has proper user defined values, instantiate it
    if (param.throughputController.bytesPerPeriod != UINT32_MAX && param.throughputController.periodMillisecs != 0)
    {
        std::unique_ptr<FlowController> controller(new ThroughputController(param.throughputController, SWriter));
        SWriter->add_flow_controller(std::move(controller));
    }

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
                (const GUID_t& guid, WriterAttributes& param, IPersistenceService* persistence,
                    bool is_reliable) -> RTPSWriter*
            {
                if (is_reliable)
                {
                    if (persistence != nullptr)
                    {
                        return new StatefulPersistentWriter(this, guid, param, hist, listen, persistence);
                    }
                    else
                    {
                        return new StatefulWriter(this, guid, param, hist, listen);
                    }
                }
                else
                {
                    if (persistence != nullptr)
                    {
                        return new StatelessPersistentWriter(this, guid, param, hist, listen, persistence);
                    }
                    else
                    {
                        return new StatelessWriter(this, guid, param, hist, listen);
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
    auto callback = [hist, listen, &payload_pool, this]
                (const GUID_t& guid, WriterAttributes& param, IPersistenceService* persistence,
                    bool is_reliable) -> RTPSWriter*
            {
                if (is_reliable)
                {
                    if (persistence != nullptr)
                    {
                        return new StatefulPersistentWriter(this, guid, param, payload_pool, hist, listen, persistence);
                    }
                    else
                    {
                        return new StatefulWriter(this, guid, param, payload_pool, hist, listen);
                    }
                }
                else
                {
                    if (persistence != nullptr)
                    {
                        return new StatelessPersistentWriter(this, guid, param, payload_pool, hist, listen,
                                       persistence);
                    }
                    else
                    {
                        return new StatelessWriter(this, guid, param, payload_pool, hist, listen);
                    }
                }
            };
    return create_writer(WriterOut, param, entityId, isBuiltin, callback);
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
    std::string type = (param.endpoint.reliabilityKind == RELIABLE) ? "RELIABLE" : "BEST_EFFORT";
    logInfo(RTPS_PARTICIPANT, " of type " << type);
    EntityId_t entId;
    if (entityId == c_EntityId_Unknown)
    {
        if (param.endpoint.topicKind == NO_KEY)
        {
            entId.value[3] = 0x04;
        }
        else if (param.endpoint.topicKind == WITH_KEY)
        {
            entId.value[3] = 0x07;
        }
        uint32_t idnum;
        if (param.endpoint.getEntityID() > 0)
        {
            idnum = static_cast<uint32_t>(param.endpoint.getEntityID());
        }
        else
        {
            IdCounter++;
            idnum = IdCounter;
        }

        octet* c = reinterpret_cast<octet*>(&idnum);
        entId.value[2] = c[0];
        entId.value[1] = c[1];
        entId.value[0] = c[2];
        if (this->existsEntityId(entId, WRITER))
        {
            logError(RTPS_PARTICIPANT, "A reader with the same entityId already exists in this RTPSParticipant");
            return false;
        }
    }
    else
    {
        entId = entityId;
    }
    if (!param.endpoint.unicastLocatorList.isValid())
    {
        logError(RTPS_PARTICIPANT, "Unicast Locator List for Reader contains invalid Locator");
        return false;
    }
    if (!param.endpoint.multicastLocatorList.isValid())
    {
        logError(RTPS_PARTICIPANT, "Multicast Locator List for Reader contains invalid Locator");
        return false;
    }
    if (!param.endpoint.remoteLocatorList.isValid())
    {
        logError(RTPS_PARTICIPANT, "Remote Locator List for Reader contains invalid Locator");
        return false;
    }

    // Get persistence service
    IPersistenceService* persistence = nullptr;
    if (param.endpoint.durabilityKind >= TRANSIENT)
    {
        // Check if persistence guid needs to be set from property
        if (param.endpoint.persistence_guid == c_Guid_Unknown)
        {
            // Try to load persistence_guid from property
            std::string* persistence_guid_property = PropertyPolicyHelper::find_property(
                param.endpoint.properties, "dds.persistence.guid");
            if (persistence_guid_property != nullptr)
            {
                // Load persistence_guid from property
                std::istringstream(persistence_guid_property->c_str()) >> param.endpoint.persistence_guid;
                if (param.endpoint.persistence_guid == c_Guid_Unknown)
                {
                    // Wrongly configured property
                    logError(RTPS_PARTICIPANT, "Cannot configure readers's persistence GUID from '"
                            << persistence_guid_property->c_str() << "'. Wrong input");
                    return false;
                }
            }
            else
            {
                logError(RTPS_PARTICIPANT, "Cannot create reader persistence service. Persistence GUID not specified");
                return false;
            }
        }

        persistence = get_persistence_service(param.endpoint);
        if (persistence == nullptr)
        {
            logError(RTPS_PARTICIPANT, "Couldn't create persistence service for transient/persistent reader");
            return false;
        }
    }

    normalize_endpoint_locators(param.endpoint);

    RTPSReader* SReader = nullptr;
    GUID_t guid(m_guid.guidPrefix, entId);
    if (param.endpoint.reliabilityKind == BEST_EFFORT)
    {
        SReader = (persistence == nullptr) ?
                new StatelessReader(this, guid, param, hist, listen) :
                new StatelessPersistentReader(this, guid, param, hist, listen, persistence);
    }
    else if (param.endpoint.reliabilityKind == RELIABLE)
    {
        SReader = (persistence == nullptr) ?
                new StatefulReader(this, guid, param, hist, listen) :
                new StatefulPersistentReader(this, guid, param, hist, listen, persistence);
    }

    if (SReader == nullptr)
    {
        return false;
    }

#if HAVE_SECURITY

    if (!isBuiltin)
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

    if (isBuiltin)
    {
        SReader->setTrustedWriter(TrustedWriter(SReader->getGuid().entityId));
    }

    if (enable)
    {
        if (!createAndAssociateReceiverswithEndpoint(SReader))
        {
            delete(SReader);
            return false;
        }
    }

    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);
    m_allReaderList.push_back(SReader);
    if (!isBuiltin)
    {
        m_userReaderList.push_back(SReader);
    }
    *ReaderOut = SReader;

    return true;
}

RTPSReader* RTPSParticipantImpl::find_local_reader(
        const GUID_t& reader_guid)
{
    // As this is only called from RTPSDomainImpl::find_local_reader, and it has
    // the domain mutex taken, there is no need to take the participant mutex
    // std::lock_guard<std::recursive_mutex> guard(*mp_mutex);

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
    // As this is only called from RTPSDomainImpl::find_local_reader, and it has
    // the domain mutex taken, there is no need to take the participant mutex
    // std::lock_guard<std::recursive_mutex> guard(*mp_mutex);

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
        const ReaderQos& rqos)
{
    return this->mp_builtinProtocols->addLocalReader(reader, topicAtt, rqos);
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
        const ReaderQos& rqos)
{
    return this->mp_builtinProtocols->updateLocalReader(reader, topicAtt, rqos);
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
    if (kind == WRITER)
    {
        for (std::vector<RTPSWriter*>::const_iterator it = m_userWriterList.begin(); it != m_userWriterList.end(); ++it)
        {
            if (ent == (*it)->getGuid().entityId)
            {
                return true;
            }
        }
    }
    else
    {
        for (std::vector<RTPSReader*>::const_iterator it = m_userReaderList.begin(); it != m_userReaderList.end(); ++it)
        {
            if (ent == (*it)->getGuid().entityId)
            {
                return true;
            }
        }
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
        Endpoint* pend)
{
    /*	This function...
        - Asks the network factory for new resources
        - Encapsulates the new resources within the ReceiverControlBlock list
        - Associated the endpoint to the new elements in the list
        - Launches the listener thread
     */
    // 1 - Ask the network factory to generate the elements that do still not exist
    std::vector<ReceiverResource> newItems;                         //Store the newly created elements
    std::vector<ReceiverResource> newItemsBuffer;                   //Store intermediate results
    //Iterate through the list of unicast and multicast locators the endpoint has... unless its empty
    //In that case, just use the standard
    if (pend->getAttributes().unicastLocatorList.empty() && pend->getAttributes().multicastLocatorList.empty())
    {
        //Default unicast
        pend->getAttributes().unicastLocatorList = m_att.defaultUnicastLocatorList;
    }
    createReceiverResources(pend->getAttributes().unicastLocatorList, false, true);
    createReceiverResources(pend->getAttributes().multicastLocatorList, false, true);

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
    LocatorList_t finalList;
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
            logWarning(RTPS_PARTICIPANT, "Cannot create send resource for endpoint remote locator (" <<
                    pend->getGuid() << ", " << (*it) << ")");
        }
    }

    return true;
}

void RTPSParticipantImpl::createReceiverResources(
        LocatorList_t& Locator_list,
        bool ApplyMutation,
        bool RegisterReceiver)
{
    std::vector<std::shared_ptr<ReceiverResource>> newItemsBuffer;

#if HAVE_SECURITY
    // An auxilary buffer is needed in the ReceiverResource to to decrypt the message,
    // that imposes a limit in the received messages size even if the transport allows (uint32_t) messages size.
    uint32_t max_receiver_buffer_size =
            is_secure() ? std::numeric_limits<uint16_t>::max() : std::numeric_limits<uint32_t>::max();
#else
    uint32_t max_receiver_buffer_size = std::numeric_limits<uint32_t>::max();
#endif // if HAVE_SECURITY

    for (auto it_loc = Locator_list.begin(); it_loc != Locator_list.end(); ++it_loc)
    {
        bool ret = m_network_Factory.BuildReceiverResources(*it_loc, newItemsBuffer, max_receiver_buffer_size);
        if (!ret && ApplyMutation)
        {
            uint32_t tries = 0;
            while (!ret && (tries < m_att.builtin.mutation_tries))
            {
                tries++;
                *it_loc = applyLocatorAdaptRule(*it_loc);
                ret = m_network_Factory.BuildReceiverResources(*it_loc, newItemsBuffer, max_receiver_buffer_size);
            }
        }

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
}

void RTPSParticipantImpl::createSenderResources(
        const LocatorList_t& locator_list)
{
    std::unique_lock<std::timed_mutex> lock(m_send_resources_mutex_);

    for (auto it_loc = locator_list.begin(); it_loc != locator_list.end(); ++it_loc)
    {
        m_network_Factory.build_send_resources(send_resource_list_, *it_loc);
    }
}

void RTPSParticipantImpl::createSenderResources(
        const Locator_t& locator)
{
    std::unique_lock<std::timed_mutex> lock(m_send_resources_mutex_);

    m_network_Factory.build_send_resources(send_resource_list_, locator);
}

bool RTPSParticipantImpl::deleteUserEndpoint(
        Endpoint* p_endpoint)
{
    m_receiverResourcelistMutex.lock();
    for (auto it = m_receiverResourcelist.begin(); it != m_receiverResourcelist.end(); ++it)
    {
        it->mp_receiver->removeEndpoint(p_endpoint);
    }
    m_receiverResourcelistMutex.unlock();

    bool found = false, found_in_users = false;
    {
        if (p_endpoint->getAttributes().endpointKind == WRITER)
        {
            std::lock_guard<std::recursive_mutex> guard(*mp_mutex);
            for (auto wit = m_userWriterList.begin(); wit != m_userWriterList.end(); ++wit)
            {
                if ((*wit)->getGuid().entityId == p_endpoint->getGuid().entityId) //Found it
                {
                    m_userWriterList.erase(wit);
                    found_in_users = true;
                    break;
                }
            }
            for (auto wit = m_allWriterList.begin(); wit != m_allWriterList.end(); ++wit)
            {
                if ((*wit)->getGuid().entityId == p_endpoint->getGuid().entityId) //Found it
                {
                    m_allWriterList.erase(wit);
                    found = true;
                    break;
                }
            }
        }
        else
        {
            std::lock_guard<std::recursive_mutex> guard(*mp_mutex);
            for (auto rit = m_userReaderList.begin(); rit != m_userReaderList.end(); ++rit)
            {
                if ((*rit)->getGuid().entityId == p_endpoint->getGuid().entityId) //Found it
                {
                    m_userReaderList.erase(rit);
                    found_in_users = true;
                    break;
                }
            }
            for (auto rit = m_allReaderList.begin(); rit != m_allReaderList.end(); ++rit)
            {
                if ((*rit)->getGuid().entityId == p_endpoint->getGuid().entityId) //Found it
                {
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

        //REMOVE FOR BUILTINPROTOCOLS
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
    }
    //	std::lock_guard<std::recursive_mutex> guardEndpoint(*p_endpoint->getMutex());
    delete(p_endpoint);
    return true;
}

void RTPSParticipantImpl::normalize_endpoint_locators(
        EndpointAttributes& endpoint_att)
{
    // Locators with port 0, calculate port.
    for (Locator_t& loc : endpoint_att.unicastLocatorList)
    {
        m_network_Factory.fillDefaultUnicastLocator(domain_id_, loc, m_att);
    }
    for (Locator_t& loc : endpoint_att.multicastLocatorList)
    {
        m_network_Factory.fillDefaultUnicastLocator(domain_id_, loc, m_att);
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
        logWarning(RTPS_PARTICIPANT,
                "Remote Endpoints can only be activated with static discovery protocol over PDP simple protocol");
        return false;
    }

    if (PDPSimple* pS = dynamic_cast<PDPSimple*>(mp_builtinProtocols->mp_PDP))
    {
        return pS->newRemoteEndpointStaticallyDiscovered(pguid, userDefinedId, kind);
    }

    return false;
}

void RTPSParticipantImpl::ResourceSemaphorePost()
{
    if (mp_ResourceSemaphore != nullptr)
    {
        mp_ResourceSemaphore->post();
    }
}

void RTPSParticipantImpl::ResourceSemaphoreWait()
{
    if (mp_ResourceSemaphore != nullptr)
    {
        mp_ResourceSemaphore->wait();
    }
}

void RTPSParticipantImpl::assert_remote_participant_liveliness(
        const GuidPrefix_t& remote_guid)
{
    if (mp_builtinProtocols && mp_builtinProtocols->mp_PDP)
    {
        mp_builtinProtocols->mp_PDP->assert_remote_participant_liveliness(remote_guid);
    }
}

uint32_t RTPSParticipantImpl::getMaxMessageSize() const
{
#if HAVE_SECURITY
    // An auxilary buffer is needed in the ReceiverResource to to decrypt the message,
    // that imposes a limit in the received messages size even if the transport allows (uint32_t) messages size.
    // So the sender limits also its size.
    uint32_t max_receiver_buffer_size =
            is_secure() ? std::numeric_limits<uint16_t>::max() : std::numeric_limits<uint32_t>::max();
#else
    uint32_t max_receiver_buffer_size = std::numeric_limits<uint32_t>::max();
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

    return_value = mp_builtinProtocols->mp_PDP->getEDP()->pairing_remote_reader_with_local_writer_after_security(
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

    return_value = mp_builtinProtocols->mp_PDP->getEDP()->pairing_remote_writer_with_local_reader_after_security(
        local_reader, remote_writer_data);
    if (!return_value && mp_builtinProtocols->mp_WLP != nullptr)
    {
        return_value = mp_builtinProtocols->mp_WLP->pairing_remote_writer_with_local_reader_after_security(
            local_reader, remote_writer_data);
    }

    return return_value;
}

#endif // if HAVE_SECURITY

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

std::unique_ptr<RTPSMessageGroup_t> RTPSParticipantImpl::get_send_buffer()
{
    return send_buffers_->get_buffer(this);
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
    if (m_att.builtin.metatrafficMulticastLocatorList == MulticastLocatorList
            && m_att.builtin.metatrafficUnicastLocatorList == UnicastLocatorList)
    {
        // no mutation
        return false;
    }

    // If one of the locators is 0.0.0.0 we must replace it by all local interfaces like the framework does
    std::list<Locator_t> unicast_real_locators;
    LocatorListConstIterator it = UnicastLocatorList.begin(), old_it;
    LocatorList_t locals;

    do
    {
        // copy ordinary locators till the first ANY
        old_it = it;
        it = std::find_if(it, UnicastLocatorList.end(), IPLocator::isAny);

        // copy ordinary locators
        std::copy(old_it, it, std::back_inserter(unicast_real_locators));

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
            std::transform(locals.begin(),
                    locals.end(),
                    std::back_inserter(unicast_real_locators),
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
    struct ResetLogical : public std::unary_function<Locator_t, const Locator_t&>
    {
        typedef std::vector<std::shared_ptr<fastdds::rtps::TransportDescriptorInterface>> Transports;

        ResetLogical(
                const Transports& tp)
            : Transports_(tp)
            , tcp4(nullptr)
            , tcp6(nullptr)
        {
            for (auto desc : Transports_)
            {
                if (nullptr == tcp4)
                {
                    tcp4 = dynamic_cast<fastdds::rtps::TCPv4TransportDescriptor*>(desc.get());
                }

                if (nullptr == tcp6)
                {
                    tcp6 = dynamic_cast<fastdds::rtps::TCPv6TransportDescriptor*>(desc.get());
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

        Locator_t operator ()(
                const Locator_t& loc) const
        {
            Locator_t ret(loc);
            switch (loc.kind)
            {
                case LOCATOR_KIND_TCPv4:
                    IPLocator::setPhysicalPort(ret, Tcp4ListeningPort());
                    break;
                case LOCATOR_KIND_TCPv6:
                    IPLocator::setPhysicalPort(ret, Tcp6ListeningPort());
                    break;
            }
            return ret;
        }

        // reference to the transports
        const Transports& Transports_;
        TCPTransportDescriptor* tcp4, * tcp6;

    }
    transform_functor(m_att.userTransports);

    // transform-copy
    std::set<Locator_t> update_attributes;

    std::transform(m_att.builtin.metatrafficMulticastLocatorList.begin(),
            m_att.builtin.metatrafficMulticastLocatorList.end(),
            std::inserter(update_attributes, update_attributes.begin()),
            transform_functor);

    std::transform(m_att.builtin.metatrafficUnicastLocatorList.begin(),
            m_att.builtin.metatrafficUnicastLocatorList.end(),
            std::inserter(update_attributes, update_attributes.begin()),
            transform_functor);

    std::set<Locator_t> original_ones;

    std::transform(MulticastLocatorList.begin(),
            MulticastLocatorList.end(),
            std::inserter(original_ones, original_ones.begin()),
            transform_functor);

    std::transform(unicast_real_locators.begin(),
            unicast_real_locators.end(),
            std::inserter(original_ones, original_ones.begin()),
            transform_functor);

    // if equal then no mutation took place on physical ports
    return !(update_attributes == original_ones);
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
