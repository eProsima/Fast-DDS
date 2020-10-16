// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DiscoveryDataBase.hpp
 *
 */

#ifndef _FASTDDS_RTPS_DISCOVERY_DATABASE_H_
#define _FASTDDS_RTPS_DISCOVERY_DATABASE_H_

#include <vector>
#include <map>
#include <mutex>

#include <fastrtps/utils/fixed_size_string.hpp>
#include <fastdds/rtps/writer/ReaderProxy.h>
#include <fastdds/rtps/common/CacheChange.h>
#include <fastrtps/utils/DBQueue.h>

#include "./DiscoveryDataFilter.hpp"
#include "./DiscoveryParticipantInfo.hpp"
#include "./DiscoveryEndpointInfo.hpp"
#include "./DiscoveryDataQueueInfo.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace ddb {

/**
 * Class to manage the discovery data base
 *@ingroup DISCOVERY_MODULE
 */
class DiscoveryDataBase
    : public eprosima::fastdds::rtps::ddb::PDPDataFilter<DiscoveryDataBase>
    , public eprosima::fastdds::rtps::ddb::EDPDataFilter<DiscoveryDataBase>
    , public eprosima::fastdds::rtps::ddb::EDPDataFilter<DiscoveryDataBase, false>
{

public:

    class AckedFunctor;

    ////////////
    // Functions to process_writers_acknowledgements()
    // Return the functor, class that works as a lambda
    AckedFunctor functor(
            eprosima::fastrtps::rtps::CacheChange_t* );

    class AckedFunctor
    {
        using argument_type = eprosima::fastrtps::rtps::ReaderProxy*;
        using result_type = void;

        // friend class DiscoveryDataBase;
        friend AckedFunctor DiscoveryDataBase::functor(
                eprosima::fastrtps::rtps::CacheChange_t* );

        // Stateful constructor
        // This constructor generates the only object that keeps the state
        // all other constructors reference this object state
        AckedFunctor(
                DiscoveryDataBase* db,
                eprosima::fastrtps::rtps::CacheChange_t* change);

    public:

        // Stateless constructors
        AckedFunctor(
                const AckedFunctor&);

        AckedFunctor(
                AckedFunctor&& r)
        // delegates in copy constructor
            : AckedFunctor(r)
        {
        }

        AckedFunctor() = delete;

        ~AckedFunctor();

        void operator () (
                eprosima::fastrtps::rtps::ReaderProxy* reader_proxy);

        operator bool() const
        {
            return external_pending_;
        }

    private:

        DiscoveryDataBase* db_;
        eprosima::fastrtps::rtps::CacheChange_t* change_;
        // stateful functor is the one contructed form the database
        bool pending_; // stateful functor state
        bool& external_pending_; // references stateful functor
    };

    friend class AckedFunctor;

    DiscoveryDataBase(
            fastrtps::rtps::GuidPrefix_t server_guid_prefix,
            std::vector<fastrtps::rtps::GuidPrefix_t> servers);

    ~DiscoveryDataBase();

    ////////////
    // Functions to update queue from listener
    /* Add a new CacheChange_t to database queue
     *    1. Check whether the change is already in the database (queue lock)
     *    2. If the change is new, then add it to data_queue_ (queue lock)
     * @return: True if the change was added, false otherwise.
     */
    bool update(
            eprosima::fastrtps::rtps::CacheChange_t* change,
            std::string topic_name);

    bool update(
            eprosima::fastrtps::rtps::CacheChange_t* change,
            DiscoveryParticipantChangeData participant_change_data);

    //! Enables the possibility to add new entries to the database. Enable by default.
    void enable()
    {
        enabled_ = true;
    }

    //! Disable the possibility to add new entries to the database
    void disable()
    {
        enabled_ = false;
    }

    //! Check whether the database is enabled
    bool is_enabled()
    {
        return enabled_;
    }

    /* Clear all the collections in the database
     * @return: The changes that can be released
     */
    std::vector<fastrtps::rtps::CacheChange_t*> clear();

    ////////////
    // Functions to is_relevant
    // Return whether a PDP change is relevant for a given reader
    bool pdp_is_relevant(
            const eprosima::fastrtps::rtps::CacheChange_t& change,
            const eprosima::fastrtps::rtps::GUID_t& reader_guid) const;

    // Return whether a EDP publications change is relevant for a given reader
    bool edp_publications_is_relevant(
            const eprosima::fastrtps::rtps::CacheChange_t& change,
            const eprosima::fastrtps::rtps::GUID_t& reader_guid) const;

    // Return whether a EDP subscription change is relevant for a given reader
    bool edp_subscriptions_is_relevant(
            const eprosima::fastrtps::rtps::CacheChange_t& change,
            const eprosima::fastrtps::rtps::GUID_t& reader_guid) const;

    ////////////
    // Functions to process PDP and EDP data queues
    void process_pdp_data_queue();

    bool process_edp_data_queue();

    ////////////
    // Functions to process_dirty_topics()
    bool process_dirty_topics();

    ////////////
    // Functions to process_disposals()
    const std::vector<eprosima::fastrtps::rtps::CacheChange_t*> changes_to_dispose();

    void clear_changes_to_dispose();

    /* Delete all information relative to the entity that produced a CacheChange
     * @change: That entity's CacheChange.
     * @return: True if the entity was deleted, false otherwise.
     */
    bool delete_entity_of_change(
            fastrtps::rtps::CacheChange_t* change);

    ////////////
    // Functions to process_to_send_lists()
    const std::vector<eprosima::fastrtps::rtps::CacheChange_t*> pdp_to_send();

    void clear_pdp_to_send();

    const std::vector<eprosima::fastrtps::rtps::CacheChange_t*> edp_publications_to_send();

    void clear_edp_publications_to_send();

    const std::vector<eprosima::fastrtps::rtps::CacheChange_t*> edp_subscriptions_to_send();

    void clear_edp_subscriptions_to_send();

    const std::vector<eprosima::fastrtps::rtps::CacheChange_t*> changes_to_release();

    void clear_changes_to_release();


    ////////////
    // Static Functions to work with GUIDs
    static bool is_participant(
            const eprosima::fastrtps::rtps::CacheChange_t* ch);

    static bool is_writer(
            const eprosima::fastrtps::rtps::CacheChange_t* ch);

    static bool is_reader(
            const eprosima::fastrtps::rtps::CacheChange_t* ch);

    static eprosima::fastrtps::rtps::GUID_t guid_from_change(
            const eprosima::fastrtps::rtps::CacheChange_t* ch);

    ////////////
    // Functions to work with own server

    // return the cache change of the server
    fastrtps::rtps::CacheChange_t* cache_change_own_participant();

    const std::vector<fastrtps::rtps::GuidPrefix_t> direct_clients_and_servers();

    fastrtps::rtps::LocatorList_t participant_metatraffic_locators(
            fastrtps::rtps::GuidPrefix_t participant_guid_prefix);

    // return a list of participants that are not the server one
    const std::vector<fastrtps::rtps::GuidPrefix_t> remote_participants();

    // set if the server has been acked by all the reader proxys in server
    bool server_acked_by_all() const
    {
        return server_acked_by_all_.load();
    }

    void server_acked_by_all(
            bool s)
    {
        server_acked_by_all_.store(s);
    }

    bool server_acked_by_my_servers();

    std::vector<fastrtps::rtps::GuidPrefix_t> ack_pending_servers();

    ////////////
    // Data structures utils

    // Check if the data queue is empty
    bool data_queue_empty();

protected:

    // change a cacheChange by update or new disposal
    void update_change_and_unmatch_(
            fastrtps::rtps::CacheChange_t* new_change,
            ddb::DiscoverySharedInfo& entity);

    // update the acks
    void add_ack_(
            const eprosima::fastrtps::rtps::CacheChange_t* change,
            const eprosima::fastrtps::rtps::GuidPrefix_t& acked_entity);

    ////////////
    // Mutex Functions
    void lock_()
    {
        mutex_.lock();
    }

    void unlock_()
    {
        mutex_.unlock();
    }

    ////////////
    // functions to manage new cacheChanges in update

    void create_participant_from_change_(
            eprosima::fastrtps::rtps::CacheChange_t* ch,
            const DiscoveryParticipantChangeData& change_data);

    void create_writers_from_change_(
            eprosima::fastrtps::rtps::CacheChange_t* ch,
            const std::string& topic_name);

    void create_readers_from_change_(
            eprosima::fastrtps::rtps::CacheChange_t* ch,
            const std::string& topic_name);

    // change ack relevants and matched between entities participants and endpoints
    void match_writer_reader_(
            const eprosima::fastrtps::rtps::GUID_t& writer_guid,
            const eprosima::fastrtps::rtps::GUID_t& reader_guid);

    void process_dispose_participant_(
            eprosima::fastrtps::rtps::CacheChange_t* ch);

    void process_dispose_writer_(
            eprosima::fastrtps::rtps::CacheChange_t* ch);

    void process_dispose_reader_(
            eprosima::fastrtps::rtps::CacheChange_t* ch);

    ////////////
    // functions to manage disposals and clean entities

    // unmatch in every other entity including its readers and writers
    void unmatch_participant_(
            const eprosima::fastrtps::rtps::GuidPrefix_t& guid_prefix);

    // unmatch all the readers and erase it from writers_by_topic
    void unmatch_writer_(
            const eprosima::fastrtps::rtps::GUID_t& guid);

    // unmatch all the writers and erase it from readers_by_topic
    void unmatch_reader_(
            const eprosima::fastrtps::rtps::GUID_t& guid);

    // delete an entity and set its change to release. Assumes the entity has been unmatched before
    bool delete_participant_entity_(
            const fastrtps::rtps::GuidPrefix_t& guid_prefix);

    std::map<eprosima::fastrtps::rtps::GuidPrefix_t, DiscoveryParticipantInfo>::iterator delete_participant_entity_(
            std::map<eprosima::fastrtps::rtps::GuidPrefix_t, DiscoveryParticipantInfo>::iterator it);

    // delete an entity and set its change to release. Assumes the entity has been unmatched before
    bool delete_writer_entity_(
            const fastrtps::rtps::GUID_t& guid);

    std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator delete_writer_entity_(
            std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator it);

    // delete an entity and set its change to release. Assumes the entity has been unmatched before
    bool delete_reader_entity_(
            const fastrtps::rtps::GUID_t& guid);

    std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator delete_reader_entity_(
            std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator it);

    // return if there are more than one writer in the participant in the same topic
    bool repeated_writer_topic_(
            const eprosima::fastrtps::rtps::GuidPrefix_t& participant,
            const std::string& topic_name);

    // return if there are more than one reader in the participant in the same topic
    bool repeated_reader_topic_(
            const eprosima::fastrtps::rtps::GuidPrefix_t& participant,
            const std::string& topic_name);

    void remove_writer_from_topic_(
            const eprosima::fastrtps::rtps::GUID_t& writer_guid,
            const std::string& topic_name);

    void remove_reader_from_topic_(
            const eprosima::fastrtps::rtps::GUID_t& reader_guid,
            const std::string& topic_name);

    // create a new topic in writers and readers and fill it with virtuals
    void create_topic_(
            const std::string& topic_name);

    void add_writer_to_topic_(
            const eprosima::fastrtps::rtps::GUID_t& writer_guid,
            const std::string& topic_name);

    void add_reader_to_topic_(
            const eprosima::fastrtps::rtps::GUID_t& reader_guid,
            const std::string& topic_name);

    //! Add a topic to the list of dirty topics, unless it's already present
    // Return true if added, false if already there
    bool set_dirty_topic_(
            std::string topic);

    // Add data in pdp_to_send if not already in it
    bool add_pdp_to_send_(
            eprosima::fastrtps::rtps::CacheChange_t* change);

    // Add data in edp_to_send if not already in it
    bool add_edp_publications_to_send_(
            eprosima::fastrtps::rtps::CacheChange_t* change);

    // Add data in edp_to_send if not already in it
    bool add_edp_subscriptions_to_send_(
            eprosima::fastrtps::rtps::CacheChange_t* change);

    // Get all the writers in given topic and in virtual topic
    std::vector<eprosima::fastrtps::rtps::GUID_t> get_writers_in_topic(
            const std::string& topic_name);

    // Get all the readers in given topic and in virtual topic
    std::vector<eprosima::fastrtps::rtps::GUID_t> get_readers_in_topic(
            const std::string& topic_name);

    ////////////////
    // Variables

    fastrtps::DBQueue<eprosima::fastdds::rtps::ddb::DiscoveryPDPDataQueueInfo> pdp_data_queue_;

    fastrtps::DBQueue<eprosima::fastdds::rtps::ddb::DiscoveryEDPDataQueueInfo> edp_data_queue_;

    //! Covenient per-topic mapping of readers and writers to speed-up queries
    std::map<std::string, std::vector<eprosima::fastrtps::rtps::GUID_t>> readers_by_topic_;
    std::map<std::string, std::vector<eprosima::fastrtps::rtps::GUID_t>> writers_by_topic_;

    //! Collection of participant proxies that:
    //  - stores the CacheChange_t
    //  - keeps track of its acknowledgement status
    //  - keeps an account of participant's readers and writers
    std::map<eprosima::fastrtps::rtps::GuidPrefix_t, DiscoveryParticipantInfo> participants_;

    //! Collection of reader and writer proxies that:
    //  - stores the CacheChange_t
    //  - keeps track of its acknowledgement status
    //  - stores the topic name (only matching criteria available)
    std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo> readers_;
    std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo> writers_;

    //! Collection of topics whose related endpoints have changed and require a match recalculation
    std::vector<std::string> dirty_topics_;

    //! Collection of changes to take out of the server builtin writers
    std::vector<eprosima::fastrtps::rtps::CacheChange_t*> disposals_;

    //! Collection of changes to put into the server builtin writers
    std::vector<eprosima::fastrtps::rtps::CacheChange_t*> pdp_to_send_;
    std::vector<eprosima::fastrtps::rtps::CacheChange_t*> edp_publications_to_send_;
    std::vector<eprosima::fastrtps::rtps::CacheChange_t*> edp_subscriptions_to_send_;

    //! changes that are no longer associated to living endpoints and should be returned to it's pool
    std::vector<eprosima::fastrtps::rtps::CacheChange_t*> changes_to_release_;

    //! Mutex
    mutable std::recursive_mutex mutex_;

    //! GUID prefix from own server
    const fastrtps::rtps::GuidPrefix_t server_guid_prefix_;

    //! Is own server DATA(p) acked by all other clients
    std::atomic<bool> server_acked_by_all_;

    //! List of GUID prefixes of the remote servers
    std::vector<fastrtps::rtps::GuidPrefix_t> servers_;

    // The number of local servers known by the database
    uint16_t local_servers_count_ = 0;

    // The virtual topic associated with virtual writers and readers
    const std::string virtual_topic_ = "eprosima_server_virtual_topic";

    // Whether the database is enabled
    std::atomic<bool> enabled_;

};


} /* namespace ddb */
} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_RTPS_DISCOVERY_DATABASE_H_ */
