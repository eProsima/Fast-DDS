// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DataWriterHistory.hpp
 */

#ifndef FASTDDS_PUBLISHER__DATAWRITERHISTORY_HPP
#define FASTDDS_PUBLISHER__DATAWRITERHISTORY_HPP

#include <chrono>
#include <mutex>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/rtps/attributes/ResourceManagement.hpp>
#include <fastdds/rtps/common/InstanceHandle.hpp>
#include <fastdds/rtps/common/Time_t.hpp>
#include <fastdds/rtps/history/IChangePool.hpp>
#include <fastdds/rtps/history/IPayloadPool.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>

#include <fastdds/publisher/history/DataWriterInstance.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * Class DataWriterHistory, implementing a WriterHistory with support for keyed topics and HistoryQOS.
 * This class is created by the PublisherImpl and should not be used by the user directly.
 * @ingroup FASTDDS_MODULE
 */
class DataWriterHistory : public rtps::WriterHistory
{

public:

    static rtps::HistoryAttributes to_history_attributes(
            const HistoryQosPolicy& history_qos,
            const ResourceLimitsQosPolicy& resource_limits_qos,
            const rtps::TopicKind_t& topic_kind,
            uint32_t payloadMaxSize,
            rtps::MemoryManagementPolicy_t mempolicy);

    /**
     * Constructor of the DataWriterHistory.
     *
     * @param payload_pool                 Pool to use for allocation of payloads.
     * @param change_pool                  Pool to use for allocation of changes.
     * @param history_qos                  HistoryQosPolicy of the DataWriter creating this history.
     * @param resource_limits_qos          ResourceLimitsQosPolicy of the DataWriter creating this history.
     * @param topic_kind                   TopicKind of the DataWriter creating this history.
     * @param payloadMax                   Maximum payload size.
     * @param mempolicy                    Set whether the payloads can dynamically resized or not.
     * @param unack_sample_remove_functor  Functor to call DDS listener callback on_unacknowledged_sample_removed
     */
    DataWriterHistory(
            const std::shared_ptr<rtps::IPayloadPool>& payload_pool,
            const std::shared_ptr<rtps::IChangePool>& change_pool,
            const HistoryQosPolicy& history_qos,
            const ResourceLimitsQosPolicy& resource_limits_qos,
            const rtps::TopicKind_t& topic_kind,
            uint32_t payloadMax,
            rtps::MemoryManagementPolicy_t mempolicy,
            std::function<void (const rtps::InstanceHandle_t&)> unack_sample_remove_functor);

    virtual ~DataWriterHistory();

    /**
     * Rebuild instances loaded from DB. Does nothing if the topic doesn't have key.
     */
    void rebuild_instances();

    /*!
     * @brief Tries to reserve resources for the new instance.
     *
     * @param [in]  instance_handle    Instance's key.
     * @param [in]  lock               Lock which should be unlock in case the operation has to wait.
     * @param [in]  max_blocking_time  Maximum time the operation should be waiting.
     * @param [out] payload            Pointer to a serialized payload structure where the serialized payload of the
     *                                 newly allocated instance should be written.
     *
     * @return True if resources were reserved successfully.
     */
    bool register_instance(
            const rtps::InstanceHandle_t& instance_handle,
            std::unique_lock<RecursiveTimedMutex>& lock,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time,
            rtps::SerializedPayload_t*& payload);

    /**
     * This operation can be used to retrieve the serialized payload of the instance key that corresponds to an
     * @ref eprosima::dds::Entity::instance_handle_ "instance_handle".
     *
     * This operation will return @c nullptr if the InstanceHandle_t handle does not correspond to an existing
     * data-object known to the DataWriterHistory.
     *
     * @param [in] handle  Handle to the instance to retrieve the key values from.
     *
     * @return Pointer to the serialized payload of the sample with which the instance was registered.
     */
    rtps::SerializedPayload_t* get_key_value(
            const rtps::InstanceHandle_t& handle);

    /**
     * Add a change comming from the DataWriter.
     * @param change Pointer to the change
     * @param wparams Extra write parameters.
     * @param lock
     * @param max_blocking_time
     * @return True if added.
     */
    bool add_pub_change(
            rtps::CacheChange_t* change,
            rtps::WriteParams& wparams,
            std::unique_lock<RecursiveTimedMutex>& lock,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time);

