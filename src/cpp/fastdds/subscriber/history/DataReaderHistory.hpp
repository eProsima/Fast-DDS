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
 * @file DataReaderHistory.hpp
 */

#ifndef _FASTDDS_SUBSCRIBER_HISTORY_DATAREADERHISTORY_HPP_
#define _FASTDDS_SUBSCRIBER_HISTORY_DATAREADERHISTORY_HPP_

#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <utility>

#include <fastcdr/cdr/fixed_size_string.hpp>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>

#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastdds/dds/topic/TopicDescription.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastdds/rtps/attributes/ResourceManagement.hpp>
#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/InstanceHandle.hpp>
#include <fastdds/rtps/common/SequenceNumber.hpp>
#include <fastdds/rtps/history/ReaderHistory.hpp>

#include <fastdds/subscriber/DataReaderImpl/StateFilter.hpp>

#include <fastdds/utils/collections/ResourceLimitedContainerConfig.hpp>

#include "DataReaderHistoryCounters.hpp"
#include "DataReaderInstance.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

/**
 * Class DataReaderHistory, container of the different CacheChanges of a DataReader
 */
class DataReaderHistory : public eprosima::fastdds::rtps::ReaderHistory
{
public:

    using MemoryManagementPolicy_t = eprosima::fastdds::rtps::MemoryManagementPolicy_t;
    using InstanceHandle_t = eprosima::fastdds::rtps::InstanceHandle_t;
    using CacheChange_t = eprosima::fastdds::rtps::CacheChange_t;
    using GUID_t = eprosima::fastdds::rtps::GUID_t;
    using SequenceNumber_t = eprosima::fastdds::rtps::SequenceNumber_t;

    using InstanceCollection = std::map<InstanceHandle_t, std::shared_ptr<DataReaderInstance>>;
    using instance_info = InstanceCollection::iterator;

    /**
     * Constructor.
     * Requires information about the DataReader.
     *
     * @param type  Type information. Needed to know if the type is keyed, as long as the maximum serialized size.
     * @param topic Topic description. Topic and type name are used on debug messages.
     * @param qos   DataReaderQoS policy. History related limits are taken from here.
     */
    DataReaderHistory(
            const TypeSupport& type,
            const TopicDescription& topic,
            const DataReaderQos& qos);

    ~DataReaderHistory() override;

    /**
     * Remove a specific change from the history.
     * No Thread Safe.
     *
     * @param removal iterator to the CacheChange_t to remove.
     * @param release defaults to true and hints if the CacheChange_t should return to the pool
     *
     * @return iterator to the next CacheChange_t or end iterator.
     */
    iterator remove_change_nts(
            const_iterator removal,
            bool release = true) override;

    /**
     * Remove a specific change from the history.
     * No Thread Safe.
     *
     * @param removal iterator to the CacheChange_t to remove.
     * @param [in] max_blocking_time Maximum time this method has to complete the task.
     * @param release defaults to true and hints if the CacheChange_t should return to the pool
     *
     * @return iterator to the next CacheChange_t or end iterator.
     */
    iterator remove_change_nts(
            const_iterator removal,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time,
            bool release = true) override;

    /**
     * Check if a new change can be added to this history.
     *
     * @param [in]  writer_guid                    GUID of the writer where the change came from.
     * @param [in]  total_payload_size             Total payload size of the incoming change.
     * @param [in]  unknown_missing_changes_up_to  The number of changes from the same writer with a lower sequence
     *                                             number that could potentially be received in the future.
     * @param [out] will_never_be_accepted         When the method returns @c false, this parameter will inform
     *                                             whether the change could be accepted in the future or not.
     *
     * @pre change should not be present in the history
     *
     * @return Whether a call to received_change will succeed when called with the same arguments.
     */
    bool can_change_be_added_nts(
            const GUID_t& writer_guid,
            uint32_t total_payload_size,
            size_t unknown_missing_changes_up_to,
            bool& will_never_be_accepted) const override;

    /**
     * Called when a change is received by the RTPS reader.
     * Will add the change to the history.
     *
     * @pre Change should not be already present in the history.
     *
     * @param [in] change The received change
     * @param unknown_missing_changes_up_to Number of missing changes before this one
     *
     * @return Whether the operation succeeded.
     */
    bool received_change(
            CacheChange_t* change,
            size_t unknown_missing_changes_up_to) override;

    /**
     * Called when a change is received by the RTPS reader.
     * Will add the change to the history.
     *
     * @pre Change should not be already present in the history.
     *
     * @param [in] change The received change
     * @param [in] unknown_missing_changes_up_to Number of missing changes before this one
     * @param [out] rejection_reason In case of been rejected the sample, it will contain the reason of the rejection.
     *
     * @return Whether the operation succeeded.
     */
    bool received_change(
            CacheChange_t* change,
            size_t unknown_missing_changes_up_to,
            SampleRejectedStatusKind& rejection_reason) override;

