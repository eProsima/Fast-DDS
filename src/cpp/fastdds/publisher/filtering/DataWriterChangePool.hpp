// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DataWriterChangePool.hpp
 */

#ifndef _FASTDDS_PUBLISHER_FILTERING_DATAWRITERCHANGEPOOL_HPP_
#define _FASTDDS_PUBLISHER_FILTERING_DATAWRITERCHANGEPOOL_HPP_

#include <fastrtps/utils/collections/ResourceLimitedContainerConfig.hpp>

#include <fastdds/publisher/filtering/DataWriterCacheChange.hpp>
#include <rtps/history/CacheChangePool.h>
#include <rtps/history/PoolConfig.h>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * A CacheChangePool that allocates DataWriterCacheChange objects.
 */
class DataWriterChangePool : public fastrtps::rtps::CacheChangePool
{
public:

    DataWriterChangePool(
            const fastrtps::rtps::PoolConfig& config,
            const fastrtps::ResourceLimitedContainerConfig& filter_allocation)
        : fastrtps::rtps::CacheChangePool()
        , filter_allocation_(filter_allocation)
    {
        init(config);
    }

protected:

    virtual fastrtps::rtps::CacheChange_t* create_change() const
    {
        return new DataWriterCacheChange(filter_allocation_);
    }

    virtual void destroy_change(
            fastrtps::rtps::CacheChange_t* change) const
    {
        DataWriterCacheChange* writer_change = static_cast<DataWriterCacheChange*>(change);
        delete writer_change;
    }

    fastrtps::ResourceLimitedContainerConfig filter_allocation_;
};

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  //_FASTDDS_PUBLISHER_FILTERING_DATAWRITERCHANGEPOOL_HPP_
