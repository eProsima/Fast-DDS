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
 * @file StatelessWriter.hpp
 */
#ifndef FASTDDS_RTPS_WRITER__STATELESSWRITER_HPP
#define FASTDDS_RTPS_WRITER__STATELESSWRITER_HPP

#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>

#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/common/Time_t.hpp>
#include <fastdds/rtps/history/IChangePool.hpp>
#include <fastdds/rtps/history/IPayloadPool.hpp>
#include <fastdds/rtps/interfaces/IReaderDataFilter.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>
#include <fastdds/utils/collections/ResourceLimitedVector.hpp>

#include <rtps/writer/BaseWriter.hpp>
#include <rtps/writer/ChangeForReader.hpp>
#include <rtps/writer/ReaderLocator.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Class StatelessWriter, specialization of BaseWriter that manages writers that don't keep state of the matched readers.
 * @ingroup WRITER_MODULE
 */
class StatelessWriter : public BaseWriter
{

public:

    StatelessWriter(
            RTPSParticipantImpl* participant,
            const GUID_t& guid,
            const WriterAttributes& attributes,
            FlowController* flow_controller,
            WriterHistory* history,
            WriterListener* listener = nullptr);

    virtual ~StatelessWriter();

    void local_actions_on_writer_removed() override;

    //vvvvvvvvvvvvvvvvvvvvv [Exported API] vvvvvvvvvvvvvvvvvvvvv

    bool matched_reader_add_edp(
            const ReaderProxyData& data) override;

    bool matched_reader_remove(
            const GUID_t& reader_guid) override;

    bool matched_reader_is_matched(
            const GUID_t& reader_guid) final;

    void reader_data_filter(
            IReaderDataFilter* filter) final
    {
        reader_data_filter_ = filter;
    }

    const IReaderDataFilter* reader_data_filter() const final
    {
        return reader_data_filter_;
    }

    bool has_been_fully_delivered(
            const SequenceNumber_t& seq_num) const final;

    bool is_acked_by_all(
            const SequenceNumber_t& seq_num) const final;

    bool wait_for_all_acked(
            const dds::Duration_t& max_wait) final;

    void update_attributes(
            const WriterAttributes& att) final
    {
        static_cast<void>(att);
        //FOR NOW THERE IS NOTHING TO UPDATE.
    }

    bool get_disable_positive_acks() const final;

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
#endif // FASTDDS_STATISTICS

    //^^^^^^^^^^^^^^^^^^^^^^ [Exported API] ^^^^^^^^^^^^^^^^^^^^^^^

    //vvvvvvvvvvvvvvvvvvvvv [BaseWriter API] vvvvvvvvvvvvvvvvvvvvvv

    void unsent_change_added_to_history(
            CacheChange_t* change,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time) override;

    bool change_removed_by_history(
            CacheChange_t* change,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time) override;

    DeliveryRetCode deliver_sample_nts(
            CacheChange_t* cache_change,
            RTPSMessageGroup& group,
            LocatorSelectorSender& locator_selector,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time) final;

    LocatorSelectorSender& get_general_locator_selector() final
    {
        return locator_selector_;
    }

    LocatorSelectorSender& get_async_locator_selector() final
    {
        return locator_selector_;
    }

    bool send_nts(
            const std::vector<NetworkBuffer>& buffers,
            const uint32_t& total_bytes,
            const LocatorSelectorSender& locator_selector,
            std::chrono::steady_clock::time_point& max_blocking_time_point) const final;

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
            const std::chrono::steady_clock::time_point&,
            std::unique_lock<RecursiveTimedMutex>&) final;

    bool wait_for_acknowledgement(
            const SequenceNumber_t& seq,
            const std::chrono::steady_clock::time_point& max_blocking_time_point,
            std::unique_lock<RecursiveTimedMutex>& lock) final;

    //^^^^^^^^^^^^^^^^^^^^^^ [BaseWriter API] ^^^^^^^^^^^^^^^^^^^^^^^

    /**
     * Get the number of matched readers
     * @return Number of the matched readers
     */
    inline size_t get_matched_readers_size() const
    {
        std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
        return matched_remote_readers_.size()
               + matched_local_readers_.size()
               + matched_datasharing_readers_.size();
    }

protected:

    mutable LocatorList_t fixed_locators_;

    virtual bool send_to_fixed_locators(
            const std::vector<eprosima::fastdds::rtps::NetworkBuffer>& buffers,
            const uint32_t& total_bytes,
            std::chrono::steady_clock::time_point& max_blocking_time_point) const;

private:

    void init(
            RTPSParticipantImpl* participant,
            const WriterAttributes& attributes);

    void get_builtin_guid();

    bool has_builtin_guid();

    void update_reader_info(
            bool create_sender_resources);

    bool datasharing_delivery(
            CacheChange_t* change);

    bool intraprocess_delivery(
            CacheChange_t* change,
            ReaderLocator& reader_locator);

    bool is_inline_qos_expected_ = false;
    ResourceLimitedVector<std::unique_ptr<ReaderLocator>> matched_remote_readers_;

    std::condition_variable_any unsent_changes_cond_;

    uint64_t current_sequence_number_sent_ = 0;

    FragmentNumber_t current_fragment_sent_ = 0;

    uint64_t last_sequence_number_sent_ = 0;

    ResourceLimitedVector<std::unique_ptr<ReaderLocator>> matched_local_readers_;
    ResourceLimitedVector<std::unique_ptr<ReaderLocator>> matched_datasharing_readers_;
    ResourceLimitedVector<std::unique_ptr<ReaderLocator>> matched_readers_pool_;

    LocatorSelectorSender locator_selector_;

    IReaderDataFilter* reader_data_filter_ = nullptr;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_WRITER__STATELESSWRITER_HPP

