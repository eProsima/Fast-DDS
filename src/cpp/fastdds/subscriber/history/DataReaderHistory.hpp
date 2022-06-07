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
#include <utility>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>

#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastdds/dds/topic/TopicDescription.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastdds/rtps/common/CacheChange.h>
#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/common/InstanceHandle.h>
#include <fastdds/rtps/common/SequenceNumber.h>
#include <fastdds/rtps/history/ReaderHistory.h>
#include <fastdds/rtps/resources/ResourceManagement.h>

#include <fastrtps/utils/fixed_size_string.hpp>
#include <fastrtps/utils/collections/ResourceLimitedContainerConfig.hpp>

#include "DataReaderInstance.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

/**
 * Class DataReaderHistory, container of the different CacheChanges of a DataReader
 */
class DataReaderHistory : public eprosima::fastrtps::rtps::ReaderHistory
{
public:

    using MemoryManagementPolicy_t = eprosima::fastrtps::rtps::MemoryManagementPolicy_t;
    using InstanceHandle_t = eprosima::fastrtps::rtps::InstanceHandle_t;
    using CacheChange_t = eprosima::fastrtps::rtps::CacheChange_t;
    using GUID_t = eprosima::fastrtps::rtps::GUID_t;
    using SequenceNumber_t = eprosima::fastrtps::rtps::SequenceNumber_t;

    using instance_info = std::pair<InstanceHandle_t, DataReaderInstance*>;

    /**
     * Constructor. Requires information about the DataReader.
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
     * No Thread Safe
     * @param removal iterator to the CacheChange_t to remove.
     * @param release defaults to true and hints if the CacheChange_t should return to the pool
     * @return iterator to the next CacheChange_t or end iterator.
     */
    iterator remove_change_nts(
            const_iterator removal,
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
     * Called when a change is received by the Subscriber. Will add the change to the history.
     * @pre Change should not be already present in the history.
     * @param[in] change The received change
     * @param unknown_missing_changes_up_to Number of missing changes before this one
     * @return
     */
    bool received_change(
            CacheChange_t* change,
            size_t unknown_missing_changes_up_to) override;

    /*!
     * Called when a change is received by the Subscriber. Will add the change to the history.
     * @pre Change should not be already present in the history.
     * @param[in] change The received change
     * @param[in] unknown_missing_changes_up_to Number of missing changes before this one
     * @param[out] rejection_reason In case of been rejected the sample, it will contain the reason of the rejection.
     * @return
     */
    bool received_change(
            CacheChange_t* change,
            size_t unknown_missing_changes_up_to,
            SampleRejectedStatusKind& rejection_reason) override;

    /**
     * Called when a fragmented change is received completely by the Subscriber. Will find its instance and store it.
     * @pre Change should be already present in the history.
     * @param[in] change The received change
     * @param[in] unknown_missing_changes_up_to Number of missing changes before this one
     * @return
     */
    bool completed_change(
            CacheChange_t* change) override;

    /*!
     * Called when a fragmented change is received completely by the Subscriber. Will find its instance and store it.
     * @pre Change should be already present in the history.
     * @param[in] change The received change
     * @param[in] unknown_missing_changes_up_to Number of missing changes before this one
     * @param[out] rejection_reason In case of been rejected the sample, it will contain the reason of the rejection.
     * @return
     */
    bool completed_change(
            CacheChange_t* change,
            size_t unknown_missing_changes_up_to,
            SampleRejectedStatusKind& rejection_reason) override;

    /**
     * @brief Returns information about the first untaken sample.
     * @param [out] info SampleInfo structure to store first untaken sample information.
     * @return true if sample info was returned. false if there is no sample to take.
     */
    bool get_first_untaken_info(
            SampleInfo& info);

    /**
     * This method is called to remove a change from the SubscriberHistory.
     * @param change Pointer to the CacheChange_t.
     * @return True if removed.
     */
    bool remove_change_sub(
            CacheChange_t* change);

    /**
     * This method is called to remove a change from the SubscriberHistory.
     * @param [in]     change Pointer to the CacheChange_t.
     * @param [in,out] it     Iterator pointing to change on input. Will point to next valid change on output.
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
     * @brief A method to set the next deadline for the given instance
     * @param handle The handle to the instance
     * @param next_deadline_us The time point when the deadline will occur
     * @return True if the deadline was set correctly
     */
    bool set_next_deadline(
            const InstanceHandle_t& handle,
            const std::chrono::steady_clock::time_point& next_deadline_us);

    /**
     * @brief A method to get the next instance handle that will miss the deadline and the time when the deadline will occur
     * @param handle The handle to the instance
     * @param next_deadline_us The time point when the instance will miss the deadline
     * @return True if the deadline was retrieved successfully
     */
    bool get_next_deadline(
            InstanceHandle_t& handle,
            std::chrono::steady_clock::time_point& next_deadline_us);

    /**
     * @brief Get the list of changes corresponding to an instance handle.
     * @param handle The handle to the instance.
     * @param exact  Indicates if the handle should match exactly (true) or if the first instance greater than the
     *               input handle should be returned.
     * @return A pair where:
     *         - @c first is a boolean indicating if an instance was found
     *         - @c second is a pair where:
     *           - @c first is the handle of the returned instance
     *           - @c second is a pointer to a DataReaderInstance that holds information about the returned instance
     *
     * @remarks When used on a NO_KEY topic, an instance will only be returned when called with
     *          `handle = HANDLE_NIL` and `exact = false`.
     */
    std::pair<bool, instance_info> lookup_instance(
            const InstanceHandle_t& handle,
            bool exact) const;

    void update_instance_nts(
            CacheChange_t* const change);

    void writer_not_alive(
            const fastrtps::rtps::GUID_t& writer_guid);

    void check_and_remove_instance(
            instance_info& instance_info);

private:

    using InstanceCollection = std::map<InstanceHandle_t, DataReaderInstance>;

    //!Resource limits for allocating the array of changes per instance
    eprosima::fastrtps::ResourceLimitedContainerConfig key_changes_allocation_;
    //!Resource limits for allocating the array of alive writers per instance
    eprosima::fastrtps::ResourceLimitedContainerConfig key_writers_allocation_;
    //!Map where keys are instance handles and values vectors of cache changes
    InstanceCollection keyed_changes_;
    //!HistoryQosPolicy values.
    HistoryQosPolicy history_qos_;
    //!ResourceLimitsQosPolicy values.
    ResourceLimitsQosPolicy resource_limited_qos_;
    //!Topic name
    fastrtps::string_255 topic_name_;
    //!Type name
    fastrtps::string_255 type_name_;
    //!Whether the type has keys
    bool has_keys_;
    //!TopicDataType
    fastdds::dds::TopicDataType* type_;

    //!Type object to deserialize Key
    void* get_key_object_;

    /// Function to compute the instance handle of a received change
    std::function<bool(CacheChange_t*)> compute_key_for_change_fn_;
    /// Function processing a received change
    std::function<bool(CacheChange_t*, size_t, SampleRejectedStatusKind&)> receive_fn_;
    /// Function processing a completed fragmented change
    std::function<bool(CacheChange_t*, DataReaderInstance&, size_t, SampleRejectedStatusKind&)> complete_fn_;

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
     * @param[in] change The received change
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
