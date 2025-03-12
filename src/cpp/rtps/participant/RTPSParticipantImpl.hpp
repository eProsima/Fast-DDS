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
 * @file RTPSParticipantImpl.hpp
 */

#ifndef FASTDDS_RTPS_PARTICIPANT__RTPSPARTICIPANTIMPL_H
#define FASTDDS_RTPS_PARTICIPANT__RTPSPARTICIPANTIMPL_H
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <list>
#include <mutex>
#include <set>
#include <sys/types.h>
#include <vector>

#if defined(_WIN32)
#include <process.h>
#else
#include <unistd.h>
#endif // if defined(_WIN32)

#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/builtin/data/ContentFilterProperty.hpp>
#include <fastdds/rtps/builtin/data/SubscriptionBuiltinTopicData.hpp>
#include <fastdds/rtps/builtin/data/ParticipantBuiltinTopicData.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/history/IChangePool.hpp>
#include <fastdds/rtps/history/IPayloadPool.hpp>
#include <fastdds/rtps/transport/SenderResource.hpp>

#include "../flowcontrol/FlowControllerFactory.hpp"
#include <fastdds/utils/TypePropagation.hpp>
#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>
#include <rtps/messages/MessageReceiver.h>
#include <rtps/messages/RTPSMessageGroup_t.hpp>
#include <rtps/messages/SendBuffersManager.hpp>
#include <rtps/network/NetworkFactory.hpp>
#include <rtps/network/ReceiverResource.h>
#include <rtps/reader/LocalReaderPointer.hpp>
#include <rtps/resources/ResourceEvent.h>
#include <statistics/rtps/monitor-service/interfaces/IConnectionsObserver.hpp>
#include <statistics/rtps/monitor-service/interfaces/IConnectionsQueryable.hpp>
#include <statistics/rtps/StatisticsBase.hpp>
#include <statistics/types/monitorservice_types.hpp>
#include <utils/shared_mutex.hpp>

#if HAVE_SECURITY
#include <fastdds/rtps/Endpoint.hpp>
#include <rtps/security/accesscontrol/ParticipantSecurityAttributes.h>
#include <rtps/security/SecurityManager.h>
#include <rtps/security/SecurityPluginFactory.h>
#endif // if HAVE_SECURITY

namespace eprosima {

namespace fastdds {

#ifdef FASTDDS_STATISTICS

namespace statistics {
namespace rtps {

struct IStatusQueryable;
struct IStatusObserver;
struct IConnectionsObserver;
class SimpleQueryable;
class MonitorService;

} // namespace rtps
} // namespace statistics

#endif //FASTDDS_STATISTICS

namespace rtps {

class BaseReader;
class BaseWriter;

} // namespace rtps

namespace dds {
namespace builtin {

class TypeLookupManager;

} // namespace builtin
} // namespace dds
} // namespace fastdds

namespace fastdds {

class MessageReceiver;

namespace rtps {

struct PublicationBuiltinTopicData;
struct TopicDescription;
struct RemoteLocatorList;
class RTPSParticipant;
class RTPSParticipantListener;
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
class PDP;
class PDPSimple;
class IPersistenceService;
class WLP;

/**
 * @brief Class RTPSParticipantImpl, it contains the private implementation of the RTPSParticipant functions and
 * allows the creation and removal of writers and readers. It manages the send and receive threads.
 * @ingroup RTPS_MODULE
 */
class RTPSParticipantImpl
    : public fastdds::statistics::StatisticsParticipantImpl,
    public fastdds::statistics::rtps::IConnectionsQueryable
#if HAVE_SECURITY
    , private security::SecurityPluginFactory
#endif // if HAVE_SECURITY
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
        MessageReceiver* mp_receiver;                  //Associated Readers/Writers inside of MessageReceiver

        ReceiverControlBlock(
                std::shared_ptr<ReceiverResource>& rec)
            : Receiver(rec)
            , mp_receiver(nullptr)
        {
        }

        ReceiverControlBlock(
                ReceiverControlBlock&& origen)
            : Receiver(origen.Receiver)
            , mp_receiver(origen.mp_receiver)
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

        ReceiverControlBlock(
                const ReceiverControlBlock&) = delete;
        const ReceiverControlBlock& operator =(
                const ReceiverControlBlock&) = delete;

    } ReceiverControlBlock;

public:

    /**
     * @param param
     * @param guidP
     * @param part
     * @param plisten
     */
    RTPSParticipantImpl(
            uint32_t domain_id,
            const RTPSParticipantAttributes& param,
            const GuidPrefix_t& guidP,
            RTPSParticipant* part,
            RTPSParticipantListener* plisten = nullptr);

