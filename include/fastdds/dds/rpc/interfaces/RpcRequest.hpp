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
 * @file RpcRequest.hpp
 */

#ifndef FASTDDS_DDS_RPC_INTERFACES__RPCREQUEST_HPP
#define FASTDDS_DDS_RPC_INTERFACES__RPCREQUEST_HPP

#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/RemoteLocators.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

/**
 * An interface with generic RPC requests functionality.
 */
class RpcRequest
{
public:

    /**
     * Destructor.
     */
    virtual ~RpcRequest() noexcept = default;

    /**
     * @brief Get the GUID of the client that made the request.
     *
     * @return The GUID of the client that made the request.
     */
    virtual const eprosima::fastdds::rtps::GUID_t& get_client_id() const = 0;

    /**
     * @brief Get the locators of the client that made the request.
     *
     * @return The locators of the client that made the request.
     */
    virtual const eprosima::fastdds::rtps::RemoteLocatorList& get_client_locators() const = 0;

};

}  // namespace rpc
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_DDS_RPC_INTERFACES__RPCREQUEST_HPP
