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
 * @file RpcStatusCode.hpp
 */

#ifndef FASTDDS_DDS_RPC_INTERFACES__RPCSTATUSCODE_HPP
#define FASTDDS_DDS_RPC_INTERFACES__RPCSTATUSCODE_HPP

#include <cstdint>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

/**
 * Status codes for RPC operations.
 */
using RpcStatusCode = uint32_t;

/**
 * Status code for successful operations.
 */
constexpr RpcStatusCode RPC_STATUS_CODE_OK = 0;

}  // namespace rpc
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_DDS_RPC_INTERFACES__RPCSTATUSCODE_HPP
