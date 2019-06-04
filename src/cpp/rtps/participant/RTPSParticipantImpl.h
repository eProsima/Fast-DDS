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
 * @file RTPSParticipantImpl.h
 */

#ifndef RTPSParticipantIMPL_H_
#define RTPSParticipantIMPL_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <sys/types.h>
#include <mutex>
#include <atomic>
#include <chrono>
#include <fastrtps/utils/Semaphore.h>

#if defined(_WIN32)
#include <process.h>
#else
#include <unistd.h>
#endif

#include <fastrtps/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastrtps/rtps/common/Guid.h>
#include <fastrtps/rtps/builtin/discovery/endpoint/EDPSimple.h>
#include <fastrtps/rtps/builtin/data/ReaderProxyData.h>
#include <fastrtps/rtps/builtin/data/WriterProxyData.h>

#include <fastrtps/rtps/network/NetworkFactory.h>
#include <fastrtps/rtps/network/ReceiverResource.h>
#include <fastrtps/rtps/network/SenderResource.h>
#include <fastrtps/rtps/messages/MessageReceiver.h>

#if HAVE_SECURITY
#include <fastrtps/rtps/security/accesscontrol/ParticipantSecurityAttributes.h>
#include "../security/SecurityManager.h"
#endif

namespace eprosima {
namespace fastrtps{

class WriterQos;
class ReaderQos;
class TopicAttributes;
class MessageReceiver;

namespace rtps
{
class RTPSParticipant;
class RTPSParticipantListener;
class ResourceEvent;
class AsyncWriterThread;
class BuiltinProtocols;
struct CDRMessage_t;
class Endpoint;
class RTPSWriter;
class WriterAttributes;
class WriterHistory;
class WriterListener;
class RTPSReader;
class ReaderAttributes;
class ReaderHistory;
class ReaderListener;
class StatefulReader;
class PDPSimple;
class FlowController;
class IPersistenceService;

/**
    * @brief Class RTPSParticipantImpl, it contains the private implementation of the RTPSParticipant functions and
    * allows the creation and removal of writers and readers. It manages the send and receive threads.
    * @ingroup RTPS_MODULE
    */
class RTPSParticipantImpl
{
    /*
    Receiver Control block is a struct we use to encapsulate the resources that take part in message reception.
    It contains:
    -A ReceiverResource (as produced by the NetworkFactory Element)
    -Its associated MessageReceiver
    */
    typedef struct ReceiverControlBlock
    {
        std::shared_ptr<ReceiverResource> Receiver;
        MessageReceiver* mp_receiver; //Associated Readers/Writers inside of MessageReceiver
        ReceiverControlBlock(std::shared_ptr<ReceiverResource>& rec) :Receiver(rec), mp_receiver(nullptr)
        {
        }
        ReceiverControlBlock(ReceiverControlBlock&& origen) :Receiver(origen.Receiver), mp_receiver(origen.mp_receiver)
        {
            origen.mp_receiver = nullptr;
            origen.Receiver.reset();
        }

        void disable()
        {
            if (Receiver != nullptr)
            {
                Receiver->disable();
            }
        }

    private:
        ReceiverControlBlock(const ReceiverControlBlock&) = delete;
        const ReceiverControlBlock& operator=(const ReceiverControlBlock&) = delete;

    } ReceiverControlBlock;
public:
    /**
        * @param param
        * @param guidP
        * @param part
        * @param plisten
        */
    RTPSParticipantImpl(const RTPSParticipantAttributes &param, const GuidPrefix_t& guidP, RTPSParticipant* part,
        RTPSParticipantListener* plisten = nullptr);

    virtual ~RTPSParticipantImpl();

    /**
        * Get associated GUID
        * @return Associated GUID
        */
    inline const GUID_t& getGuid() const { return m_guid; }

    void setGuid(GUID_t& guid);

    //! Announce RTPSParticipantState (force the sending of a DPD message.)
    void announceRTPSParticipantState();

    //!Stop the RTPSParticipant Announcement (used in tests to avoid multiple packets being send)
    void stopRTPSParticipantAnnouncement();

    //!Reset to timer to make periodic RTPSParticipant Announcements.
    void resetRTPSParticipantAnnouncement();

