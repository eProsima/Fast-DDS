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

#include <atomic>
#include <cstdint>
#include <map>
#include <mutex>
#include <set>

#include <fastdds/config.hpp>
#include <fastdds/dds/core/policy/ParameterTypes.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/common/SampleIdentity.hpp>
#include <fastdds/statistics/rtps/StatisticsCommon.hpp>
#include <statistics/rtps/GuidUtils.hpp>
#include <statistics/rtps/messages/RTPSStatisticsMessages.hpp>
#include <statistics/types/types.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class MessageReceiver;
class PDP;

} // rtps

namespace statistics {

#ifdef FASTDDS_STATISTICS

// RTPSWriter and RTPSReader statistics members
struct StatisticsAncillary
{
    std::set<std::shared_ptr<IListener>> listeners;
    std::atomic<uint32_t> enabled_writers_mask{0};
    virtual ~StatisticsAncillary() = default;
};

struct StatisticsWriterAncillary
    : public StatisticsAncillary
{
    unsigned long long data_counter = {};
    unsigned long long gap_counter = {};
    unsigned long long resent_counter = {};
    std::chrono::time_point<std::chrono::steady_clock> last_history_change_ = std::chrono::steady_clock::now();
};

struct StatisticsReaderAncillary
    : public StatisticsAncillary
{
    std::chrono::time_point<std::chrono::steady_clock> last_history_change_ = std::chrono::steady_clock::now();
};

// lambda function to traverse the listener collection
template<class Function>
Function StatisticsListenersImpl::for_each_listener(
        Function f)
{
    // Use a collection copy to prevent locking on traversal
    std::unique_lock<fastdds::RecursiveTimedMutex> lock(get_statistics_mutex());
    if (members_)
    {
        auto listeners = members_->listeners;
        lock.unlock();

        for (auto& listener : listeners)
        {
            f(listener);
        }
    }

    return f;
}

class StatisticsParticipantImpl
{
    friend class fastdds::rtps::PDP;
    friend class fastdds::rtps::MessageReceiver;

    // statistics members protection only
    std::recursive_mutex statistics_mutex_;

public:

    using GUID_t = fastdds::rtps::GUID_t;

private:

    // RTPS_SENT ancillary
    struct rtps_sent_data
    {
        unsigned long long packet_count = {};
        unsigned long long byte_count = {};
    };

    std::map<fastdds::rtps::Locator_t, rtps_sent_data> traffic_;

    // RTPS_LOST ancillary
    using lost_traffic_key = std::pair<fastdds::rtps::GuidPrefix_t, fastdds::rtps::Locator_t>;
    struct lost_traffic_value
    {
        uint64_t first_sequence = 0;
        Entity2LocatorTraffic data{};
        rtps::StatisticsSubmessageData::Sequence seq_data{};
    };
    std::map<lost_traffic_key, lost_traffic_value> lost_traffic_;

    // PDP_PACKETS ancillary
    unsigned long long pdp_counter_ = {};
    // EDP_PACKETS ancillary
    unsigned long long edp_counter_ = {};

    // Mask of enabled statistics writers
    std::atomic<uint32_t> enabled_writers_mask_{0};

    /*
     * Retrieve the GUID_t from derived class
     * @return endpoint GUID_t
     */
    const GUID_t& get_guid() const;

protected:

    class ListenerProxy
        : public IListener
        , public std::enable_shared_from_this<ListenerProxy>
    {
        // the external_ listener is the actual key value
        mutable uint32_t mask_;
        std::shared_ptr<IListener> external_;

    public:

        ListenerProxy(
                std::shared_ptr<IListener> listener,
                uint32_t mask)
            : mask_(mask)
            , external_(listener)
        {
        }

        void on_statistics_data(
                const Data& data) override;

        uint32_t mask() const;
        void mask(
                uint32_t update) const;

        bool operator <(
                const ListenerProxy& right) const;

        std::shared_ptr<IListener> get_shared_ptr() const
        {
            return std::static_pointer_cast<IListener>(const_cast<ListenerProxy*>(this)->shared_from_this());
        }

    };

