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

#ifndef FASTDDS_DDS_RPC__REMOTEEXCEPTIONCODE_T_HPP
#define FASTDDS_DDS_RPC__REMOTEEXCEPTIONCODE_T_HPP

#include <cstdint>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

/**
 * Enumeration of possible error codes that can be returned by a remote service.
 * Extracted from DDS-RPC v1.0 - 7.5.2 Mapping of Error Codes.
 */
enum class RemoteExceptionCode_t : int32_t
{
    /// The request was executed successfully.
    REMOTE_EX_OK,
    /// Operation is valid but it is unsupported (a.k.a. not implemented).
    REMOTE_EX_UNSUPPORTED,
    /// The value of a parameter passed has an illegal value.
    REMOTE_EX_INVALID_ARGUMENT,
    /// The remote service ran out of resources while processing the request.
    REMOTE_EX_OUT_OF_RESOURCES,
    /// The operation called is unknown.
    REMOTE_EX_UNKNOWN_OPERATION,
    /// A generic, unspecified exception was raised by the service implementation.
    REMOTE_EX_UNKNOWN_EXCEPTION
};

} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_RPC__REMOTEEXCEPTIONCODE_T_HPP
