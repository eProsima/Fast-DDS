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
 * @file StatusCondition.hpp
 *
 */

#ifndef _FASTDDS_STATUSCONDITION_HPP_
#define _FASTDDS_STATUSCONDITION_HPP_

#include <fastrtps/fastrtps_all.h>
#include <fastdds/dds/core/conditions/Condition.hpp>
#include <fastdds/rtps/common/Types.h>
#include <dds/core/status/State.hpp>

#include <future>

using eprosima::fastrtps::types::ReturnCode_t;

namespace eprosima {
namespace fastdds {
namespace dds {

class Entity;

/**
 * @brief The StatusCondition class
 */
class RTPS_DllAPI StatusCondition : public Condition
{
public:

    RTPS_DllAPI StatusCondition(
            Entity* entity);


    RTPS_DllAPI StatusCondition(
            Entity* entity,
            std::function<void()> functor);

    /**
     * @brief set_enabled_statuses Change the set of status enabled for an entity.
     * This operation may change the trigger_value
     * @param mask New set of enabled statuses
     * @return true
     */
    RTPS_DllAPI ReturnCode_t set_enabled_statuses(
            const ::dds::core::status::StatusMask& mask);

    /**
     * @brief get_enabled_statuses Retrieves the set of statuses that are taken into account to change the
     * trigger_value
     * @return StatusMask with the relevant statuses set to 1
     */
    RTPS_DllAPI const ::dds::core::status::StatusMask& get_enabled_statuses() const;


    /**
     * @brief get_triggered_status Retrieves the list of statuses that are triggered since the last time the
     * application read it
     * @return StatusMask with the triggered statuses set to 1
     */
    RTPS_DllAPI const ::dds::core::status::StatusMask& get_triggered_status() const;

    /**
     * @brief get_entity Retrieves the entity asociated with the StatusCondition
     * @return Pointer to the entity
     */
    RTPS_DllAPI Entity* get_entity();

    /**
     * @brief notify_status_change Notifies to the StatusCondition of a status change within the application
     * @param mask StatusMask that indicates which status is changed
     */
    RTPS_DllAPI void notify_status_change(
            const ::dds::core::status::StatusMask& mask);

    /**
     * @brief set_status_as_read Notifies to the StatusCondition that the user already manage the change related
     * to a concrete status
     * @param mask StatusMask that indicated which status is managed
     */
    RTPS_DllAPI void set_status_as_read(
            const ::dds::core::status::StatusMask& mask);

    RTPS_DllAPI inline bool operator ==(
            StatusCondition* obj) const;

    RTPS_DllAPI bool operator ==(
            Condition* obj) const override;

private:

    RTPS_DllAPI void set_trigger_value(
            bool value);

    //!Associated Entity
    Entity* entity_;

    //!Set of statuses relevant to the StatusCondition
    ::dds::core::status::StatusMask status_mask_;

    //!Set of statuses triggered
    ::dds::core::status::StatusMask status_change_flag_;

};

} //namespace dds
} //namespace fastdds
} //namespace eprosima

#endif /* _FASTDDS_STATUSCONDITION_HPP_ */
