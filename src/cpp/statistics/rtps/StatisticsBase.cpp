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

#include "StatisticsBase.hpp"
#include <fastdds/dds/log/Log.hpp>
#include <statistics/types/types.h>

using namespace eprosima::fastdds::statistics;

bool StatisticsListenersImpl::add_statistics_listener_impl(
        std::shared_ptr<fastdds::statistics::IListener> listener)
{
    if(!listener)
    {
        // avoid nullptr
        return false;
    }

    std::lock_guard<fastrtps::RecursiveTimedMutex> lock(get_statistics_mutex());

    // if the collection is not initialized do it
    if (!members_)
    {
        members_.reset(new StatisticsAncillary());
    }

    // add the new listener
    return members_->listeners.insert(listener).second;
}

bool StatisticsListenersImpl::remove_statistics_listener_impl(
        std::shared_ptr<fastdds::statistics::IListener> listener)
{
    std::lock_guard<fastrtps::RecursiveTimedMutex> lock(get_statistics_mutex());

    if(!members_ || !listener)
    {
        // avoid nullptr
        return false;
    }

    return 1 == members_->listeners.erase(listener);
}

void StatisticsParticipantImpl::ListenerProxy::on_statistics_data(const Data& data)
{
    // only delegate if the mask matches
    if ( mask_ & data._d() )
    {
        external_->on_statistics_data(data);
    }
}

bool StatisticsParticipantImpl::ListenerProxy::operator<(const ListenerProxy& right) const
{
    return external_ < right.external_;
}

uint32_t StatisticsParticipantImpl::ListenerProxy::mask() const
{
    return mask_;
}

void StatisticsParticipantImpl::ListenerProxy::mask(uint32_t update) const
{
    mask_ = update;
}

constexpr bool StatisticsParticipantImpl::are_datawriters_involved(const uint32_t mask) const
{
    using namespace fastdds::statistics;

    constexpr uint32_t writers_maks = HISTORY2HISTORY_LATENCY \
        | PUBLICATION_THROUGHPUT \
        | RTPS_SENT \
        | RESENT_DATAS \
        | HEARTBEAT_COUNT \
        | DATA_COUNT;

    return writers_maks & mask;
}

constexpr bool StatisticsParticipantImpl::are_datareaders_involved(const uint32_t mask) const
{
    using namespace fastdds::statistics;

    constexpr uint32_t readers_maks = HISTORY2HISTORY_LATENCY \
        | SUBSCRIPTION_THROUGHPUT \
        | RTPS_LOST \
        | ACKNACK_COUNT \
        | NACKFRAG_COUNT \
        | GAP_COUNT;

    return readers_maks & mask;
}

bool StatisticsParticipantImpl::add_statistics_listener(
        std::shared_ptr<fastdds::statistics::IListener> listener,
        fastdds::statistics::EventKind kind)
{
    std::lock_guard<std::recursive_mutex> lock(*getParticipantMutex());

    uint32_t mask = kind, new_mask, old_mask;

    if(!listener || 0 == mask)
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

        if( old_mask == new_mask )
        {
            // nop
            return false;
        }

        proxy.mask(new_mask);
    }

    // Check if the listener should be register in the writers
    bool writers_res = true;
    if (are_datawriters_involved(new_mask)
            && !are_datawriters_involved(old_mask)
            && ((writers_res = register_in_datawriter(proxy.get_shared_ptr())) == false))
    {
        logError(RTPS_STATISTICS, "Fail to register statistical listener in all writers");
    }

    // Check if the listener should be register in the writers
    bool readers_res = true;
    if (are_datareaders_involved(new_mask)
            && !are_datareaders_involved(old_mask)
            && ((readers_res = register_in_datareader(proxy.get_shared_ptr())) == false))
    {
        logError(RTPS_STATISTICS, "Fail to register statistical listener in all readers");
    }

    // TODO Barro: check and register discovery listeners

    return writers_res && readers_res;
}

bool StatisticsParticipantImpl::remove_statistics_listener(
        std::shared_ptr<fastdds::statistics::IListener> listener,
        fastdds::statistics::EventKind kind)
{
    using namespace std;

    std::lock_guard<std::recursive_mutex> lock(*getParticipantMutex());

    uint32_t mask = kind, new_mask, old_mask;

    if(!listener || 0 == mask)
    {
        // avoid nullptr
        return false;
    }

    ProxyCollection::iterator it;
    auto proxy = make_shared<ListenerProxy>(listener, mask);
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

    bool writers_res = true;
    if (!are_datawriters_involved(new_mask)
            && are_datawriters_involved(old_mask)
            && ((writers_res = unregister_in_datawriter(proxy->get_shared_ptr())) == false))
    {
        logError(RTPS_STATISTICS, "Fail to revoke registration of statistical listener in all writers");
    }

    bool readers_res = true;
    if (!are_datareaders_involved(new_mask)
            && are_datareaders_involved(old_mask)
            && ((readers_res = unregister_in_datareader(proxy->get_shared_ptr())) == false))
    {
        logError(RTPS_STATISTICS, "Fail to revoke registration of statistical listener in all readers");
    }

    // TODO Barro: check and unregister discovery listeners

    return writers_res && readers_res
        && ((old_mask & mask) == mask); // return false if there were unregistered entities
}