    /**
     * Add a change comming from the DataWriter.
     *
     * @param change             Pointer to the change
     * @param wparams            Extra writer parameters.
     * @param pre_commit         Functor receiving a CacheChange_t& to perform actions after the
     *                           change has been added to the history, but before notifying the RTPS writer.
     * @param lock               Lock to the history mutex.
     * @param max_blocking_time  Maximum time point to wait for room on the history.
     *
     * @return True if added.
     */
    template<typename PreCommitHook>
    bool add_pub_change_with_commit_hook(
            rtps::CacheChange_t* change,
            rtps::WriteParams& wparams,
            PreCommitHook pre_commit,
            std::unique_lock<RecursiveTimedMutex>& lock,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
    {
        bool returnedValue = false;
        bool add = prepare_change(change, lock, max_blocking_time);

        if (add)
        {
    #if HAVE_STRICT_REALTIME
            if (this->add_change_with_commit_hook(change, wparams, pre_commit, max_blocking_time))
    #else
            auto time_point = std::chrono::steady_clock::now() + std::chrono::hours(24);
            if (this->add_change_with_commit_hook(change, wparams, pre_commit, time_point))
    #endif // if HAVE_STRICT_REALTIME
            {
                EPROSIMA_LOG_INFO(RTPS_HISTORY,
                        " Change " << change->sequenceNumber << " added with key: " << change->instanceHandle
                                   << " and " << change->serializedPayload.length << " bytes");
                returnedValue = true;
            }
        }

        return returnedValue;
    }

    /**
     * Remove all change from the associated history.
     * @param removed Number of elements removed.
     * @return True if all elements were removed.
     */
    bool removeAllChange(
            size_t* removed);

    /**
     * Remove the change with the minimum sequence Number.
     * @return True if removed.
     */
    bool removeMinChange();

    /**
     * Remove a change by the publisher History.
     * @param change Pointer to the CacheChange_t.
     * @return True if removed.
     */
    bool remove_change_pub(
            rtps::CacheChange_t* change);

    /**
     * Remove a change by the publisher History.
     * @param change Pointer to the CacheChange_t.
     * @param [in] max_blocking_time Maximum time this method has to complete the task.
     * @return True if removed.
     */
    bool remove_change_pub(
            rtps::CacheChange_t* change,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time);

    bool remove_change_g(
            rtps::CacheChange_t* a_change) override;

    bool remove_change_g(
            rtps::CacheChange_t* a_change,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time) override;

    bool remove_instance_changes(
            const rtps::InstanceHandle_t& handle,
            const rtps::SequenceNumber_t& seq_up_to);

    /**
     * @brief Sets the next deadline for the given instance
     * @param handle The instance handle
     * @param next_deadline_us The time point when the deadline will occur
     * @return True if deadline was set successfully
     */
    bool set_next_deadline(
            const rtps::InstanceHandle_t& handle,
            const std::chrono::steady_clock::time_point& next_deadline_us);

    /**
     * @brief Returns the deadline for the instance that is next going to 'expire'
     * @param handle The handle for the instance that will next miss the deadline
     * @param next_deadline_us The time point when the deadline will occur
     * @return True if deadline could be retrieved for the given instance
     */
    bool get_next_deadline(
            rtps::InstanceHandle_t& handle,
            std::chrono::steady_clock::time_point& next_deadline_us);

    /*!
     * @brief Checks if the instance's key is registered.
     * @param [in] handle Instance's key.
     * return `true` if instance's key is registered in the history.
     */
    bool is_key_registered(
            const rtps::InstanceHandle_t& handle);

    /**
     * Waits till the last change in the instance history has been acknowledged.
     * @param handle Instance's handle.
     * @param lock Lock which should be unlock in case the operation has to wait.
     * @param max_blocking_time Maximum time the operation should be waiting.
     * @return true when the last change of the instance history is acknowleged, false when timeout is reached.
     */
    bool wait_for_acknowledgement_last_change(
            const rtps::InstanceHandle_t& handle,
            std::unique_lock<RecursiveTimedMutex>& lock,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time);

private:

    typedef std::map<rtps::InstanceHandle_t, detail::DataWriterInstance> t_m_Inst_Caches;

    //!Map where keys are instance handles and values are vectors of cache changes associated
    t_m_Inst_Caches keyed_changes_;
    //!Time point when the next deadline will occur (only used for topics with no key)
    std::chrono::steady_clock::time_point next_deadline_us_;
    //!HistoryQosPolicy values.
    HistoryQosPolicy history_qos_;
    //!ResourceLimitsQosPolicy values.
    ResourceLimitsQosPolicy resource_limited_qos_;
    //!TopicKind
    rtps::TopicKind_t topic_kind_;

    //! Unacknowledged sample removed functor
    std::function<void (const rtps::InstanceHandle_t&)> unacknowledged_sample_removed_functor_;

    /**
     * @brief Method that finds a key in the DataWriterHistory or tries to add it if not found
     * @param [in]  instance_handle  Instance of the key.
     * @param [in]  payload          Serialized payload of the sample for which the instance is being registered.
     * @param [out] map_it           A map iterator to the given key.
     * @return True if the key was found or could be added to the map
     */
    bool find_or_add_key(
            const rtps::InstanceHandle_t& instance_handle,
            const rtps::SerializedPayload_t& payload,
            t_m_Inst_Caches::iterator* map_it);

    /**
     * Add a change comming from the Publisher.
     * @param change Pointer to the change
     * @param lock
     * @param max_blocking_time
     * @return True if added.
     */
    bool prepare_change(
            rtps::CacheChange_t* change,
            std::unique_lock<RecursiveTimedMutex>& lock,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time);

    /**
     * @brief Check if a specific change has been acknowledged or fully delivered if disable positive ACKs QoS is
     *        enabled.
     *
     * @param change CacheChange to check
     * @return true if acknowledged or fully delivered. False otherwise.
     */
    bool change_is_acked_or_fully_delivered(
            const rtps::CacheChange_t* change);

};

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif // FASTDDS_PUBLISHER__DATAWRITERHISTORY_HPP
