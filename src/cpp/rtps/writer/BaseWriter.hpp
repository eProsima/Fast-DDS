// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file BaseWriter.hpp
 */

#ifndef RTPS_WRITER__BASEWRITER_HPP
#define RTPS_WRITER__BASEWRITER_HPP

#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

#include <fastdds/fastdds_dll.hpp>
#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/core/status/BaseStatus.hpp>
#include <fastdds/rtps/Endpoint.hpp>
#include <fastdds/rtps/common/FragmentNumber.hpp>
#include <fastdds/rtps/common/SequenceNumber.hpp>
#include <fastdds/rtps/common/VendorId_t.hpp>
#include <fastdds/rtps/common/Time_t.hpp>
#include <fastdds/rtps/transport/NetworkBuffer.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>
#include <fastdds/statistics/IListeners.hpp>
#include <fastdds/statistics/rtps/StatisticsCommon.hpp>
#include <fastdds/statistics/rtps/monitor_service/connections_fwd.hpp>
#include <fastdds/utils/TimedMutex.hpp>

#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/writer/DeliveryRetCode.hpp>
#include <rtps/writer/LocatorSelectorSender.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

struct CacheChange_t;
class DataSharingNotifier;
class FlowController;
struct GUID_t;
class ICacheChangePool;
class IPayloadPool;
class RTPSMessageGroup;
class RTPSParticipantImpl;
class WriterAttributes;
class WriterHistory;
class WriterListener;

