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

#include <dds/core/status/Status.hpp>
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
            const ::dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::all())
        : status_mask_(mask)
        , enable_(false)
    {
    }

    // TODO Implement StatusCondition
    // virtual const StatusCondition& get_statuscondition() const = 0;

    fastrtps::types::ReturnCode_t enable()
    {
        enable_ = true;
        return fastrtps::types::ReturnCode_t::RETCODE_OK;
    }

    void close()
    {
        enable_ = false;
    }

    const ::dds::core::status::StatusMask& get_status_mask() const
    {
        return status_mask_;
    }

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
        if (enable_)
        {
            return true;
        }
        return false;
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

    ::dds::core::status::StatusMask status_mask_;
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
            const ::dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::all())
        : Entity(mask)
    {
    }

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_ENTITY_HPP_
