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
 * @file SampleInfoPool.hpp
 */

#ifndef _FASTDDS_SUBSCRIBER_DATAREADERIMPL_SAMPLEINFOPOOL_HPP_
#define _FASTDDS_SUBSCRIBER_DATAREADERIMPL_SAMPLEINFOPOOL_HPP_

#include <algorithm>
#include <cassert>

#include <fastdds/dds/core/LoanableCollection.hpp>
#include <fastdds/dds/core/LoanableSequence.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>

#include <fastdds/utils/collections/ResourceLimitedContainerConfig.hpp>
#include <fastdds/utils/collections/ResourceLimitedVector.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

struct SampleInfoPool
{
    explicit SampleInfoPool(
            const DataReaderQos& qos)
        : free_items_(qos.reader_resource_limits().sample_infos_allocation)
        , used_items_(qos.reader_resource_limits().sample_infos_allocation)
    {
        for (size_t n = 0; n < qos.reader_resource_limits().sample_infos_allocation.initial; ++n)
        {
            free_items_.push_back(new SampleInfo());
        }
    }

    ~SampleInfoPool()
    {
        for (SampleInfo* it : free_items_)
        {
            delete it;
        }
    }

    size_t num_allocated()
    {
        return used_items_.size();
    }

    SampleInfo* get_item()
    {
        SampleInfo** result = nullptr;

        if (free_items_.empty())
        {
            result = used_items_.push_back(new SampleInfo());
        }
        else
        {
            result = used_items_.push_back(free_items_.back());
            static_cast<void>(result);
            assert(result != nullptr);
            free_items_.pop_back();
        }

        return result ? *result : nullptr;
    }

    void return_item(
            SampleInfo* item)
    {
        bool removed = used_items_.remove(item);
        static_cast<void>(removed);
        assert(removed);

        SampleInfo** result = free_items_.push_back(item);
        static_cast<void>(result);
        assert(result != nullptr);
    }

private:

    using collection_type = eprosima::fastdds::ResourceLimitedVector<SampleInfo*>;

    collection_type free_items_;
    collection_type used_items_;
};

} /* namespace detail */
} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif  // _FASTDDS_SUBSCRIBER_DATAREADERIMPL_SAMPLEINFOPOOL_HPP_
