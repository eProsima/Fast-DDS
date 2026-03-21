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
 * @file StatisticsBase.cpp
 */

#include <statistics/rtps/StatisticsBase.hpp>

#include <algorithm>
#include <cmath>
#include <string>

#include <fastdds/dds/core/policy/ParameterTypes.hpp>
#include <fastdds/dds/log/Log.hpp>

#include <rtps/participant/RTPSParticipantImpl.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {

static void add_bytes(
        Entity2LocatorTraffic& traffic,
        const rtps::StatisticsSubmessageData::Sequence& distance)
{
    uint64_t count = traffic.packet_count();
    int16_t high = traffic.byte_magnitude_order();
    uint64_t low = traffic.byte_count();

    count += distance.sequence;
    high += distance.bytes_high;
    low += distance.bytes;
    high += (low < traffic.byte_count());

    traffic.packet_count(count);
    traffic.byte_magnitude_order(high);
    traffic.byte_count(low);
}

static void sub_bytes(
        Entity2LocatorTraffic& traffic,
        uint64_t bytes)
{
    uint64_t count = traffic.packet_count();
    int16_t high = traffic.byte_magnitude_order();
    uint64_t low = traffic.byte_count();

    if (count > 0)
    {
        count--;
        low -= bytes;
        high -= (low > traffic.byte_count());

        traffic.packet_count(count);
        traffic.byte_magnitude_order(high);
        traffic.byte_count(low);
    }
}

detail::Locator_s to_statistics_type(
        fastdds::rtps::Locator_t locator)
{
    return *reinterpret_cast<detail::Locator_s*>(&locator);
}

fastdds::rtps::Locator_t to_fastdds_type(
        detail::Locator_s locator)
{
    return *reinterpret_cast<fastdds::rtps::Locator_t*>(&locator);
}

detail::GUID_s to_statistics_type(
        fastdds::rtps::GUID_t guid)
{
    return *reinterpret_cast<detail::GUID_s*>(&guid);
}

fastdds::rtps::GUID_t to_fastdds_type(
        detail::GUID_s guid)
{
    return *reinterpret_cast<fastdds::rtps::GUID_t*>(&guid);
}

detail::SampleIdentity_s to_statistics_type(
        fastdds::rtps::SampleIdentity sample_id)
{
    return *reinterpret_cast<detail::SampleIdentity_s*>(&sample_id);
}

StatisticsAncillary* StatisticsListenersImpl::get_aux_members() const
{
    return members_.get();
}

bool StatisticsListenersImpl::add_statistics_listener_impl(
        std::shared_ptr<fastdds::statistics::IListener> listener)
{
    if (!listener)
    {
        // avoid nullptr
        return false;
    }

    std::lock_guard<fastdds::RecursiveTimedMutex> lock(get_statistics_mutex());

    // add the new listener
    return members_->listeners.insert(listener).second;
}

bool StatisticsListenersImpl::remove_statistics_listener_impl(
        std::shared_ptr<fastdds::statistics::IListener> listener)
{
    std::lock_guard<fastdds::RecursiveTimedMutex> lock(get_statistics_mutex());

    if (!listener)
    {
        // avoid nullptr
        return false;
    }

    return 1 == members_->listeners.erase(listener);
}

void StatisticsListenersImpl::set_enabled_statistics_writers_mask_impl(
        uint32_t enabled_writers)
{
    std::unique_lock<fastdds::RecursiveTimedMutex> lock(get_statistics_mutex());
    if (members_)
    {
        members_->enabled_writers_mask.store(enabled_writers);
    }
}

bool StatisticsListenersImpl::are_statistics_writers_enabled(
        uint32_t checked_enabled_writers)
{
    // Check if the corresponding writer is enabled
    std::unique_lock<fastdds::RecursiveTimedMutex> lock(get_statistics_mutex());
    if (members_)
    {
        // Casting a number other than 1 to bool is not guaranteed to yield true
        return (0 != (members_->enabled_writers_mask & checked_enabled_writers));
    }
    return false;
}

const eprosima::fastdds::rtps::GUID_t& StatisticsParticipantImpl::get_guid() const
{
    using eprosima::fastdds::rtps::RTPSParticipantImpl;

    static_assert(
        std::is_base_of<StatisticsParticipantImpl, RTPSParticipantImpl>::value,
        "This method should be called from an actual RTPSParticipantImpl");

    return static_cast<const RTPSParticipantImpl*>(this)->getGuid();
}

std::recursive_mutex& StatisticsParticipantImpl::get_statistics_mutex()
{
    return statistics_mutex_;
}

