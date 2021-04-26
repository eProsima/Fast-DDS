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
 * @file StatisticsWriterImpl.hpp
 */

#include <statistics/rtps/StatisticsBase.hpp>

#include <fastdds/rtps/writer/RTPSWriter.h>

using namespace eprosima::fastdds::statistics;

using eprosima::fastrtps::RecursiveTimedMutex;
using eprosima::fastrtps::rtps::RTPSWriter;
using eprosima::fastrtps::rtps::GUID_t;

StatisticsWriterImpl::StatisticsWriterImpl()
{
    init_statistics<StatisticsWriterAncillary>();
}

StatisticsWriterAncillary* StatisticsWriterImpl::get_members() const
{
    static_assert(
        std::is_base_of<StatisticsAncillary, StatisticsWriterAncillary>::value,
        "Auxiliary structure must derive from StatisticsAncillary");

    return static_cast<StatisticsWriterAncillary*>(get_aux_members());
}

RecursiveTimedMutex& StatisticsWriterImpl::get_statistics_mutex()
{
    static_assert(
        std::is_base_of<StatisticsWriterImpl, RTPSWriter>::value,
        "Must be call from a writer.");

    return static_cast<RTPSWriter*>(this)->getMutex();
}

const GUID_t& StatisticsWriterImpl::get_guid() const
{
    using eprosima::fastrtps::rtps::RTPSWriter;

    static_assert(
        std::is_base_of<StatisticsWriterImpl, RTPSWriter>::value,
        "This method should be called from an actual RTPSWriter");

    return static_cast<const RTPSWriter*>(this)->getGuid();
}

void StatisticsWriterImpl::on_data()
{
    EntityCount notification;
    notification.guid(to_statistics_type(get_guid()));

    {
        std::lock_guard<fastrtps::RecursiveTimedMutex> lock(get_statistics_mutex());
        notification.count(++get_members()->data_counter);
    }

    // Perform the callbacks
    Data data;
    // note that the setter sets RESENT_DATAS by default
    data.entity_count(notification);
    data._d(EventKind::DATA_COUNT);

    for_each_listener([&data](const std::shared_ptr<IListener>& listener)
            {
                listener->on_statistics_data(data);
            });
}

void StatisticsWriterImpl::on_data_frag()
{
    // there is no specific EventKind thus it will be redirected to DATA_COUNT
    on_data();
}

void StatisticsWriterImpl::on_heartbeat(
        uint32_t count)
{
    EntityCount notification;
    notification.guid(to_statistics_type(get_guid()));
    notification.count(count);

    // Perform the callbacks
    Data data;
    // note that the setter sets RESENT_DATAS by default
    data.entity_count(notification);
    data._d(EventKind::HEARTBEAT_COUNT);

    for_each_listener([&data](const std::shared_ptr<IListener>& listener)
            {
                listener->on_statistics_data(data);
            });
}

void StatisticsWriterImpl::on_gap()
{
    EntityCount notification;
    notification.guid(to_statistics_type(get_guid()));

    {
        std::lock_guard<fastrtps::RecursiveTimedMutex> lock(get_statistics_mutex());
        notification.count(++get_members()->gap_counter);
    }

    // Perform the callbacks
    Data data;
    // note that the setter sets RESENT_DATAS by default
    data.entity_count(notification);
    data._d(EventKind::GAP_COUNT);

    for_each_listener([&data](const std::shared_ptr<IListener>& listener)
            {
                listener->on_statistics_data(data);
            });
}
