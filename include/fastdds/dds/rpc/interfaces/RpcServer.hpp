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
 * @file RpcServer.hpp
 */

#ifndef FASTDDS_DDS_RPC_INTERFACES__RPCSERVER_HPP
#define FASTDDS_DDS_RPC_INTERFACES__RPCSERVER_HPP

#include <memory>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

// Forward declaration of RpcRequest class.
class RpcRequest;

/**
 * An interface with generic RPC server functionality.
 */
class RpcServer
{
public:

    /**
     * Destructor.
     */
    virtual ~RpcServer() noexcept = default;

    /**
     * @brief Run the server.
     *
     * This method starts the server and begins processing requests.
     * The method will block until the server is stopped.
     */
    virtual void run() = 0;

    /**
     * @brief Stop the server.
     *
     * This method stops the server and releases all resources.
     * It will cancel all pending requests, and wait for all processing threads to finish before returning.
     */
    virtual void stop() = 0;

    /**
     * @brief Perform execution of a client request.
     *
     * @param request The client request to execute.
     */
    virtual void execute_request(
            const std::shared_ptr<RpcRequest>& request) = 0;

};

}  // namespace rpc
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_DDS_RPC_INTERFACES__RPCSERVER_HPP