    /**
     * @param param
     * @param guidP
     * @param persistence_guid
     * @param part
     * @param plisten
     */
    RTPSParticipantImpl(
            uint32_t domain_id,
            const RTPSParticipantAttributes& param,
            const GuidPrefix_t& guidP,
            const GuidPrefix_t& persistence_guid,
            RTPSParticipant* part,
            RTPSParticipantListener* plisten = nullptr);

    virtual ~RTPSParticipantImpl();

    // Create receiver resources and start builtin protocols
    void enable();

    // Stop builtin protocols and delete receiver resources
    void disable();

    /**
     * Get associated GUID
     * @return Associated GUID
     */
    inline const GUID_t& getGuid() const
    {
        return m_guid;
    }

    void setGuid(
            GUID_t& guid);

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
    bool newRemoteEndpointDiscovered(
            const GUID_t& pguid,
            int16_t userDefinedId,
            EndpointKind_t kind);

    /**
     * Assert the liveliness of a remote participant
     * @param remote_guid GuidPrefix_t of the participant.
     */
    void assert_remote_participant_liveliness(
            const GuidPrefix_t& remote_guid);

    /**
     * Get the RTPSParticipant ID
     * @return RTPSParticipant ID
     */
    inline uint32_t getRTPSParticipantID() const
    {
        return (uint32_t)m_att.participantID;
    }

    //!Get Pointer to the Event Resource.
    ResourceEvent& getEventResource()
    {
        return mp_event_thr;
    }

    /**
     * Send a message to several locations
     * @param buffers Vector of buffers to send.
     * @param total_bytes Total number of bytes to send.
     * @param sender_guid GUID of the producer of the message.
     * @param destination_locators_begin Iterator at the first destination locator.
     * @param destination_locators_end Iterator at the end destination locator.
     * @param max_blocking_time_point execution time limit timepoint.
     * @return true if at least one locator has been sent.
     */
    template<class LocatorIteratorT>
    bool sendSync(
            const std::vector<NetworkBuffer>& buffers,
            const uint32_t& total_bytes,
            const GUID_t& sender_guid,
            const LocatorIteratorT& destination_locators_begin,
            const LocatorIteratorT& destination_locators_end,
            std::chrono::steady_clock::time_point& max_blocking_time_point)
    {
        bool ret_code = false;
#if HAVE_STRICT_REALTIME
        std::unique_lock<std::timed_mutex> lock(m_send_resources_mutex_, std::defer_lock);
        if (lock.try_lock_until(max_blocking_time_point))
#else
        std::unique_lock<std::timed_mutex> lock(m_send_resources_mutex_);
#endif // if HAVE_STRICT_REALTIME
        {
            ret_code = true;

            for (auto& send_resource : send_resource_list_)
            {
                LocatorIteratorT locators_begin = destination_locators_begin;
                LocatorIteratorT locators_end = destination_locators_end;
                send_resource->send(buffers, total_bytes, &locators_begin, &locators_end,
                        max_blocking_time_point);
            }

            lock.unlock();

            // notify statistics module
            on_rtps_send(
                sender_guid,
                destination_locators_begin,
                destination_locators_end,
                total_bytes);

            // checkout if sender is a discovery endpoint
            on_discovery_packet(
                sender_guid,
                destination_locators_begin,
                destination_locators_end);
        }

        return ret_code;
    }

    /**
     * Get the participant listener
     * @return participant listener
     */
    inline RTPSParticipantListener* getListener()
    {
        std::lock_guard<std::mutex> _(mutex_);
        return mp_participantListener;
    }

    /**
     * @brief Modifies the participant listener
     * @param listener
     */
    void set_listener(
            RTPSParticipantListener* listener)
    {
        std::lock_guard<std::mutex> _(mutex_);
        mp_participantListener = listener;
    }

    std::vector<std::string> getParticipantNames() const;

    /**
     * Get the participant
     * @return participant
     */
    inline RTPSParticipant* getUserRTPSParticipant()
    {
        return mp_userParticipant;
    }

    uint32_t getMaxMessageSize() const;

    uint32_t getMaxDataSize();

    uint32_t calculateMaxDataSize(
            uint32_t length);

#if HAVE_SECURITY
    uint32_t calculate_extra_size_for_rtps_message()
    {
        uint32_t ret_val = 0u;
        if (security_attributes_.is_rtps_protected)
        {
            ret_val = m_security_manager.calculate_extra_size_for_rtps_message();
        }

        return ret_val;
    }

    security::SecurityManager& security_manager()
    {
        return m_security_manager;
    }

    const security::ParticipantSecurityAttributes& security_attributes()
    {
        return security_attributes_;
    }

    inline bool is_security_initialized() const
    {
        return m_security_manager.is_security_initialized();
    }

    inline bool is_secure() const
    {
        return m_security_manager.is_security_active();
    }

    bool pairing_remote_reader_with_local_writer_after_security(
            const GUID_t& local_writer,
            const ReaderProxyData& remote_reader_data);

