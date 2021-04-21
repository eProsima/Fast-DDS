// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file StatisticsReaderImpl.hpp
 */

#include <statistics/rtps/StatisticsBase.hpp>

#include <fastdds/rtps/reader/RTPSReader.h>

using namespace eprosima::fastdds::statistics;

using eprosima::fastrtps::RecursiveTimedMutex;
using eprosima::fastrtps::rtps::RTPSReader;
using eprosima::fastrtps::rtps::GUID_t;

StatisticsReaderImpl::StatisticsReaderImpl()
{
    init_statistics<StatisticsReaderAncillary>();
}

/* Uncomment when a member is added to the StatisticsReaderAncillary
   StatisticsReaderAncillary* StatisticsReaderImpl::get_members() const
   {
    static_assert(
            std::is_base_of<StatisticsAncillary,StatisticsReaderAncillary>::value,
            "Auxiliary structure must derive from StatisticsAncillary");

    return static_cast<StatisticsReaderAncillary*>(get_aux_members());
   }
 */

RecursiveTimedMutex& StatisticsReaderImpl::get_statistics_mutex()
{
    static_assert(
        std::is_base_of<StatisticsReaderImpl, RTPSReader>::value,
        "Must be call from a writer.");

    return static_cast<RTPSReader*>(this)->getMutex();
}

const GUID_t& StatisticsReaderImpl::get_guid()
{
    static_assert(
        std::is_base_of<StatisticsReaderImpl, RTPSReader>::value,
        "This method should be called from an actual RTPSReader");

    return static_cast<RTPSReader*>(this)->getGuid();
}

void StatisticsReaderImpl::on_acknack(
        int32_t count)
{
    EntityCount notification;
    notification.guid(to_statistics_type(get_guid()));
    notification.count(count);

    // Perform the callback
    Data data;
    // note that the setter sets RESENT_DATAS by default
    data.entity_count(notification);
    data._d(EventKind::ACKNACK_COUNT);

    for_each_listener([&data](const std::shared_ptr<IListener>& listener)
            {
                listener->on_statistics_data(data);
            });
}

void StatisticsReaderImpl::on_nackfrag(
        int32_t count)
{
    EntityCount notification;
    notification.guid(to_statistics_type(get_guid()));
    notification.count(count);

    // Perform the callback
    Data data;
    // note that the setter sets RESENT_DATAS by default
    data.entity_count(notification);
    data._d(EventKind::NACKFRAG_COUNT);

    for_each_listener([&data](const std::shared_ptr<IListener>& listener)
            {
                listener->on_statistics_data(data);
            });
}
