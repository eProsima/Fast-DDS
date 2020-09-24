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

#include <fastdds/rtps/builtin/discovery/DiscoveryDataFilter.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

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

    bool pdp_is_relevant(
            const fastrtps::rtps::CacheChange_t& change,
            const fastrtps::rtps::GUID_t& reader_guid) const
    {
        (void)change;
        (void)reader_guid;
        return true;
    }

    bool edp_publications_is_relevant(
            const fastrtps::rtps::CacheChange_t& change,
            const fastrtps::rtps::GUID_t& reader_guid) const
    {
        (void)change;
        (void)reader_guid;
        return true;
    }

    bool edp_subscriptions_is_relevant(
            const fastrtps::rtps::CacheChange_t& change,
            const fastrtps::rtps::GUID_t& reader_guid) const
    {
        (void)change;
        (void)reader_guid;
        return true;
    }

    ////////////
    // Functions to update queue from listener
    /* Add a new CacheChange_t to database queue
     *    1. Check whether the change is already in the database (queue lock)
     *    2. If the change is new, then add it to data_queue_ (queue lock)
     * @return: True if the change was added, false otherwise.
     */
    bool update(
            eprosima::fastrtps::rtps::CacheChange_t* change,
            std::string topic_name = {})
    {

        (void)change;
        (void)topic_name;
        return true; // dummy
    }

};

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_RTPS_DISCOVERY_DATABASE_H_ */
