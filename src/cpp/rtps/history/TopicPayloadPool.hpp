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
 * @file Base.hpp
 */

#ifndef RTPS_HISTORY_TOPICPAYLOADPOOL_HPP
#define RTPS_HISTORY_TOPICPAYLOADPOOL_HPP

#include <fastdds/rtps/common/CacheChange.h>
#include <fastdds/rtps/history/IPayloadPool.h>
#include <fastdds/rtps/resources/ResourceManagement.h>
#include <fastdds/dds/log/Log.hpp>
#include <rtps/history/PoolConfig.h>
#include <rtps/history/ITopicPayloadPool.h>

#include <memory>
#include <vector>
#include <mutex>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class TopicPayloadPool : public ITopicPayloadPool
{

public:

    TopicPayloadPool() = default;

    virtual ~TopicPayloadPool()
    {
        logInfo(RTPS_UTILS, "PayloadPool destructor");

        for (PayloadNode* payload : all_payloads_)
        {
            delete payload;
        }
    }

    virtual bool get_payload(
            uint32_t size,
            CacheChange_t& cache_change) override;

    virtual bool get_payload(
            SerializedPayload_t& data,
            IPayloadPool*& data_owner,
            CacheChange_t& cache_change) override;

    virtual bool release_payload(
            CacheChange_t& cache_change) override;

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
            bool is_reader) override;

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
            bool is_reader) override;

    size_t payload_pool_allocated_size() const override
    {
        return all_payloads_.size();
    }

    size_t payload_pool_available_size() const override
    {
        return free_payloads_.size();
    }

    static std::shared_ptr<ITopicPayloadPool> get(
            PoolConfig config);

protected:

    class PayloadNode
    {
    public:

        octet* buffer = nullptr;
        static constexpr uint8_t size_offset = 0;
        static constexpr uint8_t index_offset = sizeof(uint32_t);
        static constexpr uint8_t reference_offset = 2 * sizeof(uint32_t);
        static constexpr uint8_t data_offset = 3 * sizeof(uint32_t);

        explicit PayloadNode(
                uint32_t size)
        {
            buffer = (octet*)calloc(size + data_offset, sizeof(octet));
            data_size(size);
        }

        explicit PayloadNode(
                octet* data)
        {
            buffer = data - data_offset;
        }

        ~PayloadNode()
        {
            free(buffer);
        }

        bool resize (
                uint32_t size)
        {
            octet* old_buffer = buffer;
            buffer = (octet*)realloc(buffer, size + data_offset);
            if (!buffer)
            {
                buffer = old_buffer;
                return false;
            }
            memset(buffer + data_offset + data_size(), 0, (size - data_size()) * sizeof(octet));
            data_size(size);
            return true;
        }

        uint32_t data_size() const
        {
            return *reinterpret_cast<uint32_t*>(buffer + size_offset);
        }

        static uint32_t data_size(
                octet* data)
        {
            return *reinterpret_cast<uint32_t*>(data - data_offset + size_offset);
        }

        void data_size(
                uint32_t size)
        {
            *reinterpret_cast<uint32_t*>(buffer + size_offset) = size;
        }

        uint32_t data_index() const
        {
            return *reinterpret_cast<uint32_t*>(buffer + index_offset);
        }

        void data_index(
                uint32_t index)
        {
            *reinterpret_cast<uint32_t*>(buffer + index_offset) = index;
        }

        octet* data() const
        {
            return buffer + data_offset;
        }

        static uint32_t data_index(
                octet* data)
        {
            return *reinterpret_cast<uint32_t*>(data - data_offset + index_offset);
        }

        void reference()
        {
            ++(*reinterpret_cast<uint32_t*>(buffer + reference_offset));
        }

        bool dereference()
        {
            return --(*reinterpret_cast<uint32_t*>(buffer + reference_offset));
        }

        static void reference(
                octet* data)
        {
            ++(*reinterpret_cast<uint32_t*>(data - data_offset + reference_offset));
        }

        static bool dereference(
                octet* data)
        {
            return --(*reinterpret_cast<uint32_t*>(data - data_offset + reference_offset));
        }

    };

    /**
     * Adds a new payload in the pool, but does not add it to the list of free payloads
     */
    virtual PayloadNode* allocate(
            uint32_t size);

    PayloadNode* do_allocate(
            uint32_t size);

    virtual void update_maximum_size(
            const PoolConfig& config,
            bool is_reserve);

    /**
     * Ensures the pool has capacity for at least @c num_payloads elements.
     *
     * @param [IN] min_num_payloads Minimum number of payloads reserved in the pool
     * @param [IN] size             Size to allocate for the payloads that need to be added to the pool
     *
     * @pre
     *   - @c size <= @c max_pool_size_
     * @post
     *   - @c get_allPayloadsSize() >= @c min_num_payloads
     */
    virtual void reserve (
            uint32_t min_num_payloads,
            uint32_t size);

    /**
     * Ensures the pool has capacity for at most @c num_payloads elements.
     *
     * @param [IN] max_num_payloads Maximum number of payloads reserved in the pool
     *
     * @return @c true on success, @c false otherwise
     *
     * @post
     *   - On success, get_allPayloadsSize() <= max_num_payloads
     *   - On failure, memory for some payloads may have been released, but get_allPayloadsSize() > min_num_payloads
     */
    bool shrink (
            uint32_t max_num_payloads);

    /**
     * Resizes a payload to have @c size octets, and updates the buffer of the pool accordingly.
     *
     * @param [IN,OUT] payload   Payload buffer to resize
     * @param [IN,OUT] size      Size of the payload buffer, will be updated with the final size
     * @param [IN]     new_size  Number of octets that the Payload needs to fit
     *
     * @return @c true on success, @c false otherwise
     *
     * @pre
     *   - @c payload is allocated in the pool
     *   - @c payload is not in the buffer of free payloads. Only payloads marked as 'used' can be resized.
     * @post
     *   - On success, @c payload has @c size octets
     *   - On failure, nothing has changed
     *   - If the buffer is enlarged, newly allocated octets are initialized to zero.
     */

    bool resize_payload (
            octet*& payload,
            uint32_t& size,
            uint32_t new_size);

    virtual MemoryManagementPolicy_t memory_policy() const = 0;

    uint32_t max_pool_size_             = 0;  //< Maximum size of the pool
    uint32_t infinite_histories_count_  = 0;  //< Number of infinite histories reserved
    uint32_t finite_max_pool_size_      = 0;  //< Maximum size of the pool if no infinite histories were reserved

    std::vector<PayloadNode*> free_payloads_; //< Payloads that are free
    std::vector<PayloadNode*> all_payloads_;  //< All payloads

    std::mutex mutex_;

};


}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

#endif  // RTPS_HISTORY_TOPICPAYLOADPOOL_HPP
