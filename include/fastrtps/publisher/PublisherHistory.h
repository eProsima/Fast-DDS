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

#include <fastrtps/rtps/resources/ResourceManagement.h>

#include "../rtps/history/WriterHistory.h"
#include "../qos/QosPolicies.h"
#include "../common/KeyedChanges.h"

namespace eprosima {
namespace fastrtps {

class PublisherImpl;

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
         * @param pimpl Pointer to the PublisherImpl.
         * @param payloadMax Maximum payload size.
         * @param history QOS of the associated History.
         * @param resource ResourceLimits for the History.
         * @param mempolicy Set wether the payloads ccan dynamically resized or not.
         */
        PublisherHistory(
            PublisherImpl* pimpl,
            uint32_t payloadMax,
            const HistoryQosPolicy& history,
            const ResourceLimitsQosPolicy& resource,
            rtps::MemoryManagementPolicy_t mempolicy);

        virtual ~PublisherHistory();

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
                rtps::WriteParams &wparams,
                std::unique_lock<std::recursive_timed_mutex>& lock,
                std::chrono::time_point<std::chrono::steady_clock> max_blocking_time);

        /**
         * Remove all change from the associated history.
         * @param removed Number of elements removed.
         * @return True if all elements were removed.
         */
        bool removeAllChange(size_t* removed);

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
        bool remove_change_pub(rtps::CacheChange_t* change);

        virtual bool remove_change_g(rtps::CacheChange_t* a_change);

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

private:

        typedef std::map<rtps::InstanceHandle_t, KeyedChanges> t_m_Inst_Caches;

        //!Map where keys are instance handles and values are vectors of cache changes associated
        t_m_Inst_Caches keyed_changes_;
        //!Time point when the next deadline will occur (only used for topics with no key)
        std::chrono::steady_clock::time_point next_deadline_us_;
        //!HistoryQosPolicy values.
        HistoryQosPolicy m_historyQos;
        //!ResourceLimitsQosPolicy values.
        ResourceLimitsQosPolicy m_resourceLimitsQos;
        //!Publisher Pointer
        PublisherImpl* mp_pubImpl;

        /**
         * @brief Method that finds a key in m_keyedChanges or tries to add it if not found
         * @param a_change The change to get the key from
         * @param map_it A map iterator to the given key
         * @return True if the key was found or could be added to the map
         */
        bool find_key(
                rtps::CacheChange_t* a_change,
                t_m_Inst_Caches::iterator* map_it);
};

} /* namespace fastrtps */
} /* namespace eprosima */
#endif
#endif /* PUBLISHERHISTORY_H_ */