    void loose_next_change();

    /**
        * Activate a Remote Endpoint defined in the Static Discovery.
        * @param pguid GUID_t of the endpoint.
        * @param userDefinedId userDeinfed Id of the endpoint.
        * @param kind kind of endpoint
        * @return True if correct.
        */
    bool newRemoteEndpointDiscovered(const GUID_t& pguid, int16_t userDefinedId, EndpointKind_t kind);

    /**
        * Assert the liveliness of a remote participant
        * @param guidP GuidPrefix_t of the participant.
        */
    void assertRemoteRTPSParticipantLiveliness(const GuidPrefix_t& guidP);

    /**
        * Get the RTPSParticipant ID
        * @return RTPSParticipant ID
        */
    inline uint32_t getRTPSParticipantID() const { return (uint32_t)m_att.participantID; };

    //!Post to the resource semaphore
    void ResourceSemaphorePost();

    //!Wait for the resource semaphore
    void ResourceSemaphoreWait();

    //!Get Pointer to the Event Resource.
    ResourceEvent& getEventResource();

    //!Send Method - Deprecated - Stays here for reference purposes
    bool sendSync(
            CDRMessage_t* msg,
            Endpoint *pend,
            const Locator_t& destination_loc,
            std::chrono::steady_clock::time_point& max_blocking_time_point);

    //!Get the participant Mutex
    std::recursive_mutex* getParticipantMutex() const { return mp_mutex; };

    /**
        * Get the participant listener
        * @return participant listener
        */
    inline RTPSParticipantListener* getListener() { return mp_participantListener; }

    std::vector<std::string> getParticipantNames() const;

    /**
        * Get the participant
        * @return participant
        */
    inline RTPSParticipant* getUserRTPSParticipant() { return mp_userParticipant; }

    std::vector<std::unique_ptr<FlowController>>& getFlowControllers() { return m_controllers; }

    /*!
        * @remarks Non thread-safe.
        */
    const std::vector<RTPSWriter*>& getAllWriters() const;

    /*!
        * @remarks Non thread-safe.
        */
    const std::vector<RTPSReader*>& getAllReaders() const;

    uint32_t getMaxMessageSize() const;

    uint32_t getMaxDataSize();

    uint32_t calculateMaxDataSize(uint32_t length);

#if HAVE_SECURITY
    security::SecurityManager& security_manager() { return m_security_manager; }

    const security::ParticipantSecurityAttributes& security_attributes() { return security_attributes_; }

    bool is_security_initialized() const { return m_security_manager_initialized; }

    bool is_secure() const { return m_is_security_active; }

    bool pairing_remote_reader_with_local_writer_after_security(const GUID_t& local_writer,
        const ReaderProxyData& remote_reader_data);

    bool pairing_remote_writer_with_local_reader_after_security(const GUID_t& local_reader,
        const WriterProxyData& remote_writer_data);

#endif

    PDPSimple* pdpsimple();

    bool get_remote_writer_info(const GUID_t& writerGuid, WriterProxyData& returnedInfo);

    bool get_remote_reader_info(const GUID_t& readerGuid, ReaderProxyData& returnedInfo);

    NetworkFactory& network_factory() { return m_network_Factory; }

    uint32_t get_min_network_send_buffer_size() { return m_network_Factory.get_min_send_buffer_size(); }

private:
    //!Attributes of the RTPSParticipant
    RTPSParticipantAttributes m_att;
    //!Guid of the RTPSParticipant.
    GUID_t m_guid;
    //! Sending resources. - DEPRECATED -Stays commented for reference purposes
    // ResourceSend* mp_send_thr;
    //! Event Resource
    ResourceEvent* mp_event_thr;
    //! BuiltinProtocols of this RTPSParticipant
    BuiltinProtocols* mp_builtinProtocols;
    //!Semaphore to wait for the listen thread creation.
    Semaphore* mp_ResourceSemaphore;
    //!Id counter to correctly assign the ids to writers and readers.
    uint32_t IdCounter;
    //!Writer List.
    std::vector<RTPSWriter*> m_allWriterList;
    //!Reader List
    std::vector<RTPSReader*> m_allReaderList;
    //!Listen thread list.
    //!Writer List.
    std::vector<RTPSWriter*> m_userWriterList;
    //!Reader List
    std::vector<RTPSReader*> m_userReaderList;
    //!Network Factory
    NetworkFactory m_network_Factory;

#if HAVE_SECURITY
        // Security manager
        security::SecurityManager m_security_manager;
        // Security manager initialization result
        bool m_security_manager_initialized;
        // Security activation flag
        bool m_is_security_active;
#endif

