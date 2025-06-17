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
 * @file RpcServerSchedulingStrategy.hpp
 */

#ifndef FASTDDS_DDS_RPC_INTERFACES__RPCSERVERSCHEDULINGSTRATEGY_HPP
#define FASTDDS_DDS_RPC_INTERFACES__RPCSERVERSCHEDULINGSTRATEGY_HPP

#include <memory>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

class RpcRequest;
class RpcServer;

/**
 * An interface with generic RPC requests functionality.
 */
class RpcServerSchedulingStrategy
{
public:

    /**
     * Destructor.
     */
    virtual ~RpcServerSchedulingStrategy() noexcept = default;

    /**
     * @brief Schedule a request for processing.
     *
     * This method is called when a request is received and should be processed by the server.
     * The implementation should decide how to handle the request, whether to process it immediately,
     * or to queue it for later processing.
     *
     * A call to server->execute_request(request) should eventually be made to process the request.
     *
     * @param request  The request to schedule.
     * @param server   The server instance that should process the request.
     */
    virtual void schedule_request(
            const std::shared_ptr<RpcRequest>& request,
            const std::shared_ptr<RpcServer>& server) = 0;

    /**
     * @brief Informs that a server has been stopped and all its requests have been cancelled.
     *
     * @param server  The server instance that has been stopped.
     */
    virtual void server_stopped(
            const std::shared_ptr<RpcServer>& server) = 0;

};

}  // namespace rpc
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_DDS_RPC_INTERFACES__RPCSERVERSCHEDULINGSTRATEGY_HPP
