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

#ifndef FASTRTPS_RTPS_READER_WRITERPROXY_H_
#define FASTRTPS_RTPS_READER_WRITERPROXY_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <mutex>

#include "../common/Types.h"
#include "../common/Locator.h"
#include "../common/CacheChange.h"
#include "../attributes/ReaderAttributes.h"
#include "../../utils/collections/ResourceLimitedVector.hpp"

#include <foonathan/memory/container.hpp>
#include <foonathan/memory/memory_pool.hpp>

#include<set>

// Testing purpose
#ifndef TEST_FRIENDS
#define TEST_FRIENDS
#endif // TEST_FRIENDS

namespace eprosima {
namespace fastrtps {
namespace rtps {

class RTPSParticipantImpl;
class StatefulReader;
class HeartbeatResponseDelay;
class WriterProxyLiveliness;
class InitialAckNack;
class RTPSMessageGroup_t;

/**
 * Class WriterProxy that contains the state of each matched writer for a specific reader.
 * @ingroup READER_MODULE
 */
class WriterProxy
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
            const ResourceLimitedContainerConfig& changes_allocation);

    /**
     * Activate this proxy associating it to a remote writer.
     * @param attributes RemoteWriterAttributes of the writer for which to keep state.
     */
    void start(const RemoteWriterAttributes& attributes);

    /**
     * Disable this proxy.
     */
    void stop();

    /**
     * Set initial value for last acked sequence number.
     * @param[in] seq_num last acked sequence number.
     */
    void loaded_from_storage_nts(const SequenceNumber_t& seq_num);

    /**
     * Get the maximum sequenceNumber received from this Writer.
     * @return the maximum sequence number.
     */
    const SequenceNumber_t available_changes_max() const;

    /**
     * Update the missing changes up to the provided sequenceNumber.
     * All changes with status UNKNOWN with seq_num <= input seq_num are marked MISSING.
     * @param[in] seq_num Pointer to the SequenceNumber.
     */
    void missing_changes_update(const SequenceNumber_t& seq_num);

    /**
     * Update the lost changes up to the provided sequenceNumber.
     * All changes with status UNKNOWN or MISSING with seq_num < input seq_num are marked LOST.
     * @param[in] seq_num Pointer to the SequenceNumber.
     */
    void lost_changes_update(const SequenceNumber_t& seq_num);

    /**
     * The provided change is marked as RECEIVED.
     * @param seq_num Sequence number of the change
     * @return True if correct.
     */
    bool received_change_set(const SequenceNumber_t& seq_num);

    /**
     * Set a change as RECEIVED and NOT RELEVANT.
     * @param seq_num Sequence number of the change
     * @return true on success
     */
    bool irrelevant_change_set(const SequenceNumber_t& seq_num);

    /**
     * Called when a change has been removed from the reader's history.
     * @param seq_num Sequence number of the removed change.
     */
    void change_removed_from_history(const SequenceNumber_t& seq_num);

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
    size_t unknown_missing_changes_up_to(const SequenceNumber_t& seq_num) const;

    /**
     * Get the attributes of the writer represented by this proxy.
     * @return const reference to the attributes of the writer represented by this proxy.
     */
    inline const RemoteWriterAttributes& attributes() const
    {
        return attributes_;
    }

    /**
     * Get the GUID of the writer represented by this proxy.
     * @return const reference to the GUID of the writer represented by this proxy.
     */
    inline const GUID_t& guid() const
    {
        return attributes_.guid;
    }

    /**
     * Get the ownership strength of the writer represented by this proxy.
     * @return ownership strength of the writer represented by this proxy.
     */
    inline uint16_t ownership_strength() const
    {
        return attributes_.ownershipStrength;
    }

    /**
     * Get the locators that should be used to send data to the writer represented by this proxy.
     * @return the locators that should be used to send data to the writer represented by this proxy.
     */
    inline const LocatorList_t& remote_locators_shrinked() const
    {
        return attributes_.endpoint.unicastLocatorList.empty() ?
            attributes_.endpoint.multicastLocatorList :
            attributes_.endpoint.unicastLocatorList;
    }

    /**
     * Check if the writer is alive
     * @return true if the writer is alive
     */
    inline bool is_alive() const
    {
        return is_alive_;
    };

    /**
     * Set the writer as alive
     */
    void assert_liveliness();

