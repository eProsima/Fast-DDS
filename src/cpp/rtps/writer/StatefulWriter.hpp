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
 * @file StatefulWriter.hpp
 */

#ifndef FASTDDS_RTPS_WRITER__STATEFULWRITER_HPP
#define FASTDDS_RTPS_WRITER__STATEFULWRITER_HPP

#include <condition_variable>
#include <mutex>

#include <fastdds/rtps/common/VendorId_t.hpp>
#include <fastdds/rtps/history/IChangePool.hpp>
#include <fastdds/rtps/history/IPayloadPool.hpp>
#include <fastdds/rtps/interfaces/IReaderDataFilter.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>
#include <fastdds/utils/collections/ResourceLimitedVector.hpp>

#include <rtps/writer/BaseWriter.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class ReaderProxy;
class TimedEvent;

/**
 * Class StatefulWriter, specialization of BaseWriter that maintains information of each matched Reader.
 * @ingroup WRITER_MODULE
 */
class StatefulWriter : public BaseWriter
{

public:

    StatefulWriter(
            RTPSParticipantImpl* impl,
            const GUID_t& guid,
            const WriterAttributes& att,
            fastdds::rtps::FlowController* flow_controller,
            WriterHistory* hist,
            WriterListener* listen = nullptr);

    virtual ~StatefulWriter();

    void local_actions_on_writer_removed() override;

    //vvvvvvvvvvvvvvvvvvvvv [Exported API] vvvvvvvvvvvvvvvvvvvvv

    bool matched_reader_add_edp(
            const ReaderProxyData& data) final;

    bool matched_reader_remove(
            const GUID_t& reader_guid) final;

    bool matched_reader_is_matched(
            const GUID_t& reader_guid) final;

    void reader_data_filter(
            fastdds::rtps::IReaderDataFilter* filter) final;

    const fastdds::rtps::IReaderDataFilter* reader_data_filter() const final;

    bool has_been_fully_delivered(
            const SequenceNumber_t& seq_num) const final;

    bool is_acked_by_all(
            const SequenceNumber_t& a_change) const final;

    bool wait_for_all_acked(
            const dds::Duration_t& max_wait) final;

    void update_attributes(
            const WriterAttributes& att) final;

    bool get_disable_positive_acks() const final
    {
        return disable_positive_acks_;
    }

    /**
     * @brief Fills the provided vector with the GUIDs of the matched readers.
     *
     * @param[out] guids Vector to be filled with the GUIDs of the matched readers.
     * @return True if the operation was successful.
     */
    bool matched_readers_guids(
            std::vector<GUID_t>& guids) const final;

#ifdef FASTDDS_STATISTICS
    bool get_connections(
            fastdds::statistics::rtps::ConnectionList& connection_list) final;
#endif // ifdef FASTDDS_STATISTICS

    //^^^^^^^^^^^^^^^^^^^^^^ [Exported API] ^^^^^^^^^^^^^^^^^^^^^^^

    //vvvvvvvvvvvvvvvvvvvvv [BaseWriter API] vvvvvvvvvvvvvvvvvvvvvv

    void unsent_change_added_to_history(
            CacheChange_t* p,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time) override;

    bool change_removed_by_history(
            CacheChange_t* a_change,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time) override;

    DeliveryRetCode deliver_sample_nts(
            CacheChange_t* cache_change,
            RTPSMessageGroup& group,
            LocatorSelectorSender& locator_selector,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time) final;

    LocatorSelectorSender& get_general_locator_selector() final
    {
        return locator_selector_general_;
    }

    LocatorSelectorSender& get_async_locator_selector() final
    {
        return locator_selector_async_;
    }

    bool process_acknack(
            const GUID_t& writer_guid,
            const GUID_t& reader_guid,
            uint32_t ack_count,
            const SequenceNumberSet_t& sn_set,
            bool final_flag,
            bool& result,
            fastdds::rtps::VendorId_t origin_vendor_id) final;

    bool process_nack_frag(
            const GUID_t& writer_guid,
            const GUID_t& reader_guid,
            uint32_t ack_count,
            const SequenceNumber_t& seq_num,
            const FragmentNumberSet_t& fragments_state,
            bool& result,
            fastdds::rtps::VendorId_t origin_vendor_id) final;

    bool try_remove_change(
            const std::chrono::steady_clock::time_point& max_blocking_time_point,
            std::unique_lock<RecursiveTimedMutex>& lock) final;

    bool wait_for_acknowledgement(
            const SequenceNumber_t& seq,
            const std::chrono::steady_clock::time_point& max_blocking_time_point,
            std::unique_lock<RecursiveTimedMutex>& lock) final;

    //^^^^^^^^^^^^^^^^^^^^^^ [BaseWriter API] ^^^^^^^^^^^^^^^^^^^^^^^

