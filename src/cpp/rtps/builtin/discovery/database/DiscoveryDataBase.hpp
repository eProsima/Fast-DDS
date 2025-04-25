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

#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <set>
#include <vector>

#include <nlohmann/json.hpp>

#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>

#include <rtps/builtin/discovery/database/DiscoveryDataFilter.hpp>
#include <rtps/builtin/discovery/database/DiscoveryDataQueueInfo.hpp>
#include <rtps/builtin/discovery/database/DiscoveryEndpointInfo.hpp>
#include <rtps/builtin/discovery/database/DiscoveryParticipantInfo.hpp>
#include <rtps/writer/ReaderProxy.hpp>
#include <utils/DBQueue.hpp>

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
            eprosima::fastdds::rtps::CacheChange_t* );

    class AckedFunctor
    {
        using argument_type = eprosima::fastdds::rtps::ReaderProxy*;
        using result_type = void;

        // friend class DiscoveryDataBase;
        friend AckedFunctor DiscoveryDataBase::functor(
                eprosima::fastdds::rtps::CacheChange_t* );

        // Stateful constructor
        // This constructor generates the only object that keeps the state
        // all other constructors reference this object state
        AckedFunctor(
                DiscoveryDataBase* db,
                eprosima::fastdds::rtps::CacheChange_t* change);

        AckedFunctor() = delete;

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

        ~AckedFunctor();

        void operator () (
                const eprosima::fastdds::rtps::ReaderProxy* reader_proxy);

        operator bool() const
        {
            return external_pending_;
        }

    private:

        DiscoveryDataBase* db_;
        eprosima::fastdds::rtps::CacheChange_t* change_;
        // stateful functor is the one contructed form the database
        bool pending_; // stateful functor state
        bool& external_pending_; // references stateful functor
    };

    friend class AckedFunctor;

    DiscoveryDataBase(
            const fastdds::rtps::GuidPrefix_t& server_guid_prefix);

    ~DiscoveryDataBase();

    ////////////
    // Functions to update queue from listener
    /* Add a new CacheChange_t to database queue
     *    1. Check whether the change is already in the database (queue lock)
     *    2. If the change is new, then add it to data_queue_ (queue lock)
     * @return: True if the change was added, false otherwise.
     */
    bool update(
            eprosima::fastdds::rtps::CacheChange_t* change,
            const std::string& topic_name);

    bool update(
            eprosima::fastdds::rtps::CacheChange_t* change,
            DiscoveryParticipantChangeData participant_change_data);

    //! Enables the possibility to add new entries to the database. Enable by default.
    void enable()
    {
        enabled_ = true;
    }

    // enable ddb in persistence mode and open the file to backup up in append mode
    void persistence_enable(
            const std::string& backup_file_name);

    //! Disable the possibility to add new entries to the database
    void disable()
    {
        EPROSIMA_LOG_INFO(DISCOVERY_DATABASE, "DISCOVERY DATA BASE DISABLED");
        enabled_ = false;
    }

    //! Check whether the database is enabled
    bool is_enabled()
    {
        return enabled_;
    }

    //! Check if the DDB is restoring a backup at the moment
    bool backup_in_progress()
    {
        return processing_backup_;
    }

    //! Disable the possibility to add new entries to the database
    // This is different than lock_incoming_data() because it does not store the changes or lock the listener
    // This method is only used when the Server is restoring the backup file, and so sending messages to the
    // listener that should not be process
    void backup_in_progress(
            bool v)
    {
        processing_backup_ = v;
    }

    /* Clear all the collections in the database
     * @return: The changes that can be released
     */
    std::vector<fastdds::rtps::CacheChange_t*> clear();

    ////////////
    // Functions to is_relevant
    // Return whether a PDP change is relevant for a given reader
    bool pdp_is_relevant(
            const eprosima::fastdds::rtps::CacheChange_t& change,
            const eprosima::fastdds::rtps::GUID_t& reader_guid) const;

    // Return whether a EDP publications change is relevant for a given reader
    bool edp_publications_is_relevant(
            const eprosima::fastdds::rtps::CacheChange_t& change,
            const eprosima::fastdds::rtps::GUID_t& reader_guid) const;

    // Return whether a EDP subscription change is relevant for a given reader
    bool edp_subscriptions_is_relevant(
            const eprosima::fastdds::rtps::CacheChange_t& change,
            const eprosima::fastdds::rtps::GUID_t& reader_guid) const;

    ////////////
    // Functions to process PDP and EDP data queues
    void process_pdp_data_queue();

    bool process_edp_data_queue();

    ////////////
    // Functions to process_dirty_topics()
    bool process_dirty_topics();

    ////////////
    // Functions to process_disposals()
    const std::vector<eprosima::fastdds::rtps::CacheChange_t*> changes_to_dispose();

    void clear_changes_to_dispose();

    /* Delete all information relative to the entity that produced a CacheChange
     * @change: That entity's CacheChange.
     * @return: True if the entity was deleted, false otherwise.
     */
    bool delete_entity_of_change(
            fastdds::rtps::CacheChange_t* change);

    ////////////
    // Functions to process_to_send_lists()
    const std::vector<eprosima::fastdds::rtps::CacheChange_t*> pdp_to_send();

    void clear_pdp_to_send();

    const std::vector<eprosima::fastdds::rtps::CacheChange_t*> edp_publications_to_send();

    void clear_edp_publications_to_send();

    const std::vector<eprosima::fastdds::rtps::CacheChange_t*> edp_subscriptions_to_send();

    void clear_edp_subscriptions_to_send();

    const std::vector<eprosima::fastdds::rtps::CacheChange_t*> changes_to_release();

    void clear_changes_to_release();


    ////////////
    // Static Functions to work with GUIDs
    static bool is_participant(
            const eprosima::fastdds::rtps::GUID_t& guid);

    static bool is_writer(
            const eprosima::fastdds::rtps::GUID_t& guid);

    static bool is_reader(
            const eprosima::fastdds::rtps::GUID_t& guid);

    static bool is_participant(
            const eprosima::fastdds::rtps::CacheChange_t* ch);

    static bool is_writer(
            const eprosima::fastdds::rtps::CacheChange_t* ch);

    static bool is_reader(
            const eprosima::fastdds::rtps::CacheChange_t* ch);

    static eprosima::fastdds::rtps::GUID_t guid_from_change(
            const eprosima::fastdds::rtps::CacheChange_t* ch);

    ////////////
    // Functions to work with own server

    // return the cache change of the server
    fastdds::rtps::CacheChange_t* cache_change_own_participant();

    const std::vector<fastdds::rtps::GuidPrefix_t> direct_clients_and_servers();

    LocatorList participant_metatraffic_locators(
            fastdds::rtps::GuidPrefix_t participant_guid_prefix);

    // return a list of participants that are not the server one
    const std::vector<fastdds::rtps::GuidPrefix_t> remote_participants();

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

    ////////////
    // Data structures utils

    // Check if the data queue is empty
    bool data_queue_empty();

    void to_json(
            nlohmann::json& j) const;

    bool from_json(
            nlohmann::json& j,
            std::map<eprosima::fastdds::rtps::InstanceHandle_t, fastdds::rtps::CacheChange_t*>& changes_map);

    // This function erase the last backup and all the changes that has arrived since then and create
    // a new backup that shows the actual state of the database.
    // This way we can simulate the state of the database from a clean state of json backup, or from
    // an state in the middle of an routine execution, and every message that has arrived and has not
    // been process.
    // By this, we do not lose any change or information in any case.
    // This function must be called with the incoming datas blocked
    void clean_backup();

    // Lock the incoming of new data to the DDB queue. This locks the Listener as well
    void lock_incoming_data()
    {
        data_queues_mutex_.lock();
    }

    // Unlock the incoming of new data to the DDB queue
    void unlock_incoming_data()
    {
        data_queues_mutex_.unlock();
    }

    // Return string with virtual topic default name
    std::string virtual_topic() const
    {
        return virtual_topic_;
    }

    // Return number of updated entities since last call to this same function
    int updates_since_last_checked()
    {
        return new_updates_.exchange(0);
    }

    // Check if a participant is stored as local. If the participant does not exist, it returns false
    bool is_participant_local(
            const eprosima::fastdds::rtps::GuidPrefix_t& participant_prefix);

    //! Add a server to the list of remote servers
    void add_server(
            const fastdds::rtps::GuidPrefix_t server);

    //! Remove a server from the list of remote servers
    void remove_server(
            const fastdds::rtps::GuidPrefix_t server);

    // Removes all the changes whose original sender was entity_guid_prefix from writer_history
    void remove_related_alive_from_history_nts(
            fastdds::rtps::WriterHistory* writer_history,
            const fastdds::rtps::GuidPrefix_t& entity_guid_prefix);

