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
 * @file RemoteInvalidArgumentError.hpp
 */

#ifndef FASTDDS_DDS_RPC_EXCEPTIONS__REMOTEINVALIDARGUMENTERROR_HPP
#define FASTDDS_DDS_RPC_EXCEPTIONS__REMOTEINVALIDARGUMENTERROR_HPP

#include <fastdds/fastdds_dll.hpp>
#include <fastdds/dds/rpc/exceptions/RpcRemoteException.hpp>
#include <fastdds/dds/rpc/RemoteExceptionCode_t.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

/**
 * Exception thrown by the RPC API when the server reports invalid arguments.
 */
class RemoteInvalidArgumentError : public RpcRemoteException
{

public:

    /**
     * Constructor.
     */
    RemoteInvalidArgumentError()
        : RemoteInvalidArgumentError("The value of a parameter passed has an illegal value")
    {
    }

    /**
     * Constructor with custom message.
     *
     * @param msg The exception message.
     */
    RemoteInvalidArgumentError(
            const char* msg)
        : RpcRemoteException(RemoteExceptionCode_t::REMOTE_EX_INVALID_ARGUMENT, msg)
    {
    }

    /**
     * Copy constructor.
     */
    RemoteInvalidArgumentError(
            const RemoteInvalidArgumentError& other) noexcept = default;

    /**
     * Copy assignment.
     */
    RemoteInvalidArgumentError& operator =(
            const RemoteInvalidArgumentError& other) noexcept = default;

    /**
     * Destructor.
     */
    virtual ~RemoteInvalidArgumentError() noexcept = default;

};

}  // namespace rpc
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_DDS_RPC_EXCEPTIONS__REMOTEINVALIDARGUMENTERROR_HPP
