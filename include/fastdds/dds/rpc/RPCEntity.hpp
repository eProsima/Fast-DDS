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

#ifndef FASTDDS_DDS_RPC__RPCENTITY_HPP
#define FASTDDS_DDS_RPC__RPCENTITY_HPP

#include <fastdds/dds/core/detail/DDSReturnCode.hpp>
#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

/**
 * @brief Abstract base class for all RPC Objects
 */
class RPCEntity
{

public:

    /**
     * @brief Enables the entity
     */
    virtual ReturnCode_t enable() = 0;

    /**
     * @brief Disables the entity
     */
    virtual ReturnCode_t close() = 0;

    /**
     * @brief Check if the entity is enabled
     */
    virtual bool is_enabled() const = 0;

protected:

    /**
     * @brief Destructor
     */
    ~RPCEntity() = default;

};

} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_RPC__RPCENTITY_HPP
