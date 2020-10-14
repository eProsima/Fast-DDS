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
 * @file History.h
 *
 */

#ifndef _FASTDDS_RTPS_HISTORY_H_
#define _FASTDDS_RTPS_HISTORY_H_

#include <fastrtps/fastrtps_dll.h>

#include <fastdds/rtps/history/IChangePool.h>
#include <fastdds/rtps/history/IPayloadPool.h>

#include <fastdds/rtps/common/SequenceNumber.h>
#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/attributes/HistoryAttributes.h>
#include <fastrtps/utils/TimedMutex.hpp>

#include <cassert>
#include <functional>
#include <memory>
#include <mutex>

namespace eprosima {
namespace fastrtps {
namespace rtps {

/**
 * Class History, container of the different CacheChanges and the methods to access them.
 * @ingroup COMMON_MODULE
 */
class History
{
protected:

    History(
            const HistoryAttributes&  att);
    History(
            History&&) = delete;
    History& operator =(
            History&&) = delete;
    virtual ~History();

public:

    using iterator = std::vector<CacheChange_t*>::iterator;
    using reverse_iterator = std::vector<CacheChange_t*>::reverse_iterator;
    using const_iterator = std::vector<CacheChange_t*>::const_iterator;

    //!Attributes of the History
    HistoryAttributes m_att;
    /**
     * Reserve a CacheChange_t from the CacheChange pool.
     * @param[out] change Pointer to pointer to the CacheChange_t to reserve
     * @param[in] calculateSizeFunc Function to calculate the size of the change.
     * @return True is reserved
     */
    RTPS_DllAPI inline bool reserve_Cache(
            CacheChange_t** change,
            const std::function<uint32_t()>& calculateSizeFunc)
    {
        std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
        CacheChange_t* reserved_change = nullptr;
        if (change_pool_->reserve_cache(reserved_change))
        {
            uint32_t payload_size = m_att.memoryPolicy == MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE ?
                    max_payload_size_ : calculateSizeFunc();
            if (payload_pool_->get_payload(payload_size, *reserved_change))
            {
                *change = reserved_change;
                return true;
            }

            change_pool_->release_cache(reserved_change);
        }

        return false;
    }

    RTPS_DllAPI inline bool reserve_Cache(
            CacheChange_t** change,
            uint32_t dataSize)
    {
        return reserve_Cache(change, [dataSize]()
                       {
                           return dataSize;
                       });
    }

    /**
     * release a previously reserved CacheChange_t.
     * @param ch Pointer to the CacheChange_t.
     */
    RTPS_DllAPI inline void release_Cache(
            CacheChange_t* ch)
    {
        std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
        do_release_cache(ch);
    }

    /**
     * Check if the history is full
     * @return true if the History is full.
     */
    RTPS_DllAPI bool isFull()
    {
        return m_isHistoryFull;
    }

    /**
     * Get the History size.
     * @return Size of the history.
     */
    RTPS_DllAPI size_t getHistorySize()
    {
        std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
        return m_changes.size();
    }

    /**
     * Remove all changes from the History
     * @return True if everything was correctly removed.
     */
    RTPS_DllAPI bool remove_all_changes();

    /**
     * Remove a specific change from the history.
     * @param ch Pointer to the CacheChange_t.
     * @return True if removed.
     */
    RTPS_DllAPI virtual bool remove_change(
            CacheChange_t* ch);

    /**
     * Find a specific change in the history using the matches_change method criteria.
     * @param ch Pointer to the CacheChange_t to search for.
     * @return an iterator if a suitable change is found
     */
    RTPS_DllAPI const_iterator find_change(
            CacheChange_t* ch);

    /**
     * Verifies if an element of the changes collection matches a given change
     * Derived classes have more info on how to identify univocally a change and should override.
     * @param ch_inner element of the collection to compare with the given change
     * @param ch_outer Pointer to the CacheChange_t to identify.
     * @return true if the iterator identifies this change.
     */
    RTPS_DllAPI virtual bool matches_change(
            const CacheChange_t* ch_inner,
            CacheChange_t* ch_outer);
    /**
     * Remove a specific change from the history.
     * @param removal iterator to the CacheChange_t to remove.
     * @param release defaults to true and hints if the CacheChange_t should return to the pool
     * @return iterator to the next CacheChange_t or end iterator.
     */
    RTPS_DllAPI virtual iterator remove_change(
            const_iterator removal,
            bool release = true);

    /**
     * Get the beginning of the changes history iterator.
     * @return Iterator to the beginning of the vector.
     */
    RTPS_DllAPI iterator changesBegin()
    {
        return m_changes.begin();
    }

    RTPS_DllAPI reverse_iterator changesRbegin()
    {
        return m_changes.rbegin();
    }

    /**
     * Get the end of the changes history iterator.
     * @return Iterator to the end of the vector.
     */
    RTPS_DllAPI iterator changesEnd()
    {
        return m_changes.end();
    }

    RTPS_DllAPI reverse_iterator changesRend()
    {
        return m_changes.rend();
    }

    /**
     * Get the minimum CacheChange_t.
     * @param min_change Pointer to pointer to the minimum change.
     * @return True if correct.
     */
    RTPS_DllAPI bool get_min_change(
            CacheChange_t** min_change);

    /**
     * Get the maximum CacheChange_t.
     * @param max_change Pointer to pointer to the maximum change.
     * @return True if correct.
     */
    RTPS_DllAPI bool get_max_change(
            CacheChange_t** max_change);

    /**
     * Get the maximum serialized payload size
     * @return Maximum serialized payload size
     */
    RTPS_DllAPI inline uint32_t getTypeMaxSerialized()
    {
        return max_payload_size_;
    }

    /*!
     * Get the mutex
     * @return Mutex
     */
    RTPS_DllAPI inline RecursiveTimedMutex* getMutex()
    {
        assert(mp_mutex != nullptr); return mp_mutex;
    }

    RTPS_DllAPI bool get_change(
            const SequenceNumber_t& seq,
            const GUID_t& guid,
            CacheChange_t** change) const;

    const_iterator get_change_nts(
            const SequenceNumber_t& seq,
            const GUID_t& guid,
            CacheChange_t** change,
            const_iterator hint) const;

    /**
     * @brief A method to get the change with the earliest timestamp
     * @param change Pointer to pointer to earliest change
     * @return True on success
     */
    bool get_earliest_change(
            CacheChange_t** change);

protected:

    //!Vector of pointers to the CacheChange_t.
    std::vector<CacheChange_t*> m_changes;

    //!Variable to know if the history is full without needing to block the History mutex.
    bool m_isHistoryFull;

    std::shared_ptr<IPayloadPool> payload_pool_;

    //!Pool of cache changes reserved when the History is created.
    std::shared_ptr<IChangePool> change_pool_;

    uint32_t max_payload_size_;

    //!Print the seqNum of the changes in the History (for debuggisi, mng purposes).
    void print_changes_seqNum2();

    //!Mutex for the History.
    RecursiveTimedMutex* mp_mutex;

    void do_release_cache(
            CacheChange_t* ch);
};

} /* namespace rtps     */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* _FASTDDS_RTPS_HISTORY_H_ */
