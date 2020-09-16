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
 * @file IPayloadPool.h
 */

#ifndef _FASTDDS_RTPS_HISTORY_IPAYLOADPOOL_H_
#define _FASTDDS_RTPS_HISTORY_IPAYLOADPOOL_H_

#include <fastdds/rtps/common/CacheChange.h>

#include <cstdint>
#include <functional>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class IPayloadPool
{
public:

    virtual ~IPayloadPool() = default;

    virtual bool get_payload(
            uint32_t size,
            const SampleIdentity& sample_identity,
            CacheChange_t& cache_change) = 0;

    virtual bool get_payload(
            const SerializedPayload_t& data,
            CacheChange_t& cache_change) = 0;

    virtual bool release_payload(
            CacheChange_t& cache_change) = 0;
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */


#endif /* _FASTDDS_RTPS_HISTORY_IPAYLOADPOOL_H_ */
