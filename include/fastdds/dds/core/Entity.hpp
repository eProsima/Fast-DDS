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
#include <fastdds/dds/core/conditions/StatusCondition.hpp>
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
class RTPS_DllAPI Entity
{
public:

    Entity(
            const ::dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::all())
        : status_condition_(this)
    {
        status_condition_.set_enabled_statuses(mask);
    }

    fastrtps::types::ReturnCode_t enable()
    {
        enable_ = true;
        return fastrtps::types::ReturnCode_t::RETCODE_OK;
    }

    void close()
    {
        enable_ = false;
    }

    const StatusCondition& get_statuscondition() const
    {
        return status_condition_;
    }

    const ::dds::core::status::StatusMask& get_status_changes() const
    {
        return status_condition_.get_statuses();
    }

    const fastrtps::rtps::InstanceHandle_t& get_instance_handle() const
    {
        return instance_handle_;
    }

    bool operator ==(
            const Entity& other) const
    {
        return (this->instance_handle_ == other.instance_handle_);
    }

protected:

    void set_instance_handle(
            const fastrtps::rtps::InstanceHandle_t& handle)
    {
        instance_handle_ = handle;
    }

    StatusCondition status_condition_;

    fastrtps::rtps::InstanceHandle_t instance_handle_;

    bool enable_;

};

class RTPS_DllAPI DomainEntity : public Entity
{
public:
    DomainEntity(
            const ::dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::all())
        : Entity(mask)
    {
    }

    fastrtps::types::ReturnCode_t enable()
    {
        return Entity::enable();
    }

    void close()
    {
        Entity::close();
    }

    const StatusCondition& get_statuscondition() const
    {
        return Entity::get_statuscondition();
    }

    const ::dds::core::status::StatusMask& get_status_changes() const
    {
        return Entity::get_status_changes();
    }

    const fastrtps::rtps::InstanceHandle_t& get_instance_handle() const
    {
        return Entity::get_instance_handle();
    }

    bool operator ==(
            const Entity& other) const
    {
        return Entity::operator ==(other);
    }

protected:

    void set_instance_handle(
            const fastrtps::rtps::InstanceHandle_t& handle)
    {
        Entity::set_instance_handle(handle);
    }
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_ENTITY_HPP_