    bool pairing_remote_writer_with_local_reader_after_security(
            const GUID_t& local_reader,
            const WriterProxyData& remote_writer_data);

    /**
     * @brief Checks whether the writer has security attributes enabled
     * @param writer_attributes Attibutes of the writer as given to the create_writer
     */
    bool is_security_enabled_for_writer(
            const WriterAttributes& writer_attributes);

    /**
     * @brief Checks whether the reader has security attributes enabled
     * @param reader_attributes Attibutes of the reader as given to the create_reader
     */
    bool is_security_enabled_for_reader(
            const ReaderAttributes& reader_attributes);

    security::Logging* create_builtin_logging_plugin() override;

#endif // if HAVE_SECURITY

    PDPSimple* pdpsimple();

    PDP* pdp();

    WLP* wlp();

    fastdds::dds::builtin::TypeLookupManager* typelookup_manager() const;

    bool is_intraprocess_only() const
    {
        return is_intraprocess_only_;
    }

    NetworkFactory& network_factory()
    {
        return m_network_Factory;
    }

    inline bool has_shm_transport()
    {
        return has_shm_transport_;
    }

    uint32_t get_min_network_send_buffer_size()
    {
        return m_network_Factory.get_min_send_buffer_size();
    }

    /**
     * Get the list of locators from which this participant may send data.
     *
     * @param [out] locators  LocatorList_t where the list of locators will be stored.
     */
    void get_sending_locators(
            rtps::LocatorList_t& locators) const;

    /***
     * @returns A pointer to a local reader given its endpoint guid, or nullptr if not found.
     */
    std::shared_ptr<LocalReaderPointer> find_local_reader(
            const GUID_t& reader_guid);

    /***
     * @returns A pointer to a local writer given its endpoint guid, or nullptr if not found.
     */
    BaseWriter* find_local_writer(
            const GUID_t& writer_guid);

    /**
     * @brief Fills a new entityId if set to unknown, or checks if a entity already exists with that
     * entityId in other case.
     * @param entityId to check of fill. If filled, EntityKind will be "vendor-specific" (0x01)
     * @return True if filled or the entityId is available.
     */
    bool get_new_entity_id(
            EntityId_t& entityId);

    void set_check_type_function(
            std::function<bool(const std::string&)>&& check_type);

    bool check_type(
            const std::string& type_name)
    {
        if (type_check_fn_ != nullptr)
        {
            return type_check_fn_(type_name);
        }
        return false;
    }

    std::unique_ptr<RTPSMessageGroup_t> get_send_buffer(
            const std::chrono::steady_clock::time_point& max_blocking_time);
    void return_send_buffer(
            std::unique_ptr <RTPSMessageGroup_t>&& buffer);

    uint32_t get_domain_id() const;

    //!Compare metatraffic locators list searching for mutations
    bool did_mutation_took_place_on_meta(
            const LocatorList_t& MulticastLocatorList,
            const LocatorList_t& UnicastLocatorList) const;

    //! Getter client_override flag
    bool client_override()
    {
        return client_override_;
    }

    //! Setter client_override flag
    void client_override(
            bool value)
    {
        client_override_ = value;
    }

    //! Retrieve persistence guid prefix
    GuidPrefix_t get_persistence_guid_prefix() const
    {
        return m_persistence_guid.guidPrefix;
    }

    bool is_initialized() const
    {
        return initialized_;
    }

private:

    //! DomainId
    uint32_t domain_id_;
    //!Attributes of the RTPSParticipant
    RTPSParticipantAttributes m_att;
    //! Metatraffic unicast port used by default on this participant
    uint32_t metatraffic_unicast_port_ = 0;
    //!Guid of the RTPSParticipant.
    GUID_t m_guid;
    //! String containing the RTPSParticipant Guid.
    std::string guid_str_;
    //!Persistence guid of the RTPSParticipant
    GUID_t m_persistence_guid;
    //! Event Resource
    ResourceEvent mp_event_thr;
    //! BuiltinProtocols of this RTPSParticipant
    BuiltinProtocols* mp_builtinProtocols;
    //!Id counter to correctly assign the ids to writers and readers.
    std::atomic<uint32_t> IdCounter;
    //! Mutex to safely access endpoints collections
    mutable shared_mutex endpoints_list_mutex;
    //!Writer List.
    std::vector<BaseWriter*> m_allWriterList;
    //!Reader List
    std::vector<BaseReader*> m_allReaderList;
    //!Listen thread list.
    //!Writer List.
    std::vector<BaseWriter*> m_userWriterList;
    //!Reader List
    std::vector<BaseReader*> m_userReaderList;
    //!Network Factory
    NetworkFactory m_network_Factory;
    //! Type cheking function
    std::function<bool(const std::string&)> type_check_fn_;
    //!Pool of send buffers
    std::unique_ptr<SendBuffersManager> send_buffers_;
    //! Maximum number of bytes allowed for an RTPS datagram generated by this writer.
    uint32_t max_output_message_size_ = std::numeric_limits<uint32_t>::max();

