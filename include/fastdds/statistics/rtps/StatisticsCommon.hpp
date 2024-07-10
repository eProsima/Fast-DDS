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
 * @file StatisticsCommon.hpp
 */

#ifndef FASTDDS_STATISTICS_RTPS__STATISTICSCOMMON_HPP
#define FASTDDS_STATISTICS_RTPS__STATISTICSCOMMON_HPP

#include <memory>
#include <type_traits>

#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/SampleIdentity.hpp>
#include <fastdds/rtps/common/Time_t.hpp>
#include <fastdds/statistics/IListeners.hpp>
#include <fastdds/utils/TimedMutex.hpp>


namespace eprosima {

namespace fastdds {
namespace rtps {

class RTPSMessageGroup;

} // rtps

namespace statistics {

#ifdef FASTDDS_STATISTICS

// Members are private details
struct StatisticsAncillary;

class StatisticsListenersImpl
{
    std::unique_ptr<StatisticsAncillary> members_;

protected:

    /**
     * Create a class A auxiliary structure
     * @return true if successfully created
     */
    template<class A>
    void init_statistics()
    {
        static_assert(
            std::is_base_of<StatisticsAncillary, A>::value,
            "Auxiliary structure must derive from StatisticsAncillary");

        if (!members_)
        {
            members_.reset(new A);
        }
    }

    /**
     * Returns the auxiliary members
     * @return The specialized auxiliary structure for each class
     */
    StatisticsAncillary* get_aux_members() const;

    /**
     * Add a listener to receive statistics backend callbacks
     * @param listener
     * @return true if successfully added
     */
    bool add_statistics_listener_impl(
            std::shared_ptr<fastdds::statistics::IListener> listener);

    /**
     * Remove a listener from receiving statistics backend callbacks
     * @param listener
     * @return true if successfully removed
     */
    bool remove_statistics_listener_impl(
            std::shared_ptr<fastdds::statistics::IListener> listener);

    /**
     * @brief Set the enabled statistics writers mask
     *
     * @param enabled_writers The new mask to set
     */
    void set_enabled_statistics_writers_mask_impl(
            uint32_t enabled_writers);

    /**
     * @brief Check whether the statistics writers in the input mask are enabled
     *
     * @param checked_enabled_writers The mask of writers to check
     * @return True if all enabled, false otherwise
     */
    bool are_statistics_writers_enabled(
            uint32_t checked_enabled_writers);

    /**
     * Lambda function to traverse the listener collection
     * @param f function object to apply to each listener
     * @return function object after being applied to each listener
     */
    template<class Function>
    Function for_each_listener(
            Function f);

    /**
     * Retrieve endpoint mutexes from derived class
     * @return defaults to the endpoint mutex
     */
    virtual fastdds::RecursiveTimedMutex& get_statistics_mutex() = 0;