    /**
     * Called when a fragmented change is received completely by the RTPS reader.
     * Will find its instance and store it.
     *
     * @pre Change should be already present in the history.
     *
     * @param [in] change The received change
     *
     * @return Whether the operation succeeded.
     */
    bool completed_change(
            CacheChange_t* change) override;

    /**
     * Called when a fragmented change is received completely by the RTPS reader.
     * Will find its instance and store it.
     *
     * @pre Change should be already present in the history.
     *
     * @param [in] change The received change
     * @param [in] unknown_missing_changes_up_to Number of missing changes before this one
     * @param [out] rejection_reason In case of been rejected the sample, it will contain the reason of the rejection.
     *
     * @return Whether the operation succeeded.
     */
    bool completed_change(
            CacheChange_t* change,
            size_t unknown_missing_changes_up_to,
            SampleRejectedStatusKind& rejection_reason) override;

    /**
     * @brief Returns information about the first untaken sample.
     *
     * @param [out] info SampleInfo structure to store first untaken sample information.
     *
     * @return true if sample info was returned, false if there is no sample to take.
     */
    bool get_first_untaken_info(
            SampleInfo& info);

    /**
     * This method is called to remove a change from the DataReaderHistory.
     *
     * @param change Pointer to the CacheChange_t.
     *
     * @return True if removed.
     */
    bool remove_change_sub(
            CacheChange_t* change);

    /**
     * This method is called to remove a change from the DataReaderHistory.
     *
     * @param [in]     change Pointer to the CacheChange_t.
     * @param [in,out] it     Iterator pointing to change on input. Will point to next valid change on output.
     *
     * @return True if removed.
     */
    bool remove_change_sub(
            CacheChange_t* change,
            DataReaderInstance::ChangeCollection::iterator& it);

    /**
     * Called when a writer is unmatched from the reader holding this history.
     *
     * This method will remove all the changes on the history that came from the writer being unmatched and which have
     * not yet been notified to the user.
     *
     * @param writer_guid        GUID of the writer being unmatched.
     * @param last_notified_seq  Last sequence number from the specified writer that was notified to the user.
     */
    void writer_unmatched(
            const GUID_t& writer_guid,
            const SequenceNumber_t& last_notified_seq) override;

    /**
     * @brief A method to set the next deadline for the given instance.
     *
     * @param handle The handle to the instance
     * @param next_deadline_us The time point when the deadline will occur
     * @param [in] deadline_missed true value when is called because the deadline was missed.
     *
     * @return True if the deadline was set correctly
     */
    bool set_next_deadline(
            const InstanceHandle_t& handle,
            const std::chrono::steady_clock::time_point& next_deadline_us,
            bool deadline_missed = false);

    /**
     * @brief A method to get the next instance handle that will miss the deadline and the time when the deadline will occur.
     *
     * @param handle The handle to the instance
     * @param next_deadline_us The time point when the instance will miss the deadline
     *
     * @return True if the deadline was retrieved successfully
     */
    bool get_next_deadline(
            InstanceHandle_t& handle,
            std::chrono::steady_clock::time_point& next_deadline_us);

    /**
     * Get the number of samples pending to be read.
     *
     * @param mark_as_read  Whether the unread samples should be marked as read or not.
     *
     * @return the number of samples on the reader history that have never been read.
     */
    uint64_t get_unread_count(
            bool mark_as_read);

    /**
     * @brief Check whether an instance handle is present in the history.
     *
     * @param handle The handle of the instance to check.
     *
     * @return true when the topic has keys and the handle corresponds to an instance present in the history.
     * @return false otherwise.
     */
    bool is_instance_present(
            const InstanceHandle_t& handle) const;

    /**
     * @brief Get an iterator to an instance with available data.
     *
     * @param handle The handle to the instance.
     * @param exact  Indicates if the handle should match exactly (true) or if the first instance greater than the
     *               input handle should be returned.
     *
     * @return A pair where:
     *         - @c first is a boolean indicating if an instance was found
     *         - @c second is an iterator to the data available instances collection
     *
     * @remarks When used on a NO_KEY topic, an instance will only be returned when called with
     *          `handle = HANDLE_NIL` and `exact = false`.
     */
    std::pair<bool, instance_info> lookup_available_instance(
            const InstanceHandle_t& handle,
            bool exact);

    /**
     * @brief Given an instance advance the iterator to the next instance with available data.
     *
     * @param handle        The handle of the instance returned by a previous call to lookup_available_instance or
     *                      next_available_instance_nts.
     * @param current_info  The iterator to be advanced.
     *
     * @return A pair where:
     *         - @c first is a boolean indicating if another instance with available data is present
     *         - @c second is an iterator pointing to the next instance with available data
     */
    std::pair<bool, instance_info> next_available_instance_nts(
            const InstanceHandle_t& handle,
            const instance_info& current_info);