    /**
     * Client override flag: SIMPLE participant that has been overriden with the environment variable and transformed
     * into a client.
     */
    bool client_override_;

    //! Autogenerated metatraffic locators flag
    bool internal_metatraffic_locators_;
    //! Autogenerated default locators flag
    bool internal_default_locators_;

#if HAVE_SECURITY
    // Security manager
    security::SecurityManager m_security_manager;
#endif // if HAVE_SECURITY

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

    //! Determine if the RTPSParticipantImpl was initialized successfully.
    bool initialized_ = false;

    //! Ignored entities collections
    std::set<GuidPrefix_t> ignored_participants_;
    std::set<GUID_t> ignored_writers_;
    std::set<GUID_t> ignored_readers_;
    //! Protect ignored entities collection concurrent access
    mutable shared_mutex ignored_mtx_;

    void setup_guids(
            const GuidPrefix_t& persistence_guid);
    bool setup_transports();
    void setup_timed_events();
    void setup_meta_traffic();
    void setup_user_traffic();
    void setup_initial_peers();
    void setup_output_traffic();
    bool setup_builtin_protocols();

    RTPSParticipantImpl& operator =(
            const RTPSParticipantImpl&) = delete;

    /**
     * Method to check if a specific entityId already exists in this RTPSParticipant
     * @param ent EnityId to check
     * @param kind Endpoint Kind.
     * @return True if exists.
     */
    bool entity_id_exists(
            const EntityId_t& ent,
            EndpointKind_t kind) const;

    /**
     * Method to check if the EntityId conditions are coherent with the endpoint:
     * - Checks if it already exists in this RTPSParticipant.
     * - It is consistent with the topic kind of the endpoint.
     *
     * @param entity_id EntityId to check
     * @param endpoint_kind Endpoint Kind.
     * @param topic_kind Topic kind.
     * @return True if the EntityId conditions are correct.
     */
    bool check_entity_id_conditions(
            const EntityId_t& entity_id,
            EndpointKind_t endpoint_kind,
            TopicKind_t topic_kind) const;

    /**
     * Assign an endpoint to the ReceiverResources, based on its LocatorLists.
     * @param endp Pointer to the endpoint.
     * @return True if correct.
     */
    bool assignEndpointListenResources(
            Endpoint* endp);

    /** Assign an endpoint to the ReceiverResources as specified specifically on parameter list
     * @param pend Pointer to the endpoint.
     * @param lit Locator list iterator.
     * @param isMulticast Boolean indicating that is multicast.
     * @param isFixed Boolean indicating that is a fixed listenresource.
     * @return True if assigned.
     */
    bool assignEndpoint2LocatorList(
            Endpoint* pend,
            LocatorList_t& list);

    /** Create the new ReceiverResources needed for a new Locator, contains the calls to assignEndpointListenResources
        and consequently assignEndpoint2LocatorList
        @param pend - Pointer to the endpoint which triggered the creation of the Receivers.
        @param unique_flows - Whether unique listening ports should be created for this endpoint.
        @param initial_unique_port - First unique listening port to try.
        @param final_unique_port - Unique listening port that will not be tried.
     */
    bool createAndAssociateReceiverswithEndpoint(
            Endpoint* pend,
            bool unique_flows = false,
            uint16_t initial_unique_port = 0,
            uint16_t final_unique_port = 0);

    /** Create non-existent SendResources based on the Locator list of the entity
        @param pend - Pointer to the endpoint whose SenderResources are to be created
     */
    bool createSendResources(
            Endpoint* pend);

    /** Add participant's external locators to endpoint's when none available
        @param endpoint - Pointer to the endpoint whose external locators are to be set
     */
    void setup_external_locators(
            Endpoint* endpoint);

    /** When we want to create a new Resource but the physical channel specified by the Locator
        can not be opened, we want to mutate the Locator to open a more or less equivalent channel.
        @param loc -  Locator we want to change
     */
    Locator_t& applyLocatorAdaptRule(
            Locator_t& loc);

    /**
     * Update port for all endpoint locators when it has a value of 0 and then
     * apply locator normalization.
     *
     * @param [in, out] endpoint_att  EndpointAttributes to be updated
     */
    void normalize_endpoint_locators(
            EndpointAttributes& endpoint_att);

    //! Participant Mutex
    mutable std::mutex mutex_;

    //!Will this participant use intraprocess only?
    bool is_intraprocess_only_;

#ifdef FASTDDS_STATISTICS
    std::unique_ptr<fastdds::statistics::rtps::MonitorService> monitor_server_;
    std::unique_ptr<fastdds::statistics::rtps::SimpleQueryable> simple_queryable_;
    std::atomic<const fastdds::statistics::rtps::IConnectionsObserver*> conns_observer_;
#endif // ifdef FASTDDS_STATISTICS

