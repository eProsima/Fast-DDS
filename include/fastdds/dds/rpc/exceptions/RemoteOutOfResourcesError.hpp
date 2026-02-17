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
 * @file RemoteOutOfResourcesError.hpp
 */

#ifndef FASTDDS_DDS_RPC_EXCEPTIONS__REMOTEOUTOFRESOURCESERROR_HPP
#define FASTDDS_DDS_RPC_EXCEPTIONS__REMOTEOUTOFRESOURCESERROR_HPP

#include <fastdds/fastdds_dll.hpp>
#include <fastdds/dds/rpc/exceptions/RpcRemoteException.hpp>
#include <fastdds/dds/rpc/RemoteExceptionCode_t.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

/**
 * Exception thrown by the RPC API when the server reports out of resources.
 */
class RemoteOutOfResourcesError : public RpcRemoteException
{

public:

    /**
     * Constructor.
     */
    RemoteOutOfResourcesError()
        : RemoteOutOfResourcesError("The remote service ran out of resources while processing the request")
    {
    }

    /**
     * Constructor with custom message.
     *
     * @param msg The exception message.
     */
    RemoteOutOfResourcesError(
            const char* msg)
        : RpcRemoteException(RemoteExceptionCode_t::REMOTE_EX_OUT_OF_RESOURCES, msg)
    {
    }

    /**
     * Copy constructor.
     */
    RemoteOutOfResourcesError(
            const RemoteOutOfResourcesError& other) noexcept = default;

    /**
     * Copy assignment.
     */
    RemoteOutOfResourcesError& operator =(
            const RemoteOutOfResourcesError& other) noexcept = default;

    /**
     * Destructor.
     */
    virtual ~RemoteOutOfResourcesError() noexcept = default;

};

}  // namespace rpc
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_DDS_RPC_EXCEPTIONS__REMOTEOUTOFRESOURCESERROR_HPP
