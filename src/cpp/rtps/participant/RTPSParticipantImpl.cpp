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

#include "RTPSParticipantImpl.h"

#include "../flowcontrol/ThroughputController.h"
#include "../persistence/PersistenceService.h"

#include <fastrtps/rtps/resources/ResourceEvent.h>
#include <fastrtps/rtps/resources/AsyncWriterThread.h>

#include <fastrtps/rtps/messages/MessageReceiver.h>

#include <fastrtps/rtps/writer/StatelessWriter.h>
#include <fastrtps/rtps/writer/StatefulWriter.h>
#include <fastrtps/rtps/writer/StatelessPersistentWriter.h>
#include <fastrtps/rtps/writer/StatefulPersistentWriter.h>

#include <fastrtps/rtps/reader/StatelessReader.h>
#include <fastrtps/rtps/reader/StatefulReader.h>
#include <fastrtps/rtps/reader/StatelessPersistentReader.h>
#include <fastrtps/rtps/reader/StatefulPersistentReader.h>

#include <fastrtps/rtps/participant/RTPSParticipant.h>
#include <fastrtps/transport/UDPv4TransportDescriptor.h>

#include <fastrtps/rtps/RTPSDomain.h>

#include <fastrtps/rtps/builtin/BuiltinProtocols.h>
#include <fastrtps/rtps/builtin/discovery/participant/PDPSimple.h>
#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>
#include <fastrtps/rtps/builtin/liveliness/WLP.h>

#include <fastrtps/utils/IPFinder.h>
#include <fastrtps/utils/eClock.h>

#include <fastrtps/utils/Semaphore.h>

#include <mutex>
#include <algorithm>

#include <fastrtps/log/Log.h>

