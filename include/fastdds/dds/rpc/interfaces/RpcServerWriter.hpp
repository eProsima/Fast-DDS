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
 * @file RpcServerWriter.hpp
 */

#ifndef FASTDDS_DDS_RPC_INTERFACES__RPCSERVERWRITER_HPP
#define FASTDDS_DDS_RPC_INTERFACES__RPCSERVERWRITER_HPP

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

/**
 * An interface for a server-side RPC writer.
 *
 * Would be used to write replies from a server on an operation with a `@feed` annotated return type.
 */
template <typename T>
class RpcServerWriter
{
public:

    /**
     * Destructor.
     */
    virtual ~RpcServerWriter() noexcept = default;

    /**
     * Copy write operation.
     *
     * Will add a value to the replies feed, that would be eventually sent to the client.
     * May block depending on the configured queue sizes in both the client and the server.
     *
     * @param value The value to write to the replies feed.
     *
     * @throw RpcBrokenPipeException if the communication with the client breaks.
     * @throw RpcTimeoutException if the operation needs to block for a time longer than the configured timeout.
     * @throw RpcFeedCancelledException if the client cancels the output feed.
     */
    virtual void write(
            const T& value) = 0;

    /**
     * Move write operation.
     *
     * Will add a value to the replies feed, that would be eventually sent to the client.
     * May block depending on the configured queue sizes in both the client and the server.
     *
     * @param value The value to write to the replies feed.
     *
     * @throw RpcBrokenPipeException if the communication with the client breaks.
     * @throw RpcTimeoutException if the operation needs to block for a time longer than the configured timeout.
     * @throw RpcFeedCancelledException if the client cancels the output feed.
     */
    virtual void write(
            T&& value) = 0;

};

}  // namespace rpc
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_DDS_RPC_INTERFACES__RPCSERVERWRITER_HPP