    //! Encapsulates all associated resources on a Receiving element.
    std::list<ReceiverControlBlock> m_receiverResourcelist;
    //! Receiver resource list needs its own mutext to avoid a race condition.
    std::mutex m_receiverResourcelistMutex;

    //!SenderResource List
    std::timed_mutex m_send_resources_mutex_;
    SendResourceList send_resource_list_;

    //!Participant Listener
    RTPSParticipantListener* mp_participantListener;
    //!Pointer to the user participant
    RTPSParticipant* mp_userParticipant;

    RTPSParticipantImpl& operator=(const RTPSParticipantImpl&) = delete;

    /**
        * Method to check if a specific entityId already exists in this RTPSParticipant
        * @param ent EnityId to check
        * @param kind Endpoint Kind.
        * @return True if exists.
        */
    bool existsEntityId(const EntityId_t& ent, EndpointKind_t kind) const;

    /**
        * Assign an endpoint to the ReceiverResources, based on its LocatorLists.
        * @param endp Pointer to the endpoint.
        * @return True if correct.
        */
    bool assignEndpointListenResources(Endpoint* endp);

    /** Assign an endpoint to the ReceiverResources as specified specifically on parameter list
        * @param pend Pointer to the endpoint.
        * @param lit Locator list iterator.
        * @param isMulticast Boolean indicating that is multicast.
        * @param isFixed Boolean indicating that is a fixed listenresource.
        * @return True if assigned.
        */
    bool assignEndpoint2LocatorList(Endpoint* pend, LocatorList_t& list);

    /** Create the new ReceiverResources needed for a new Locator, contains the calls to assignEndpointListenResources
        and consequently assignEndpoint2LocatorList
        @param pend - Pointer to the endpoint which triggered the creation of the Receivers
        */
    bool createAndAssociateReceiverswithEndpoint(Endpoint * pend);

    /** Create non-existent SendResources based on the Locator list of the entity
        @param pend - Pointer to the endpoint whose SenderResources are to be created
        */
    bool createSendResources(Endpoint *pend);

    /** When we want to create a new Resource but the physical channel specified by the Locator
        can not be opened, we want to mutate the Locator to open a more or less equivalent channel.
        @param loc -  Locator we want to change
        */
    Locator_t& applyLocatorAdaptRule(Locator_t &loc);

    /**
     * Update port for all endpoint locators when it has a value of 0 and then
     * apply locator normalization.
     *
     * @param [in, out] endpoint_att  EndpointAttributes to be updated
     */
    void normalize_endpoint_locators(EndpointAttributes& endpoint_att);

    //!Participant Mutex
    std::recursive_mutex* mp_mutex;

    /*
        * Flow controllers for this participant.
        */
    std::vector<std::unique_ptr<FlowController> > m_controllers;

#if HAVE_SECURITY
        security::ParticipantSecurityAttributes security_attributes_;
#endif

        /**
         * Get persistence service from factory, using endpoint attributes (or participant
         * attributes if endpoint does not define a persistence service config)
         */
        IPersistenceService* get_persistence_service(const EndpointAttributes& param);

    public:

        const RTPSParticipantAttributes& getRTPSParticipantAttributes() const
        {
            return this->m_att;
        }

        /**
         * Create a Writer in this RTPSParticipant.
         * @param Writer Pointer to pointer of the Writer, used as output. Only valid if return==true.
         * @param param WriterAttributes to define the Writer.
         * @param entityId EntityId assigned to the Writer.
         * @param isBuiltin Bool value indicating if the Writer is builtin (Discovery or Liveliness protocol) or is created for the end user.
         * @return True if the Writer was correctly created.
         */
        bool createWriter(RTPSWriter** Writer, WriterAttributes& param,WriterHistory* hist,WriterListener* listen,
                const EntityId_t& entityId = c_EntityId_Unknown,bool isBuiltin = false);

