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

#include <rtps/writer/BaseWriter.hpp>
#include <statistics/types/types.hpp>


namespace eprosima {
namespace fastdds {
namespace statistics {

using eprosima::fastdds::rtps::BaseWriter;
using eprosima::fastdds::rtps::GUID_t;

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
        std::is_base_of<StatisticsWriterImpl, BaseWriter>::value,
        "Must be call from a writer.");

    return static_cast<BaseWriter*>(this)->getMutex();
}

const GUID_t& StatisticsWriterImpl::get_guid() const
{
    using eprosima::fastdds::rtps::BaseWriter;

    static_assert(
        std::is_base_of<StatisticsWriterImpl, BaseWriter>::value,
        "This method should be called from an actual BaseWriter");

    return static_cast<const BaseWriter*>(this)->getGuid();
}

void StatisticsWriterImpl::on_sample_datas(
        const fastdds::rtps::SampleIdentity& sample_identity,
        size_t num_sent_submessages)
{
    if (!are_statistics_writers_enabled(EventKind::SAMPLE_DATAS))
    {
        return;
    }

    SampleIdentityCount notification;
    notification.sample_id(to_statistics_type(sample_identity));
    notification.count(static_cast<uint64_t>(num_sent_submessages));

    // Perform the callbacks
    Data data;
    // note that the setter sets SAMPLE_DATAS by default
    data.sample_identity_count(std::move(notification));

    for_each_listener([&data](const std::shared_ptr<IListener>& listener)
            {
                listener->on_statistics_data(data);
            });
}

void StatisticsWriterImpl::on_data_generated(
        size_t num_destinations)
{
    std::lock_guard<fastdds::RecursiveTimedMutex> lock(get_statistics_mutex());
    auto members = get_members();
    members->data_counter += static_cast<uint64_t>(num_destinations);
}

void StatisticsWriterImpl::on_data_sent()
{
    if (!are_statistics_writers_enabled(EventKind::DATA_COUNT))
    {
        return;
    }

    EntityCount notification;
    notification.guid(to_statistics_type(get_guid()));

    {
        std::lock_guard<fastdds::RecursiveTimedMutex> lock(get_statistics_mutex());
        auto members = get_members();
        notification.count(members->data_counter);
    }

    // Perform the callbacks
    Data data;
    // note that the setter sets RESENT_DATAS by default
    data.entity_count(std::move(notification));
    data._d(EventKind::DATA_COUNT);

    for_each_listener([&data](const std::shared_ptr<IListener>& listener)
            {
                listener->on_statistics_data(data);
            });
}

void StatisticsWriterImpl::on_heartbeat(
        uint32_t count)
{
    if (!are_statistics_writers_enabled(EventKind::HEARTBEAT_COUNT))
    {
        return;
    }

    EntityCount notification;
    notification.guid(to_statistics_type(get_guid()));
    notification.count(count);

    // Perform the callbacks
    Data data;
    // note that the setter sets RESENT_DATAS by default
    data.entity_count(std::move(notification));
    data._d(EventKind::HEARTBEAT_COUNT);

    for_each_listener([&data](const std::shared_ptr<IListener>& listener)
            {
                listener->on_statistics_data(data);
            });
}

void StatisticsWriterImpl::on_gap()
{
    if (!are_statistics_writers_enabled(EventKind::GAP_COUNT))
    {
        return;
    }

    EntityCount notification;
    notification.guid(to_statistics_type(get_guid()));

    {
        std::lock_guard<fastdds::RecursiveTimedMutex> lock(get_statistics_mutex());
        notification.count(++get_members()->gap_counter);
    }

    // Perform the callbacks
    Data data;
    // note that the setter sets RESENT_DATAS by default
    data.entity_count(std::move(notification));
    data._d(EventKind::GAP_COUNT);

    for_each_listener([&data](const std::shared_ptr<IListener>& listener)
            {
                listener->on_statistics_data(data);
            });
}

void StatisticsWriterImpl::on_resent_data(
        uint32_t to_send)
{
    if ( 0 == to_send )
    {
        return;
    }

    if (!are_statistics_writers_enabled(EventKind::RESENT_DATAS))
    {
        return;
    }

    EntityCount notification;
    notification.guid(to_statistics_type(get_guid()));

    {
        std::lock_guard<fastdds::RecursiveTimedMutex> lock(get_statistics_mutex());
        notification.count(get_members()->resent_counter += to_send);
    }

    // Perform the callbacks
    Data data;
    // note that the setter sets RESENT_DATAS by default
    data.entity_count(std::move(notification));

    for_each_listener([&data](const std::shared_ptr<IListener>& listener)
            {
                listener->on_statistics_data(data);
            });
}

void StatisticsWriterImpl::on_publish_throughput(
        uint32_t payload)
{
    using namespace std;
    using namespace chrono;

    if (payload > 0 )
    {
        if (!are_statistics_writers_enabled(EventKind::PUBLICATION_THROUGHPUT))
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

        for_each_listener([&data](const std::shared_ptr<IListener>& listener)
                {
                    listener->on_statistics_data(data);
                });
    }
}

}  // namespace statistics
}  // namespace fastdds
}  // namespace eprosima