    /**
     * Get the mutex
     * @return Associated mutex
     */
    inline std::recursive_mutex* get_mutex()
    {
        return &mutex_;
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
     * @param[in] seq_num Sequence number of the cache change to check.
     * @return true if the cache change was received, false otherwise.
     */
    bool change_was_received(const SequenceNumber_t& seq_num) const;

    /**
     * Sends a preemptive acknack to the writer represented by this proxy.
     * @param buffer Message buffer to use for serialization.
     */
    void perform_initial_ack_nack(RTPSMessageGroup_t& buffer) const;

    /**
     * Sends the necessary acknac and nackfrag messages to answer the last received heartbeat message.
     * @param buffer Message buffer to use for serialization.
     */
    void perform_heartbeat_response(RTPSMessageGroup_t& buffer) const;

    /**
     * Process an incoming heartbeat from the writer represented by this proxy.
     * @param count Count field of the heartbeat message.
     * @param first_seq First sequence field of the heartbeat message.
     * @param last_seq Last sequence field of the heartbeat message.
     * @param final_flag Final flag of the heartbeat message.
     * @param liveliness_flag Liveliness flag of the heartbeat message.
     * @return true if the message is processed, false if the message is ignored.
     */
    bool process_heartbeat(
            uint32_t count,
            const SequenceNumber_t& first_seq,
            const SequenceNumber_t& last_seq,
            bool final_flag,
            bool liveliness_flag);

    /**
     * Set a new value for the interval of the heartbeat response event.
     * @param interval New interval value.
     */
    void update_heartbeat_response_interval(const Duration_t& interval);

private:

    /*!
     * @brief Add ChangeFromWriter_t up to the sequenceNumber passed, but not including this.
     * Ex: If you have seqNums 1,2,3 and you receive seq_num 6, you need to add 4 and 5.
     * @param sequence_number
     * @param default_status ChangeFromWriter_t added will be created with this ChangeFromWriterStatus_t.
     * @return True if sequence_number will be the next after last element in the changes_from_writer_ container.
     * @remarks No thread-safe.
     */
    bool maybe_add_changes_from_writer_up_to(
            const SequenceNumber_t& sequence_number, 
            const ChangeFromWriterStatus_t default_status = ChangeFromWriterStatus_t::UNKNOWN);

    bool received_change_set(
            const SequenceNumber_t& seq_num, 
            bool is_relevance);

    void cleanup();

    //! Pointer to associated StatefulReader.
    StatefulReader* reader_;
    //! Parameters of the WriterProxy
    RemoteWriterAttributes attributes_;
    //!Timed event to postpone the heartbeatResponse.
    HeartbeatResponseDelay* heartbeat_response_;
    //!To check the liveliness Status periodically.
    WriterProxyLiveliness* writer_proxy_liveliness_;
    //! Timed event to send initial acknack.
    InitialAckNack* initial_acknack_;
    //! Last Heartbeatcount.
    uint32_t last_heartbeat_count_;
    //!Indicates if the heartbeat has the final flag set.
    bool heartbeat_final_flag_;
    //!Is the writer alive
    bool is_alive_;
    //!Mutex Pointer
    mutable std::recursive_mutex mutex_;

    using pool_allocator_t =
        foonathan::memory::memory_pool<foonathan::memory::node_pool, foonathan::memory::heap_allocator>;

    //!Memory pool allocator for changes_from_writer_
    pool_allocator_t changes_pool_;
    //!Vector containing the ChangeFromWriter_t objects.
    foonathan::memory::set<ChangeFromWriter_t, pool_allocator_t> changes_from_writer_;
    SequenceNumber_t changes_from_writer_low_mark_;
    //! Store last ChacheChange_t notified.
    SequenceNumber_t last_notified_;
    //!To fool RTPSMessageGroup when using this proxy as single destination
    ResourceLimitedVector<GUID_t> guid_as_vector_;

    using ChangeIterator = decltype(changes_from_writer_)::iterator;

    void for_each_set_status_from(
            ChangeIterator first,
            ChangeIterator last,
            ChangeFromWriterStatus_t status,
            ChangeFromWriterStatus_t new_status);

    void for_each_set_status_from_and_maybe_remove(
            ChangeIterator first,
            ChangeIterator last,
            ChangeFromWriterStatus_t status,
            ChangeFromWriterStatus_t or_status,
            ChangeFromWriterStatus_t new_status);
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif
#endif /* FASTRTPS_RTPS_READER_WRITERPROXY_H_ */
