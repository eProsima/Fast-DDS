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
 * @file CacheChangePool.h
 */

#ifndef RTPS_HISTORY_CACHECHANGEPOOL_H_
#define RTPS_HISTORY_CACHECHANGEPOOL_H_

#include <fastdds/rtps/common/CacheChange.h>
#include <fastdds/rtps/history/IChangePool.h>
#include <fastdds/rtps/resources/ResourceManagement.h>

#include <rtps/history/PoolConfig.h>

#include <vector>
#include <algorithm>
#include <cstdint>
#include <cstddef>

namespace eprosima {
namespace fastrtps {
namespace rtps {

/**
 * Class CacheChangePool, used by the HistoryCache to pre-reserve a number of CacheChange_t to avoid dynamically
 * reserving memory in the middle of execution loops.
 * @ingroup COMMON_MODULE
 */
class CacheChangePool : public IChangePool
{
public:

    virtual ~CacheChangePool();

    /**
     * Constructor.
     * @param config   Pool configuration (member @c payload_initial_size is not being used).
     * @param f        Functor to be called on all preallocated elements.
     */
    template<class UnaryFunction>
    CacheChangePool(
            const PoolConfig& config,
            UnaryFunction f)
        : CacheChangePool(config)
    {
        std::for_each(all_caches_.begin(), all_caches_.end(), f);
    }

    /**
     * Constructor.
     * @param config   Pool configuration (member @c payload_initial_size is not being used).
     */
    CacheChangePool(
            const PoolConfig& config);

    bool reserve_cache(
            CacheChange_t*& cache_change) override;

    bool release_cache(
            CacheChange_t* cache_change) override;

    //!Get the size of the cache vector; all of them (reserved and not reserved).
    size_t get_allCachesSize()
    {
        return all_caches_.size();
    }

    //!Get the number of free caches.
    size_t get_freeCachesSize()
    {
        return free_caches_.size();
    }

private:

    uint32_t current_pool_size_ = 0;
    uint32_t max_pool_size_ = 0;
    MemoryManagementPolicy_t memory_mode_ = MemoryManagementPolicy_t::DYNAMIC_RESERVE_MEMORY_MODE;

    std::vector<CacheChange_t*> free_caches_;
    std::vector<CacheChange_t*> all_caches_;

    bool allocateGroup(
            uint32_t num_caches);

    CacheChange_t* allocateSingle();

    //! Returns a CacheChange to the free caches pool
    void return_cache_to_pool(
            CacheChange_t* ch);

};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif /* RTPS_HISTORY_CACHECHANGEPOOL_H_ */
