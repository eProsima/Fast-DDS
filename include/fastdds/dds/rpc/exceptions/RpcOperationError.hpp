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
 * @file RpcOperationError.hpp
 */

#ifndef FASTDDS_DDS_RPC_EXCEPTIONS__RPCOPERATIONERROR_HPP
#define FASTDDS_DDS_RPC_EXCEPTIONS__RPCOPERATIONERROR_HPP

#include <string>

#include <fastdds/fastdds_dll.hpp>
#include <fastdds/dds/rpc/exceptions/RpcException.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

/**
 * Base class for exceptions thrown by the RPC API when the server communicates an error.
 */
class RpcOperationError : public RpcException
{

public:

    /**
     * Constructor.
     */
    RpcOperationError(
            const std::string& message)
        : RpcException(message)
    {
    }

    /**
     * Constructor.
     */
    RpcOperationError(
            const char* message)
        : RpcException(message)
    {
    }

    /**
     * Copy constructor.
     */
    RpcOperationError(
            const RpcOperationError& other) noexcept = default;

    /**
     * Copy assignment.
     */
    RpcOperationError& operator =(
            const RpcOperationError& other) noexcept = default;

    /**
     * Destructor.
     */
    virtual ~RpcOperationError() noexcept = default;

};

}  // namespace rpc
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_DDS_RPC_EXCEPTIONS__RPCOPERATIONERROR_HPP
