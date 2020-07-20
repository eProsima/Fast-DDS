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

#ifndef _FASTDDS_RTPS_ICHANGEPOOL_H_
#define _FASTDDS_RTPS_ICHANGEPOOL_H_

namespace eprosima {
namespace fastrtps {
namespace rtps {

struct CacheChange_t;

class IChangePool
{
public:

    virtual ~IChangePool() = default;

    virtual bool reserve_cache(
            CacheChange_t*& cache_change) = 0;

    virtual bool release_cache(
            CacheChange_t* cache_change) = 0;
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */


#endif /* _FASTDDS_RTPS_ICHANGEPOOL_H_ */
