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
 * @file DataWriterInstancePool.hpp
 */

#ifndef _FASTDDS_PUBLISHER_DATAWRITERIMPL_INSTANCE_POOL_HPP_
#define _FASTDDS_PUBLISHER_DATAWRITERIMPL_INSTANCE_POOL_HPP_

#include "fastdds/dds/core/policy/QosPolicies.hpp"
#include <algorithm>
#include <cassert>

#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/publisher/history/DataWriterInstance.hpp>
#include <fastrtps/types/TypesBase.h>
#include <fastrtps/utils/collections/ResourceLimitedContainerConfig.hpp>
#include <fastrtps/utils/collections/ResourceLimitedVector.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

struct DataWriterInstancePool
{
    using ReturnCode_t = eprosima::fastrtps::types::ReturnCode_t;

    explicit DataWriterInstancePool(
            const ResourceLimitsQosPolicy& qos)
        : free_items_(qos.max_instances)
        , used_items_(qos.max_instances)
    {
        for (int32_t n = 0; n < qos.max_instances; ++n)
        {
            free_items_.push_back(new DataWriterInstance(qos.max_samples_per_instance));
        }
    }

    ~DataWriterInstancePool()
    {
        for (DataWriterInstance* it : free_items_)
        {
            delete it;
        }
    }

    size_t num_allocated()
    {
        return used_items_.size();
    }

    DataWriterInstance* get_item()
    {
        DataWriterInstance** result = nullptr;

        if (free_items_.empty())
        {
            result = used_items_.push_back(new DataWriterInstance());
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
            DataWriterInstance* item)
    {
        bool removed = used_items_.remove(item);
        static_cast<void>(removed);
        assert(removed);

        DataWriterInstance** result = free_items_.push_back(item);
        static_cast<void>(result);
        assert(result != nullptr);
    }

private:

    using collection_type = eprosima::fastrtps::ResourceLimitedVector<DataWriterInstance*>;

    collection_type free_items_;
    collection_type used_items_;
};

} /* namespace detail */
} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif  // _FASTDDS_PUBLISHER_DATAWRITERIMPL_INSTANCE_POOL_HPP_
