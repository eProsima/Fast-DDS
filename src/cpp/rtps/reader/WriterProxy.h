// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file WriterProxy.h
 */

#ifndef FASTDDS_RTPS_READER_WRITERPROXY_H_
#define FASTDDS_RTPS_READER_WRITERPROXY_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <set>
#include <vector>

#include <foonathan/memory/container.hpp>
#include <foonathan/memory/memory_pool.hpp>

#include <fastdds/rtps/common/Types.hpp>
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/rtps/attributes/ReaderAttributes.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAllocationAttributes.hpp>
#include <fastdds/rtps/messages/RTPSMessageSenderInterface.hpp>
#include <fastdds/utils/collections/ResourceLimitedVector.hpp>
#include <fastdds/rtps/common/LocatorSelectorEntry.hpp>

#include <rtps/builtin/data/WriterProxyData.hpp>

// Testing purpose
#ifndef TEST_FRIENDS
#define TEST_FRIENDS
#endif // TEST_FRIENDS

namespace eprosima {
namespace fastdds {
namespace rtps {

class RTPSParticipantImpl;
class StatefulReader;
class RTPSMessageGroup_t;
class TimedEvent;

/**
 * Class WriterProxy that contains the state of each matched writer for a specific reader.
 * @ingroup READER_MODULE
 */
class WriterProxy : public RTPSMessageSenderInterface
{
    TEST_FRIENDS

public:

    ~WriterProxy();

    /**
     * Constructor.
     * @param reader Pointer to the StatefulReader creating this proxy.
     * @param changes_allocation Configuration for the set of Change
     */
    WriterProxy(
            StatefulReader* reader,
            const RemoteLocatorsAllocationAttributes& loc_alloc,
            const ResourceLimitedContainerConfig& changes_allocation);

    /**
     * Activate this proxy associating it to a remote writer.
     * @param attributes WriterProxyData of the writer for which to keep state.
     * @param initial_sequence Sequence number of last acknowledged change.
     */
    void start(
            const WriterProxyData& attributes,
            const SequenceNumber_t& initial_sequence);

    /**
     * Activate this proxy associating it to a remote writer.
     * @param attributes WriterProxyData of the writer for which to keep state.
     * @param initial_sequence Sequence number of last acknowledged change.
     * @param is_datasharing Whether the writer is datasharing with us or not.
     */
    void start(
            const WriterProxyData& attributes,
            const SequenceNumber_t& initial_sequence,
            bool is_datasharing);

    /**
     * Update information on the remote writer.
     * @param attributes WriterProxyData with updated information of the writer.
     */
    void update(
            const WriterProxyData& attributes);

    /**
     * Disable this proxy.
     */
    void stop();

    /**
     * Get the maximum sequenceNumber received from this Writer.
     * @return the maximum sequence number.
     */
    const SequenceNumber_t available_changes_max() const;

    /**
     * Update the missing changes up to the provided sequenceNumber.
     * All changes with status UNKNOWN with seq_num <= input seq_num are marked MISSING.
     * @param [in] seq_num Pointer to the SequenceNumber.
     */
    void missing_changes_update(
            const SequenceNumber_t& seq_num);

    /**
     * Update the lost changes up to the provided sequenceNumber.
     * All changes with status UNKNOWN or MISSING with seq_num < input seq_num are marked LOST.
     * @param [in] seq_num Pointer to the SequenceNumber.
     */
    int32_t lost_changes_update(
            const SequenceNumber_t& seq_num);

    /**
     * The provided change is marked as RECEIVED.
     * @param seq_num Sequence number of the change
     * @return True if correct.
     */
    bool received_change_set(
            const SequenceNumber_t& seq_num);

    /**
     * Set a change as RECEIVED and NOT RELEVANT.
     * @param seq_num Sequence number of the change
     * @return true on success
     */
    bool irrelevant_change_set(
            const SequenceNumber_t& seq_num);

    /**
     * Check if this proxy has any missing change.
     * @return true when there is at least one missing change on this proxy.
     */
    bool are_there_missing_changes() const;

    /**
     * The method returns a SequenceNumberSet_t containing the sequence number of all missing changes.
     * @return Sequence number set of missing changes.
     */
    SequenceNumberSet_t missing_changes() const;