namespace eprosima {
namespace fastrtps{
namespace rtps {

static EntityId_t TrustedWriter(const EntityId_t& reader)
{
    return
        (reader == c_EntityId_SPDPReader) ? c_EntityId_SPDPWriter :
        (reader == c_EntityId_SEDPPubReader) ? c_EntityId_SEDPPubWriter :
        (reader == c_EntityId_SEDPSubReader) ? c_EntityId_SEDPSubWriter :
        (reader == c_EntityId_ReaderLiveliness) ? c_EntityId_WriterLiveliness :
        c_EntityId_Unknown;
}

Locator_t& RTPSParticipantImpl::applyLocatorAdaptRule(Locator_t& loc)
{
    // This is a completely made up rule
    // It is transport responsability to interpret this new port.
    loc.port += m_att.port.participantIDGain;
    return loc;
}

RTPSParticipantImpl::RTPSParticipantImpl(const RTPSParticipantAttributes& PParam, const GuidPrefix_t& guidP,
        RTPSParticipant* par, RTPSParticipantListener* plisten)
    : m_att(PParam)
    , m_guid(guidP ,c_EntityId_RTPSParticipant)
    , mp_event_thr(nullptr)
    , mp_builtinProtocols(nullptr)
    , mp_ResourceSemaphore(new Semaphore(0))
    , IdCounter(0)
#if HAVE_SECURITY
    , m_security_manager(this)
#endif
    , mp_participantListener(plisten)
    , mp_userParticipant(par)
    , mp_mutex(new std::recursive_mutex())
{
    // Builtin transport by default
    if (PParam.useBuiltinTransports)
    {
        UDPv4TransportDescriptor descriptor;
        descriptor.sendBufferSize = m_att.sendSocketBufferSize;
        descriptor.receiveBufferSize = m_att.listenSocketBufferSize;
        m_network_Factory.RegisterTransport(&descriptor);
    }

    // User defined transports
    for (const auto& transportDescriptor : PParam.userTransports)
    {
        m_network_Factory.RegisterTransport(transportDescriptor.get());
    }

    mp_userParticipant->mp_impl = this;
    mp_event_thr = new ResourceEvent();
    mp_event_thr->init_thread(this);

    // Throughput controller, if the descriptor has valid values
    if (PParam.throughputController.bytesPerPeriod != UINT32_MAX && PParam.throughputController.periodMillisecs != 0)
    {
        std::unique_ptr<FlowController> controller(new ThroughputController(PParam.throughputController, this));
        m_controllers.push_back(std::move(controller));
    }

    /// Creation of metatraffic locator and receiver resources
    uint32_t metatraffic_multicast_port = m_att.port.getMulticastPort(m_att.builtin.domainId);
    uint32_t metatraffic_unicast_port = m_att.port.getUnicastPort(m_att.builtin.domainId,
                                                                  static_cast<uint32_t>(m_att.participantID));

    /* If metatrafficMulticastLocatorList is empty, add mandatory default Locators
       Else -> Take them */

    /* INSERT DEFAULT MANDATORY MULTICAST LOCATORS HERE */
    if(m_att.builtin.metatrafficMulticastLocatorList.empty() && m_att.builtin.metatrafficUnicastLocatorList.empty())
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

    createReceiverResources(m_att.builtin.metatrafficMulticastLocatorList, true);
    createReceiverResources(m_att.builtin.metatrafficUnicastLocatorList, true);

    // Initial peers
    if(m_att.builtin.initialPeersList.empty())
    {
        m_att.builtin.initialPeersList = m_att.builtin.metatrafficMulticastLocatorList;
    }
    else
    {
        LocatorList_t initial_peers;
        initial_peers.swap(m_att.builtin.initialPeersList);

        std::for_each(initial_peers.begin(), initial_peers.end(),
            [&](Locator_t& locator) {
                m_network_Factory.configureInitialPeerLocator(locator, m_att);
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

        m_network_Factory.getDefaultUnicastLocators(m_att.defaultUnicastLocatorList, m_att);
    }
    else
    {
        // Locator with port 0, calculate port.
        std::for_each(m_att.defaultUnicastLocatorList.begin(), m_att.defaultUnicastLocatorList.end(),
            [&](Locator_t& loc)
        {
            m_network_Factory.fillDefaultUnicastLocator(loc, m_att);
        });

    }

    // Normalize unicast locators.
    m_network_Factory.NormalizeLocators(m_att.defaultUnicastLocatorList);

    createReceiverResources(m_att.defaultUnicastLocatorList, true);

    if (!hasLocatorsDefined)
    {
        logInfo(RTPS_PARTICIPANT, m_att.getName() << " Created with NO default Unicast Locator List, adding Locators:"
            << m_att.defaultUnicastLocatorList);
    }

    createReceiverResources(m_att.defaultMulticastLocatorList, true);

#if HAVE_SECURITY
    // Start security
    // TODO(Ricardo) Get returned value in future.
    m_security_manager_initialized = m_security_manager.init(security_attributes_, PParam.properties, m_is_security_active);
#endif

    //START BUILTIN PROTOCOLS
    mp_builtinProtocols = new BuiltinProtocols();
    if(!mp_builtinProtocols->initBuiltinProtocols(this,m_att.builtin))
    {
        logError(RTPS_PARTICIPANT, "The builtin protocols were not correctly initialized");
    }
    logInfo(RTPS_PARTICIPANT,"RTPSParticipant \"" <<  m_att.getName() << "\" with guidPrefix: " <<m_guid.guidPrefix);
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
    // Disable Retries on Transports
    m_network_Factory.Shutdown();

    // Safely abort threads.
    for(auto& block : m_receiverResourcelist)
    {
        block.Receiver->UnregisterReceiver(block.mp_receiver);
        block.disable();
    }

    while(m_userReaderList.size() > 0)
    {
        deleteUserEndpoint(static_cast<Endpoint*>(*m_userReaderList.begin()));
    }

    while(m_userWriterList.size() > 0)
    {
        deleteUserEndpoint(static_cast<Endpoint*>(*m_userWriterList.begin()));
    }

    delete(this->mp_builtinProtocols);

#if HAVE_SECURITY
    m_security_manager.destroy();
#endif

    // Destruct message receivers
    for (auto& block : m_receiverResourcelist)
    {
        delete block.mp_receiver;
    }
    m_receiverResourcelist.clear();

    delete(this->mp_ResourceSemaphore);
    delete(this->mp_userParticipant);
    send_resource_list_.clear();

    delete(this->mp_event_thr);
    delete(this->mp_mutex);
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
    std::string type = (param.endpoint.reliabilityKind == RELIABLE) ? "RELIABLE" :"BEST_EFFORT";
    logInfo(RTPS_PARTICIPANT," of type " << type);
    EntityId_t entId;
    if(entityId == c_EntityId_Unknown)
    {
        if(param.endpoint.topicKind == NO_KEY)
        {
            entId.value[3] = 0x03;
        }
        else if(param.endpoint.topicKind == WITH_KEY)
        {
            entId.value[3] = 0x02;
        }
        uint32_t idnum;
        if(param.endpoint.getEntityID() > 0)
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
        if(this->existsEntityId(entId, WRITER))
        {
            logError(RTPS_PARTICIPANT,"A writer with the same entityId already exists in this RTPSParticipant");
            return false;
        }
    }
    else
    {
        entId = entityId;
    }
    if(!param.endpoint.unicastLocatorList.isValid())
    {
        logError(RTPS_PARTICIPANT,"Unicast Locator List for Writer contains invalid Locator");
        return false;
    }
    if(!param.endpoint.multicastLocatorList.isValid())
    {
        logError(RTPS_PARTICIPANT,"Multicast Locator List for Writer contains invalid Locator");
        return false;
    }
    if(!param.endpoint.remoteLocatorList.isValid())
    {
        logError(RTPS_PARTICIPANT,"Remote Locator List for Writer contains invalid Locator");
        return false;
    }
    if (((param.throughputController.bytesPerPeriod != UINT32_MAX && param.throughputController.periodMillisecs != 0) ||
        (m_att.throughputController.bytesPerPeriod != UINT32_MAX && m_att.throughputController.periodMillisecs != 0))
        && param.mode != ASYNCHRONOUS_WRITER)
    {
        logError(RTPS_PARTICIPANT, "Writer has to be configured to publish asynchronously, because a flowcontroller was configured");
        return false;
    }

    // Get persistence service
    IPersistenceService* persistence = nullptr;
    if (param.endpoint.durabilityKind >= TRANSIENT)
    {
        persistence = get_persistence_service(param.endpoint);
        if (persistence == nullptr)
        {
            logError(RTPS_PARTICIPANT, "Couldn't create persistence service for transient/persistent writer");
            return false;
        }
    }

    normalize_endpoint_locators(param.endpoint);

    RTPSWriter* SWriter = nullptr;
    GUID_t guid(m_guid.guidPrefix,entId);
    if (param.endpoint.reliabilityKind == BEST_EFFORT)
    {
        SWriter = (persistence == nullptr) ?
            new StatelessWriter(this, guid, param, hist, listen) :
            new StatelessPersistentWriter(this, guid, param, hist, listen, persistence);
    }
    else if (param.endpoint.reliabilityKind == RELIABLE)
    {
        SWriter = (persistence == nullptr) ?
            new StatefulWriter(this, guid, param, hist, listen) :
            new StatefulPersistentWriter(this, guid, param, hist, listen, persistence);
    }

    if (SWriter == nullptr)
    {
        return false;
    }

#if HAVE_SECURITY
    if(!isBuiltin)
    {
        if(!m_security_manager.register_local_writer(SWriter->getGuid(),
                    param.endpoint.properties, SWriter->getAttributes().security_attributes()))
        {
            delete(SWriter);
            return false;
        }
    }
    else
    {
        if(!m_security_manager.register_local_builtin_writer(SWriter->getGuid(),
                    SWriter->getAttributes().security_attributes()))
        {
            delete(SWriter);
            return false;
        }
    }
#endif

    createSendResources(SWriter);
    if (param.endpoint.reliabilityKind == RELIABLE)
    {
        if (!createAndAssociateReceiverswithEndpoint(SWriter))
        {
            delete(SWriter);
            return false;
        }
    }

    // Asynchronous thread runs regardless of mode because of
    // nack response duties.
    AsyncWriterThread::addWriter(*SWriter);

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


bool RTPSParticipantImpl::createReader(
        RTPSReader** ReaderOut,
        ReaderAttributes& param,
        ReaderHistory* hist,
        ReaderListener* listen,
        const EntityId_t& entityId,
        bool isBuiltin,
        bool enable)
{
    std::string type = (param.endpoint.reliabilityKind == RELIABLE) ? "RELIABLE" :"BEST_EFFORT";
    logInfo(RTPS_PARTICIPANT," of type " << type);
    EntityId_t entId;
    if(entityId== c_EntityId_Unknown)
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
        if(this->existsEntityId(entId,WRITER))
        {
            logError(RTPS_PARTICIPANT,"A reader with the same entityId already exists in this RTPSParticipant");
            return false;
        }
    }
    else
    {
        entId = entityId;
    }
    if(!param.endpoint.unicastLocatorList.isValid())
    {
        logError(RTPS_PARTICIPANT,"Unicast Locator List for Reader contains invalid Locator");
        return false;
    }
    if(!param.endpoint.multicastLocatorList.isValid())
    {
        logError(RTPS_PARTICIPANT,"Multicast Locator List for Reader contains invalid Locator");
        return false;
    }
    if(!param.endpoint.remoteLocatorList.isValid())
    {
        logError(RTPS_PARTICIPANT,"Remote Locator List for Reader contains invalid Locator");
        return false;
    }

    // Get persistence service
    IPersistenceService* persistence = nullptr;
    if (param.endpoint.durabilityKind >= TRANSIENT)
    {
        persistence = get_persistence_service(param.endpoint);
        if (persistence == nullptr)
        {
            logError(RTPS_PARTICIPANT, "Couldn't create persistence service for transient/persistent reader");
            return false;
        }
    }

    normalize_endpoint_locators(param.endpoint);

    RTPSReader* SReader = nullptr;
    GUID_t guid(m_guid.guidPrefix,entId);
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

    if(!isBuiltin)
    {
        if(!m_security_manager.register_local_reader(SReader->getGuid(),
                    param.endpoint.properties, SReader->getAttributes().security_attributes()))
        {
            delete(SReader);
            return false;
        }
    }
    else
    {
        if(!m_security_manager.register_local_builtin_reader(SReader->getGuid(),
                    SReader->getAttributes().security_attributes()))
        {
            delete(SReader);
            return false;
        }
    }
#endif

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

bool RTPSParticipantImpl::enableReader(RTPSReader *reader)
{
    if (!assignEndpointListenResources(reader))
    {
        return false;
    }
    return true;
}

// Avoid to receive PDPSimple reader a DATA while calling ~PDPSimple and EDP was destroy already.
void RTPSParticipantImpl::disableReader(RTPSReader *reader)
{
    m_receiverResourcelistMutex.lock();
    for (auto it = m_receiverResourcelist.begin(); it != m_receiverResourcelist.end(); ++it)
    {
        it->mp_receiver->removeEndpoint(reader);
    }
    m_receiverResourcelistMutex.unlock();
}

bool RTPSParticipantImpl::registerWriter(RTPSWriter* Writer, const TopicAttributes& topicAtt, const WriterQos& wqos)
{
    return this->mp_builtinProtocols->addLocalWriter(Writer, topicAtt, wqos);
}

bool RTPSParticipantImpl::registerReader(RTPSReader* reader, const TopicAttributes& topicAtt, const ReaderQos& rqos)
{
    return this->mp_builtinProtocols->addLocalReader(reader, topicAtt, rqos);
}

bool RTPSParticipantImpl::updateLocalWriter(RTPSWriter* Writer, const TopicAttributes& topicAtt, const WriterQos& wqos)
{
    return this->mp_builtinProtocols->updateLocalWriter(Writer, topicAtt, wqos);
}

bool RTPSParticipantImpl::updateLocalReader(RTPSReader* reader, const TopicAttributes& topicAtt, const ReaderQos& rqos)
{
    return this->mp_builtinProtocols->updateLocalReader(reader, topicAtt, rqos);
}


/*
 *
 * AUXILIARY METHODS
 *
 *
 */


bool RTPSParticipantImpl::existsEntityId(const EntityId_t& ent,EndpointKind_t kind) const
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
bool RTPSParticipantImpl::assignEndpointListenResources(Endpoint* endp)
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

bool RTPSParticipantImpl::createAndAssociateReceiverswithEndpoint(Endpoint * pend)
{
    /*	This function...
        - Asks the network factory for new resources
        - Encapsulates the new resources within the ReceiverControlBlock list
        - Associated the endpoint to the new elements in the list
        - Launches the listener thread
    */
    // 1 - Ask the network factory to generate the elements that do still not exist
    std::vector<ReceiverResource> newItems;							//Store the newly created elements
    std::vector<ReceiverResource> newItemsBuffer;					//Store intermediate results
    //Iterate through the list of unicast and multicast locators the endpoint has... unless its empty
    //In that case, just use the standard
    if (pend->getAttributes().unicastLocatorList.empty() && pend->getAttributes().multicastLocatorList.empty())
    {
        //Default unicast
        pend->getAttributes().unicastLocatorList = m_att.defaultUnicastLocatorList;
    }
    createReceiverResources(pend->getAttributes().unicastLocatorList, false);
    createReceiverResources(pend->getAttributes().multicastLocatorList, false);

    // Associate the Endpoint with ReceiverControlBlock
    assignEndpointListenResources(pend);
    return true;
}

bool RTPSParticipantImpl::assignEndpoint2LocatorList(Endpoint* endp, LocatorList_t& list)
{
    /* Note:
       The previous version of this function associated (or created) ListenResources and added the endpoint to them.
       It then requested the list of Locators the Listener is listening to and appended to the LocatorList_t from the paremeters.

       This has been removed becuase it is considered redundant. For ReceiveResources that listen on multiple interfaces, only
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

bool RTPSParticipantImpl::createSendResources(Endpoint *pend)
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
        if(!m_network_Factory.build_send_resources(send_resource_list_, (*it)))
        {
            logWarning(RTPS_PARTICIPANT, "Cannot create send resource for endpoint remote locator (" <<
                    pend->getGuid() << ", " << (*it) << ")");
        }
    }

    return true;
}

void RTPSParticipantImpl::createReceiverResources(LocatorList_t& Locator_list, bool ApplyMutation)
{
    std::vector<std::shared_ptr<ReceiverResource>> newItemsBuffer;

    uint32_t size = m_network_Factory.get_max_message_size_between_transports();
    for (auto it_loc = Locator_list.begin(); it_loc != Locator_list.end(); ++it_loc)
    {
        bool ret = m_network_Factory.BuildReceiverResources(*it_loc, size, newItemsBuffer);
        if (!ret && ApplyMutation)
        {
            uint32_t tries = 0;
            while (!ret && (tries < m_att.builtin.mutation_tries))
            {
                tries++;
                *it_loc = applyLocatorAdaptRule(*it_loc);
                ret = m_network_Factory.BuildReceiverResources(*it_loc, size, newItemsBuffer);
            }
        }

        for (auto it_buffer = newItemsBuffer.begin(); it_buffer != newItemsBuffer.end(); ++it_buffer)
        {
            std::lock_guard<std::mutex> lock(m_receiverResourcelistMutex);
            //Push the new items into the ReceiverResource buffer
            m_receiverResourcelist.emplace_back(*it_buffer);
            //Create and init the MessageReceiver
            auto mr = new MessageReceiver(this, size);
            m_receiverResourcelist.back().mp_receiver = mr;
            //Start reception
            m_receiverResourcelist.back().Receiver->RegisterReceiver(mr);
        }
        newItemsBuffer.clear();
    }
}

void RTPSParticipantImpl::createSenderResources(LocatorList_t& Locator_list, bool ApplyMutation)
{
    std::unique_lock<std::timed_mutex> lock(m_send_resources_mutex_);

    for (auto it_loc = Locator_list.begin(); it_loc != Locator_list.end(); ++it_loc)
    {
        //TODO With two transports mutation not work.
        bool were_created = false;
        uint32_t tries = 0;

        do
        {
            if(tries > 0)
            {
                *it_loc = applyLocatorAdaptRule(*it_loc);
            }
            were_created = m_network_Factory.build_send_resources(send_resource_list_, *it_loc);
            ++tries;
        } while (!were_created && ApplyMutation && (tries <= m_att.builtin.mutation_tries));
    }
}

bool RTPSParticipantImpl::deleteUserEndpoint(Endpoint* p_endpoint)
{
    m_receiverResourcelistMutex.lock();
    for (auto it = m_receiverResourcelist.begin(); it != m_receiverResourcelist.end(); ++it)
    {
        it->mp_receiver->removeEndpoint(p_endpoint);
    }
    m_receiverResourcelistMutex.unlock();

    bool found = false, found_in_users = false;
    {
        if(p_endpoint->getAttributes().endpointKind == WRITER)
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
        if(p_endpoint->getAttributes().endpointKind == WRITER)
        {
            if (found_in_users)
            {
                mp_builtinProtocols->removeLocalWriter(static_cast<RTPSWriter*>(p_endpoint));
            }

#if HAVE_SECURITY
            if(p_endpoint->getAttributes().security_attributes().is_submessage_protected ||
                    p_endpoint->getAttributes().security_attributes().is_payload_protected)
            {
                m_security_manager.unregister_local_writer(p_endpoint->getGuid());
            }
#endif
        }
        else
        {
            if (found_in_users)
            {
                mp_builtinProtocols->removeLocalReader(static_cast<RTPSReader*>(p_endpoint));
            }

#if HAVE_SECURITY
            if(p_endpoint->getAttributes().security_attributes().is_submessage_protected ||
                    p_endpoint->getAttributes().security_attributes().is_payload_protected)
            {
                m_security_manager.unregister_local_reader(p_endpoint->getGuid());
            }
#endif
        }
    }
    //	std::lock_guard<std::recursive_mutex> guardEndpoint(*p_endpoint->getMutex());
    delete(p_endpoint);
    return true;
}

void RTPSParticipantImpl::normalize_endpoint_locators(EndpointAttributes& endpoint_att)
{
    // Locators with port 0, calculate port.
    for (Locator_t& loc : endpoint_att.unicastLocatorList)
    {
        m_network_Factory.fillDefaultUnicastLocator(loc, m_att);
    }
    for (Locator_t& loc : endpoint_att.multicastLocatorList)
    {
        m_network_Factory.fillDefaultUnicastLocator(loc, m_att);
    }

    // Normalize unicast locators
    if (!endpoint_att.unicastLocatorList.empty())
    {
        m_network_Factory.NormalizeLocators(endpoint_att.unicastLocatorList);
    }
}

ResourceEvent& RTPSParticipantImpl::getEventResource()
{
    return *this->mp_event_thr;
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

bool RTPSParticipantImpl::sendSync(
        CDRMessage_t* msg,
        Endpoint* /*pend*/,
        const Locator_t& destination_loc,
        std::chrono::steady_clock::time_point& max_blocking_time_point)
{
    bool ret_code = false;
    std::unique_lock<std::timed_mutex> lock(m_send_resources_mutex_, std::defer_lock);

    if(lock.try_lock_until(max_blocking_time_point))
    {
        ret_code = true;

        for (auto& send_resource : send_resource_list_)
        {
            send_resource->send(msg->buffer, msg->length, destination_loc);
        }
    }

    return ret_code;
}

void RTPSParticipantImpl::setGuid(GUID_t& guid)
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

bool RTPSParticipantImpl::newRemoteEndpointDiscovered(const GUID_t& pguid, int16_t userDefinedId, EndpointKind_t kind)
{
    if (m_att.builtin.use_STATIC_EndpointDiscoveryProtocol == false)
    {
        logWarning(RTPS_PARTICIPANT, "Remote Endpoints can only be activated with static discovery protocol");
        return false;
    }
    return mp_builtinProtocols->mp_PDP->newRemoteEndpointStaticallyDiscovered(pguid, userDefinedId, kind);
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

void RTPSParticipantImpl::assertRemoteRTPSParticipantLiveliness(const GuidPrefix_t& guidP)
{
    if (mp_builtinProtocols && mp_builtinProtocols->mp_PDP)
    {
        this->mp_builtinProtocols->mp_PDP->assertRemoteParticipantLiveliness(guidP);
    }
}

uint32_t RTPSParticipantImpl::getMaxMessageSize() const
{
    return m_network_Factory.get_max_message_size_between_transports();
}

uint32_t RTPSParticipantImpl::getMaxDataSize()
{
    return calculateMaxDataSize(getMaxMessageSize());
}

uint32_t RTPSParticipantImpl::calculateMaxDataSize(uint32_t length)
{
    uint32_t maxDataSize = length;

#if HAVE_SECURITY
    // If there is rtps messsage protection, reduce max size for messages,
    // because extra data is added on encryption.
    if(security_attributes_.is_rtps_protected)
    {
        maxDataSize -= m_security_manager.calculate_extra_size_for_rtps_message();
    }
#endif

    // RTPS header
    maxDataSize -= RTPSMESSAGE_HEADER_SIZE;
    return maxDataSize;
}

bool RTPSParticipantImpl::networkFactoryHasRegisteredTransports() const
{
    return m_network_Factory.numberOfRegisteredTransports() > 0;
}

#if HAVE_SECURITY
bool RTPSParticipantImpl::pairing_remote_reader_with_local_writer_after_security(const GUID_t& local_writer,
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

bool RTPSParticipantImpl::pairing_remote_writer_with_local_reader_after_security(const GUID_t& local_reader,
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

#endif

PDPSimple* RTPSParticipantImpl::pdpsimple()
{
    return mp_builtinProtocols->mp_PDP;
}

WLP* RTPSParticipantImpl::wlp()
{
    return mp_builtinProtocols->mp_WLP;
}

bool RTPSParticipantImpl::get_remote_writer_info(const GUID_t& writerGuid, WriterProxyData& returnedInfo)
{
    ParticipantProxyData pdata;
    if (this->mp_builtinProtocols->mp_PDP->lookupWriterProxyData(writerGuid, returnedInfo, pdata))
    {
        return true;
    }
    return false;
}

bool RTPSParticipantImpl::get_remote_reader_info(const GUID_t& readerGuid, ReaderProxyData& returnedInfo)
{
    ParticipantProxyData pdata;
    if (this->mp_builtinProtocols->mp_PDP->lookupReaderProxyData(readerGuid, returnedInfo, pdata))
    {
        return true;
    }
    return false;
}

IPersistenceService* RTPSParticipantImpl::get_persistence_service(const EndpointAttributes& param)
{
    IPersistenceService* ret_val;

    ret_val = PersistenceFactory::create_persistence_service(param.properties);
    return ret_val != nullptr ?
        ret_val :
        PersistenceFactory::create_persistence_service(m_att.properties);
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