bool StatisticsParticipantImpl::are_statistics_writers_enabled(
        uint32_t checked_enabled_writers)
{
    return (enabled_writers_mask_ & checked_enabled_writers);
}

void StatisticsParticipantImpl::ListenerProxy::on_statistics_data(
        const Data& data)
{
    // only delegate if the mask matches
    if ( mask_ & data._d())
    {
        external_->on_statistics_data(data);
    }
}

bool StatisticsParticipantImpl::ListenerProxy::operator <(
        const ListenerProxy& right) const
{
    return external_ < right.external_;
}

uint32_t StatisticsParticipantImpl::ListenerProxy::mask() const
{
    return mask_;
}

void StatisticsParticipantImpl::ListenerProxy::mask(
        uint32_t update) const
{
    mask_ = update;
}

bool StatisticsParticipantImpl::are_writers_involved(
        const uint32_t mask) const
{
    constexpr uint32_t writers_maks = EventKind::PUBLICATION_THROUGHPUT \
            | EventKind::RESENT_DATAS \
            | EventKind::HEARTBEAT_COUNT \
            | EventKind::GAP_COUNT \
            | EventKind::DATA_COUNT \
            | EventKind::SAMPLE_DATAS;

    return writers_maks & mask;
}

bool StatisticsParticipantImpl::are_readers_involved(
        const uint32_t mask) const
{
    constexpr uint32_t readers_maks = EventKind::HISTORY2HISTORY_LATENCY \
            | EventKind::SUBSCRIPTION_THROUGHPUT \
            | EventKind::ACKNACK_COUNT \
            | EventKind::NACKFRAG_COUNT;

    return readers_maks & mask;
}

bool StatisticsParticipantImpl::add_statistics_listener(
        std::shared_ptr<fastdds::statistics::IListener> listener,
        uint32_t kind)
{
    std::unique_lock<std::recursive_mutex> lock(get_statistics_mutex());

    uint32_t mask = kind;
    uint32_t new_mask;
    uint32_t old_mask;

    if (!listener || 0 == mask)
    {
        // avoid nullptr
        return false;
    }

    // add the new listener, and identify selection changes
    auto res = listeners_.emplace(std::make_shared<ListenerProxy>(listener, mask));
    const ListenerProxy& proxy = **res.first;

    if (res.second)
    {
        new_mask = mask;
        old_mask = 0;
    }
    else
    {
        old_mask = proxy.mask();
        new_mask = old_mask | mask;

        if ( old_mask == new_mask )
        {
            // nop
            return false;
        }

        proxy.mask(new_mask);
    }

    // no other mutex should be taken in order to prevent ABBA deadlocks
    lock.unlock();

    // Check if the listener should be register in the writers
    bool writers_res = true;
    if (are_writers_involved(new_mask)
            && !are_writers_involved(old_mask))
    {
        writers_res = register_in_writer(proxy.get_shared_ptr());
    }

    // Check if the listener should be registered in readers
    bool readers_res = true;
    if (are_readers_involved(new_mask)
            && !are_readers_involved(old_mask))
    {
        readers_res = register_in_reader(proxy.get_shared_ptr());
    }

    return writers_res && readers_res;
}

bool StatisticsParticipantImpl::remove_statistics_listener(
        std::shared_ptr<fastdds::statistics::IListener> listener,
        uint32_t kind)
{
    std::unique_lock<std::recursive_mutex> lock(get_statistics_mutex());

    uint32_t mask = kind;
    uint32_t new_mask;
    uint32_t old_mask;

    if (!listener || 0 == mask)
    {
        // avoid nullptr
        return false;
    }

    ProxyCollection::iterator it;
    auto proxy = std::make_shared<ListenerProxy>(listener, mask);
    it = listeners_.find(proxy);

    if ( listeners_.end() == it )
    {
        // not registered
        return false;
    }

    // Check where we must unregister
    proxy = *it;
    old_mask = proxy->mask();
    new_mask = old_mask & ~mask;

    if (old_mask == new_mask )
    {
        // nop
        return false;
    }

    if ( new_mask )
    {
        // update
        proxy->mask(new_mask);
    }
    else
    {
        // remove
        listeners_.erase(it);
    }

    // no other mutex should be taken in order to prevent ABBA deadlocks
    lock.unlock();

    bool writers_res = true;
    if (!are_writers_involved(new_mask)
            && are_writers_involved(old_mask))
    {
        writers_res = unregister_in_writer(proxy->get_shared_ptr());
    }

    bool readers_res = true;
    if (!are_readers_involved(new_mask)
            && are_readers_involved(old_mask))
    {
        readers_res = unregister_in_reader(proxy->get_shared_ptr());
    }

    return writers_res && readers_res
           && ((old_mask & mask) == mask); // return false if there were unregistered entities
}