    /**
     * Get the number of missing changes up to a certain sequence number.
     * @param seq_num Sequence number limiting the query.
     *                Only changes with a sequence number less than this one will be considered.
     * @return the number of missing changes with a sequence number less than seq_num.
     */
    size_t unknown_missing_changes_up_to(
            const SequenceNumber_t& seq_num) const;

    /**
     * Get the GUID of the writer represented by this proxy.
     * @return const reference to the GUID of the writer represented by this proxy.
     */
    inline const GUID_t& guid() const
    {
        return locators_entry_.remote_guid;
    }

    inline const GUID_t& persistence_guid() const
    {
        return persistence_guid_;
    }

    inline dds::LivelinessQosPolicyKind liveliness_kind() const
    {
        return liveliness_kind_;
    }

    /**
     * Get the ownership strength of the writer represented by this proxy.
     * @return ownership strength of the writer represented by this proxy.
     */
    inline uint32_t ownership_strength() const
    {
        return ownership_strength_;
    }

    /**
     * Get the locators that should be used to send data to the writer represented by this proxy.
     * @return the locators that should be used to send data to the writer represented by this proxy.
     */
    inline const ResourceLimitedVector<Locator_t>& remote_locators_shrinked() const
    {
        return locators_entry_.unicast.empty() ?
               locators_entry_.multicast :
               locators_entry_.unicast;
    }

    /**
     * Check if the writer is alive
     * @return true if the writer is alive
     */
    inline bool is_alive() const
    {
        return is_alive_;
    }

    /*!
     * @brief Returns number of ChangeFromWriter_t managed currently by the WriterProxy.
     * @return Number of ChangeFromWriter_t managed currently by the WriterProxy.
     */
    size_t number_of_changes_from_writer() const;

    /*!
     * @brief Returns next SequenceNumber_t to be notified.
     * @return Next SequenceNumber_t to be nofified or invalid SequenceNumber_t
     * if any SequenceNumber_t to be notified.
     */
    SequenceNumber_t next_cache_change_to_be_notified();

    /**
     * Checks whether a cache change was already received from this proxy.
     * @param [in] seq_num Sequence number of the cache change to check.
     * @return true if the cache change was received, false otherwise.
     */
    bool change_was_received(
            const SequenceNumber_t& seq_num) const;

    /**
     * Sends a preemptive acknack to the writer represented by this proxy.
     */
    bool perform_initial_ack_nack();

    /**
     * Sends the necessary acknac and nackfrag messages to answer the last received heartbeat message.
     */
    void perform_heartbeat_response();

    /**
     * Process an incoming heartbeat from the writer represented by this proxy.
     * @param count Count field of the heartbeat message.
     * @param first_seq First sequence field of the heartbeat message.
     * @param last_seq Last sequence field of the heartbeat message.
     * @param final_flag Final flag of the heartbeat message.
     * @param liveliness_flag Liveliness flag of the heartbeat message.
     * @param disable_positive True if positive ACKs are disabled.
     * @param [out] assert_liveliness Returns true when liveliness should be asserted on this writer
     * @return true if the message is processed, false if the message is ignored.
     */
    bool process_heartbeat(
            uint32_t count,
            const SequenceNumber_t& first_seq,
            const SequenceNumber_t& last_seq,
            bool final_flag,
            bool liveliness_flag,
            bool disable_positive,
            bool& assert_liveliness,
            int32_t& current_sample_lost);

    /**
     * Set a new value for the interval of the heartbeat response event.
     * @param interval New interval value.
     */
    void update_heartbeat_response_interval(
            const dds::Duration_t& interval);

    /**
     * Check if the destinations managed by this sender interface have changed.
     *
     * @return true if destinations have changed, false otherwise.
     */
    virtual bool destinations_have_changed() const override
    {
        return false;
    }

    /**
     * Get a GUID prefix representing all destinations.
     *
     * @return When all the destinations share the same prefix (i.e. belong to the same participant)
     * that prefix is returned. When there are no destinations, or they belong to different
     * participants, c_GuidPrefix_Unknown is returned.
     */
    virtual GuidPrefix_t destination_guid_prefix() const override
    {
        return guid_prefix_as_vector_.at(0);
    }

    /**
     * Get the GUID prefix of all the destination participants.
     *
     * @return a const reference to a vector with the GUID prefix of all destination participants.
     */
    virtual const std::vector<GuidPrefix_t>& remote_participants() const override
    {
        return guid_prefix_as_vector_;
    }

