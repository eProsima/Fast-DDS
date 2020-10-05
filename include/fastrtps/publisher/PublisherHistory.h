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
 * @file PublisherHistory.h
 *
 */

#ifndef PUBLISHERHISTORY_H_
#define PUBLISHERHISTORY_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/rtps/resources/ResourceManagement.h>

#include <fastdds/rtps/history/WriterHistory.h>
#include <fastrtps/qos/QosPolicies.h>
#include <fastrtps/common/KeyedChanges.h>
#include <fastrtps/attributes/TopicAttributes.h>

namespace eprosima {
namespace fastrtps {

/**
 * Class PublisherHistory, implementing a WriterHistory with support for keyed topics and HistoryQOS.
 * This class is created by the PublisherImpl and should not be used by the user directly.
 * @ingroup FASTRTPS_MODULE
 */
class PublisherHistory : public rtps::WriterHistory
{
public:

    /**
     * Constructor of the PublisherHistory.
     * @param topic_att TopicAttributed
     * @param payloadMax Maximum payload size.
     * @param mempolicy Set wether the payloads ccan dynamically resized or not.
     */
    PublisherHistory(
            const TopicAttributes& topic_att,
            uint32_t payloadMax,
            rtps::MemoryManagementPolicy_t mempolicy);

    virtual ~PublisherHistory();

    /**
     * Rebuild instances loaded from DB. Does nothing if the topic doesn't have key.
     */
    void rebuild_instances();

    /*!
     * @brief Tries to reserve resources for the new instance.
     * @param instance_handle Instance's key.
     * @param lock Lock which should be unlock in case the operation has to wait.
     * @param max_blocking_time Maximum time the operation should be waiting.
     * @return True if resources was reserved successfully.
     */
    bool register_instance(
            const rtps::InstanceHandle_t& instance_handle,
            std::unique_lock<RecursiveTimedMutex>& lock,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time);

    /**
     * Add a change comming from the Publisher.
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

    virtual bool remove_change_g(
            rtps::CacheChange_t* a_change);

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
     * @param[in] handle Instance's key.
     * return `true` if instance's key is registered in the history.
     */
    bool is_key_registered(
            const rtps::InstanceHandle_t& handle);

private:

    typedef std::map<rtps::InstanceHandle_t, KeyedChanges> t_m_Inst_Caches;

    //!Map where keys are instance handles and values are vectors of cache changes associated
    t_m_Inst_Caches keyed_changes_;
    //!Time point when the next deadline will occur (only used for topics with no key)
    std::chrono::steady_clock::time_point next_deadline_us_;
    //!HistoryQosPolicy values.
    HistoryQosPolicy history_qos_;
    //!ResourceLimitsQosPolicy values.
    ResourceLimitsQosPolicy resource_limited_qos_;
    //!Topic Attributes
    TopicAttributes topic_att_;

    /**
     * @brief Method that finds a key in m_keyedChanges or tries to add it if not found
     * @param instance_handle Instance of the key.
     * @param map_it A map iterator to the given key
     * @return True if the key was found or could be added to the map
     */
    bool find_or_add_key(
            const rtps::InstanceHandle_t& instance_handle,
            t_m_Inst_Caches::iterator* map_it);
};

} /* namespace fastrtps */
} /* namespace eprosima */

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif // PUBLISHERHISTORY_H_
