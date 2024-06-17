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
 * @file IPayloadPool.hpp
 */

#ifndef FASTDDS_RTPS_HISTORY__IPAYLOADPOOL_HPP
#define FASTDDS_RTPS_HISTORY__IPAYLOADPOOL_HPP

#include <cstdint>
#include <memory>

namespace eprosima {
namespace fastdds {
namespace rtps {

struct SerializedPayload_t;

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
     *     @li When a writer creates a new cache change
     *     @li When a reader receives the first fragment of a cache change
     *
     * In both cases, the received @c size will be for the whole serialized payload.
     *
     * @param [in]     size     Number of bytes required for the serialized payload.
     * @param [in,out] payload  Payload of the cache change used in the operation
     *
     * @returns whether the operation succeeded or not
     *
     * @post
     *     @li Field @c payload.payload_owner equals this
     *     @li Field @c payload.data points to a buffer of at least @c size bytes
     *     @li Field @c payload.max_size is greater than or equal to @c size
     */
    virtual bool get_payload(
            uint32_t size,
            SerializedPayload_t& payload) = 0;

    /**
     * @brief Assign a serialized payload to a new sample.
     *
     * This method will usually be called when a reader receives a whole cache change.
     *
     * @param [in,out] data     Serialized payload received
     * @param [in,out] payload  Destination serialized payload
     *
     * @returns whether the operation succeeded or not
     *
     * @note If @c data has no owner, it means it is allocated on the stack of a
     * reception thread, and a copy should be performed. If the ownership of @c data needs to be changed,
     * a consecutive call to this method needs to be performed with the arguments swapped, leveraging the
     * post-condition of this method which ensures that @c payload.payload_owner points to @c this.
     *
     * @post
     *     @li Field @c payload.payload_owner equals this
     *     @li Field @c payload.data points to a buffer of at least @c data.length bytes
     *     @li Field @c payload.length is equal to @c data.length
     *     @li Field @c payload.max_size is greater than or equal to @c data.length
     *     @li Content of @c payload.data is the same as @c data.data
     */
    virtual bool get_payload(
            const SerializedPayload_t& data,
            SerializedPayload_t& payload) = 0;

    /**
     * @brief Release a serialized payload from a sample.
     *
     * This method will be called when a cache change is removed from a history.
     *
     * @param [in,out] payload  Payload to be released
     *
     * @returns whether the operation succeeded or not
     *
     * @pre @li Field @c payload_owner of @c payload equals this
     *
     * @post @li Field @c payload_owner of @c payload is @c nullptr
     */
    virtual bool release_payload(
            SerializedPayload_t& payload) = 0;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima


#endif // FASTDDS_RTPS_HISTORY__IPAYLOADPOOL_HPP