    /**
     * Get the GUID of all destinations.
     *
     * @return a const reference to a vector with the GUID of all destinations.
     */
    virtual const std::vector<GUID_t>& remote_guids() const override
    {
        return guid_as_vector_;
    }

    /**
     * Send a message through this interface.
     *
     * @param buffers Vector of NetworkBuffers to send with data already serialized.
     * @param total_bytes Total number of bytes to send. Should be equal to the sum of the @c size field of all buffers.
     * @param max_blocking_time_point Future timepoint where blocking send should end.
     */
    virtual bool send(
            const std::vector<eprosima::fastdds::rtps::NetworkBuffer>& buffers,
            const uint32_t& total_bytes,
            std::chrono::steady_clock::time_point max_blocking_time_point) const override;

    bool is_on_same_process() const
    {
        return is_on_same_process_;
    }

    bool is_datasharing_writer() const
    {
        return is_datasharing_writer_;
    }

    /*
     * Do nothing.
     * This object always is protected by reader's mutex.
     */
    void lock() override
    {
    }

    /*
     * Do nothing.
     * This object always is protected by reader's mutex.
     */
    void unlock() override
    {
    }

private:

    enum StateCode
    {
        IDLE = 0, //! Writer Proxy is not performing any critical operations.
        BUSY, //! Writer Proxy is performing a critical operation. Some actions (e.g. stop) should wait for its completion.
        STOPPED, //! Writer Proxy has been requested to \c stop.
    };

    /**
     * Set initial value for last acked sequence number.
     * @param [in] seq_num last acked sequence number.
     */
    void loaded_from_storage(
            const SequenceNumber_t& seq_num);

    bool received_change_set(
            const SequenceNumber_t& seq_num,
            bool is_relevance);

    void cleanup();

    void clear();

    //! Pointer to associated StatefulReader.
    StatefulReader* reader_;
    //!Timed event to postpone the heartbeatResponse.
    TimedEvent* heartbeat_response_;
    //! Timed event to send initial acknack.
    TimedEvent* initial_acknack_;
    //! Last Heartbeatcount.
    std::atomic<uint32_t> last_heartbeat_count_;
    //!Indicates if the heartbeat has the final flag set.
    std::atomic<bool> heartbeat_final_flag_;
    //!Is the writer alive
    bool is_alive_;

    using pool_allocator_t =
            foonathan::memory::memory_pool<foonathan::memory::node_pool, foonathan::memory::heap_allocator>;

    //! Memory pool allocator for changes_received_
    pool_allocator_t changes_pool_;
    //! Vector containing the sequence number of the received ChangeFromWriter_t objects.
    foonathan::memory::set<SequenceNumber_t, pool_allocator_t> changes_received_;
    //! Sequence number of the highest available change
    SequenceNumber_t changes_from_writer_low_mark_;
    //! Highest sequence number informed by writer
    SequenceNumber_t max_sequence_number_;
    //! Store last ChacheChange_t notified.
    SequenceNumber_t last_notified_;
    //!To fool RTPSMessageGroup when using this proxy as single destination
    ResourceLimitedVector<GUID_t> guid_as_vector_;
    //!To fool RTPSMessageGroup when using this proxy as single destination
    ResourceLimitedVector<GuidPrefix_t> guid_prefix_as_vector_;
    //! Is the writer on the same process
    bool is_on_same_process_;
    //! Taken from QoS
    uint32_t ownership_strength_;
    //! Taken from QoS
    dds::LivelinessQosPolicyKind liveliness_kind_;
    //! Taken from proxy data
    GUID_t persistence_guid_;
    //! Taken from proxy data
    LocatorSelectorEntry locators_entry_;
    //! Is the writer datasharing
    bool is_datasharing_writer_;
    //! Wether at least one heartbeat was recevied.
    bool received_at_least_one_heartbeat_;
    //! Current state of this Writer Proxy
    std::atomic<StateCode> state_;

    using ChangeIterator = decltype(changes_received_)::iterator;

#if !defined(NDEBUG) && defined(FASTDDS_SOURCE) && defined(__unix__)
    int get_mutex_owner() const;

    int get_thread_id() const;
#endif // if !defined(NDEBUG) && defined(FASTDDS_SOURCE) && defined(__unix__)
};

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* FASTDDS_RTPS_READER_WRITERPROXY_H_ */