    /*
     * Flow controller factory.
     */
    FlowControllerFactory flow_controller_factory_;

#if HAVE_SECURITY
    security::ParticipantSecurityAttributes security_attributes_;
#endif // if HAVE_SECURITY

    //! Indicates whether the participant has shared-memory transport
    bool has_shm_transport_;

    /**
     * Get persistence service from factory, using endpoint attributes (or participant
     * attributes if endpoint does not define a persistence service config)
     */
    IPersistenceService* get_persistence_service(
            const EndpointAttributes& param);

    /**
     * Returns the Durability kind from which a endpoint is able to use the persistence service.
     */
    DurabilityKind_t get_persistence_durability_red_line(
            bool is_builtin_endpoint);

    /**
     * Check if persistence is required and return persistence service from factory,
     * using endpoint attributes (or participant
     * attributes if endpoint does not define a persistence service config)
     *
     * @param [in]  is_builtin  Whether the enpoint being created is a builtin one.
     * @param [in]  param       Attributes of the endpoint being created.
     * @param [out] service     Pointer to the persistence service.
     *
     * @return false if parameters are not consistent or the service should be created and couldn't
     * @return true if persistence service is not required
     * @return true if persistence service is created
     */
    bool get_persistence_service(
            bool is_builtin,
            const EndpointAttributes& param,
            IPersistenceService*& service);

    template<typename Functor>
    bool create_writer(
            RTPSWriter** writer_out,
            WriterAttributes& param,
            const EntityId_t& entity_id,
            bool is_builtin,
            const Functor& callback);

    template<typename Functor>
    bool create_reader(
            RTPSReader** reader_out,
            ReaderAttributes& param,
            const EntityId_t& entity_id,
            bool is_builtin,
            bool enable,
            const Functor& callback);

    /**
     * @brief Fill the default metatraffic locators.
     *
     * @param [in] att @ref RTPSParticipantAttributes in which the locators are filled.
     *
     * @note This function in meant to be used iff the locators are not provided by the user.
     */
    void get_default_metatraffic_locators(
            RTPSParticipantAttributes& att);

    /**
     * @brief Fill the default unicast locators.
     *
     * @param [in] att @ref RTPSParticipantAttributes in which the locators are filled.
     *
     * @note This function in meant to be used iff the locators are not provided by the user.
     */
    void get_default_unicast_locators(
            RTPSParticipantAttributes& att);

    bool match_local_endpoints_ = true;

    bool should_match_local_endpoints(
            const RTPSParticipantAttributes& att);

public:

    const RTPSParticipantAttributes& get_attributes() const;

    /**
     * Create a Writer in this RTPSParticipant.
     * @param Writer Pointer to pointer of the Writer, used as output. Only valid if return==true.
     * @param param WriterAttributes to define the Writer.
     * @param hist Pointer to the WriterHistory.
     * @param listen Pointer to the WriterListener.
     * @param entityId EntityId assigned to the Writer.
     * @param isBuiltin Bool value indicating if the Writer is builtin (Discovery or Liveliness protocol) or is created for the end user.
     * @return True if the Writer was correctly created.
     */
    bool createWriter(
            RTPSWriter** Writer,
            WriterAttributes& param,
            WriterHistory* hist,
            WriterListener* listen,
            const EntityId_t& entityId = c_EntityId_Unknown,
            bool isBuiltin = false);

    /**
     * Create a Writer in this RTPSParticipant with a custom payload pool.
     *
     * @param Writer     Pointer to pointer of the Writer, used as output. Only valid if return==true.
     * @param watt       WriterAttributes to define the Writer.
     * @param hist       Pointer to the WriterHistory.
     * @param listen     Pointer to the WriterListener.
     * @param entityId   EntityId assigned to the Writer.
     * @param isBuiltin  Bool value indicating if the Writer is builtin (Discovery or Liveliness protocol) or is created for the end user.
     *
     * @return True if the Writer was correctly created.
     */
    bool create_writer(
            RTPSWriter** Writer,
            WriterAttributes& watt,
            WriterHistory* hist,
            WriterListener* listen,
            const EntityId_t& entityId,
            bool isBuiltin);

    /**
     * Create a Reader in this RTPSParticipant.
     * @param Reader Pointer to pointer of the Reader, used as output. Only valid if return==true.
     * @param param ReaderAttributes to define the Reader.
     * @param hist Pointer to the ReaderHistory.
     * @param listen Pointer to the ReaderListener.
     * @param entityId EntityId assigned to the Reader.
     * @param isBuiltin Bool value indicating if the Reader is builtin (Discovery or Liveliness protocol) or is created for the end user.
     * @param enable Whether the reader should be automatically enabled.
     * @return True if the Reader was correctly created.
     */
    bool createReader(
            RTPSReader** Reader,
            ReaderAttributes& param,
            ReaderHistory* hist,
            ReaderListener* listen,
            const EntityId_t& entityId = c_EntityId_Unknown,
            bool isBuiltin = false,
            bool enable = true);

