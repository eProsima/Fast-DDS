// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file SampleLoanManager.hpp
 */

#ifndef _FASTDDS_SUBSCRIBER_DATAREADERIMPL_SAMPLELOANMANAGER_HPP_
#define _FASTDDS_SUBSCRIBER_DATAREADERIMPL_SAMPLELOANMANAGER_HPP_

#include <algorithm>
#include <cassert>

#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/rtps/common/SerializedPayload.hpp>
#include <fastdds/rtps/history/IPayloadPool.hpp>

#include <fastdds/utils/collections/ResourceLimitedContainerConfig.hpp>
#include <fastdds/utils/collections/ResourceLimitedVector.hpp>

#include <rtps/history/PoolConfig.h>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

struct SampleLoanManager
{
    using CacheChange_t = eprosima::fastdds::rtps::CacheChange_t;
    using IPayloadPool = eprosima::fastdds::rtps::IPayloadPool;
    using PoolConfig = eprosima::fastdds::rtps::PoolConfig;
    using SampleIdentity = eprosima::fastdds::rtps::SampleIdentity;
    using SerializedPayload_t = eprosima::fastdds::rtps::SerializedPayload_t;

    SampleLoanManager(
            const PoolConfig& pool_config,
            const TypeSupport& type,
            const bool is_plain)
        : is_plain_(is_plain)
        , limits_(pool_config.initial_size,
                pool_config.maximum_size ? pool_config.maximum_size : std::numeric_limits<size_t>::max(),
                1)
        , free_loans_(limits_)
        , used_loans_(limits_)
        , type_(type)
    {
        for (size_t n = 0; n < limits_.initial; ++n)
        {
            OutstandingLoanItem item;
            if (!is_plain_)
            {
                item.sample = type_->create_data();
            }
            free_loans_.push_back(std::move(item));
        }
    }

    ~SampleLoanManager()
    {
        if (!is_plain_)
        {
            for (const OutstandingLoanItem& item : free_loans_)
            {
                type_->delete_data(item.sample);
            }
        }
    }

    int32_t num_allocated() const
    {
        assert(used_loans_.size() <= static_cast<size_t>(std::numeric_limits<int32_t>::max()));
        return static_cast<int32_t>(used_loans_.size());
    }

    void get_loan(
            CacheChange_t* change,
            void*& sample)
    {
        // Early return an already loaned item
        OutstandingLoanItem* item = find_by_change(change);
        if (nullptr != item)
        {
            item->num_refs += 1;
            sample = item->sample;
            return;
        }

        // Get an item from the pool
        if (free_loans_.empty())
        {
            // Try to create a new entry
            item = used_loans_.push_back({});
            if (nullptr != item)
            {
                // Create sample if necessary
                if (!is_plain_)
                {
                    item->sample = type_->create_data();
                }
            }
        }
        else
        {
            // Reuse a free entry
            item = used_loans_.push_back(std::move(free_loans_.back()));
            assert(nullptr != item);
            free_loans_.pop_back();
        }

        // Should always find an entry, as resource limits are checked before calling this method
        assert(nullptr != item);

        // Should be the first time we loan this item
        assert(item->num_refs == 0);

        // Increment references of input payload
        change->serializedPayload.payload_owner->get_payload(change->serializedPayload, item->payload);

        // Perform deserialization
        if (is_plain_)
        {
            auto ptr = item->payload.data;
            ptr += item->payload.representation_header_size;
            item->sample = ptr;
        }
        else
        {
            type_->deserialize(item->payload, item->sample);
        }

        // Increment reference counter and return sample
        item->num_refs += 1;
        sample = item->sample;
    }

    void return_loan(
            void* sample)
    {
        OutstandingLoanItem* item = find_by_sample(sample);
        assert(nullptr != item);

        item->num_refs -= 1;
        if (item->num_refs == 0)
        {
            item->payload.payload_owner->release_payload(item->payload);
            assert(item->payload.data == nullptr);
            assert(item->payload.payload_owner == nullptr);

            item = free_loans_.push_back(std::move(*item));
            assert(nullptr != item);
            used_loans_.remove(*item);
        }
    }

private:

    struct OutstandingLoanItem
    {
        void* sample = nullptr;
        SampleIdentity identity;
        SerializedPayload_t payload;
        uint32_t num_refs = 0;

        ~OutstandingLoanItem()
        {
            // Avoid releasing payload and freeing data
            payload.payload_owner = nullptr;
            payload.data = nullptr;
        }

        OutstandingLoanItem() = default;
        OutstandingLoanItem(
                const OutstandingLoanItem&) = delete;
        OutstandingLoanItem& operator =(
                const OutstandingLoanItem&) = delete;
        OutstandingLoanItem(
                OutstandingLoanItem&&) = default;
        OutstandingLoanItem& operator =(
                OutstandingLoanItem&&) = default;

        bool operator == (
                const OutstandingLoanItem& other) const
        {
            return other.sample == sample && other.payload.data == payload.data;
        }

    };

    using collection_type = eprosima::fastdds::ResourceLimitedVector<OutstandingLoanItem>;

    bool is_plain_;
    eprosima::fastdds::ResourceLimitedContainerConfig limits_;
    collection_type free_loans_;
    collection_type used_loans_;
    TypeSupport type_;

    OutstandingLoanItem* find_by_change(
            CacheChange_t* change)
    {
        SampleIdentity id;
        id.writer_guid(change->writerGUID);
        id.sequence_number(change->sequenceNumber);

        auto comp = [id](const OutstandingLoanItem& item)
                {
                    return id == item.identity;
                };
        auto it = std::find_if(used_loans_.begin(), used_loans_.end(), comp);
        if (it != used_loans_.end())
        {
            return &(*it);
        }
        return nullptr;
    }

    OutstandingLoanItem* find_by_sample(
            void* sample)
    {
        auto comp = [sample](const OutstandingLoanItem& item)
                {
                    return sample == item.sample;
                };
        auto it = std::find_if(used_loans_.begin(), used_loans_.end(), comp);
        assert(it != used_loans_.end());
        return &(*it);
    }

};

} /* namespace detail */
} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif  // _FASTDDS_SUBSCRIBER_DATAREADERIMPL_SAMPLELOANMANAGER_HPP_
