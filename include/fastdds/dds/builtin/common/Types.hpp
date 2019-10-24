// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file Types.hpp
 *
 */

#ifndef TYPES_HPP
#define TYPES_HPP

#include <fastrtps/utils/fixed_size_string.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

using InstanceName = fastrtps::string_255;

enum RemoteExceptionCode_t {
    REMOTE_EX_OK,
    REMOTE_EX_UNSUPPORTED,
    REMOTE_EX_INVALID_ARGUMENT,
    REMOTE_EX_OUT_OF_RESOURCES,
    REMOTE_EX_UNKNOWN_OPERATION,
    REMOTE_EX_UNKNOWN_EXCEPTION
};

} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // TYPES_HPP
