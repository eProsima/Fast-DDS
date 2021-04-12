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

#include <fastdds/rtps/reader/RTPSReader.h>
#include <statistics/rtps/StatisticsBase.hpp>

using namespace eprosima::fastdds::statistics;

using eprosima::fastrtps::RecursiveTimedMutex;
using eprosima::fastrtps::rtps::RTPSReader;

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

void StatisticsReaderImpl::on_acknack(
        int32_t count)
{
    using eprosima::fastrtps::rtps::RTPSReader;

    static_assert(
        std::is_base_of<StatisticsReaderImpl, RTPSReader>::value,
        "This method should be called from an actual RTPSReader");

    EntityCount notification;
    notification.guid(to_statistics_type(static_cast<RTPSReader*>(this)->getGuid()));
    notification.count(count);

    // Callback
    Data d;
    // note that the setter sets RESENT_DATAS by default
    d.entity_count(notification);
    d._d(EventKind::ACKNACK_COUNT);

    for_each_listener([&d](const std::shared_ptr<IListener>& l)
            {
                l->on_statistics_data(d);
            });
}