    using Key = std::shared_ptr<ListenerProxy>;

    // specialized comparison operator, the actual key is the external listener address
    struct CompareProxies
    {
        bool operator ()(
                const Key& left,
                const Key& right) const
        {
            return *left < *right;
        }

    };

    using ProxyCollection = std::set<Key, CompareProxies>;
    ProxyCollection listeners_;

    // retrieve the participant mutex
    std::recursive_mutex& get_statistics_mutex();

    /**
     * @brief Check whether the statistics writers in the input mask are enabled
     *
     * @param checked_enabled_writers The mask of writers to check
     * @return True if all enabled, false otherwise
     */
    bool are_statistics_writers_enabled(
            uint32_t checked_enabled_writers);

    /** Register a listener in participant RTPSWriter entities.
     * @param listener smart pointer to the listener interface to register
     * @param guid RTPSWriter identifier. If unknown the listener is registered in all enable ones
     * @return true on success
     */
    virtual bool register_in_writer(
            std::shared_ptr<fastdds::statistics::IListener> listener,
            GUID_t guid = GUID_t::unknown()) = 0;

    /** Register a listener in participant RTPSReader entities.
     * @param listener smart pointer to the listener interface to register
     * @param guid RTPSReader identifier. If unknown the listener is registered in all enable ones
     * @return true on success
     */
    virtual bool register_in_reader(
            std::shared_ptr<fastdds::statistics::IListener> listener,
            GUID_t guid = GUID_t::unknown()) = 0;

    /** Unregister a listener in participant RTPSWriter entities.
     * @param listener smart pointer to the listener interface to unregister
     * @return true on success
     */
    virtual bool unregister_in_writer(
            std::shared_ptr<fastdds::statistics::IListener> listener) = 0;

    /** Unregister a listener in participant RTPSReader entities.
     * @param listener smart pointer to the listener interface to unregister
     * @return true on success
     */
    virtual bool unregister_in_reader(
            std::shared_ptr<fastdds::statistics::IListener> listener) = 0;

    /** Auxiliary method to traverse the listener collection
     * @param f functor to traverse the listener collection
     * @return functor copy after traversal
     */
    template<class Function>
    Function for_each_listener(
            Function f)
    {
        std::unique_lock<std::recursive_mutex> lock(get_statistics_mutex());
        auto temp_listeners = listeners_;
        lock.unlock();

        for (auto& listener : temp_listeners)
        {
            f(listener);
        }

        return f;
    }

    /** Checks if callback events require writer specific callbacks
     * @param mask callback events to be queried
     * @return if a mask statistics::EventKind may require participant writers update
     */
    bool are_writers_involved(
            const uint32_t mask) const;

    /** Checks if callback events require reader specific callbacks
     * @param mask callback events to be queried
     * @return if a mask statistics::EventKind may require participant readers update
     */
    bool are_readers_involved(
            const uint32_t mask) const;

    /*
     * Process a received statistics submessage.
     * @param [in] source_participant GUID prefix of the participant sending the message.
     * @param [in] source_locator Locator indicating the sending address.
     * @param [in] reception_locator Locator indicating the listening address.
     * @param [in] data Statistics submessage received.
     * @param [in] datagram_size The size in bytes of the received datagram.
     */
    void on_network_statistics(
            const fastdds::rtps::GuidPrefix_t& source_participant,
            const fastdds::rtps::Locator_t& source_locator,
            const fastdds::rtps::Locator_t& reception_locator,
            const rtps::StatisticsSubmessageData& data,
            uint64_t datagram_size);

    /*
     * Process a received statistics submessage timestamp, informing of network latency.
     * @param [in] source_participant GUID prefix of the participant sending the message.
     * @param [in] reception_locator Locator indicating the listening address.
     * @param [in] ts The timestamp of the statistics submessage received.
     */
    void process_network_timestamp(
            const fastdds::rtps::GuidPrefix_t& source_participant,
            const fastdds::rtps::Locator_t& reception_locator,
            const rtps::StatisticsSubmessageData::TimeStamp& ts);

