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
 * @file Entity.hpp
 *
 */

#ifndef _FASTDDS_ENTITY_HPP_
#define _FASTDDS_ENTITY_HPP_

#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/rtps/common/InstanceHandle.h>
#include <fastrtps/types/TypesBase.h>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * @brief The Entity class is the abstract base class for all the objects that support QoS policies, a listener and
 * a status condition.
 *
 */
class Entity
{
public:

    RTPS_DllAPI Entity(
            const StatusMask& mask = StatusMask::all())
        : status_mask_(mask)
        , status_changes_(StatusMask::none())
        , enable_(false)
    {
    }

    virtual ~Entity() = default;

    /**
     * @brief This operation enables the Entity
     * @return true
     */
    virtual fastrtps::types::ReturnCode_t enable()
    {
        enable_ = true;
        return fastrtps::types::ReturnCode_t::RETCODE_OK;
    }

    /**
     * @brief This operation disables the Entity and frees all resources held.
     */
    virtual void close()
    {
        enable_ = false;
    }

    /**
     * @brief Retrieves the set of statuses for the Entity
     * @return Reference to the StatusMask with the relevant statuses set to 1
     */
    RTPS_DllAPI const StatusMask& get_status_mask() const
    {
        return status_mask_;
    }

    /**
     * @brief retrieves the list of communication statuses that are triggered.
     * @detail Triggered statuses are those whose value has changed since the last
     * time the application read the status
     */
    RTPS_DllAPI const StatusMask& get_status_changes() const
    {
        return status_changes_;
    }

    /**
     * @brief get_instance_handle Retrieves the instance handler that represents the Entity
     * @return Reference to the InstanceHandle
     */
    const fastrtps::rtps::InstanceHandle_t& get_instance_handle() const
    {
        return instance_handle_;
    }

    /**
     * @brief is_enabled Checks if the Entity is enabled
     * @return true if enabled, false if not
     */
    RTPS_DllAPI bool is_enabled() const
    {
        return enable_;
    }

    RTPS_DllAPI bool operator ==(
            const Entity& other) const
    {
        return (this->instance_handle_ == other.instance_handle_);
    }

protected:

    RTPS_DllAPI void set_instance_handle(
            const fastrtps::rtps::InstanceHandle_t& handle)
    {
        instance_handle_ = handle;
    }

    StatusMask status_mask_;

    StatusMask status_changes_;

    fastrtps::rtps::InstanceHandle_t instance_handle_;

    bool enable_;

};

/**
 * @brief The DomainEntity class Subclass of Entity created in order to differentiate between DomainParticipants
 * and the rest of Entities
 */
class DomainEntity : public Entity
{
public:

    RTPS_DllAPI DomainEntity(
            const StatusMask& mask = StatusMask::all())
        : Entity(mask)
    {
    }

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_ENTITY_HPP_
