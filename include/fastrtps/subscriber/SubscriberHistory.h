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
         * Constructor. Requires information about the subscriner
         * @param pimpl Pointer to the subscriber implementation
         * @param payloadMax Maximum payload size per change
         * @param history History QoS policy for the reader
         * @param resource Resource Limit QoS policy for the reader
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

        bool readNextBuffer(SerializedPayload_t* data, SampleInfo_t* info);
        bool takeNextBuffer(SerializedPayload_t* data, SampleInfo_t* info);


        /**
         * This method is called to remove a change from the SubscriberHistory.
         * @param change Pointer to the CacheChange_t.
         * @return True if removed.
         */
        bool remove_change_sub(rtps::CacheChange_t* change);

        //!Increase the unread count.
        inline void increaseUnreadCount()
        {
            ++m_unreadCacheCount;
        }

        //!Decrease the unread count.
        inline void decreaseUnreadCount()
        {
            if(m_unreadCacheCount>0)
                --m_unreadCacheCount;
        }

        /** Get the unread count.
         * @return Unread count
         */
        inline uint64_t getUnreadCount() const
        {
            return m_unreadCacheCount;
        }

        /**
         * A method that resturns the latest sample for each topic key
         * @param samples A vector where the latest sample for each key will be placed. Must be long enough
         * @param num_samples The number of samples in the vector
         */
        void get_latest_samples(
                std::vector<CacheChange_t*>& samples,
                int& num_samples);

    private:

        /**
         * @brief A struct storing a vector of cache changes and the latest change in the group
         * @ingroup FASTRTPS_MODULE
         */
        struct KeyedChanges
        {
            //! Default constructor
            KeyedChanges()
                : cache_changes_()
                , latest_change_(new CacheChange_t)
            {}

            //! Copy constructor
            KeyedChanges(const KeyedChanges& other)
                : cache_changes_(other.cache_changes_)
                , latest_change_(new CacheChange_t)
            {
                latest_change_->copy(other.latest_change_);
            }

            //! Destructor
            ~KeyedChanges()
            {
                delete latest_change_;
            }

            //! A vector of cache changes
            std::vector<CacheChange_t*> cache_changes_;
            //! The latest cache change in the struct
            CacheChange_t* latest_change_;
        };

        typedef std::map<rtps::InstanceHandle_t, KeyedChanges> t_m_Inst_Caches;
        typedef std::vector<rtps::CacheChange_t*> t_v_Caches;

        //!Number of unread CacheChange_t.
        uint64_t m_unreadCacheCount;
        //!Map where keys are instance handles and values vectors of cache changes
        t_m_Inst_Caches m_keyedChanges;
        //!HistoryQosPolicy values.
        HistoryQosPolicy m_historyQos;
        //!ResourceLimitsQosPolicy values.
        ResourceLimitsQosPolicy m_resourceLimitsQos;
        //!Publisher Pointer
        SubscriberImpl* mp_subImpl;
        //!The latest cache change received (only used for topics with no key)
        CacheChange_t* mp_latestCacheChange;

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
};

} /* namespace fastrtps */
} /* namespace eprosima */
#endif
#endif /* SUBSCRIBERHISTORY_H_ */