    /**
     * Create a Reader in this RTPSParticipant with a custom payload pool.
     * @param Reader Pointer to pointer of the Reader, used as output. Only valid if return==true.
     * @param param ReaderAttributes to define the Reader.
     * @param payload_pool Shared pointer to the IPayloadPool
     * @param hist Pointer to the ReaderHistory.
     * @param listen Pointer to the ReaderListener.
     * @param entityId EntityId assigned to the Reader.
     * @param isBuiltin Bool value indicating if the Reader is builtin (Discovery or Liveliness protocol) or is created for the end user.
     * @param enable Whether the reader should be automatically enabled.
     * @return True if the Reader was correctly created.
     */
    bool createReader(
            RTPSReader** Reader,
            ReaderAttributes& param,
            const std::shared_ptr<IPayloadPool>& payload_pool,
            ReaderHistory* hist,
            ReaderListener* listen,
            const EntityId_t& entityId = c_EntityId_Unknown,
            bool isBuiltin = false,
            bool enable = true);

    bool enableReader(
            RTPSReader* reader);

    void disableReader(
            RTPSReader* reader);

    /**
     * Register a Writer in the BuiltinProtocols.
     *
     * @param Writer  Pointer to the RTPSWriter.
     * @param topic   Information regarding the topic where the writer is registering.
     * @param qos     Qos policies of the writer.
     *
     * @return True if correctly registered.
     */
    bool register_writer(
            RTPSWriter* Writer,
            const TopicDescription& topic,
            const fastdds::dds::WriterQos& qos);

    /**
     * Register a Reader in the BuiltinProtocols.
     *
     * @param Reader          Pointer to the RTPSReader.
     * @param topic           Information regarding the topic where the reader is registering.
     * @param qos             Qos policies of the reader.
     * @param content_filter  Optional content filtering information.
     *
     * @return True if correctly registered.
     */
    bool register_reader(
            RTPSReader* Reader,
            const TopicDescription& topic,
            const fastdds::dds::ReaderQos& qos,
            const ContentFilterProperty* content_filter = nullptr);

    /**
     * Update participant attributes.
     * @param patt New participant attributes.
     */
    void update_attributes(
            const RTPSParticipantAttributes& patt);

    /**
     * Update local writer QoS
     * @param rtps_writer Writer to update.
     * @param wqos        New QoS for the writer.
     * @return True on success
     */
    bool update_writer(
            RTPSWriter* rtps_writer,
            const fastdds::dds::WriterQos& wqos);

    /**
     * Update local reader QoS
     * @param rtps_reader      Reader to update.
     * @param rqos             New QoS for the reader.
     * @param content_filter   Optional content filtering information.
     * @return True on success
     */
    bool update_reader(
            RTPSReader* rtps_reader,
            const fastdds::dds::ReaderQos& rqos,
            const ContentFilterProperty* content_filter = nullptr);

    /**
     * Delete a user endpoint
     * @param Endpoint to delete
     * @return True on success
     */
    bool deleteUserEndpoint(
            const GUID_t&);

    //! Delete all user endpoints, builtin are disposed in its related classes
    void deleteAllUserEndpoints();

    /** Traverses the user writers collection transforming its elements with a provided functor
     * @param f - Functor applied to each element. Must accept a reference as parameter. Should return true to keep iterating.
     * @return Functor provided in order to allow aggregates retrieval
     */
    template<class Functor>
    Functor forEachUserWriter(
            Functor f)
    {
        // check if we are reentrying
        shared_lock<shared_mutex> _(endpoints_list_mutex);

        // traverse the list
        for (BaseWriter* pw : m_userWriterList)
        {
            if (!f(*pw))
            {
                break;
            }
        }

        return f;
    }

    /** Traverses the user readers collection transforming its elements with a provided functor
     * @param f - Functor applied to each element. Must accept a reference as parameter. Should return true to keep iterating.
     * @return Functor provided in order to allow aggregates retrieval
     */
    template<class Functor>
    Functor forEachUserReader(
            Functor f)
    {
        // check if we are reentrying
        shared_lock<shared_mutex> _(endpoints_list_mutex);

        for (BaseReader* pr : m_userReaderList)
        {
            if (!f(*pr))
            {
                break;
            }
        }

        return f;
    }

