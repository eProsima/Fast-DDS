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
 * @file RemoteUnknownExceptionError.hpp
 */

#ifndef FASTDDS_DDS_RPC_EXCEPTIONS__REMOTEUNKNOWNEXCEPTIONERROR_HPP
#define FASTDDS_DDS_RPC_EXCEPTIONS__REMOTEUNKNOWNEXCEPTIONERROR_HPP

#include <fastdds/fastdds_dll.hpp>
#include <fastdds/dds/rpc/exceptions/RpcRemoteException.hpp>
#include <fastdds/dds/rpc/RemoteExceptionCode_t.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

/**
 * Exception thrown by the RPC API when the service implementation raises an unspecified exception.
 */
class RemoteUnknownExceptionError : public RpcRemoteException
{

public:

    /**
     * Constructor.
     */
    RemoteUnknownExceptionError()
        : RemoteUnknownExceptionError("A generic, unspecified exception was raised by the service implementation")
    {
    }

    /**
     * Constructor with custom message.
     *
     * @param msg The exception message.
     */
    RemoteUnknownExceptionError(
            const char* msg)
        : RpcRemoteException(RemoteExceptionCode_t::REMOTE_EX_UNKNOWN_EXCEPTION, msg)
    {
    }

    /**
     * Copy constructor.
     */
    RemoteUnknownExceptionError(
            const RemoteUnknownExceptionError& other) noexcept = default;

    /**
     * Copy assignment.
     */
    RemoteUnknownExceptionError& operator =(
            const RemoteUnknownExceptionError& other) noexcept = default;

    /**
     * Destructor.
     */
    virtual ~RemoteUnknownExceptionError() noexcept = default;

};

}  // namespace rpc
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_DDS_RPC_EXCEPTIONS__REMOTEUNKNOWNEXCEPTIONERROR_HPP