class BaseWriter
    : public fastdds::rtps::RTPSWriter
    , public fastdds::statistics::StatisticsWriterImpl
{

public:

    //vvvvvvvvvvvvvvvvvvvvv [Exported API] vvvvvvvvvvvvvvvvvvvvv

    bool matched_reader_add(
            const SubscriptionBuiltinTopicData& rqos) final;

    WriterListener* get_listener() const final;

    bool set_listener(
            WriterListener* listener) final;

    bool is_async() const final;

    virtual void local_actions_on_writer_removed();

#ifdef FASTDDS_STATISTICS

    bool add_statistics_listener(
            std::shared_ptr<fastdds::statistics::IListener> listener) final;

    bool remove_statistics_listener(
            std::shared_ptr<fastdds::statistics::IListener> listener) final;

    void set_enabled_statistics_writers_mask(
            uint32_t enabled_writers) final;

#endif // FASTDDS_STATISTICS

    //^^^^^^^^^^^^^^^^^^^^^^^ [Exported API] ^^^^^^^^^^^^^^^^^^^^^^^

    //vvvvvvvvvvvvvvvvvvvv [Implementation API] vvvvvvvvvvvvvvvvvvvv

    /**
     * Add a matched reader.
     * @param data Pointer to the ReaderProxyData object added.
     * @return True if added.
     */
    virtual bool matched_reader_add_edp(
            const ReaderProxyData& data) = 0;

    /**
     * Add a change to the unsent list.
     * @param change Pointer to the change to add.
     * @param [in] max_blocking_time Maximum time this method has to complete the task.
     */
    virtual void unsent_change_added_to_history(
            CacheChange_t* change,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time) = 0;

    /**
     * Indicate the writer that a change has been removed by the history due to some HistoryQos requirement.
     * @param a_change Pointer to the change that is going to be removed.
     * @param [in] max_blocking_time Maximum time this method has to complete the task.
     * @return True if removed correctly.
     */
    virtual bool change_removed_by_history(
            CacheChange_t* a_change,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time) = 0;

    /**
     * Tells writer the sample can be sent to the network.
     * This function should be used by a fastdds::rtps::FlowController.
     *
     * @param cache_change Pointer to the CacheChange_t that represents the sample which can be sent.
     * @param group RTPSMessageGroup reference uses for generating the RTPS message.
     * @param locator_selector RTPSMessageSenderInterface reference uses for selecting locators. The reference has to
     * be a member of this RTPSWriter object.
     * @param max_blocking_time Future timepoint where blocking send should end.
     * @return Return code.
     * @note Must be non-thread safe.
     */
    virtual DeliveryRetCode deliver_sample_nts(
            CacheChange_t* cache_change,
            RTPSMessageGroup& group,
            LocatorSelectorSender& locator_selector,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time) = 0;

    /**
     * @brief Get the general locator selector.
     *
     * @return Reference to the general locator selector.
     */
    virtual LocatorSelectorSender& get_general_locator_selector() = 0;

    /**
     * @brief Get the async locator selector.
     *
     * @return Reference to the async locator selector.
     */
    virtual LocatorSelectorSender& get_async_locator_selector() = 0;

    /**
     * Send a message through this interface.
     *
     * @param buffers Vector of NetworkBuffers to send with data already serialized.
     * @param total_bytes Total number of bytes to send. Should be equal to the sum of the @c size field of all buffers.
     * @param locator_selector RTPSMessageSenderInterface reference uses for selecting locators. The reference has to
     * be a member of this RTPSWriter object.
     * @param max_blocking_time_point Future timepoint where blocking send should end.
     */
    virtual bool send_nts(
            const std::vector<eprosima::fastdds::rtps::NetworkBuffer>& buffers,
            const uint32_t& total_bytes,
            const LocatorSelectorSender& locator_selector,
            std::chrono::steady_clock::time_point& max_blocking_time_point) const;

    /**
     * Process an incoming ACKNACK submessage.
     * @param [in] writer_guid      GUID of the writer the submessage is directed to.
     * @param [in] reader_guid      GUID of the reader originating the submessage.
     * @param [in] ack_count        Count field of the submessage.
     * @param [in] sn_set           Sequence number bitmap field of the submessage.
     * @param [in] final_flag       Final flag field of the submessage.
     * @param [out] result          true if the writer could process the submessage.
     *                             Only valid when returned value is true.
     * @param [in] origin_vendor_id VendorId of the source participant from which the message was received
     * @return true when the submessage was destinated to this writer, false otherwise.
     */
    virtual bool process_acknack(
            const GUID_t& writer_guid,
            const GUID_t& reader_guid,
            uint32_t ack_count,
            const SequenceNumberSet_t& sn_set,
            bool final_flag,
            bool& result,
            fastdds::rtps::VendorId_t origin_vendor_id) = 0;

    /**
     * Process an incoming NACKFRAG submessage.
     * @param [in] writer_guid      GUID of the writer the submessage is directed to.
     * @param [in] reader_guid      GUID of the reader originating the submessage.
     * @param [in] ack_count        Count field of the submessage.
     * @param [in] seq_num          Sequence number field of the submessage.
     * @param [in] fragments_state  Fragment number bitmap field of the submessage.
     * @param [out] result          true if the writer could process the submessage.
     *                             Only valid when returned value is true.
     * @param [in] origin_vendor_id VendorId of the source participant from which the message was received
     * @return true when the submessage was destinated to this writer, false otherwise.
     */
    virtual bool process_nack_frag(
            const GUID_t& writer_guid,
            const GUID_t& reader_guid,
            uint32_t ack_count,
            const SequenceNumber_t& seq_num,
            const FragmentNumberSet_t& fragments_state,
            bool& result,
            fastdds::rtps::VendorId_t origin_vendor_id) = 0;

    /**
     * Tries to remove a change waiting a maximum of the provided microseconds.
     * @param max_blocking_time_point Maximum time to wait for.
     * @param lock Lock of the Change list.
     * @return at least one change has been removed
     */
    virtual bool try_remove_change(
            const std::chrono::steady_clock::time_point& max_blocking_time_point,
            std::unique_lock<RecursiveTimedMutex>& lock) = 0;

    /**
     * Waits till a change has been acknowledged.
     * @param seq Sequence number to wait for acknowledgement.
     * @param max_blocking_time_point Maximum time to wait for.
     * @param lock Lock of the Change list.
     * @return true when change was acknowledged, false when timeout is reached.
     */
    virtual bool wait_for_acknowledgement(
            const SequenceNumber_t& seq,
            const std::chrono::steady_clock::time_point& max_blocking_time_point,
            std::unique_lock<RecursiveTimedMutex>& lock) = 0;

    //^^^^^^^^^^^^^^^^^^^^ [Implementation API] ^^^^^^^^^^^^^^^^^^^^

    /**
     * @brief Get the WriterHistory associated with this writer.
     *
     * @return pointer to the WriterHistory associated with this writer.
     */
    inline WriterHistory* get_history() const
    {
        return history_;
    }

    /**
     * @brief Get biggest output payload size allowed by this writer.
     *
     * @return Maximum number of bytes allowed for the payload.
     */
    uint32_t get_max_allowed_payload_size();

    /**
     * @brief Get the RTPS participant that this writer belongs to.
     *
     * @return pointer to the RTPSParticipantImpl object that created this writer.
     */
    inline RTPSParticipantImpl* get_participant_impl() const
    {
        return mp_RTPSParticipant;
    }

    /**
     * @brief Inform if data is sent to readers separately.
     *
     * @return true if separate sending is enabled
     */
    inline bool get_separate_sending() const
    {
        return separate_sending_enabled_;
    }

    /**
     * @brief A method to retrieve the liveliness kind
     *
     * @return Liveliness kind
     */
    const dds::LivelinessQosPolicyKind& get_liveliness_kind() const;

    /**
     * @brief A method to retrieve the liveliness lease duration
     *
     * @return Lease duration
     */
    const dds::Duration_t& get_liveliness_lease_duration() const;

    /**
     * @brief A method to return the liveliness announcement period
     *
     * @return The announcement period
     */
    const dds::Duration_t& get_liveliness_announcement_period() const;

    /**
     * @brief Notify the writer that it has lost liveliness
     */
    void liveliness_lost();

    /**
     * @return Whether the writer is data sharing compatible or not
     */
    bool is_datasharing_compatible() const;

    bool is_datasharing_compatible_with(
            const dds::DataSharingQosPolicy& qos) const;

    /**
     * Get Min Seq Num in History.
     * @return Minimum sequence number in history
     */
    SequenceNumber_t get_seq_num_min();

    /**
     * Get Max Seq Num in History.
     * @return Maximum sequence number in history
     */
    SequenceNumber_t get_seq_num_max();

    /**
     * @brief Get a pointer to a BaseWriter object from a RTPSWriter pointer.
     *
     * @param writer  Pointer to the RTPSWriter object.
     *
     * @return Pointer to the BaseWriter object.
     */
    static BaseWriter* downcast(
            RTPSWriter* writer);

    /**
     * @brief Get a pointer to a BaseWriter object from a Endpoint pointer.
     *
     * @param endpoint  Pointer to the Endpoint object.
     *
     * @return Pointer to the BaseWriter object.
     */
    static BaseWriter* downcast(
            Endpoint* endpoint);

    virtual ~BaseWriter();

protected:

    BaseWriter(
            RTPSParticipantImpl* impl,
            const GUID_t& guid,
            const WriterAttributes& att,
            FlowController* flow_controller,
            WriterHistory* hist,
            WriterListener* listen = nullptr);

    void init(
            const WriterAttributes& att);

    void add_guid(
            LocatorSelectorSender& locator_selector,
            const GUID_t& remote_guid);

    void compute_selected_guids(
            LocatorSelectorSender& locator_selector);

    void update_cached_info_nts(
            LocatorSelectorSender& locator_selector);

    void add_statistics_sent_submessage(
            CacheChange_t* change,
            size_t num_locators);

    /// Liveliness lost status of this writer
    LivelinessLostStatus liveliness_lost_status_;
    /// Is the data sent directly or announced by HB and THEN sent to the ones who ask for it?.
    bool push_mode_ = true;

    /// Flow controller.
    FlowController* flow_controller_;
    /// Maximum number of bytes allowed for an RTPS datagram generated by this writer.
    uint32_t max_output_message_size_ = std::numeric_limits<uint32_t>::max();

    /// WriterHistory
    WriterHistory* history_ = nullptr;
    /// Listener
    WriterListener* listener_ = nullptr;
    /// Asynchronous publication activated
    bool is_async_ = false;
    /// Separate sending activated
    bool separate_sending_enabled_ = false;

    /// The liveliness kind of this writer
    dds::LivelinessQosPolicyKind liveliness_kind_;
    /// The liveliness lease duration of this writer
    dds::Duration_t liveliness_lease_duration_;
    /// The liveliness announcement period
    dds::Duration_t liveliness_announcement_period_;

private:

    /**
     * @brief Calculate the maximum payload size that can be sent in a single datagram.
     *
     * @param datagram_length Length of the datagram.
     *
     * @return Maximum payload size.
     */
    uint32_t calculate_max_payload_size(
            uint32_t datagram_length);

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif  // RTPS_WRITER__BASEWRITER_HPP