void StatisticsParticipantImpl::set_enabled_statistics_writers_mask(
        uint32_t enabled_writers)
{
    enabled_writers_mask_.store(enabled_writers);
}

uint32_t StatisticsParticipantImpl::get_enabled_statistics_writers_mask()
{
    return enabled_writers_mask_.load();
}

void StatisticsParticipantImpl::on_network_statistics(
        const fastdds::rtps::GuidPrefix_t& source_participant,
        const fastdds::rtps::Locator_t& source_locator,
        const fastdds::rtps::Locator_t& reception_locator,
        const rtps::StatisticsSubmessageData& data,
        uint64_t datagram_size)
{
    static_cast<void>(source_locator);
    static_cast<void>(reception_locator);
    process_network_timestamp(source_participant, data.destination, data.ts);
    process_network_sequence(source_participant, data.destination, data.seq, datagram_size);
}

void StatisticsParticipantImpl::process_network_timestamp(
        const fastdds::rtps::GuidPrefix_t& source_participant,
        const fastdds::rtps::Locator_t& reception_locator,
        const rtps::StatisticsSubmessageData::TimeStamp& ts)
{
    using namespace eprosima::fastdds::rtps;

    if (!are_statistics_writers_enabled(EventKind::NETWORK_LATENCY))
    {
        return;
    }

    eprosima::fastdds::rtps::Time_t source_ts(ts.seconds, ts.fraction);
    eprosima::fastdds::rtps::Time_t current_ts;
    eprosima::fastdds::rtps::Time_t::now(current_ts);
    auto latency = static_cast<float>((current_ts - source_ts).to_ns());

    Locator2LocatorData notification;
    notification.src_locator().port(0);
    notification.src_locator().kind(reception_locator.kind);
    auto locator_addr = notification.src_locator().address().data();
    std::copy(source_participant.value, source_participant.value + source_participant.size, locator_addr);
    locator_addr += source_participant.size;
    std::copy(c_EntityId_RTPSParticipant.value, c_EntityId_RTPSParticipant.value + EntityId_t::size, locator_addr);

    notification.dst_locator(to_statistics_type(reception_locator));
    notification.data(latency);

    // Perform the callbacks
    Data callback_data;
    // note that the setter sets NETWORK_LATENCY by default
    callback_data.locator2locator_data(notification);

    for_each_listener([&callback_data](const Key& listener)
            {
                listener->on_statistics_data(callback_data);
            });
}

void StatisticsParticipantImpl::process_network_sequence(
        const fastdds::rtps::GuidPrefix_t& source_participant,
        const fastdds::rtps::Locator_t& reception_locator,
        const rtps::StatisticsSubmessageData::Sequence& seq,
        uint64_t datagram_size)
{
    if (!are_statistics_writers_enabled(EventKind::RTPS_LOST))
    {
        return;
    }

    lost_traffic_key key(source_participant, reception_locator);
    bool should_notify = false;
    Entity2LocatorTraffic notification;

    {
        std::lock_guard<std::recursive_mutex> lock(get_statistics_mutex());
        lost_traffic_value& value = lost_traffic_[key];

        if (value.first_sequence > seq.sequence)
        {
            // Datagrams before the first received one are ignored
            return;
        }

        if (value.first_sequence == 0)
        {
            // This is the first time we receive a statistics sequence from source_participant on reception_locator
            GUID_t guid(source_participant, ENTITYID_RTPSParticipant);
            value.data.src_guid(to_statistics_type(guid));
            value.data.dst_locator(to_statistics_type(reception_locator));
            value.first_sequence = seq.sequence;
        }
        else if (seq.sequence != value.seq_data.sequence)
        {
            // Detect discontinuity. We will only notify in that case
            should_notify = seq.sequence != (value.seq_data.sequence + 1);
            if (should_notify)
            {
                if (seq.sequence > value.seq_data.sequence)
                {
                    // Received sequence is higher, data has been lost
                    add_bytes(value.data, rtps::StatisticsSubmessageData::Sequence::distance(value.seq_data, seq));
                }

                // We should never count the current received datagram
                sub_bytes(value.data, datagram_size);

                notification = value.data;
            }
        }

        if (seq.sequence > value.seq_data.sequence)
        {
            value.seq_data = seq;
        }
    }

    if (should_notify)
    {
        // Perform the callbacks
        Data data;
        // note that the setter sets RTPS_SENT by default
        data.entity2locator_traffic(notification);
        data._d(EventKind::RTPS_LOST);

        for_each_listener([&data](const Key& listener)
                {
                    listener->on_statistics_data(data);
                });
    }
}

