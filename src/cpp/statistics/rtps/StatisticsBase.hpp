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
 * @file StatisticsBase.hpp
 */

#ifndef _STATISTICS_RTPS_STATISTICSBASE_HPP_
#define _STATISTICS_RTPS_STATISTICSBASE_HPP_

#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/common/Locator.h>
#include <fastdds/statistics/rtps/StatisticsCommon.hpp>
#include <fastrtps/config.h>
#include <statistics/types/types.h>

#include <mutex>
#include <set>

namespace eprosima {
namespace fastdds {
namespace statistics {

#ifdef FASTDDS_STATISTICS

// RTPSWriter and RTPSReader statistics members
struct StatisticsAncillary
{
    std::set<std::shared_ptr<IListener>> listeners;
};

struct StatisticsWriterAncillary
    : public StatisticsAncillary
{
    unsigned long long data_counter = {};
};

struct StatisticsReaderAncillary
    : public StatisticsAncillary
{
};

// lambda function to traverse the listener collection
template<class Function>
Function StatisticsListenersImpl::for_each_listener(
        Function f)
{
    std::lock_guard<fastrtps::RecursiveTimedMutex> lock(get_statistics_mutex());

    if (members_)
    {
        for(auto& listener : members_->listeners)
        {
            f(listener);
        }
    }

    return f;
}

class StatisticsParticipantImpl
{
    // RTPS_SENT ancillary
    struct rtps_sent_data
    {
        unsigned long long packet_count = {};
        unsigned long long byte_count = {};
    };

    std::map<fastrtps::rtps::Locator_t, rtps_sent_data> traffic;

protected:

    class ListenerProxy
        : public IListener
        , public std::enable_shared_from_this<ListenerProxy>
    {
        // the external_ listener is the actual key value
        mutable uint32_t mask_;
        std::shared_ptr<IListener> external_;

        public:

        ListenerProxy(std::shared_ptr<IListener> listener, uint32_t mask)
            : mask_(mask)
            , external_(listener)
        {
        }

        void on_statistics_data(const Data& data) override;

        uint32_t mask() const;
        void mask(uint32_t update) const;

        bool operator<(const ListenerProxy& right) const;

        std::shared_ptr<IListener> get_shared_ptr() const
        {
            return std::static_pointer_cast<IListener>(const_cast<ListenerProxy*>(this)->shared_from_this());
        }
    };

    using GUID_t = fastrtps::rtps::GUID_t;
    using Key = std::shared_ptr<ListenerProxy>;

    // specialized comparison operator, the actual key is the external listener address
    struct CompareProxies
    {
        bool operator()(const Key& left, const Key& right) const
        {
            return *left < *right;
        }
    };

    using ProxyCollection = std::set<Key, CompareProxies>;
    ProxyCollection listeners_;

    // retrieve the participant mutex
    std::recursive_mutex& get_statistics_mutex();

    /** Register a listener in participant DataWriter entities.
     * @param listener, smart pointer to the listener interface to register
     * @param guid, DataWriter identifier. If unknown the listener is registered in all enable ones
     * @return true on success
     */
    virtual bool register_in_datawriter(
            std::shared_ptr<fastdds::statistics::IListener> listener,
            GUID_t guid = GUID_t::unknown()) = 0;

    /** Register a listener in participant DataReader entities.
     * @param listener, smart pointer to the listener interface to register
     * @param guid, DataReader identifier. If unknown the listener is registered in all enable ones
     * @return true on success
     */
    virtual bool register_in_datareader(
            std::shared_ptr<fastdds::statistics::IListener> listener,
            GUID_t guid = GUID_t::unknown()) = 0;

    /** Unregister a listener in participant DataWriter entities.
     * @param listener, smart pointer to the listener interface to unregister
     * @param guid, DataWriter identifier. If unknown the listener is unregistered in all enable ones
     * @return true on success
     */
    virtual bool unregister_in_datawriter(
            std::shared_ptr<fastdds::statistics::IListener> listener,
            GUID_t guid = GUID_t::unknown()) = 0;

    /** Unregister a listener in participant DataReader entities.
     * @param listener, smart pointer to the listener interface to unregister
     * @param guid, DataReader identifier. If unknown the listener is unregistered in all enable ones
     * @return true on success
     */
    virtual bool unregister_in_datareader(
            std::shared_ptr<fastdds::statistics::IListener> listener,
            GUID_t guid = GUID_t::unknown()) = 0;

    // lambda function to traverse the listener collection
    template<class Function>
    Function for_each_listener(
            Function f)
    {
        std::lock_guard<std::recursive_mutex> lock(get_statistics_mutex());

        for(auto& listener : listeners_)
        {
            f(listener);
        }

        return f;
    }

    // returns if a mask statistics::EventKind may require participant writers update
    bool are_datawriters_involved(const uint32_t mask) const;

    // returns if a mask statistics::EventKind may require participant readers update
    bool are_datareaders_involved(const uint32_t mask) const;

    // TODO: methods for listeners callbacks

    /*
     * Report a message is send by the participant
     * @param loc, destination
     * @param payload_size, size of the current message
     */
    void on_rtps_sent(
            const fastrtps::rtps::Locator_t & loc,
            unsigned long payload_size);

    /*
     * Report a message is send by the participant
     * @param destination_locators_begin, begin of locators range
     * @param destination_locators_end, send of locators range
     * @param payload_size, size of the current message
     */
    template<class LocatorIteratorT>
    void on_rtps_send(
        const LocatorIteratorT& destination_locators_begin,
        const LocatorIteratorT& destination_locators_end,
        unsigned long payload_size)
    {
        auto it = destination_locators_begin;
        while(it != destination_locators_end)
        {
            on_rtps_sent(*it, payload_size);
            ++it;
        }
    }

public:

    bool add_statistics_listener(
            std::shared_ptr<fastdds::statistics::IListener> listener,
            fastdds::statistics::EventKind kind);

    bool remove_statistics_listener(
            std::shared_ptr<fastdds::statistics::IListener> listener,
            fastdds::statistics::EventKind kind);
};

// auxiliary conversion functions
detail::Locator_s to_statistics_type(fastrtps::rtps::Locator_t);
detail::GUID_s to_statistics_type(fastrtps::rtps::GUID_t);

#else // dummy implementation

struct StatisticsAncillary {};

class StatisticsParticipantImpl
{
protected:

    // inline methods for listeners callbacks

    /*
     * Report a message is send by the participant
     * @param destination_locators_begin, begin of locators range
     * @param destination_locators_end, send of locators range
     * @param payload_size, size of the current message
     */
    template<class LocatorIteratorT>
    inline void on_rtps_send(
                const LocatorIteratorT&,
                const LocatorIteratorT&,
                unsigned long)
    {
    }

};

#endif // FASTDDS_STATISTICS

} // namespace statistics
} // namespace fastdds
} // namespace eprosima

#endif // _STATISTICS_RTPS_STATISTICSBASE_HPP_
