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
#include <shared_mutex>

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

using share_mutex_t = std::recursive_mutex;

// simplify namespace handling
using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

/**
 * Class to manage the discovery data base
 *@ingroup DISCOVERY_MODULE
 */
class DiscoveryDataBase
    : public PDPDataFilter<DiscoveryDataBase>
    , public EDPDataFilter<DiscoveryDataBase>
    , public EDPDataFilter<DiscoveryDataBase, false>
{
public:

    class AckedFunctor;

    ////////////
    // Functions to process_writers_acknowledgements()
    // Return the functor, class that works as a lambda
    AckedFunctor functor(
            CacheChange_t* );

    class AckedFunctor
    {
        using argument_type = ReaderProxy*;
        using result_type = void;

        // friend class DiscoveryDataBase;
        friend AckedFunctor DiscoveryDataBase::functor(
                CacheChange_t* );

        AckedFunctor() = delete;

        // Stateful constructor
        // This constructor generates the only object that keeps the state
        // all other constructors reference this object state
        AckedFunctor(
                DiscoveryDataBase* db,
                CacheChange_t* change);

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
                const ReaderProxy* reader_proxy);

        operator bool() const
        {
            return external_pending_;
        }

    private:

        DiscoveryDataBase* db_;
        CacheChange_t* change_;
        // stateful functor is the one contructed form the database
        bool pending_; // stateful functor state
        bool& external_pending_; // references stateful functor
    };

    friend class AckedFunctor;

    DiscoveryDataBase(
            GuidPrefix_t server_guid_prefix);

    ////////////
    // Functions to update queue from listener
    /* Add a new CacheChange_t to database queue
     *    1. Check whether the change is already in the database (queue lock)
     *    2. If the change is new, then add it to data_queue_ (queue lock)
     * @return: True if the change was added, false otherwise.
     */
    bool update(
            CacheChange_t* change,
            std::string topic_name = "");


    ////////////
    // Functions to is_relevant
    // Return whether a PDP change is relevant for a given reader
    bool pdp_is_relevant(
            const CacheChange_t& change,
            const GUID_t& reader_guid) const;

    // Return whether a EDP publications change is relevant for a given reader
    bool edp_publications_is_relevant(
            const CacheChange_t& change,
            const GUID_t& reader_guid) const;

    // Return whether a EDP subscription change is relevant for a given reader
    bool edp_subscriptions_is_relevant(
            const CacheChange_t& change,
            const GUID_t& reader_guid) const;

    /* Delete all information relative to the entity that produced a CacheChange
     * @change: That entity's CacheChange.
     * @return: True if the entity was deleted, false otherwise.
     */
    bool delete_entity_of_change(
            CacheChange_t* change);


    ////////////
    // Functions to process_data_queue()
    bool process_data_queue();

    void create_participant_from_change(
            CacheChange_t* ch);

    void create_writers_from_change(
            CacheChange_t* ch,
            const std::string& topic_name);

    void create_readers_from_change(
            CacheChange_t* ch,
            const std::string& topic_name);

    void process_dispose_participant(
            CacheChange_t* ch);

    void process_dispose_writer(
            CacheChange_t* ch,
            const std::string& topic_name);

    void process_dispose_reader(
            CacheChange_t* ch,
            const std::string& topic_name);

    ////////////
    // Functions to process_dirty_topics()
    bool process_dirty_topics();

    ////////////
    // Functions to process_disposals()
    const std::vector<CacheChange_t*> changes_to_dispose();

    void clear_changes_to_dispose();

    ////////////
    // Functions to process_to_send_lists()
    const std::vector<CacheChange_t*> pdp_to_send();

    void clear_pdp_to_send();

    const std::vector<CacheChange_t*> edp_publications_to_send();

    void clear_edp_publications_to_send();

    const std::vector<CacheChange_t*> edp_subscriptions_to_send();

    void clear_edp_subscriptions_to_send();

    const std::vector<CacheChange_t*> changes_to_release();

    void clear_changes_to_release();


    ////////////
    // Static Functions to work with GUIDs
    static bool is_participant(
            const CacheChange_t* ch);

    static bool is_writer(
            const CacheChange_t* ch);

    static bool is_reader(
            const CacheChange_t* ch);

    static GUID_t guid_from_change(
            const CacheChange_t* ch);

    CacheChange_t* cache_change_own_participant();

    const std::vector<GuidPrefix_t> remote_participants();

protected:

    // update the acks
    void add_ack_(
            const CacheChange_t* change,
            const GuidPrefix_t& acked_entity);


    ////////////
    // Mutex Functions
    void exclusive_lock_()
    {
        sh_mtx_.lock();
    }

    void shared_lock_()
    {
        sh_mtx_.lock();
        //sh_mtx_.lock();
    }

    void exclusive_unlock_()
    {
        sh_mtx_.unlock();
    }

    void shared_unlock_()
    {
        sh_mtx_.unlock();
        //sh_mtx_.unlock();
    }

    //! Incoming discovery traffic populated by the listeners, PDP database is already updated on notify
    DBQueue<DiscoveryDataQueueInfo> data_queue_;

    //! Covenient per-topic mapping of readers and writers to speed-up queries
    std::map<std::string, std::vector<GUID_t>> readers_by_topic_;
    std::map<std::string, std::vector<GUID_t>> writers_by_topic_;

    //! Collection of participant proxies that:
    //  - stores the CacheChange_t
    //  - keeps track of its acknowledgement status
    //  - keeps an account of participant's readers and writers
    std::map<GuidPrefix_t, DiscoveryParticipantInfo> participants_;

    //! Collection of reader and writer proxies that:
    //  - stores the CacheChange_t
    //  - keeps track of its acknowledgement status
    //  - stores the topic name (only matching criteria available)
    std::map<GUID_t, DiscoveryEndpointInfo> readers_;
    std::map<GUID_t, DiscoveryEndpointInfo> writers_;

    //! Collection of topics whose related endpoints have changed and require a match recalculation
    std::vector<std::string> dirty_topics_;

    //! Collection of changes to take out of the server builtin writers
    std::vector<CacheChange_t*> disposals_;

    //! Collection of changes to put into the server builtin writers
    std::vector<CacheChange_t*> pdp_to_send_;
    std::vector<CacheChange_t*> edp_publications_to_send_;
    std::vector<CacheChange_t*> edp_subscriptions_to_send_;

    //! changes that are no longer associated to living endpoints and should be returned to it's pool
    std::vector<CacheChange_t*> changes_to_release_;

    // mutexes
    mutable share_mutex_t sh_mtx_;

    GuidPrefix_t server_guid_prefix_;

};

} /* namespace ddb */
} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_RTPS_DISCOVERY_DATABASE_H_ */
