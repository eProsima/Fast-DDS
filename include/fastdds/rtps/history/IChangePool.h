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
 * @file IChangePool.h
 */

#ifndef _FASTDDS_RTPS_HISTORY_ICHANGEPOOL_H_
#define _FASTDDS_RTPS_HISTORY_ICHANGEPOOL_H_

namespace eprosima {
namespace fastrtps {
namespace rtps {

struct CacheChange_t;

/**
 * An interface for classes responsible of cache changes allocation management.
 */
class IChangePool
{
public:

    virtual ~IChangePool() = default;

    /**
     * @brief Get a new cache change from the pool
     *
     * @param [out] cache_change   Pointer to the new cache change.
     *
     * @returns whether the operation succeeded or not
     *
     * @pre @c cache_change is @c nullptr
     *
     * @post
     *     @li @c cache_change is not nullptr
     *     @li @c *cache_change equals @c CacheChange_t() except for the contents of @c serializedPayload
     */
    virtual bool reserve_cache(
            CacheChange_t*& cache_change) = 0;

    /**
     * @brief Return a cache change to the pool
     *
     * @param [in] cache_change   Pointer to the cache change to release.
     *
     * @returns whether the operation succeeded or not
     *
     * @pre
     *     @li @c cache_change is not @c nullptr
     *     @li @c cache_change points to a cache change obtained from a call to @c this->reserve_cache
     */
    virtual bool release_cache(
            CacheChange_t* cache_change) = 0;
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */


#endif /* _FASTDDS_RTPS_HISTORY_ICHANGEPOOL_H_ */
