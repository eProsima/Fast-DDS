// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file ReadCondition.cpp
 */

#include <fastdds/dds/subscriber/ReadCondition.hpp>
#include <fastdds/dds/subscriber/InstanceState.hpp>
#include <fastdds/dds/subscriber/SampleState.hpp>
#include <fastdds/dds/subscriber/ViewState.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

namespace detail {

struct ReadConditionImpl
{
};

}  // namespace detail


ReadCondition::ReadCondition(
        DataReader* /*parent*/)
    : Condition()
    , impl_(new detail::ReadConditionImpl())
{
}

ReadCondition::~ReadCondition()
{
}

bool ReadCondition::get_trigger_value() const
{
    return false;
}

DataReader* ReadCondition::get_datareader() const
{
    return nullptr;
}

SampleStateMask ReadCondition::get_sample_state_mask() const
{
    return ANY_SAMPLE_STATE;
}

ViewStateMask ReadCondition::get_view_state_mask() const
{
    return ANY_VIEW_STATE;
}

InstanceStateMask ReadCondition::get_instance_state_mask() const
{
    return ANY_INSTANCE_STATE;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
