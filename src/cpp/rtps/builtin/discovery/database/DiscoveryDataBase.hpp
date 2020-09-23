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
#include <mutex>  // For std::unique_lock
#include <shared_mutex>

#include <fastrtps/utils/fixed_size_string.hpp>
#include <fastdds/rtps/writer/ReaderProxy.h>
#include <fastdds/rtps/common/CacheChange.h>

#include "./DiscoveryDataFilter.hpp"
#include "./DiscoveryParticipantInfo.hpp"
#include "./DiscoveryEndpointInfo.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace ddb {


struct CacheChangeCmp
{
    bool operator ()(
            const eprosima::fastrtps::rtps::CacheChange_t& a,
            const eprosima::fastrtps::rtps::CacheChange_t& b) const
    {
        (void) a;
        (void) b;
        return true;
    }

};

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


    class ParticipantAckedFunctor {
        using argument_type = eprosima::fastrtps::rtps::ReaderProxy*;
        using result_type = void;
    public:

        ParticipantAckedFunctor(DiscoveryDataBase* db, eprosima::fastrtps::rtps::CacheChange_t* change)
            : db_(db)
            , cache_(change)
        {
            //db_.lock();
        }

        ~ParticipantAckedFunctor()
        {
            //db_.unlock();
        }

        void operator() (eprosima::fastrtps::rtps::ReaderProxy* reader_proxy)
        {
            (void) reader_proxy;
            /*
            bool status = reader_proxy->change_is_acked(cache_->sequenceNumber);
            // relevant_participants_builtin_ack_status has an update method that only sets the status if the proxy
            // is already there.
            participant_->relevant_participants_builtin_ack_status.update(reader_proxy->guid_prefix, status);
            pending_ |= !status;
            */
        }

        bool pending()
        {
            return pending_;
        }

        const std::vector<eprosima::fastrtps::rtps::CacheChange_t*> changes_to_dispose()
        {
            return db_->disposals_;
        }

        void clear_changes_to_dispose()
        {
        }

        const std::vector<eprosima::fastrtps::rtps::CacheChange_t*> pdp_to_send()
        {
            return db_->pdp_to_send_;
        }

        void clear_pdp_to_send()
        {
        }

        const std::vector<eprosima::fastrtps::rtps::CacheChange_t*> edp_publications_to_send()
        {
            return db_->edp_publications_to_send_;
        }

        void clear_edp_publications_to_send()
        {
        }

        const std::vector<eprosima::fastrtps::rtps::CacheChange_t*> edp_subscriptions_to_send()
        {
            return db_->edp_subscriptions_to_send_;
        }

        void clear_edp_subscriptions_to_send()
        {
        }

    private:

        eprosima::fastrtps::rtps::ParticipantProxyData* participant_ = nullptr;
        DiscoveryDataBase* db_;
        eprosima::fastrtps::rtps::CacheChange_t* cache_;
        bool pending_ = false;

    };
    friend class ParticipantAckedFunctor;

    /* Add a new CacheChange_t to database queue
     *    1. Check whether the change is already in the database (shared lock)
     *    2. If the change is new, then add it to data_queue_ (exclusive lock)
     * @return: True if the change was added, false otherwise.
     */
    bool update(
            eprosima::fastrtps::rtps::CacheChange_t* cache,
            std::string topic_name,
            eprosima::fastrtps::rtps::GUID_t* entity);

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

    ParticipantAckedFunctor functor(eprosima::fastrtps::rtps::CacheChange_t* change)
    {
        return DiscoveryDataBase::ParticipantAckedFunctor(this, change);
    }

private:

    std::map<eprosima::fastrtps::rtps::CacheChange_t*, eprosima::fastrtps::rtps::GUID_t, CacheChangeCmp> data_map_;

    std::map<eprosima::fastrtps::string_255, eprosima::fastrtps::rtps::GUID_t> readers_by_topic_;

    std::map<eprosima::fastrtps::string_255, eprosima::fastrtps::rtps::GUID_t> writers_by_topic_;

    std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryParticipantInfo> participants_;

    std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo> readers_;

    std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo> writers_;

    std::vector<eprosima::fastrtps::rtps::CacheChange_t*> disposals_;

    std::vector<eprosima::fastrtps::string_255> dirty_topics_;

    std::vector<eprosima::fastrtps::rtps::CacheChange_t*> pdp_to_send_;

    std::vector<eprosima::fastrtps::rtps::CacheChange_t*> edp_publications_to_send_;

    std::vector<eprosima::fastrtps::rtps::CacheChange_t*> edp_subscriptions_to_send_;


    // mutexes

    //std::shared_mutex sh_mtx_;

};


} /* namespace ddb */
} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_RTPS_DISCOVERY_DATABASE_H_ */