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



namespace eprosima {
namespace fastrtps {

class PublisherImpl;

/**
 * Class PublisherHistory, implementing a WriterHistory with support for keyed topics and HistoryQOS.
 * This class is created by the PublisherImpl and should not be used by the user directly.
 * @ingroup FASTRTPS_MODULE
 */
class PublisherHistory:public rtps::WriterHistory
{
    public:
        /**
         * Constructor of the PublisherHistory.
         * @param pimpl Pointer to the PublisherImpl.
         * @param payloadInitialSize Initial payload size.
         * @param mempolicy Set wether the payloads ccan dynamically resized or not.
         * @param history QOS of the associated History.
         * @param resource ResourceLimits for the History.
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
         * @param wparams
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
         * Returns the latest sample for each topic key
         * @param samples A vector where the the latest sample for each key will be placed. Must be long enough
         * @param num_samples The number of samples in the vector
         */
        void get_latest_samples(
                std::vector<CacheChange_t*> &samples,
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
            {
            }

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

        //!Map where keys are instance handles and values are vectors of cache changes associated
        t_m_Inst_Caches m_keyedChanges;
        //!HistoryQosPolicy values.
        HistoryQosPolicy m_historyQos;
        //!ResourceLimitsQosPolicy values.
        ResourceLimitsQosPolicy m_resourceLimitsQos;
        //!Publisher Pointer
        PublisherImpl* mp_pubImpl;
        //!The latest cache change written (only used for topics with no key)
        CacheChange_t* mp_latestCacheChange;

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