    /** Helper function that creates ReceiverResources based on a Locator_t List, possibly mutating
     * some and updating the list. DOES NOT associate endpoints with it.
     * @param Locator_list - Locator list to be used to create the ReceiverResources
     * @param ApplyMutation - True if we want to create a Resource with a "similar" locator if the one we provide is unavailable
     * @param RegisterReceiver - True if we want the receiver to be registered. Useful for receivers created after participant is enabled.
     * @param log_when_creation_fails - True if a log warning shall be issued for each locator when a receiver resource cannot be created.
     * @return True if a receiver resource was created for at least a locator in the list, false otherwise.
     */
    bool createReceiverResources(
            LocatorList_t& Locator_list,
            bool ApplyMutation,
            bool RegisterReceiver,
            bool log_when_creation_fails);

    void createSenderResources(
            const LocatorList_t& locator_list);

    void createSenderResources(
            const Locator_t& locator);

    void createSenderResources(
            const RemoteLocatorList& locator_list,
            const EndpointAttributes& param);

    /**
     * Creates sender resources for the given locator selector entry by calling the NetworkFactory's
     * build_send_resources method.
     *
     * @param locator_selector The locator selector entry for which sender resources need to be created.
     */
    void createSenderResources(
            const LocatorSelectorEntry& locator_selector);

    bool networkFactoryHasRegisteredTransports() const;

    /**
     * Function run when the RTPSDomain is notified that the environment file has changed.
     */
    void environment_file_has_changed();

    /**
     * @brief Query if the participant is found in the ignored collection
     *
     * @param [in] participant_guid Participant to be queried
     * @return True if found in the ignored collection. False otherwise.
     */
    bool is_participant_ignored(
            const GuidPrefix_t& participant_guid);

    /**
     * @brief Query if the writer is found in the ignored collection
     *
     * @param [in] writer_guid Writer to be queried
     * @return True if found in the ignored collection. False otherwise.
     */
    bool is_writer_ignored(
            const GUID_t& writer_guid);

    /**
     * @brief Query if the reader is found in the ignored collection
     *
     * @param [in] reader_guid Reader to be queried
     * @return True if found in the ignored collection. False otherwise.
     */
    bool is_reader_ignored(
            const GUID_t& reader_guid);

    /**
     * @brief Add a Participant into the corresponding ignore collection.
     *
     * @param [in] participant_guid Participant that is to be ignored.
     * @return True if correctly included into the ignore collection. False otherwise.
     */
    bool ignore_participant(
            const GuidPrefix_t& participant_guid);

    /**
     * @brief Add a Writer into the corresponding ignore collection.
     *
     * @param [in] writer_guid Writer that is to be ignored.
     * @return True if correctly included into the ignore collection. False otherwise.
     */
    bool ignore_writer(
            const GUID_t& writer_guid);

    /**
     * @brief Add a Reader into the corresponding ignore collection.
     *
     * @param [in] reader_guid Reader that is to be ignored.
     * @return True if correctly included into the ignore collection. False otherwise.
     */
    bool ignore_reader(
            const GUID_t& reader_guid);

    /**
     * @brief Returns registered transports' netmask filter information (transport's netmask filter kind and allowlist).
     *
     * @return A vector with all registered transports' netmask filter information.
     */
    std::vector<TransportNetmaskFilterInfo> get_netmask_filter_info() const;

    /**
     * @brief Fills the provided @ref PublicationBuiltinTopicData with the information of the
     * writer identified by writer_guid.
     *
     * @param[out] data @ref PublicationBuiltinTopicData to fill.
     * @param[in] writer_guid GUID of the writer to get the information from.
     * @return True if the writer was found and the data was filled.
     */
    bool get_publication_info(
            PublicationBuiltinTopicData& data,
            const GUID_t& writer_guid) const;

    /**
     * @brief Fills the provided @ref SubscriptionBuiltinTopicData with the information of the
     * reader identified by reader_guid.
     *
     * @param[out] data @ref SubscriptionBuiltinTopicData to fill.
     * @param[in] reader_guid GUID of the reader to get the information from.
     * @return True if the reader was found and the data was filled.
     */
    bool get_subscription_info(
            SubscriptionBuiltinTopicData& data,
            const GUID_t& reader_guid) const;

    template <EndpointKind_t kind, octet no_key, octet with_key>
    static bool preprocess_endpoint_attributes(
            const EntityId_t& entity_id,
            std::atomic<uint32_t>& id_count,
            EndpointAttributes& att,
            EntityId_t& entId);

#if HAVE_SECURITY
    void set_endpoint_rtps_protection_supports(
            Endpoint* endpoint,
            bool support)
    {
        endpoint->supports_rtps_protection_ = support;
    }

#endif // if HAVE_SECURITY

#ifdef FASTDDS_STATISTICS

    /** Register a listener in participant RTPSWriter entities.
     * @param listener, smart pointer to the listener interface to register
     * @param guid, RTPSWriter identifier. If unknown the listener is registered in all enable ones
     * @return true on success
     */
    bool register_in_writer(
            std::shared_ptr<fastdds::statistics::IListener> listener,
            GUID_t guid = GUID_t::unknown()) override;

