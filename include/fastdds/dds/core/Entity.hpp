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

#ifndef FASTDDS_DDS_CORE__ENTITY_HPP
#define FASTDDS_DDS_CORE__ENTITY_HPP

#include <fastdds/dds/common/InstanceHandle.hpp>
#include <fastdds/dds/core/condition/StatusCondition.hpp>
#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>

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

    /**
     * @brief Constructor
     *
     * @param mask StatusMask (default: all)
     */
    FASTDDS_EXPORTED_API Entity(
            const StatusMask& mask = StatusMask::all())
        : status_mask_(mask)
        , status_condition_(this)
        , enable_(false)
    {
    }

    /**
     * @brief This operation enables the Entity
     *
     * @return RETCODE_OK
     */
    virtual ReturnCode_t enable()
    {
        enable_ = true;
        return RETCODE_OK;
    }

    /**
     * @brief This operation disables the Entity before closing it
     */
    void close()
    {
        enable_ = false;
    }

    /**
     * @brief Retrieves the set of relevant statuses for the Entity
     *
     * @return Reference to the StatusMask with the relevant statuses set to 1
     */
    FASTDDS_EXPORTED_API const StatusMask& get_status_mask() const
    {
        return status_mask_;
    }

    /**
     * @brief Retrieves the set of triggered statuses in the Entity
     *
     * Triggered statuses are the ones whose value has changed
     * since the last time the application read the status.
     * When the entity is first created or if the entity is not enabled,
     * all communication statuses are in the non-triggered state,
     * so the list returned by the get_status_changes operation will be empty.
     * The list of statuses returned by the get_status_changes operation
     * refers to the status that are triggered on the Entity itself
     * and does not include statuses that apply to contained entities.
     *
     * @return const reference to the StatusMask with the triggered statuses set to 1
     */
    FASTDDS_EXPORTED_API const StatusMask& get_status_changes() const;

    /**
     * @brief Retrieves the instance handler that represents the Entity
     *
     * @return Reference to the InstanceHandle
     */
    const InstanceHandle_t& get_instance_handle() const
    {
        return instance_handle_;
    }

    /**
     * @brief Checks if the Entity is enabled
     *
     * @return true if enabled, false if not
     */
    FASTDDS_EXPORTED_API bool is_enabled() const
    {
        return enable_;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const Entity& other) const
    {
        return (this->instance_handle_ == other.instance_handle_);
    }

    /**
     * @brief Allows access to the StatusCondition associated with the Entity
     *
     * @return Reference to StatusCondition object
     */
    FASTDDS_EXPORTED_API StatusCondition& get_statuscondition()
    {
        return status_condition_;
    }

    /**
     * @brief Allows access to the StatusCondition associated with the Entity
     *
     * @return Const Reference to StatusCondition object
     */
    FASTDDS_EXPORTED_API const StatusCondition& get_statuscondition() const
    {
        return status_condition_;
    }

protected:

    /**
     * @brief Setter for the Instance Handle
     *
     * @param handle Instance Handle
     */
    FASTDDS_EXPORTED_API void set_instance_handle(
            const InstanceHandle_t& handle)
    {
        instance_handle_ = handle;
    }

    //! StatusMask with relevant statuses set to 1
    StatusMask status_mask_;

    //! Condition associated to the Entity
    StatusCondition status_condition_;

    //! InstanceHandle associated to the Entity
    InstanceHandle_t instance_handle_;

    //! Boolean that states if the Entity is enabled or disabled
    bool enable_;
};

/**
 * @brief The DomainEntity class is a subclass of Entity created in order to differentiate between DomainParticipants
 * and the rest of Entities
 */
class DomainEntity : public Entity
{
public:

    /**
     * @brief Constructor
     *
     * @param mask StatusMask (default: all)
     */
    DomainEntity(
            const StatusMask& mask = StatusMask::all())
        : Entity(mask)
    {
    }

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_CORE__ENTITY_HPP
