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
 * @file RpcException.hpp
 */

#ifndef FASTDDS_DDS_RPC_EXCEPTIONS__RPCEXCEPTION_HPP
#define FASTDDS_DDS_RPC_EXCEPTIONS__RPCEXCEPTION_HPP

#include <stdexcept>
#include <string>

#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

/**
 * Base class for all exceptions thrown by the RPC API.
 */
class RpcException
{

public:

    /**
     * Constructor.
     *
     * @param message The exception message.
     */
    explicit RpcException(
            const std::string& message)
        : logic_error_(message)
    {
    }

    /**
     * Constructor.
     *
     * @param message The exception message.
     */
    explicit RpcException(
            const char* message)
        : logic_error_(message)
    {
    }

    /**
     * Copy constructor.
     */
    RpcException(
            const RpcException& other) noexcept = default;

    /**
     * Copy assignment.
     */
    RpcException& operator =(
            const RpcException& other) noexcept = default;

    /**
     * Destructor.
     */
    virtual ~RpcException() noexcept = default;

    /**
     * Returns the explanatory string.
     */
    const char* what() const noexcept
    {
        return logic_error_.what();
    }

private:

    std::logic_error logic_error_;

};

}  // namespace rpc
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_DDS_RPC_EXCEPTIONS__RPCEXCEPTION_HPP