    /**
     * @brief Sends a change directly to a intraprocess reader.
     *
     * @param change        Pointer to the change to deliver.
     * @param reader_proxy  Pointer to the proxy representing the reader to deliver the change to.
     *
     * @return True on success.
     */
    bool intraprocess_delivery(
            CacheChange_t* change,
            ReaderProxy* reader_proxy);

    /**
     * @brief Sends a gap directly to an intraprocess reader.
     *
     * @param reader_proxy  Pointer to the proxy representing the reader to deliver the gap to.
     * @param seq_num       Sequence number from this writer that the reader should skip.
     *
     * @return True on success.
     */
    bool intraprocess_gap(
            ReaderProxy* reader_proxy,
            const SequenceNumber_t& seq_num)
    {
        return intraprocess_gap(reader_proxy, seq_num, seq_num + 1);
    }

    /**
     * @brief Sends a gap directly to an intraprocess reader.
     *
     * The destination reader would skip all the sequence numbers in the range [first_seq, last_seq).
     *
     * @param reader_proxy  Pointer to the proxy representing the reader to deliver the gap to.
     * @param first_seq     First sequence number in the range that the reader should skip.
     * @param last_seq      Last sequence number in the range that the reader should skip.
     *
     * @return True on success.
     */
    bool intraprocess_gap(
            ReaderProxy* reader_proxy,
            const SequenceNumber_t& first_seq,
            const SequenceNumber_t& last_seq);

    /**
     * @brief Sends a heartbeat directly to an intraprocess reader.
     *
     * @param reader_proxy  Pointer to the proxy representing the reader to deliver the heartbeat to.
     * @param liveliness    True if the heartbeat is a liveliness one.
     *
     * @return True on success.
     */
    bool intraprocess_heartbeat(
            ReaderProxy* reader_proxy,
            bool liveliness = false);

    /**
     * @brief Increment the HB count.
     */
    inline void increment_hb_count()
    {
        on_heartbeat(++heartbeat_count_);
    }

    template <typename Function>
    Function for_each_reader_proxy(
            Function f) const
    {
        // we cannot directly pass iterators neither const_iterators to matched_readers_ because then the functor would
        // be able to modify ReaderProxy elements
        for ( const ReaderProxy* rp : matched_local_readers_ )
        {
            f(rp);
        }
        for ( const ReaderProxy* rp : matched_datasharing_readers_ )
        {
            f(rp);
        }
        for ( const ReaderProxy* rp : matched_remote_readers_ )
        {
            f(rp);
        }

        return f;
    }

    bool all_readers_updated();

    /**
     * Find a Reader Proxy in this writer.
     *
     * @param [in] readerGuid The GUID_t of the reader.
     * @param [out] RP Pointer to pointer to return the ReaderProxy.
     *
     * @return True if correct.
     */
    bool matched_reader_lookup(
            GUID_t& readerGuid,
            ReaderProxy** RP);

    /**
     * Get count of heartbeats
     * @return count of heartbeats
     */
    inline Count_t get_heartbeat_count() const
    {
        return this->heartbeat_count_;
    }

    /**
     * Get the number of matched readers.
     *
     * @return Number of the matched readers
     */
    inline size_t get_matched_readers_size() const
    {
        std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
        return matched_remote_readers_.size()
               + matched_local_readers_.size()
               + matched_datasharing_readers_.size();
    }

    /**
     * Update the WriterTimes attributes of all associated ReaderProxy.
     *
     * @param times WriterTimes parameter.
     */
    void update_times(
            const WriterTimes& times);

    /**
     * Update the period of the disable positive ACKs policy.
     *
     * @param att WriterAttributes parameter.
     */
    void update_positive_acks_times(
            const WriterAttributes& att);

    /**
     * Get the next sequence number to be sent.
     *
     * @return Next sequence number.
     */
    SequenceNumber_t next_sequence_number() const;

    /**
     * @brief Sends a periodic heartbeat.
     *
     * @param final       Final flag
     * @param liveliness  Liveliness flag
     *
     * @return True on success.
     */
    bool send_periodic_heartbeat(
            bool final = false,
            bool liveliness = false);

    /**
     * @brief Sends a heartbeat to a remote reader.
     *
     * @param remoteReaderProxy Pointer to the ReaderProxy.
     * @param liveliness        Liveliness flag
     * @param force             Force flag
     *
     * @remarks This function is non thread-safe.
     */
    void send_heartbeat_to_nts(
            ReaderProxy& remoteReaderProxy,
            bool liveliness = false,
            bool force = false);

    void perform_nack_response();

    void perform_nack_supression(
            const GUID_t& reader_guid);

protected:

    void rebuild_status_after_load();