    /**
     * Retrieve the GUID_t from derived class
     * @return endpoint GUID_t
     */
    virtual const fastdds::rtps::GUID_t& get_guid() const = 0;
};

// Members are private details
struct StatisticsWriterAncillary;

class StatisticsWriterImpl
    : protected StatisticsListenersImpl
{
    friend class fastdds::rtps::RTPSMessageGroup;

    /**
     * Create the auxiliary structure
     * @return nullptr on failure
     */
    StatisticsWriterAncillary* get_members() const;

    /**
     * Retrieve endpoint mutexes from derived class
     * @return defaults to the endpoint mutex
     */
    fastdds::RecursiveTimedMutex& get_statistics_mutex() final;

    /**
     * Retrieve the GUID_t from derived class
     * @return endpoint GUID_t
     */
    const fastdds::rtps::GUID_t& get_guid() const final;

protected:

    /**
     * Constructor. Mandatory member initialization.
     */
    StatisticsWriterImpl();

    // TODO: methods for listeners callbacks

    /**
     * @brief Report a change on the number of DATA / DATAFRAG submessages sent for a specific sample.
     * @param sample_identity SampleIdentity of the affected sample.
     * @param num_sent_submessages Current total number of submessages sent for the affected sample.
     */
    void on_sample_datas(
            const fastdds::rtps::SampleIdentity& sample_identity,
            size_t num_sent_submessages);

    /**
     * @brief Report that a HEARTBEAT message is sent
     * @param current count of heartbeats
     */
    void on_heartbeat(
            uint32_t count);

    /**
     * @brief Report that a DATA / DATA_FRAG message is generated
     * @param num_destinations number of locators to which the message will be sent
     */
    void on_data_generated(
            size_t num_destinations);

    /// Notify listeners of DATA / DATA_FRAG counts
    void on_data_sent();

    /**
     * @brief Reports publication throughtput based on last added sample to writer's history
     * @param payload size of the message sent
     */
    void on_publish_throughput(
            uint32_t payload);

    /// Report that a GAP message is sent
    void on_gap();

    /*
     * @brief Report that several changes are marked for redelivery
     * @param number of changes to redeliver
     */
    void on_resent_data(
            uint32_t to_send);
};

// Members are private details
struct StatisticsReaderAncillary;

class StatisticsReaderImpl
    : protected StatisticsListenersImpl
{
    friend class fastdds::rtps::RTPSMessageGroup;

    /**
     * Create the auxiliary structure
     * @return nullptr on failure
     */
    StatisticsReaderAncillary* get_members() const;

    /**
     * Retrieve endpoint mutexes from derived class
     * @return defaults to the endpoint mutex
     */
    fastdds::RecursiveTimedMutex& get_statistics_mutex() final;

    /**
     * Retrieve the GUID_t from derived class
     * @return endpoint GUID_t
     */
    const fastdds::rtps::GUID_t& get_guid() const final;

protected:

    /**
     * Constructor. Mandatory member initialization.
     */
    StatisticsReaderImpl();

    // TODO: methods for listeners callbacks

    /**
     * @brief Report that a sample has been notified to the user.
     * @param writer_guid GUID of the writer from where the sample was received.
     * @param source_timestamp Source timestamp received from the writer for the sample being notified.
     */
    void on_data_notify(
            const fastdds::rtps::GUID_t& writer_guid,
            const fastdds::rtps::Time_t& source_timestamp);

    /**
     * @brief Report that an ACKNACK message is sent
     * @param count current count of ACKNACKs
     */
    void on_acknack(
            int32_t count);

    /**
     * @brief Report that a NACKFRAG message is sent
     * @param count current count of NACKFRAGs
     */
    void on_nackfrag(
            int32_t count);

    /**
     * @brief Reports subscription throughtput based on last added sample to reader's history
     * @param payload size of the message received
     */
    void on_subscribe_throughput(
            uint32_t payload);
};

#else // when FASTDDS_STATISTICS is not defined a dummy implementation is used

class StatisticsWriterImpl
{
    friend class fastdds::rtps::RTPSMessageGroup;

protected:

    // TODO: methods for listeners callbacks

    /**
     * @brief Report a change on the number of DATA / DATAFRAG submessages sent for a specific sample.
     * Parameter: SampleIdentity of the affected sample.
     * Parameter: Current total number of submessages sent for the affected sample.
     */
    inline void on_sample_datas(
            const fastdds::rtps::SampleIdentity&,
            size_t)
    {
    }

    /**
     * @brief Report that a HEARTBEAT message is sent
     * Parameter: current count of heartbeats
     */
    inline void on_heartbeat(
            uint32_t)
    {
    }

    /**
     * @brief Report that a DATA / DATA_FRAG message is generated
     * Parameter: number of locators to which the message will be sent
     */
    inline void on_data_generated(
            size_t)
    {
    }

    /// Notify listeners of DATA / DATA_FRAG counts
    inline void on_data_sent()
    {
    }

    /**
     * @brief Reports publication throughtput based on last added sample to writer's history
     * Parameter: size of the message sent
     */
    inline void on_publish_throughput(
            uint32_t)
    {
    }

    /// Report that a GAP message is sent
    inline void on_gap()
    {
    }

    /*
     * @brief Report that several changes are marked for redelivery
     * Parameter: number of changes to redeliver
     */
    inline void on_resent_data(
            uint32_t)
    {
    }

};

class StatisticsReaderImpl
{
    friend class fastdds::rtps::RTPSMessageGroup;

protected:

    // TODO: methods for listeners callbacks

    /**
     * @brief Report that a sample has been notified to the user.
     * Parameter: GUID of the writer from where the sample was received.
     * Parameter: Source timestamp received from the writer for the sample being notified.
     */
    inline void on_data_notify(
            const fastdds::rtps::GUID_t&,
            const fastdds::rtps::Time_t&)
    {
    }

    /**
     * @brief Report that an ACKNACK message is sent
     * Parameter: current count of ACKNACKs
     */
    inline void on_acknack(
            int32_t)
    {
    }

    /**
     * @brief Report that a NACKFRAG message is sent
     * Parameter: current count of NACKFRAGs
     */
    inline void on_nackfrag(
            int32_t)
    {
    }

    /**
     * @brief Reports subscription throughtput based on last added sample to reader's history
     * Parameter: size of the message received
     */
    inline void on_subscribe_throughput(
            uint32_t)
    {
    }

};

#endif // FASTDDS_STATISTICS

} // namespace statistics
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_STATISTICS_RTPS__STATISTICSCOMMON_HPP
