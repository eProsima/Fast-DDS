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
 * @file ITopicPayloadPool.hpp
 */

#ifndef RTPS_HISTORY_ITOPICPAYLOADPOOL_HPP
#define RTPS_HISTORY_ITOPICPAYLOADPOOL_HPP

#include <fastdds/rtps/history/IPayloadPool.hpp>
#include <rtps/history/PoolConfig.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

class ITopicPayloadPool : public IPayloadPool
{
public:

    /**
     * @brief Ensures the pool has capacity to fullfill the requirements of a new history.
     *
     * @param [in]  config              The new history's pool requirements.
     * @param [in]  is_reader_history   True if the new history is for a reader. False otherwise.
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
    virtual bool reserve_history(
            const PoolConfig& config,
            bool is_reader) = 0;

    /**
     * @brief Informs the pool that some history requirements are not longer active.
     *
     * The pool can release some resources that are not needed any longer.
     *
     * @param [in]  config              The old history's pool requirements, which are no longer active.
     * @param [in]  is_reader_history   True if the history was for a reader. False otherwise.
     *
     * @pre
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
    virtual bool release_history(
            const PoolConfig& config,
            bool is_reader) = 0;

    /**
     * @brief Get the number of allocated payloads (reserved and not reserved).
     */
    virtual size_t payload_pool_allocated_size() const = 0;

    /**
     * @brief Get the number of available payloads (not reserved).
     */
    virtual size_t payload_pool_available_size() const = 0;

};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima


#endif  // RTPS_HISTORY_ITOPICPAYLOADPOOL_HPP
