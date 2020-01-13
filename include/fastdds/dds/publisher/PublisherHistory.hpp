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
 * @file PublisherHistory.hpp
 *
 */

#ifndef PUBLISHERHISTORY_HPP_
#define PUBLISHERHISTORY_HPP_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/rtps/resources/ResourceManagement.h>

#include <fastdds/rtps/history/WriterHistory.h>
#include <fastrtps/qos/QosPolicies.h>
#include <fastrtps/common/KeyedChanges.h>
#include <fastrtps/attributes/TopicAttributes.h>
#include <fastrtps/rtps/common/Types.h>

using namespace eprosima::fastrtps::types;

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * Class PublisherHistory, implementing a WriterHistory with support for keyed topics and HistoryQOS.
 * This class is created by the PublisherImpl and should not be used by the user directly.
 * @ingroup FASTDDS_MODULE
 */
class PublisherHistory : public fastrtps::rtps::WriterHistory
{
public:

    /**
     * Constructor of the PublisherHistory.
     * @param topic_att TopicAttributed
     * @param payloadMax Maximum payload size.
     * @param mempolicy Set wether the payloads ccan dynamically resized or not.
     */
    PublisherHistory(
            const fastrtps::TopicAttributes& topic_att,
            uint32_t payloadMax,
            fastrtps::rtps::MemoryManagementPolicy_t mempolicy);

    virtual ~PublisherHistory();

    /**
     * Add a change comming from the Publisher.
     * @param change Pointer to the change
     * @param wparams Extra write parameters.
     * @param lock
     * @param max_blocking_time
     * @return True if added.
     */
    ReturnCode_t add_pub_change(
            fastrtps::rtps::CacheChange_t* change,
            fastrtps::rtps::WriteParams& wparams,
            std::unique_lock<fastrtps::RecursiveTimedMutex>& lock,
            std::chrono::time_point<std::chrono::steady_clock> max_blocking_time);

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
            fastrtps::rtps::CacheChange_t* change);

    virtual bool remove_change_g(
            fastrtps::rtps::CacheChange_t* a_change);

    /**
     * @brief Sets the next deadline for the given instance
     * @param handle The instance handle
     * @param next_deadline_us The time point when the deadline will occur
     * @return True if deadline was set successfully
     */
    bool set_next_deadline(
            const fastrtps::rtps::InstanceHandle_t& handle,
            const std::chrono::steady_clock::time_point& next_deadline_us);

    /**
     * @brief Returns the deadline for the instance that is next going to 'expire'
     * @param handle The handle for the instance that will next miss the deadline
     * @param next_deadline_us The time point when the deadline will occur
     * @return True if deadline could be retrieved for the given instance
     */
    bool get_next_deadline(
            fastrtps::rtps::InstanceHandle_t& handle,
            std::chrono::steady_clock::time_point& next_deadline_us);

private:

    typedef std::map<fastrtps::rtps::InstanceHandle_t, fastrtps::KeyedChanges> t_m_Inst_Caches;

    //!Map where keys are instance handles and values are vectors of cache changes associated
    t_m_Inst_Caches keyed_changes_;
    //!Time point when the next deadline will occur (only used for topics with no key)
    std::chrono::steady_clock::time_point next_deadline_us_;
    //!HistoryQosPolicy values.
    HistoryQosPolicy history_qos_;
    //!ResourceLimitsQosPolicy values.
    ResourceLimitsQosPolicy resource_limited_qos_;
    //!Topic Attributes
    fastrtps::TopicAttributes topic_att_;

    /**
     * @brief Method that finds a key in m_keyedChanges or tries to add it if not found
     * @param a_change The change to get the key from
     * @param map_it A map iterator to the given key
     * @return True if the key was found or could be added to the map
     */
    bool find_key(
            fastrtps::rtps::CacheChange_t* a_change,
            t_m_Inst_Caches::iterator* map_it);
};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
#endif
#endif /* PUBLISHERHISTORY_HPP_ */