    /*
     * Process a received statistics submessage sequence, informing of network loss.
     * @param [in] source_participant GUID prefix of the participant sending the message.
     * @param [in] reception_locator Locator indicating the listening address.
     * @param [in] seq The sequencing info ot the statistics submessage received.
     * @param [in] datagram_size The size in bytes of the received datagram.
     */
    void process_network_sequence(
            const fastdds::rtps::GuidPrefix_t& source_participant,
            const fastdds::rtps::Locator_t& reception_locator,
            const rtps::StatisticsSubmessageData::Sequence& seq,
            uint64_t datagram_size);

    /*
     * Report a message that is sent by the participant
     * @param loc destination
     * @param payload_size size of the current message
     */
    void on_rtps_sent(
            const fastdds::rtps::Locator_t& loc,
            unsigned long payload_size);

    /*
     * Report a message that is sent by the participant
     * @param sender_guid GUID of the entity producing the message
     * @param destination_locators_begin start of locators range
     * @param destination_locators_end end of locators range
     * @param payload_size size of the current message
     */
    template<class LocatorIteratorT>
    void on_rtps_send(
            const GUID_t& sender_guid,
            const LocatorIteratorT& destination_locators_begin,
            const LocatorIteratorT& destination_locators_end,
            unsigned long payload_size)
    {
        if (false == is_statistics_builtin(sender_guid.entityId))
        {
            auto it = destination_locators_begin;
            while (it != destination_locators_end)
            {
                on_rtps_sent(*it, payload_size);
                ++it;
            }
        }
    }

    /**
     * @brief Report that a new entity is discovered
     * @param id discovered entity GUID_t
     * @param properties The property list of the discoved entity
     */
    void on_entity_discovery(
            const GUID_t& id,
            const fastdds::dds::ParameterPropertyList_t& properties);

    /*
     * Auxiliary method to report PDP message exchange.
     * @param packages number of pdp packages sent
     */
    void on_pdp_packet(
            const uint32_t packages);

    /*
     * Auxiliary method to report EDP message exchange.
     * @param packages number of edp packages sent
     */
    void on_edp_packet(
            const uint32_t packages);

    /*
     * Report discovery protocols message exchange.
     * We filtered the non discovery traffic here to minimize presence of statistics code in endpoints implementation.
     * @param sender_guid GUID_t to filter
     * @param destination_locators_begin start of locators range
     * @param destination_locators_end end of locators range
     */
    template<class LocatorIteratorT>
    void on_discovery_packet(
            const GUID_t& sender_guid,
            const LocatorIteratorT& destination_locators_begin,
            const LocatorIteratorT& destination_locators_end)
    {
        if ( destination_locators_begin != destination_locators_end )
        {
            void (StatisticsParticipantImpl::* discovery_callback)(
                    const uint32_t) = nullptr;

            switch (sender_guid.entityId.to_uint32())
            {
                case ENTITYID_SPDP_BUILTIN_RTPSParticipant_WRITER:
                case ENTITYID_SPDP_BUILTIN_RTPSParticipant_READER:
                    discovery_callback = &StatisticsParticipantImpl::on_pdp_packet;
                    break;
                case ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER:
                case ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER:
                case ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER:
                case ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER:
#if HAVE_SECURITY
                case ENTITYID_SEDP_BUILTIN_PUBLICATIONS_SECURE_WRITER:
                case ENTITYID_SEDP_BUILTIN_PUBLICATIONS_SECURE_READER:
                case ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_SECURE_WRITER:
                case ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_SECURE_READER:
#endif  // HAVE_SECURITY
                    discovery_callback = &StatisticsParticipantImpl::on_edp_packet;
                    break;
                default:
                    return; // ignore
            }

            uint32_t packages = 0;
            auto it = destination_locators_begin;
            while (it != destination_locators_end)
            {
                ++it;
                ++packages;
            }

            (this->*discovery_callback)(packages);
        }
    }

public:

