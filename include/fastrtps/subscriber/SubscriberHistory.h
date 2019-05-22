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
 * @file SubscriberHistory.h
 *
 */

#ifndef SUBSCRIBERHISTORY_H_
#define SUBSCRIBERHISTORY_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastrtps/rtps/resources/ResourceManagement.h>
#include "../rtps/history/ReaderHistory.h"
#include "../qos/QosPolicies.h"
#include "../common/KeyedChanges.h"
#include "SampleInfo.h"

namespace eprosima {
namespace fastrtps {

namespace rtps{
class WriterProxy;
}

class SubscriberImpl;

/**
 * Class SubscriberHistory, container of the different CacheChanges of a subscriber
 *  @ingroup FASTRTPS_MODULE
 */
class SubscriberHistory: public rtps::ReaderHistory
{
    public:

        /**
         * Constructor. Requires information about the subscriber.
         * @param pimpl Pointer to the subscriber implementation.
         * @param payloadMax Maximum payload size per change.
         * @param history History QoS policy for the reader.
         * @param resource Resource Limit QoS policy for the reader.
         * @param mempolicy Set wether the payloads ccan dynamically resized or not.
         */
        SubscriberHistory(
            SubscriberImpl* pimpl,
            uint32_t payloadMax,
            const HistoryQosPolicy& history,
            const ResourceLimitsQosPolicy& resource,
            rtps::MemoryManagementPolicy_t mempolicy);

        virtual ~SubscriberHistory();

        /**
         * Called when a change is received by the Subscriber History. Will add the change to the history
         * if it wasn't already present
         * @param[in] change The received change
         * @param unknown_missing_changes_up_to Number of missing changes before this one
         * @return
         */
        bool received_change(
            rtps::CacheChange_t* change,
            size_t unknown_missing_changes_up_to);

        /** @name Read or take data methods.
         * Methods to read or take data from the History.
         * @param data Pointer to the object where you want to read or take the information.
         * @param info Pointer to a SampleInfo_t object where you want
         * to store the information about the retrieved data
         */
        ///@{
        bool readNextData(void* data, SampleInfo_t* info);
        bool takeNextData(void* data, SampleInfo_t* info);
        ///@}

        bool readNextBuffer(rtps::SerializedPayload_t* data, SampleInfo_t* info);
        bool takeNextBuffer(rtps::SerializedPayload_t* data, SampleInfo_t* info);


        /**
         * This method is called to remove a change from the SubscriberHistory.
         * @param change Pointer to the CacheChange_t.
         * @return True if removed.
         */
        bool remove_change_sub(rtps::CacheChange_t* change);

        /** Get the unread count.
         * @return Unread count
         */
        inline uint64_t getUnreadCount() const
        {
            std::lock_guard<std::recursive_timed_mutex> guard(*mp_mutex);
            return m_unreadCacheCount;
        }

        /**
         * @brief A method to set the next deadline for the given instance
         * @param handle The handle to the instance
         * @param next_deadline_us The time point when the deadline will occur
         * @return True if the deadline was set correctly
         */
        bool set_next_deadline(
                const rtps::InstanceHandle_t& handle,
                const std::chrono::steady_clock::time_point& next_deadline_us);

        /**
         * @brief A method to get the next instance handle that will miss the deadline and the time when the deadline will occur
         * @param handle The handle to the instance
         * @param next_deadline_us The time point when the instance will miss the deadline
         * @return True if the deadline was retrieved successfully
         */
        bool get_next_deadline(
                rtps::InstanceHandle_t& handle,
                std::chrono::steady_clock::time_point& next_deadline_us);

    private:

        typedef std::map<rtps::InstanceHandle_t, KeyedChanges> t_m_Inst_Caches;

        //!Number of unread CacheChange_t.
        uint64_t m_unreadCacheCount;
        //!Map where keys are instance handles and values vectors of cache changes
        t_m_Inst_Caches keyed_changes_;
        //!Time point when the next deadline will occur (only used for topics with no key)
        std::chrono::steady_clock::time_point next_deadline_us_;
        //!HistoryQosPolicy values.
        HistoryQosPolicy m_historyQos;
        //!ResourceLimitsQosPolicy values.
        ResourceLimitsQosPolicy m_resourceLimitsQos;
        //!Publisher Pointer
        SubscriberImpl* mp_subImpl;

        //!Type object to deserialize Key
        void * mp_getKeyObject;

        /**
         * @brief Method that finds a key in m_keyedChanges or tries to add it if not found
         * @param a_change The change to get the key from
         * @param map_it A map iterator to the given key
         * @return True if it was found or could be added to the map
         */
        bool find_key(
                rtps::CacheChange_t* a_change,
                t_m_Inst_Caches::iterator* map_it);

        //!Increase the unread count.
        inline void increaseUnreadCount()
        {
            ++m_unreadCacheCount;
        }

        //!Decrease the unread count.
        inline void decreaseUnreadCount()
        {
            if (m_unreadCacheCount > 0)
                --m_unreadCacheCount;
        }
};

} /* namespace fastrtps */
} /* namespace eprosima */
#endif
#endif /* SUBSCRIBERHISTORY_H_ */
