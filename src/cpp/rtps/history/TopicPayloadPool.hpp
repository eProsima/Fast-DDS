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

#include <fastdds/rtps/attributes/ResourceManagement.hpp>
#include <fastdds/rtps/common/SerializedPayload.hpp>
#include <fastdds/rtps/history/IPayloadPool.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <rtps/history/PoolConfig.h>
#include <rtps/history/ITopicPayloadPool.h>

#include <atomic>
#include <cstddef>
#include <memory>
#include <mutex>
#include <vector>
#include <cassert>

namespace eprosima {
namespace fastdds {
namespace rtps {

class TopicPayloadPool : public ITopicPayloadPool
{

public:

    TopicPayloadPool() = default;

    virtual ~TopicPayloadPool()
    {
        EPROSIMA_LOG_INFO(RTPS_UTILS, "PayloadPool destructor");

        for (PayloadNode* payload : all_payloads_)
        {
            delete payload;
        }
    }

    bool get_payload(
            uint32_t size,
            SerializedPayload_t& payload) override;

    bool get_payload(
            const SerializedPayload_t& data,
            SerializedPayload_t& payload) override;

    bool release_payload(
            SerializedPayload_t& payload) override;

    /**
     * @brief Ensures the pool has capacity to fullfill the requirements of a new history.
     *
     * @param [in]  config              The new history's pool requirements.
     * @param [in]  is_reader_history   True if the new history is for a reader. False otherwise.
     * @return Whether the operation was successful or not.
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
     * @return Whether the operation was successful or not.
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

    static std::unique_ptr<ITopicPayloadPool> get(
            const BasicPoolConfig& config);

protected:

    class PayloadNode
    {
    public:

        explicit PayloadNode(
                uint32_t size)
        {
            if (!size)
            {
                //! At least, we need this to allocate space for a NodeInfo.
                //! In order to be able to place-construct later
                buffer = (octet*)calloc(sizeof(NodeInfo), sizeof(octet));
            }
            else
            {
                buffer = (octet*)calloc(size + sizeof(NodeInfo) - 1, sizeof(octet));
            }

            if (buffer == nullptr)
            {
                throw std::bad_alloc();
            }

            // The atomic may need some initialization depending on the platform
            new (buffer) NodeInfo();
            data_size(size);
        }

        ~PayloadNode()
        {
            info().~NodeInfo();
            free(buffer);
        }

        bool resize (
                uint32_t size)
        {
            assert(size > data_size());

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
            return info().data_size;
        }

        static uint32_t data_size(
                octet* data)
        {
            return info(data).data_size;
        }

        void data_size(
                uint32_t size)
        {
            info().data_size = size;
        }

        uint32_t data_index() const
        {
            return info().data_index;
        }

        static uint32_t data_index(
                octet* data)
        {
            return info(data).data_index;
        }

        void data_index(
                uint32_t index)
        {
            info().data_index = index;
        }

        octet* data() const
        {
            return info().data;
        }

        void reference()
        {
            info().ref_counter.fetch_add(1, std::memory_order_relaxed);
        }

        bool dereference()
        {
            return (info().ref_counter.fetch_sub(1, std::memory_order_acq_rel) == 1);
        }

        static void reference(
                octet* data)
        {
            info(data).ref_counter.fetch_add(1, std::memory_order_relaxed);
        }

        static bool dereference(
                octet* data)
        {
            return (info(data).ref_counter.fetch_sub(1, std::memory_order_acq_rel) == 1);
        }

    private:

        struct NodeInfo
        {
            std::atomic<uint32_t> ref_counter{ 0 };
            uint32_t data_size = 0;
            uint32_t data_index = 0;
            octet data[1];
        };

        octet* buffer = nullptr;

        // Payload data comes after the metadata
        static constexpr size_t data_offset = offsetof(NodeInfo, data);

        NodeInfo& info() const
        {
            return *reinterpret_cast<NodeInfo*>(buffer);
        }

        static NodeInfo& info(
                octet* data)
        {
            return *reinterpret_cast<NodeInfo*>(data - data_offset);
        }

    };

    /**
     * Adds a new payload in the pool, but does not add it to the list of free payloads
     *
     * @param [IN] size  Minimum size required for the payload data
     * @return The node representing the newly allocated payload.
     *
     * @post
     *   - @c payload_pool_allocated_size() increases by one
     *   - @c payload_pool_available_size() does not change
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
     *   - @c min_num_payloads <= @c max_pool_size_
     * @post
     *   - @c payload_pool_allocated_size() >= @c min_num_payloads
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
     *   - On success, payload_pool_allocated_size() <= max_num_payloads
     *   - On failure, memory for some payloads may have been released, but payload_pool_allocated_size() > min_num_payloads
     */
    bool shrink (
            uint32_t max_num_payloads);

    /**
     * @brief Get a serialized payload for a new sample.
     *
     * If the payload is recycled from the pool, @c resizable controls whether it can
     * be reallocated to accomodate larger sizes.
     * If @c resizable is false and there is at least one free payload in the pool, that payload will
     * be returned even though it may not reach the requested size.
     *
     * If @c resizable is true and the reallocation fails, the operation returns false and
     * the payload is returned to the pool.
     *
     * @param [in]     size          Number of bytes required for the serialized payload
     * @param [in,out] cache_change  Cache change to assign the payload to
     * @param [in]     resizable     Whether payloads recycled from the pool are resizable to accomodate larger sizes
     *
     * @returns whether the operation succeeded or not
     *
     * @post
     *   On success:
     *     @li Field @c cache_change.payload_owner equals this
     *     @li Field @c serializedPayload.data points to a buffer of at least @c size bytes
     *     @li Field @c serializedPayload.max_size is greater than or equal to @c size
     */
    virtual bool do_get_payload(
            uint32_t size,
            SerializedPayload_t& payload,
            bool resizeable);

    virtual MemoryManagementPolicy_t memory_policy() const = 0;

    uint32_t max_pool_size_             = 0;  //< Maximum size of the pool
    uint32_t infinite_histories_count_  = 0;  //< Number of infinite histories reserved
    uint32_t finite_max_pool_size_      = 0;  //< Maximum size of the pool if no infinite histories were reserved

    std::vector<PayloadNode*> free_payloads_; //< Payloads that are free
    std::vector<PayloadNode*> all_payloads_;  //< All payloads

    std::mutex mutex_;

};


}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // RTPS_HISTORY_TOPICPAYLOADPOOL_HPP
