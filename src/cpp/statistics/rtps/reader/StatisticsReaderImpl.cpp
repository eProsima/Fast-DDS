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

#include <fastdds/rtps/reader/RTPSReader.hpp>

#include <rtps/reader/BaseReader.hpp>
#include <statistics/types/types.hpp>

using eprosima::fastdds::RecursiveTimedMutex;
using eprosima::fastdds::rtps::RTPSReader;
using eprosima::fastdds::rtps::GUID_t;

namespace eprosima {
namespace fastdds {
namespace statistics {

using BaseReader = fastdds::rtps::BaseReader;

StatisticsReaderImpl::StatisticsReaderImpl()
{
    init_statistics<StatisticsReaderAncillary>();
}

StatisticsReaderAncillary* StatisticsReaderImpl::get_members() const
{
    static_assert(
        std::is_base_of<StatisticsAncillary, StatisticsReaderAncillary>::value,
        "Auxiliary structure must derive from StatisticsAncillary");

    return static_cast<StatisticsReaderAncillary*>(get_aux_members());
}

RecursiveTimedMutex& StatisticsReaderImpl::get_statistics_mutex()
{
    static_assert(
        std::is_base_of<StatisticsReaderImpl, BaseReader>::value,
        "Must be call from a writer.");

    return static_cast<BaseReader*>(this)->getMutex();
}

const GUID_t& StatisticsReaderImpl::get_guid() const
{
    static_assert(
        std::is_base_of<StatisticsReaderImpl, BaseReader>::value,
        "This method should be called from an actual RTPSReader");

    return static_cast<const BaseReader*>(this)->getGuid();
}

void StatisticsReaderImpl::on_data_notify(
        const fastdds::rtps::GUID_t& writer_guid,
        const fastdds::rtps::Time_t& source_timestamp)
{
    if (!are_statistics_writers_enabled(EventKind::HISTORY2HISTORY_LATENCY))
    {
        return;
    }

    // Get current timestamp
    fastdds::rtps::Time_t current_time;
    fastdds::rtps::Time_t::now(current_time);

    // Calc latency
    auto ns = (current_time - source_timestamp).to_ns();

    WriterReaderData notification;
    notification.reader_guid(to_statistics_type(get_guid()));
    notification.writer_guid(to_statistics_type(writer_guid));
    notification.data(static_cast<float>(ns));

    // Perform the callback
    Data data;
    // note that the setter sets HISTORY2HISTORY_LATENCY by default
    data.writer_reader_data(notification);

    for_each_listener([&data](const std::shared_ptr<IListener>& listener)
            {
                listener->on_statistics_data(data);
            });
}

void StatisticsReaderImpl::on_acknack(
        int32_t count)
{
    if (!are_statistics_writers_enabled(EventKind::ACKNACK_COUNT))
    {
        return;
    }

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
    if (!are_statistics_writers_enabled(EventKind::NACKFRAG_COUNT))
    {
        return;
    }

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

void StatisticsReaderImpl::on_subscribe_throughput(
        uint32_t payload)
{
    using namespace std;
    using namespace chrono;

    if (payload > 0 )
    {
        if (!are_statistics_writers_enabled(EventKind::SUBSCRIPTION_THROUGHPUT))
        {
            return;
        }
        // update state
        time_point<steady_clock> former_timepoint;
        auto& current_timepoint = get_members()->last_history_change_;
        {
            lock_guard<fastdds::RecursiveTimedMutex> lock(get_statistics_mutex());
            former_timepoint = current_timepoint;
            current_timepoint = steady_clock::now();
        }

        EntityData notification;
        notification.guid(to_statistics_type(get_guid()));
        notification.data(payload / duration_cast<duration<float>>(current_timepoint - former_timepoint).count());

        // Perform the callbacks
        Data data;
        // note that the setter sets PUBLICATION_THROUGHPUT by default
        data.entity_data(std::move(notification));
        data._d(EventKind::SUBSCRIPTION_THROUGHPUT);

        for_each_listener([&data](const std::shared_ptr<IListener>& listener)
                {
                    listener->on_statistics_data(data);
                });
    }
}

}  // namespace statistics
}  // namespace fastdds
}  // namespace eprosima
