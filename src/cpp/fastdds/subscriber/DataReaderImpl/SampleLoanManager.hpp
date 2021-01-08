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
 * @file SampleLoanManager.hpp
 */

#ifndef _FASTDDS_SUBSCRIBER_DATAREADERIMPL_SAMPLELOANMANAGER_HPP_
#define _FASTDDS_SUBSCRIBER_DATAREADERIMPL_SAMPLELOANMANAGER_HPP_

#include <algorithm>
#include <cassert>

#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastdds/rtps/common/CacheChange.h>
#include <fastdds/rtps/common/SerializedPayload.h>
#include <fastdds/rtps/history/IPayloadPool.h>

#include <fastrtps/types/TypesBase.h>
#include <fastrtps/utils/collections/ResourceLimitedContainerConfig.hpp>
#include <fastrtps/utils/collections/ResourceLimitedVector.hpp>

#include <rtps/history/PoolConfig.h>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

struct SampleLoanManager
{
    using CacheChange_t = eprosima::fastrtps::rtps::CacheChange_t;
    using IPayloadPool = eprosima::fastrtps::rtps::IPayloadPool;
    using PoolConfig = eprosima::fastrtps::rtps::PoolConfig;
    using ReturnCode_t = eprosima::fastrtps::types::ReturnCode_t;
    using SampleIdentity = eprosima::fastrtps::rtps::SampleIdentity;
    using SerializedPayload_t = eprosima::fastrtps::rtps::SerializedPayload_t;

    SampleLoanManager(
            const PoolConfig& pool_config,
            const TypeSupport& type)
        : limits_(pool_config.initial_size,
                pool_config.maximum_size ? pool_config.maximum_size : std::numeric_limits<size_t>::max(),
                1)
        , free_loans_(limits_)
        , used_loans_(limits_)
        , type_(type)
    {
        for (size_t n = 0; n < limits_.initial; ++n)
        {
            OutstandingLoanItem item;
            if (!type_->is_plain())
            {
                item.sample = type_->createData();
            }
            free_loans_.push_back(item);
        }
    }

    ~SampleLoanManager()
    {
        if (!type_->is_plain())
        {
            for (const OutstandingLoanItem& item : free_loans_)
            {
                type_->deleteData(item.sample);
            }
        }
    }

    ReturnCode_t get_loan(
            CacheChange_t* change,
            void*& sample)
    {
        OutstandingLoanItem* item = find_by_change(change);
        if (nullptr != item)
        {
            item->num_refs += 1;
            sample = item->sample;
            return ReturnCode_t::RETCODE_OK;
        }

        if (free_loans_.empty())
        {
            // Try to create a new entry
            item = used_loans_.push_back({});
            if (nullptr != item)
            {
                // Create sample if necessary
                if (!type_->is_plain())
                {
                    item->sample = type_->createData();
                }
            }
        }
        else
        {
            // Reuse a free entry
            item = used_loans_.push_back(free_loans_.back());
            assert(nullptr != item);
            free_loans_.pop_back();
        }

        // Early return if could not find an entry
        if (nullptr == item)
        {
            return ReturnCode_t::RETCODE_OUT_OF_RESOURCES;
        }

        // Only deserialize the first time
        if (item->num_refs == 0)
        {
            // Increment references of input payload
            CacheChange_t tmp;
            tmp.copy_not_memcpy(change);
            item->owner = change->payload_owner();
            change->payload_owner()->get_payload(change->serializedPayload, item->owner, tmp);
            item->owner = tmp.payload_owner();
            item->payload = tmp.serializedPayload;
            tmp.payload_owner(nullptr);
            tmp.serializedPayload.data = nullptr;

            // Perform deserialization
            if (type_->is_plain())
            {
                auto ptr = item->payload.data;
                ptr += item->payload.representation_header_size;
                item->sample = ptr;
            }
            else
            {
                type_->deserialize(&item->payload, item->sample);
            }
        }

        // Increment reference counter and return sample
        item->num_refs += 1;
        sample = item->sample;
        return ReturnCode_t::RETCODE_OK;
    }

    void return_loan(
            void* sample)
    {
        OutstandingLoanItem* item = find_by_sample(sample);
        assert(nullptr != item);

        item->num_refs -= 1;
        if (item->num_refs == 0)
        {
            CacheChange_t tmp;
            tmp.payload_owner(item->owner);
            tmp.serializedPayload = item->payload;
            item->owner->release_payload(tmp);
            item->payload.data = nullptr;
            item->owner = nullptr;

            item = free_loans_.push_back(*item);
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
        IPayloadPool* owner = nullptr;
        uint32_t num_refs = 0;

        ~OutstandingLoanItem()
        {
            payload.data = nullptr;
        }

        OutstandingLoanItem() = default;
        OutstandingLoanItem(
                const OutstandingLoanItem&) = default;
        OutstandingLoanItem& operator =(
                const OutstandingLoanItem&) = default;
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

    using collection_type = eprosima::fastrtps::ResourceLimitedVector<OutstandingLoanItem>;

    eprosima::fastrtps::ResourceLimitedContainerConfig limits_;
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
