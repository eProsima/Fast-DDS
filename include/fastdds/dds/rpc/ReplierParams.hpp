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

#ifndef FASTDDS_DDS_RPC__REPLIERPARAMS_HPP
#define FASTDDS_DDS_RPC__REPLIERPARAMS_HPP

#include <fastdds/dds/domain/qos/ReplierQos.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

/**
 * @brief Class that contains the parameters to create a Replier
 * @note Service must validate these parameters before creating the Replier
 */
class ReplierParams
{

public:
    
    /**
     * @brief Constructor
     * 
     */
    ReplierParams() = default;

    /**
     * @brief Destructor
     * 
     */
    ~ReplierParams() = default;

    // TODO (Carlosespicur) Define == and = operators?

    /**
     * @brief Getter for Replier QoS
     */
    const ReplierQos& qos() const
    {
        return qos_;
    }

    /**
     * @brief Setter for Replier QoS
     */
    ReplierQos& qos()
    {
        return qos_;
    }

    /**
     * @brief Setter for Replier QoS
     */
    void qos(
            const ReplierQos& qos)
    {
        qos_ = qos;
    }

private:

    ReplierQos qos_;

};
} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_RPC__REPLIERPARAMS_HPP