void StatisticsParticipantImpl::on_rtps_sent(
        const fastdds::rtps::Locator_t& loc,
        unsigned long payload_size)
{
    using namespace std;
    using eprosima::fastdds::rtps::RTPSParticipantImpl;

    if (!are_statistics_writers_enabled(EventKind::RTPS_SENT))
    {
        return;
    }

    // Compose callback and update the inner state
    Entity2LocatorTraffic notification;
    notification.src_guid(to_statistics_type(get_guid()));
    notification.dst_locator(to_statistics_type(loc));

    {
        std::lock_guard<std::recursive_mutex> lock(get_statistics_mutex());

        auto& val = traffic_[loc];
        notification.packet_count(++val.packet_count);
        notification.byte_count(val.byte_count += payload_size);
        notification.byte_magnitude_order((int16_t)floor(log10(float(val.byte_count))));
    }

    // Perform the callbacks
    Data data;
    // note that the setter sets RTPS_SENT by default
    data.entity2locator_traffic(notification);

    for_each_listener([&data](const Key& listener)
            {
                listener->on_statistics_data(data);
            });
}

void StatisticsParticipantImpl::on_entity_discovery(
        const fastdds::rtps::GUID_t& id,
        const fastdds::dds::ParameterPropertyList_t& properties)
{
    using namespace fastdds;

    if (!are_statistics_writers_enabled(EventKind::DISCOVERED_ENTITY))
    {
        return;
    }

    /**
     * @brief Get the value of a property from a property list.
     *
     * @param properties The list of properties
     * @param property_name The name of the property
     *
     * @return The value of the property. If the property is not found, then return "".
     *
     */
    auto get_physical_property_value =
            [](const dds::ParameterPropertyList_t& properties, const std::string& property_name) -> std::string
            {
                auto property = std::find_if(
                    properties.begin(),
                    properties.end(),
                    [&](const dds::ParameterProperty_t& property)
                    {
                        return property.first() == property_name;
                    });
                if (property != properties.end())
                {
                    return property->second();
                }
                return std::string("");
            };

    // Compose callback and update the inner state
    DiscoveryTime notification;
    notification.local_participant_guid(to_statistics_type(get_guid()));
    notification.remote_entity_guid(to_statistics_type(id));
    notification.host(get_physical_property_value(properties, dds::parameter_policy_physical_data_host));
    notification.user(get_physical_property_value(properties, dds::parameter_policy_physical_data_user));
    notification.process(get_physical_property_value(properties, dds::parameter_policy_physical_data_process));

    {
        // generate callback timestamp
        eprosima::fastdds::rtps::Time_t t;
        eprosima::fastdds::rtps::Time_t::now(t);
        notification.time(t.to_ns());
    }

    // Perform the callbacks
    Data data;
    data.discovery_time(notification);

    for_each_listener([&data](const Key& listener)
            {
                listener->on_statistics_data(data);
            });
}

void StatisticsParticipantImpl::on_pdp_packet(
        const uint32_t packages)
{
    if (!are_statistics_writers_enabled(EventKind::PDP_PACKETS))
    {
        return;
    }

    EntityCount notification;
    notification.guid(to_statistics_type(get_guid()));

    {
        std::lock_guard<std::recursive_mutex> lock(get_statistics_mutex());
        pdp_counter_ += packages;
        notification.count(pdp_counter_);
    }

    // Perform the callbacks
    Data data;
    // note that the setter sets RESENT_DATAS by default
    data.entity_count(notification);
    data._d(EventKind::PDP_PACKETS);

    for_each_listener([&data](const std::shared_ptr<IListener>& listener)
            {
                listener->on_statistics_data(data);
            });
}

void StatisticsParticipantImpl::on_edp_packet(
        const uint32_t packages)
{
    if (!are_statistics_writers_enabled(EventKind::EDP_PACKETS))
    {
        return;
    }

    EntityCount notification;
    notification.guid(to_statistics_type(get_guid()));

    {
        std::lock_guard<std::recursive_mutex> lock(get_statistics_mutex());
        edp_counter_ += packages;
        notification.count(edp_counter_);
    }

    // Perform the callbacks
    Data data;
    // note that the setter sets RESENT_DATAS by default
    data.entity_count(notification);
    data._d(EventKind::EDP_PACKETS);

    for_each_listener([&data](const std::shared_ptr<IListener>& listener)
            {
                listener->on_statistics_data(data);
            });
}

} // statistics
} // fastdds
} // eprosima
