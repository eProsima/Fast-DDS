// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file RpcServerReader.hpp
 */

#ifndef FASTDDS_DDS_RPC_INTERFACES__RPCSERVERREADER_HPP
#define FASTDDS_DDS_RPC_INTERFACES__RPCSERVERREADER_HPP

#include <fastdds/dds/core/Time_t.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

/**
 * An interface for a server-side RPC reader.
 *
 * Would be used to read inputs from a client on an operation with `@feed` annotated inputs.
 */
template <typename T>
class RpcServerReader
{
public:

    /**
     * Destructor.
     */
    virtual ~RpcServerReader() noexcept = default;

    /**
     * Blocking read operation.
     *
     * Will block until an input is available or the input feed has finished.
     *
     * @param value The value to read the input into.
     *
     * @return True if a value was read, false if the feed has finished.
     *
     * @throw RpcBrokenPipeException if the communication with the client breaks.
     * @throw RpcFeedCancelledException if the client cancels the input feed.
     */
    virtual bool read(
            T& value) = 0;

    /**
     * Blocking read operation with timeout.
     *
     * Will block until an input is available, the input feed has finished, or the timeout expires.
     *
     * @param value The value to read the input into.
     * @param timeout The maximum time to wait for an input.
     *
     * @return True if a value was read, false if the feed has finished.
     *
     * @throw RpcTimeoutException if the timeout expires.
     * @throw RpcBrokenPipeException if the communication with the client breaks.
     * @throw RpcFeedCancelledException if the client cancels the input feed.
     */
    virtual bool read(
            T& value,
            const eprosima::fastdds::dds::Duration_t& timeout) = 0;

};

}  // namespace rpc
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_DDS_RPC_INTERFACES__RPCSERVERREADER_HPP
