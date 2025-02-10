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
 * @file RpcClientWriter.hpp
 */

#ifndef FASTDDS_DDS_RPC_INTERFACES__RPCCLIENTWRITER_HPP
#define FASTDDS_DDS_RPC_INTERFACES__RPCCLIENTWRITER_HPP

#include <fastdds/dds/rpc/interfaces/RpcStatusCode.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

/**
 * An interface for a client-side RPC writer.
 *
 * Would be used to write inputs from a client on an operation with `@feed` annotated inputs.
 */
template <typename T>
class RpcClientWriter
{
public:

    /**
     * Destructor.
     */
    virtual ~RpcClientWriter() noexcept = default;

    /**
     * Copy write operation.
     *
     * Will add a value to the input feed, that would be eventually sent to the server.
     * May block depending on the configured queue sizes in both the client and the server.
     *
     * @param value The value to write to the input feed.
     *
     * @throw RpcBrokenPipeException if the communication with the server breaks.
     * @throw RpcTimeoutException if the operation needs to block for a time longer than the configured timeout.
     * @throw RpcOperationError if the server communicates an error.
     */
    virtual void write(
            const T& value) = 0;

    /**
     * Move write operation.
     *
     * Will add a value to the input feed, that would be eventually sent to the server.
     * May block depending on the configured queue sizes in both the client and the server.
     *
     * @param value The value to write to the input feed.
     *
     * @throw RpcBrokenPipeException if the communication with the server breaks.
     * @throw RpcTimeoutException if the operation needs to block for a time longer than the configured timeout.
     * @throw RpcOperationError if the server communicates an error.
     */
    virtual void write(
            T&& value) = 0;

    /**
     * Marks the end of the input feed.
     *
     * Will indicate to the server that no more inputs will be sent.
     * Specifying a reason different from `RPC_STATUS_CODE_OK` will indicate that the feed was finished due to an
     * error, and that the client will not be expecting a reply.
     *
     * May block depending on the configured queue sizes in both the client and the server.
     *
     * @param reason The status code to indicate the reason for finishing the feed.
     *
     * @throw RpcBrokenPipeException if the communication with the server breaks.
     * @throw RpcTimeoutException if the operation needs to block for a time longer than the configured timeout.
     */
    virtual void finish(
            RpcStatusCode reason = RPC_STATUS_CODE_OK) = 0;

};

}  // namespace rpc
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_DDS_RPC_INTERFACES__RPCCLIENTWRITER_HPP
