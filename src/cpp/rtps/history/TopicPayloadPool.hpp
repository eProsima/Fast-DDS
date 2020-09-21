// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file TopicPayloadPool.hpp
 */

#ifndef RTPS_HISTORY_TOPICPAYLOADPOOL_HPP
#define RTPS_HISTORY_TOPICPAYLOADPOOL_HPP

#include <fastdds/rtps/common/CacheChange.h>
#include <fastdds/rtps/history/IPayloadPool.h>
#include <fastdds/rtps/resources/ResourceManagement.h>
#include <rtps/history/PoolConfig.h>


namespace eprosima {
namespace fastrtps {
namespace rtps {

class TopicPayloadPool : public IPayloadPool
{
public:

    /**
     * @brief Initializes the pool.
     *
     * This is a first approach to the TopicPayloadPool API,
     * whose purpose is the development of the unit tests.
     * The final implementation may need to rearrange certain elements,
     * and tests may need to be tuned accordingly.
     *
     * For example, TopicPayloadPool will probably end up being implemented
     * in a similar fashion as BasicPayloadPool,
     * with a template class specialized for each memory policy type.
     * In that case, not all specializations may need to take the payload size in their constructor.
     * However, the API will be conceptually the same as the one described here,
     * as the user of the pool will need to use a factory method
     * that takes the payload size and then uses it or not depending on the policy.
     * Therefore, all tests should remain conceptually valid.
     *
     * @param [in]     policy        The memory management policy.
     * @param [in]     payload_size  Estimated size of the payload.<br>
     */
    TopicPayloadPool(
            MemoryManagementPolicy_t policy,
            uint32_t payload_size)
    {
        (void) policy;
        (void) payload_size;
    }

    bool get_payload(
            uint32_t size,
            CacheChange_t& cache_change) override
    {
        (void) size;
        (void) cache_change;
        return true;
    }

    bool get_payload(
            SerializedPayload_t& data,
            IPayloadPool*& data_owner,
            CacheChange_t& cache_change) override
    {
        (void) data;
        (void) data_owner;
        (void) cache_change;
        return true;
    }

    bool release_payload(
            CacheChange_t& cache_change) override
    {
        (void) cache_change;
        return true;
    }

    /**
     * @brief Ensures the pool has capacity to fullfill the requirements of a new history.
     *
     * @param [in]  config              The new history's pool requirements.
     * @param [in]  is_reader_history   True if the new history is for a reader. False otherwise.
     *
     * @pre
     *   - Current pool is configured for the same memory policy as @c config.memory_policy.
     *
     * @post
     *   - If @c config.maximum_size is not zero
     *     - The maximum size of the pool is increased by @c config.maximum_size.
     *   - else
     *     - The maximum size of the pool is set to the largest representable value.
     *   - If the pool is configured for PREALLOCATED or PREALLOCATED WITH REALLOC memory policy:
     *     - The pool has at least as many elements allocated (including elements already in use)
     *       as the sum of the @c config.initial_size for all reserved writer histories
     *       plus the maximum of the @c config.initial_size for all reserved reader histories.
     */
    bool reserve_history(
            const PoolConfig& config,
            bool is_reader)
    {
        (void) config;
        (void) is_reader;
        return true;
    }

    /**
     * @brief Informs the pool that some history requirements are not longer active.
     *
     * The pool can release some resources that are not needed any longer.
     *
     * @param [in]  config              The old history's pool requirements, which are no longer active.
     * @param [in]  is_reader_history   True if the history was for a reader. False otherwise.
     *
     * @pre
     *   - Current pool is configured for the same memory policy as @c config.memory_policy.
     *   - If all remaining histories were reserved with non zero @c config.maximum_size
     *      - The number of elements in use is less than
     *        the sum of the @c config.maximum_size for all remaining histories
     *
     * @post
     *   - If all remaining histories were reserved with non zero @c config.maximum_size
     *      - The maximum size of the pool is set to
     *        the sum of the @c config.maximum_size for all remaining histories
     *   - else
     *     - The maximum size of the pool remains the largest representable value.
     *   - If the number of allocated elements is greater than the new maximum size,
     *     the excess of elements are freed until the number of allocated elemets is equal to the new maximum.
     */
    bool release_history(
            const PoolConfig& config,
            bool is_reader)
    {
        (void) config;
        (void) is_reader;
        return true;
    }

    /**
     * @brief Get the number of allocated payloads (reserved and not reserved).
     */
    size_t get_allCachesSize()
    {
        return 0;
    }

    /**
     * @brief Get the number of available payloads (not reserved).
     */
    size_t get_freeCachesSize()
    {
        return 0;
    }

};

}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

#endif  // RTPS_HISTORY_TOPICPAYLOADPOOL_HPP