    /**
     * Registers a listener in participant's statistics callback queue
     * @param listener smart pointer to the listener being registered
     * @param kind combination of fastdds::statistics::EventKind flags used as a mask
     * @return successfully registered
     */
    bool add_statistics_listener(
            std::shared_ptr<fastdds::statistics::IListener> listener,
            uint32_t kind);

    /**
     * Unregisters a listener in participant's statistics callback queue
     * @param listener smart pointer to the listener being unregistered
     * @param kind combination of fastdds::statistics::EventKind flags used as a mask
     * @return successfully unregistered
     */
    bool remove_statistics_listener(
            std::shared_ptr<fastdds::statistics::IListener> listener,
            uint32_t kind);

    /**
     * @brief Set the enabled statistics writers mask
     *
     * @param enabled_writers The new mask to set
     */
    virtual void set_enabled_statistics_writers_mask(
            uint32_t enabled_writers);

    /**
     * @brief Get the enabled statistics writers mask
     *
     * @return The mask of enabled writers
     */
    virtual uint32_t get_enabled_statistics_writers_mask();
};

// auxiliary conversion functions
// TODO(jlbueno): private headers shall not export API
FASTDDS_EXPORTED_API detail::Locator_s to_statistics_type(fastdds::rtps::Locator_t);
FASTDDS_EXPORTED_API fastdds::rtps::Locator_t to_fastdds_type(
        detail::Locator_s);
FASTDDS_EXPORTED_API detail::GUID_s to_statistics_type(fastdds::rtps::GUID_t);
FASTDDS_EXPORTED_API fastdds::rtps::GUID_t to_fastdds_type(
        detail::GUID_s);
FASTDDS_EXPORTED_API detail::SampleIdentity_s to_statistics_type(fastdds::rtps::SampleIdentity);

#else // dummy implementation

struct StatisticsAncillary {};

class StatisticsParticipantImpl
{
    friend class fastdds::rtps::PDP;

protected:

    // inline methods for listeners callbacks

    /*
     * Process a received statistics submessage
     * @param [in] GUID prefix of the participant sending the message.
     * @param [in] Locator indicating the sending address.
     * @param [in] Locator indicating the listening address.
     * @param [in] Statistics submessage received.
     * @param [in] The size in bytes of the received datagram.
     */
    inline void on_network_statistics(
            const fastdds::rtps::GuidPrefix_t&,
            const fastdds::rtps::Locator_t&,
            const fastdds::rtps::Locator_t&,
            const rtps::StatisticsSubmessageData&,
            uint64_t)
    {
    }

    /*
     * Report a message that is sent by the participant
     * @param participant identity
     * @param start of locators range
     * @param end of locators range
     * @param size of the current message
     */
    template<class LocatorIteratorT>
    inline void on_rtps_send(
            const fastdds::rtps::GUID_t&,
            const LocatorIteratorT&,
            const LocatorIteratorT&,
            unsigned long)
    {
    }

    /**
     * @brief Report that a new entity is discovered
     *
     * @param discovered entity GUID_t
     * @param properties The property list of the discoved entity
     */
    inline void on_entity_discovery(
            const fastdds::rtps::GUID_t&,
            const fastdds::dds::ParameterPropertyList_t&)
    {
    }

    /*
     * Report discovery protocols message exchange.
     * We filtered the non discovery traffic here to minimize presence of statistics code in endpoints implementation.
     * @param GUID_t to filter
     * @param start of locators range
     * @param end of locators range
     */
    template<class LocatorIteratorT>
    inline void on_discovery_packet(
            const fastdds::rtps::GUID_t&,
            const LocatorIteratorT&,
            const LocatorIteratorT&)
    {
    }

};

#endif // FASTDDS_STATISTICS

} // namespace statistics
} // namespace fastdds
} // namespace eprosima

#endif // _STATISTICS_RTPS_STATISTICSBASE_HPP_
