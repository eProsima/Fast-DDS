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
 * @file RpcFuture.hpp
 */

#ifndef FASTDDS_DDS_RPC_INTERFACES__RPCFUTURE_HPP
#define FASTDDS_DDS_RPC_INTERFACES__RPCFUTURE_HPP

#include <future>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

template <typename T>
using RpcFuture = std::future<T>;

}  // namespace rpc
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_DDS_RPC_INTERFACES__RPCFUTURE_HPP
