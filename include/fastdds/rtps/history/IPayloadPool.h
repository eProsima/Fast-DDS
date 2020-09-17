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
#include <memory>

namespace eprosima {
namespace fastrtps {
namespace rtps {

/**
 * An interface for classes responsible of serialized payload management.
 */
class IPayloadPool
{
public:

    virtual ~IPayloadPool() = default;

    /**
     * @brief Get a serialized payload for a new sample.
     *
     * This method will usually be called in one of the following situations:
     * @li When a writer creates a new cache change
     * @li When a reader receives the first fragment of a cache change
     *
     * In both cases, the received @c size will be for the whole serialized payload.
     *
     * @param [in]     size          Number of bytes required for the serialized payload
     * @param [in,out] cache_change  Cache change to assign the payload to
     *
     * @returns whether the operation succeeded or not
     *
     * @post
     *     @li Field @c payload_owner of @c cache_change equals this
     *     @li Field @c serializedPayload.data points to a buffer of at least @c size bytes
     *     @li Field @c serializedPayload.max_size is greater than or equal to @c size
     */
    virtual bool get_payload(
            uint32_t size,
            CacheChange_t& cache_change) = 0;

    /**
     * @brief Assign a serialized payload to a new sample.
     *
     * This method will usually be called when a reader receives a whole cache change.
     *
     * @param [in]     data          Serialized payload received
     * @param [in]     data_owner    Payload pool owning incoming data
     * @param [in,out] cache_change  Cache change to assign the payload to
     *
     * @returns whether the operation succeeded or not
     *
     * @post
     *     @li Field @c payload_owner of @c cache_change equals this
     *     @li Field @c serializedPayload.data points to a buffer of at least @c data.length bytes
     *     @li Field @c serializedPayload.max_size is greater than or equal to @c data.length
     */
    virtual bool get_payload(
            const SerializedPayload_t& data,
            const IPayloadPool* data_owner,
            CacheChange_t& cache_change) = 0;

    /**
     * @brief Release a serialized payload from a sample.
     *
     * This method will be called when a cache change is removed from a history.
     *
     * @param [in,out] cache_change  Cache change to assign the payload to
     *
     * @returns whether the operation succeeded or not
     *
     * @pre @li Field @c payload_owner of @c cache_change equals this
     *
     * @post @li Field @c payload_owner of @c cache_change is @c nullptr
     */
    virtual bool release_payload(
            CacheChange_t& cache_change) = 0;
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */


#endif /* _FASTDDS_RTPS_HISTORY_IPAYLOADPOOL_H_ */
