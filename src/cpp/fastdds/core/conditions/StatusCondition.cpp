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
 * @file WaitSet.cpp
 *
 */

#include <fastdds/dds/core/conditions/Condition.hpp>
#include <fastdds/dds/core/conditions/StatusCondition.hpp>
#include <fastdds/dds/core/Entity.hpp>

using namespace eprosima;
using namespace eprosima::fastdds::dds;

StatusCondition::StatusCondition(
        Entity* entity)
    : handler(nullptr)
    , entity_(entity)
    , status_mask_(::dds::core::status::StatusMask::all())
    , status_change_flag_(::dds::core::status::StatusMask::none())
{
}

StatusCondition::StatusCondition(
        Entity* entity,
        std::function<void()> functor)
    : handler(functor)
    , entity_(entity)
    , status_mask_(::dds::core::status::StatusMask::all())
    , status_change_flag_(::dds::core::status::StatusMask::none())
{
}

ReturnCode_t StatusCondition::set_enabled_statuses(
        const ::dds::core::status::StatusMask& mask)
{
    status_mask_ = mask;

    if (!entity_->is_enabled())
    {
        set_trigger_value(false);
        return ReturnCode_t::RETCODE_OK;
    }

    //Check if the new mask change the trigger_value
    std::bitset<16> out = status_change_flag_ & status_mask_;
    set_trigger_value(out.any());
    return ReturnCode_t::RETCODE_OK;
}

const ::dds::core::status::StatusMask& StatusCondition::get_enabled_statuses() const
{
    return status_mask_;
}

const ::dds::core::status::StatusMask& StatusCondition::get_triggered_status() const
{
    return status_change_flag_;
}

Entity* StatusCondition::get_entity()
{
    return entity_;
}

void StatusCondition::notify_status_change(
        const ::dds::core::status::StatusMask& mask)
{
    status_change_flag_ |= mask;

    if (!entity_->is_enabled())
    {
        set_trigger_value(false);
        return;
    }

    std::bitset<16> out = status_change_flag_ & status_mask_;
    set_trigger_value(out.any());
}

void StatusCondition::set_status_as_read(
        const ::dds::core::status::StatusMask& mask)
{
    if (!entity_->is_enabled())
    {
        set_trigger_value(false);
        return;
    }

    status_change_flag_ ^= mask;
    std::bitset<16> out = status_change_flag_ & status_mask_;
    set_trigger_value(out.any());
}

void StatusCondition::set_trigger_value(
        bool value)
{
    trigger_value_ = value;
}

void StatusCondition::call_handler()
{
    handler();
}

void StatusCondition::set_handler(
        std::function<void()> functor)
{
    handler = functor;
}

inline bool StatusCondition::operator ==(
        StatusCondition* obj) const
{
    return (this->entity_ == obj->get_entity()) &&
           (this->status_mask_ == obj->get_enabled_statuses()) &&
           (this->status_change_flag_ == obj->get_triggered_status());
}

bool StatusCondition::operator ==(
        Condition* obj) const
{
    StatusCondition* obj1 = static_cast<StatusCondition*>(obj);
    return this == obj1;
}
