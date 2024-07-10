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
 * @file History.hpp
 *
 */

#ifndef FASTDDS_RTPS_HISTORY__HISTORY_HPP
#define FASTDDS_RTPS_HISTORY__HISTORY_HPP

#include <fastdds/fastdds_dll.hpp>
#include <fastdds/rtps/history/IChangePool.hpp>
#include <fastdds/rtps/history/IPayloadPool.hpp>
#include <fastdds/rtps/common/SequenceNumber.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/attributes/HistoryAttributes.hpp>
#include <fastdds/utils/TimedMutex.hpp>

#include <cassert>
#include <functional>
#include <memory>
#include <mutex>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Class History, container of the different CacheChanges and the methods to access them.
 * @ingroup COMMON_MODULE
 */
class History
{
protected:

    History(
            const HistoryAttributes& att);
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
     * Check if the history is full
     * @return true if the History is full.
     */
    FASTDDS_EXPORTED_API bool isFull()
    {
        return m_isHistoryFull;
    }

    /**
     * Get the History size.
     * @return Size of the history.
     */
    FASTDDS_EXPORTED_API size_t getHistorySize()
    {
        std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
        return m_changes.size();
    }

    /**
     * Find a specific change in the history using the matches_change method criteria.
     * No Thread Safe
     * @param ch Pointer to the CacheChange_t to search for.
     * @return an iterator if a suitable change is found
     */
    FASTDDS_EXPORTED_API const_iterator find_change_nts(
            CacheChange_t* ch);

    /**
     * Remove a specific change from the history.
     * No Thread Safe
     * @param removal iterator to the CacheChange_t to remove.
     * @param release defaults to true and hints if the CacheChange_t should return to the pool
     * @return iterator to the next CacheChange_t or end iterator.
     */
    FASTDDS_EXPORTED_API virtual iterator remove_change_nts(
            const_iterator removal,
            bool release = true);

    /**
     * Remove a specific change from the history.
     * No Thread Safe
     * @param removal iterator to the CacheChange_t to remove.
     * @param [in] max_blocking_time Maximum time this method has to complete the task.
     * @param release defaults to true and hints if the CacheChange_t should return to the pool
     * @return iterator to the next CacheChange_t or end iterator.
     */
    FASTDDS_EXPORTED_API virtual iterator remove_change_nts(
            const_iterator removal,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time,
            bool release = true);

    /**
     * Remove all changes from the History
     * @return True if everything was correctly removed.
     */
    FASTDDS_EXPORTED_API bool remove_all_changes();

    /**
     * Remove a specific change from the history.
     * @param ch Pointer to the CacheChange_t.
     * @return True if removed.
     */
    FASTDDS_EXPORTED_API bool remove_change(
            CacheChange_t* ch);

    /**
     * Remove a specific change from the history.
     * @param ch Pointer to the CacheChange_t.
     * @param [in] max_blocking_time Maximum time this method has to complete the task.
     * @return True if removed.
     */
    FASTDDS_EXPORTED_API bool remove_change(
            CacheChange_t* ch,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time);

    /**
     * Find a specific change in the history using the matches_change method criteria.
     * @param ch Pointer to the CacheChange_t to search for.
     * @return an iterator if a suitable change is found
     */
    FASTDDS_EXPORTED_API const_iterator find_change(
            CacheChange_t* ch)
    {
        std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
        return find_change_nts(ch);
    }

    /**
     * Verifies if an element of the changes collection matches a given change
     * Derived classes have more info on how to identify univocally a change and should override.
     * @param ch_inner element of the collection to compare with the given change
     * @param ch_outer Pointer to the CacheChange_t to identify.
     * @return true if the iterator identifies this change.
     */
    FASTDDS_EXPORTED_API virtual bool matches_change(
            const CacheChange_t* ch_inner,
            CacheChange_t* ch_outer);

    /**
     * Remove a specific change from the history.
     * @param removal iterator to the CacheChange_t to remove.
     * @param release defaults to true and hints if the CacheChange_t should return to the pool
     * @return iterator to the next CacheChange_t or end iterator.
     */
    FASTDDS_EXPORTED_API iterator remove_change(
            const_iterator removal,
            bool release = true)
    {
        std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
        return remove_change_nts(removal, release);
    }

    /**
     * Get the beginning of the changes history iterator.
     * @return Iterator to the beginning of the vector.
     */
    FASTDDS_EXPORTED_API iterator changesBegin()
    {
        return m_changes.begin();
    }

    FASTDDS_EXPORTED_API reverse_iterator changesRbegin()
    {
        return m_changes.rbegin();
    }

    /**
     * Get the end of the changes history iterator.
     * @return Iterator to the end of the vector.
     */
    FASTDDS_EXPORTED_API iterator changesEnd()
    {
        return m_changes.end();
    }

    FASTDDS_EXPORTED_API reverse_iterator changesRend()
    {
        return m_changes.rend();
    }

    /**
     * Get the minimum CacheChange_t.
     * @param min_change Pointer to pointer to the minimum change.
     * @return True if correct.
     */
    FASTDDS_EXPORTED_API bool get_min_change(
            CacheChange_t** min_change);

    /**
     * Get the maximum CacheChange_t.
     * @param max_change Pointer to pointer to the maximum change.
     * @return True if correct.
     */
    FASTDDS_EXPORTED_API bool get_max_change(
            CacheChange_t** max_change);

    /**
     * Get the maximum serialized payload size
     * @return Maximum serialized payload size
     */
    FASTDDS_EXPORTED_API inline uint32_t getTypeMaxSerialized()
    {
        return m_att.payloadMaxSize;
    }

    /*!
     * Get the mutex
     * @return Mutex
     */
    FASTDDS_EXPORTED_API inline RecursiveTimedMutex* getMutex() const
    {
        assert(mp_mutex != nullptr);
        return mp_mutex;
    }

    FASTDDS_EXPORTED_API bool get_change(
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
    bool m_isHistoryFull = false;

    //!Mutex for the History.
    RecursiveTimedMutex* mp_mutex = nullptr;

    //!Print the seqNum of the changes in the History (for debuggisi, mng purposes).
    void print_changes_seqNum2();

    FASTDDS_EXPORTED_API virtual void do_release_cache(
            CacheChange_t* ch) = 0;

    /**
     * @brief Removes the constness of a const_iterator to obtain a regular iterator.
     *
     * This function takes a const_iterator as input and returns a regular iterator by removing the constness.
     *
     * @param c_it The const_iterator to remove constness from.
     *
     * @return An iterator with the same position as the input const_iterator.
     */
    History::iterator remove_iterator_constness(
            const_iterator c_it);

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_HISTORY__HISTORY_HPP