    virtual void print_inconsistent_acknack(
            const GUID_t& writer_guid,
            const GUID_t& reader_guid,
            const SequenceNumber_t& min_requested_sequence_number,
            const SequenceNumber_t& max_requested_sequence_number,
            const SequenceNumber_t& next_sequence_number);

private:

    StatefulWriter& operator =(
            const StatefulWriter&) = delete;

    bool is_acked_by_all_nts(
            const SequenceNumber_t seq) const;

    void update_reader_info(
            LocatorSelectorSender& locator_selector,
            bool create_sender_resources);

    void select_all_readers_nts(
            RTPSMessageGroup& group,
            LocatorSelectorSender& locator_selector);

    void send_heartbeat_piggyback_nts_(
            RTPSMessageGroup& message_group,
            LocatorSelectorSender& locator_selector,
            uint32_t& last_bytes_processed);

    void send_heartbeat_nts_(
            size_t number_of_readers,
            RTPSMessageGroup& message_group,
            bool final,
            bool liveliness = false);

    void check_acked_status();

    /**
     * @brief A method called when the ack timer expires
     *
     * @details Only used if disable positive ACKs QoS is enabled
     */
    bool ack_timer_expired();

    void send_heartbeat_to_all_readers();

    void deliver_sample_to_intraprocesses(
            CacheChange_t* change);

    void deliver_sample_to_datasharing(
            CacheChange_t* change);

    DeliveryRetCode deliver_sample_to_network(
            CacheChange_t* change,
            RTPSMessageGroup& group,
            LocatorSelectorSender& locator_selector,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time);

    void prepare_datasharing_delivery(
            CacheChange_t* change);

    /**
     * Check the StatefulWriter's sequence numbers and add the required GAP messages to the provided message group.
     *
     * @param group     Reference to the Message Group to which the GAP messages are to be added.
     */
    void add_gaps_for_holes_in_history(
            RTPSMessageGroup& group);

    void init(
            RTPSParticipantImpl* pimpl,
            const WriterAttributes& att);

    /// Timed Event to manage the periodic HB to the Reader.
    TimedEvent* periodic_hb_event_;

    /// Timed Event to manage the Acknack response delay.
    TimedEvent* nack_response_event_;

    /// A timed event to mark samples as acknowledget (used only if disable positive ACKs QoS is enabled)
    TimedEvent* ack_event_;

    /// Count of the sent heartbeats.
    Count_t heartbeat_count_;
    /// WriterTimes
    WriterTimes times_;

    /// Vector containing all the remote ReaderProxies.
    ResourceLimitedVector<ReaderProxy*> matched_remote_readers_;
    /// Vector containing all the inactive, ready for reuse, ReaderProxies.
    ResourceLimitedVector<ReaderProxy*> matched_readers_pool_;

    using ReaderProxyIterator = ResourceLimitedVector<ReaderProxy*>::iterator;
    using ReaderProxyConstIterator = ResourceLimitedVector<ReaderProxy*>::const_iterator;

    /// To avoid notifying twice of the same sequence number
    SequenceNumber_t next_all_acked_notify_sequence_;
    SequenceNumber_t min_readers_low_mark_;

    // TODO Join this mutex when main mutex would not be recursive.
    std::mutex all_acked_mutex_;
    std::condition_variable all_acked_cond_;
    // TODO Also remove when main mutex not recursive.
    bool all_acked_;
    std::condition_variable_any may_remove_change_cond_;
    unsigned int may_remove_change_;

    /// True to disable piggyback heartbeats
    bool disable_heartbeat_piggyback_;
    /// True to disable positive ACKs
    bool disable_positive_acks_;
    /// Keep duration for disable positive ACKs QoS
    fastdds::dds::Duration_t keep_duration_;
    /// Last acknowledged cache change (only used if using disable positive ACKs QoS)
    SequenceNumber_t last_sequence_number_;
    /// Biggest sequence number removed from history
    SequenceNumber_t biggest_removed_sequence_number_;

    const uint32_t sendBufferSize_;

    int32_t currentUsageSendBufferSize_;

    bool there_are_remote_readers_ = false;
    bool there_are_local_readers_ = false;

    /// The filter for the reader
    fastdds::rtps::IReaderDataFilter* reader_data_filter_ = nullptr;
    /// Vector containing all the active ReaderProxies for intraprocess delivery.
    ResourceLimitedVector<ReaderProxy*> matched_local_readers_;
    /// Vector containing all the active ReaderProxies for datasharing delivery.
    ResourceLimitedVector<ReaderProxy*> matched_datasharing_readers_;
    bool there_are_datasharing_readers_ = false;

    LocatorSelectorSender locator_selector_general_;

    LocatorSelectorSender locator_selector_async_;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_WRITER__STATEFULWRITER_HPP