        /**
         * Create a Reader in this RTPSParticipant.
         * @param Reader Pointer to pointer of the Reader, used as output. Only valid if return==true.
         * @param param ReaderAttributes to define the Reader.
         * @param entityId EntityId assigned to the Reader.
         * @param isBuiltin Bool value indicating if the Reader is builtin (Discovery or Liveliness protocol) or is created for the end user.
         * @return True if the Reader was correctly created.
         */
        bool createReader(RTPSReader** Reader, ReaderAttributes& param,ReaderHistory* hist,ReaderListener* listen,
                const EntityId_t& entityId = c_EntityId_Unknown,bool isBuiltin = false, bool enable = true);

        bool enableReader(RTPSReader *reader);

        void disableReader(RTPSReader *reader);

        /**
         * Register a Writer in the BuiltinProtocols.
         * @param Writer Pointer to the RTPSWriter.
         * @param topicAtt TopicAttributes of the Writer.
         * @param wqos WriterQos.
         * @return True if correctly registered.
         */
        bool registerWriter(RTPSWriter* Writer, const TopicAttributes& topicAtt, const WriterQos& wqos);

        /**
         * Register a Reader in the BuiltinProtocols.
         * @param Reader Pointer to the RTPSReader.
         * @param topicAtt TopicAttributes of the Reader.
         * @param rqos ReaderQos.
         * @return  True if correctly registered.
         */
        bool registerReader(RTPSReader* Reader, const TopicAttributes& topicAtt, const ReaderQos& rqos);

        /**
         * Update local writer QoS
         * @param Writer Writer to update
         * @param wqos New QoS for the writer
         * @return True on success
         */
        bool updateLocalWriter(RTPSWriter* Writer, const TopicAttributes& topicAtt, const WriterQos& wqos);

        /**
         * Update local reader QoS
         * @param Reader Reader to update
         * @param rqos New QoS for the reader
         * @return True on success
         */
        bool updateLocalReader(RTPSReader* Reader, const TopicAttributes& topicAtt, const ReaderQos& rqos);



        /**
         * Get the participant attributes
         * @return Participant attributes
         */
        inline RTPSParticipantAttributes& getAttributes() {return m_att;};

        /**
         * Delete a user endpoint
         * @param Endpoint to delete
         * @return True on success
         */
        bool deleteUserEndpoint(Endpoint*);

        /**
         * Get the begin of the user reader list
         * @return Iterator pointing to the begin of the user reader list
         */
        std::vector<RTPSReader*>::iterator userReadersListBegin(){return m_userReaderList.begin();};

        /**
         * Get the end of the user reader list
         * @return Iterator pointing to the end of the user reader list
         */
        std::vector<RTPSReader*>::iterator userReadersListEnd(){return m_userReaderList.end();};

        /**
         * Get the begin of the user writer list
         * @return Iterator pointing to the begin of the user writer list
         */
        std::vector<RTPSWriter*>::iterator userWritersListBegin(){return m_userWriterList.begin();};

        /**
         * Get the end of the user writer list
         * @return Iterator pointing to the end of the user writer list
         */
        std::vector<RTPSWriter*>::iterator userWritersListEnd(){return m_userWriterList.end();};

        /** Helper function that creates ReceiverResources based on a Locator_t List, possibly mutating
          some and updating the list. DOES NOT associate endpoints with it.
          @param Locator_list - Locator list to be used to create the ReceiverResources
          @param ApplyMutation - True if we want to create a Resource with a "similar" locator if the one we provide is unavailable
          */
        void createReceiverResources(LocatorList_t& Locator_list, bool ApplyMutation);
        void createSenderResources(LocatorList_t& Locator_list, bool ApplyMutation);

        bool networkFactoryHasRegisteredTransports() const;

#if HAVE_SECURITY
    void set_endpoint_rtps_protection_supports(Endpoint* endpoint, bool support)
    {
        endpoint->supports_rtps_protection_ = support;
    }
#endif
};

}
} /* namespace rtps */
} /* namespace eprosima */
#endif
#endif /* RTPSParticipant_H_ */