protected:

    // Change a cacheChange by update or new disposal
    void update_change_and_unmatch_(
            fastdds::rtps::CacheChange_t* new_change,
            ddb::DiscoverySharedInfo& entity);

    // Update the acks
    void add_ack_(
            const eprosima::fastdds::rtps::CacheChange_t* change,
            const eprosima::fastdds::rtps::GuidPrefix_t& acked_entity);

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
            eprosima::fastdds::rtps::CacheChange_t* ch,
            const DiscoveryParticipantChangeData& change_data);

    void create_writers_from_change_(
            eprosima::fastdds::rtps::CacheChange_t* ch,
            const std::string& topic_name);

    void create_readers_from_change_(
            eprosima::fastdds::rtps::CacheChange_t* ch,
            const std::string& topic_name);

    // Functions related with create_participant_from_change_

    void match_new_server_(
            eprosima::fastdds::rtps::GuidPrefix_t& participant_prefix,
            bool is_superclient);

    void create_virtual_endpoints_(
            eprosima::fastdds::rtps::GuidPrefix_t& participant_prefix);

    static bool participant_data_has_changed_(
            const DiscoveryParticipantInfo& participant_info,
            const DiscoveryParticipantChangeData& new_change_data);

    void create_new_participant_from_change_(
            eprosima::fastdds::rtps::CacheChange_t* ch,
            const DiscoveryParticipantChangeData& change_data);

    void update_participant_from_change_(
            DiscoveryParticipantInfo& participant_info,
            eprosima::fastdds::rtps::CacheChange_t* ch,
            const DiscoveryParticipantChangeData& change_data);

    // change ack relevants and matched between entities participants and endpoints
    void match_writer_reader_(
            const eprosima::fastdds::rtps::GUID_t& writer_guid,
            const eprosima::fastdds::rtps::GUID_t& reader_guid);

    void process_dispose_participant_(
            eprosima::fastdds::rtps::CacheChange_t* ch);

    void process_dispose_writer_(
            eprosima::fastdds::rtps::CacheChange_t* ch);

    void process_dispose_reader_(
            eprosima::fastdds::rtps::CacheChange_t* ch);

    ////////////
    // functions to manage disposals and clean entities

    // unmatch in every other entity including its readers and writers
    void unmatch_participant_(
            const eprosima::fastdds::rtps::GuidPrefix_t& guid_prefix);

    // unmatch all the readers and erase it from writers_by_topic
    void unmatch_writer_(
            const eprosima::fastdds::rtps::GUID_t& guid);

    // unmatch all the writers and erase it from readers_by_topic
    void unmatch_reader_(
            const eprosima::fastdds::rtps::GUID_t& guid);

    // delete an entity and set its change to release. Assumes the entity has been unmatched before
    bool delete_participant_entity_(
            const fastdds::rtps::GuidPrefix_t& guid_prefix);

    std::map<eprosima::fastdds::rtps::GuidPrefix_t, DiscoveryParticipantInfo>::iterator delete_participant_entity_(
            std::map<eprosima::fastdds::rtps::GuidPrefix_t, DiscoveryParticipantInfo>::iterator it);

    // delete an entity and set its change to release. Assumes the entity has been unmatched before
    bool delete_writer_entity_(
            const fastdds::rtps::GUID_t& guid);

    std::map<eprosima::fastdds::rtps::GUID_t, DiscoveryEndpointInfo>::iterator delete_writer_entity_(
            std::map<eprosima::fastdds::rtps::GUID_t, DiscoveryEndpointInfo>::iterator it);

    // delete an entity and set its change to release. Assumes the entity has been unmatched before
    bool delete_reader_entity_(
            const fastdds::rtps::GUID_t& guid);

    std::map<eprosima::fastdds::rtps::GUID_t, DiscoveryEndpointInfo>::iterator delete_reader_entity_(
            std::map<eprosima::fastdds::rtps::GUID_t, DiscoveryEndpointInfo>::iterator it);

    // return if there are more than one writer in the participant in the same topic
    bool repeated_writer_topic_(
            const eprosima::fastdds::rtps::GuidPrefix_t& participant,
            const std::string& topic_name);

    // return if there are more than one reader in the participant in the same topic
    bool repeated_reader_topic_(
            const eprosima::fastdds::rtps::GuidPrefix_t& participant,
            const std::string& topic_name);

    void remove_writer_from_topic_(
            const eprosima::fastdds::rtps::GUID_t& writer_guid,
            const std::string& topic_name);

    void remove_reader_from_topic_(
            const eprosima::fastdds::rtps::GUID_t& reader_guid,
            const std::string& topic_name);

    // create a new topic in writers and readers and fill it with virtuals
    void create_topic_(
            const std::string& topic_name);

    void add_writer_to_topic_(
            const eprosima::fastdds::rtps::GUID_t& writer_guid,
            const std::string& topic_name);

    void add_reader_to_topic_(
            const eprosima::fastdds::rtps::GUID_t& reader_guid,
            const std::string& topic_name);

    //! Add a topic to the list of dirty topics, unless it's already present
    // Return true if added, false if already there
    bool set_dirty_topic_(
            const std::string& topic);

    // Add data in pdp_to_send if not already in it
    bool add_pdp_to_send_(
            eprosima::fastdds::rtps::CacheChange_t* change);

    // Add data in edp_to_send if not already in it
    bool add_edp_publications_to_send_(
            eprosima::fastdds::rtps::CacheChange_t* change);

    // Add data in edp_to_send if not already in it
    bool add_edp_subscriptions_to_send_(
            eprosima::fastdds::rtps::CacheChange_t* change);

    // Get all the writers in given topic and in virtual topic
    std::vector<eprosima::fastdds::rtps::GUID_t> get_writers_in_topic(
            const std::string& topic_name);

    // Get all the readers in given topic and in virtual topic
    std::vector<eprosima::fastdds::rtps::GUID_t> get_readers_in_topic(
            const std::string& topic_name);

    ////////////////
    // Variables

    DBQueue<eprosima::fastdds::rtps::ddb::DiscoveryPDPDataQueueInfo> pdp_data_queue_;

    DBQueue<eprosima::fastdds::rtps::ddb::DiscoveryEDPDataQueueInfo> edp_data_queue_;

    //! Convenient per-topic mapping of readers and writers to speed-up queries
    std::map<std::string, std::vector<eprosima::fastdds::rtps::GUID_t>> readers_by_topic_;
    std::map<std::string, std::vector<eprosima::fastdds::rtps::GUID_t>> writers_by_topic_;

    //! Collection of participant proxies that:
    //  - stores the CacheChange_t
    //  - keeps track of its acknowledgement status
    //  - keeps an account of participant's readers and writers
    std::map<eprosima::fastdds::rtps::GuidPrefix_t, DiscoveryParticipantInfo> participants_;

    //! Collection of reader and writer proxies that:
    //  - stores the CacheChange_t
    //  - keeps track of its acknowledgement status
    //  - stores the topic name (only matching criteria available)
    std::map<eprosima::fastdds::rtps::GUID_t, DiscoveryEndpointInfo> readers_;
    std::map<eprosima::fastdds::rtps::GUID_t, DiscoveryEndpointInfo> writers_;

    //! Collection of topics whose related endpoints have changed and require a match recalculation
    std::vector<std::string> dirty_topics_;

    //! Collection of changes to take out of the server builtin writers
    std::vector<eprosima::fastdds::rtps::CacheChange_t*> disposals_;

    //! Collection of changes to put into the server builtin writers
    std::vector<eprosima::fastdds::rtps::CacheChange_t*> pdp_to_send_;
    std::vector<eprosima::fastdds::rtps::CacheChange_t*> edp_publications_to_send_;
    std::vector<eprosima::fastdds::rtps::CacheChange_t*> edp_subscriptions_to_send_;

    //! changes that are no longer associated to living endpoints and should be returned to it's pool
    std::vector<eprosima::fastdds::rtps::CacheChange_t*> changes_to_release_;

    //! General mutex
    mutable std::recursive_mutex mutex_;

    //! Mutex to lock updating to queues
    mutable std::recursive_mutex data_queues_mutex_;

    //! GUID prefix from own server
    const fastdds::rtps::GuidPrefix_t server_guid_prefix_;

    //! Is own server DATA(p) acked by all other clients
    std::atomic<bool> server_acked_by_all_;

    //! List of GUID prefixes of the connected remote servers
    std::set<fastdds::rtps::GuidPrefix_t> servers_;

    // The virtual topic associated with virtual writers and readers
    const std::string virtual_topic_ = "eprosima_server_virtual_topic";

    // Whether the database is enabled
    std::atomic<bool> enabled_;

    // Whether it has been a new entity discovered or updated in this subroutine loop
    std::atomic<int> new_updates_;

    // Whether the database is restoring a backup
    std::atomic<bool> processing_backup_;

    // Whether the database is persistent, so it must store every cache it arrives
    bool is_persistent_;

    // File to save every cacheChange that is updated to the ddb queues
    std::string backup_file_name_;
    // This file will keep open to write it fast every time a new cache arrives
    // It needs a flush every time a new change is added
    std::ofstream backup_file_;
};


} /* namespace ddb */
} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_RTPS_DISCOVERY_DATABASE_H_ */