    /** Register a listener in participant RTPSReader entities.
     * @param listener, smart pointer to the listener interface to register
     * @param guid, RTPSReader identifier. If unknown the listener is registered in all enable ones
     * @return true on success
     */
    bool register_in_reader(
            std::shared_ptr<fastdds::statistics::IListener> listener,
            GUID_t guid = GUID_t::unknown()) override;

    /** Unregister a listener in participant RTPSWriter entities.
     * @param listener, smart pointer to the listener interface to unregister
     * @return true on success
     */
    bool unregister_in_writer(
            std::shared_ptr<fastdds::statistics::IListener> listener) override;

    /** Unregister a listener in participant RTPSReader entities.
     * @param listener, smart pointer to the listener interface to unregister
     * @return true on success
     */
    bool unregister_in_reader(
            std::shared_ptr<fastdds::statistics::IListener> listener) override;

    /**
     * @brief Set the enabled statistics writers mask
     *
     * @param enabled_writers The new mask to set
     */
    void set_enabled_statistics_writers_mask(
            uint32_t enabled_writers) override;

    /**
     * Creates the monitor service in this RTPSParticipant with the provided interfaces.
     *
     * @param sq reference to the object implementing the StatusQueryable interface.
     * It will usually be the DDS DomainParticipant
     *
     * @return A const pointer to the listener (implemented within the RTPSParticipant)
     *
     */
    const fastdds::statistics::rtps::IStatusObserver* create_monitor_service(
            fastdds::statistics::rtps::IStatusQueryable& status_queryable);

    /**
     * Creates the monitor service in this RTPSParticipant with a simple default
     * implementation of the IStatusQueryable.
     *
     * @return true if the monitor service could be correctly created.
     *
     */
    bool create_monitor_service();

    /**
     * Returns whether the monitor service in created in this RTPSParticipant.
     *
     * @return true if the monitor service is created.
     * @return false otherwise.
     *
     */
    bool is_monitor_service_created() const;

    /**
     * Enables the monitor service in this RTPSParticipant.
     *
     * @return true if the monitor service could be correctly enabled.
     *
     */
    bool enable_monitor_service() const;

    /**
     * Disables the monitor service in this RTPSParticipant. Does nothing if the service was not enabled before.
     *
     * @return true if the monitor service could be correctly disabled.
     * @return false if the service could not be properly disabled or if the monitor service was not previously enabled.
     *
     */
    bool disable_monitor_service() const;

    /**
     * fills in the ParticipantBuiltinTopicData from a MonitorService Message
     *
     * @param [out] data Proxy to fill
     * @param [in] msg MonitorService Message to get the proxy information from.
     *
     * @return true if the operation succeeds.
     */
    bool fill_discovery_data_from_cdr_message(
            ParticipantBuiltinTopicData& data,
            const fastdds::statistics::MonitorServiceStatusData& msg);

    /**
     * fills in the PublicationBuiltinTopicData from a MonitorService Message
     *
     * @param [out] data Proxy to fill.
     * @param [in] msg MonitorService Message to get the proxy information from.
     *
     * @return true if the operation succeeds.
     */
    bool fill_discovery_data_from_cdr_message(
            PublicationBuiltinTopicData& data,
            const fastdds::statistics::MonitorServiceStatusData& msg);

    /**
     * fills in the SubscriptionBuiltinTopicData from a MonitorService Message
     *
     * @param [out] data Proxy to fill.
     * @param [in] msg MonitorService Message to get the proxy information from.
     *
     * @return true if the operation succeeds.
     */
    bool fill_discovery_data_from_cdr_message(
            SubscriptionBuiltinTopicData& data,
            const fastdds::statistics::MonitorServiceStatusData& msg);

    bool get_entity_connections(
            const GUID_t&,
            fastdds::statistics::rtps::ConnectionList& conn_list) override;

    const fastdds::statistics::rtps::IConnectionsObserver* get_connections_observer()
    {
        return conns_observer_.load();
    }

#else
    bool get_entity_connections(
            const GUID_t&,
            fastdds::statistics::rtps::ConnectionList&) override
    {
        return false;
    }

#endif // FASTDDS_STATISTICS

    bool should_match_local_endpoints()
    {
        return match_local_endpoints_;
    }

    /**
     * Method called on participant removal with the set of locators associated to the participant.
     *
     * @param remote_participant_locators Set of locators associated to the participant removed.
     */
    void update_removed_participant(
            const LocatorList_t& remote_participant_locators);

    /**
     * @brief Get participant's @ref dds::utils::TypePropagation
     *
     * @return This participant's @ref dds::utils::TypePropagation
     */
    dds::utils::TypePropagation type_propagation() const;

};
} // namespace rtps
} /* namespace rtps */
} /* namespace eprosima */
#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif //FASTDDS_RTPS_PARTICIPANT__RTPSPARTICIPANTIMPL_H
