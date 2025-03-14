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
 * @file RpcRemoteException.hpp
 */

#ifndef FASTDDS_DDS_RPC_EXCEPTIONS__RPCREMOTEEXCEPTION_HPP
#define FASTDDS_DDS_RPC_EXCEPTIONS__RPCREMOTEEXCEPTION_HPP

#include <fastdds/fastdds_dll.hpp>
#include <fastdds/dds/rpc/RemoteExceptionCode_t.hpp>
#include <fastdds/dds/rpc/exceptions/RpcException.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

/**
 * Base class for exceptions that map to a RpcExceptionCode_t.
 */
class RpcRemoteException : public RpcException
{

public:

    /**
     * Constructor.
     */
    RpcRemoteException(
            RemoteExceptionCode_t code,
            const char* msg)
        : RpcException(msg)
        , code_(code)
    {
    }

    /**
     * Copy constructor.
     */
    RpcRemoteException(
            const RpcRemoteException& other) noexcept = default;

    /**
     * Copy assignment.
     */
    RpcRemoteException& operator =(
            const RpcRemoteException& other) noexcept = default;

    /**
     * Destructor.
     */
    virtual ~RpcRemoteException() noexcept = default;

    /**
     * Get the exception code.
     *
     * @return The exception code.
     */
    RemoteExceptionCode_t code() const noexcept
    {
        return code_;
    }

private:

    /// The exception code.
    RemoteExceptionCode_t code_;

};

}  // namespace rpc
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_DDS_RPC_EXCEPTIONS__RPCREMOTEEXCEPTION_HPP