    /**
     * This method is meant to be called just before calling @c end_sample_access_nts on the RTPS reader.
     * It will update the internal counters of unread and read samples.
     *
     * @param change                       Pointer to the cache change that has been processed.
     * @param is_going_to_be_mark_as_read  Whether the change is going to be marked as read.
     */
    void change_was_processed_nts(
            CacheChange_t* const change,
            bool is_going_to_be_mark_as_read);

    /**
     * Mark that a DataReaderInstance has been viewed.
     *
     * @param instance        Instance on which the view state should be modified.
     */
    void instance_viewed_nts(
            const InstanceCollection::mapped_type& instance);

    /*!
     * @brief Updates instance's information and also decides whether the sample is finally accepted or denied depending
     * on the Ownership strength.
     *
     * @param [in] change Sample received by DataReader.
     * @return true is returned when the sample is accepted and false when the sample is denied.
     */
    bool update_instance_nts(
            CacheChange_t* const change);

    void writer_not_alive(
            const fastdds::rtps::GUID_t& writer_guid);

    void check_and_remove_instance(
            instance_info& instance_info);

    StateFilter get_mask_status() const noexcept;

    /*!
     * @brief This function should be called by reader if a writer updates its ownership strength.
     *
     * @param [in] writer_guid Guid of the writer which changes its ownership strength.
     * @param [out] ownership_strength New value of the writer's Ownership strength.
     */
    void writer_update_its_ownership_strength_nts(
            const GUID_t& writer_guid,
            const uint32_t ownership_strength) override;

private:

    //!Resource limits for allocating the array of changes per instance
    eprosima::fastdds::ResourceLimitedContainerConfig key_changes_allocation_;
    //!Resource limits for allocating the array of alive writers per instance
    eprosima::fastdds::ResourceLimitedContainerConfig key_writers_allocation_;
    //!Collection of DataReaderInstance objects accessible by their handle
    InstanceCollection instances_;
    //!Collection of DataReaderInstance objects with available data, accessible by their handle
    InstanceCollection data_available_instances_;
    //!HistoryQosPolicy values.
    HistoryQosPolicy history_qos_;
    //!ResourceLimitsQosPolicy values.
    ResourceLimitsQosPolicy resource_limited_qos_;
    //!Topic name
    fastcdr::string_255 topic_name_;
    //!Type name
    fastcdr::string_255 type_name_;
    //!Whether the type has keys
    bool has_keys_;
    //!TopicDataType
    fastdds::dds::TopicDataType* type_;

    /// Function to compute the instance handle of a received change
    std::function<bool(CacheChange_t*)> compute_key_for_change_fn_;
    /// Function processing a received change
    std::function<bool(CacheChange_t*, size_t, SampleRejectedStatusKind&)> receive_fn_;
    /// Function processing a completed fragmented change
    std::function<bool(CacheChange_t*, DataReaderInstance&, size_t, SampleRejectedStatusKind&)> complete_fn_;

    /// Book-keeping counters for ReadCondition support
    DataReaderHistoryCounters counters_;

    /**
     * @brief Method that finds a key in m_keyedChanges or tries to add it if not found
     * @param a_change The change to get the key from
     * @param map_it A map iterator to the given key
     * @return True if it was found or could be added to the map
     */
    bool find_key(
            const InstanceHandle_t& handle,
            InstanceCollection::iterator& map_it);

    /**
     * @name Variants of incoming change processing.
     *       Will be called with the history mutex taken.
     * @param [in] change The received change
     * @param unknown_missing_changes_up_to Number of missing changes before this one
     * @return
     */
    ///@{
    bool received_change_keep_all(
            CacheChange_t* change,
            size_t unknown_missing_changes_up_to,
            SampleRejectedStatusKind& rejection_reason);

    bool received_change_keep_last(
            CacheChange_t* change,
            size_t unknown_missing_changes_up_to,
            SampleRejectedStatusKind& rejection_reason);
    ///@}

    /**
     * @name Variants of change reconstruction completion processing.
     *       Will be called with the history mutex taken.
     * @param change The change for which the last missing fragment has been processed.
     * @param instance Instance where the change should be added.
     * @return true when the change was added to the instance.
     * @return false when the change could not be added to the instance and has been removed from the history.
     */
    ///@{
    bool completed_change_keep_all(
            CacheChange_t* change,
            DataReaderInstance& instance,
            size_t unknown_missing_changes_up_to,
            SampleRejectedStatusKind&);

    bool completed_change_keep_last(
            CacheChange_t* change,
            DataReaderInstance& instance,
            size_t unknown_missing_changes_up_to,
            SampleRejectedStatusKind&);
    ///@}

    bool add_received_change_with_key(
            CacheChange_t* a_change,
            DataReaderInstance& instance,
            SampleRejectedStatusKind& rejection_reason);

    bool add_to_reader_history_if_not_full(
            CacheChange_t* a_change,
            SampleRejectedStatusKind& rejection_reason);

    void add_to_instance(
            CacheChange_t* a_change,
            DataReaderInstance& instance);

};

} // namespace detail
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif  // _FASTDDS_SUBSCRIBER_HISTORY_DATAREADERHISTORY_HPP_
