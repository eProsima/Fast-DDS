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

#ifndef FASTDDS_DDS_RPC__SERVICE_HPP
#define FASTDDS_DDS_RPC__SERVICE_HPP

#include <string>

#include <fastdds/dds/rpc/RPCEntity.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class DomainParticipant;

} // namespace dds

namespace dds {
namespace rpc {

/**
 * @brief Base class for a Service in the RPC communication
 */
class Service : public RPCEntity
{

public:

    /**
     * @brief Getter for the service name
     */
    virtual const std::string& get_service_name() const = 0;

    /**
     * @brief Getter for the service type name
     */
    virtual const std::string& get_service_type_name() const = 0;

protected:

    /**
     * @brief Destructor
     */
    ~Service() = default;

};

} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_RPC__SERVICE_HPP
