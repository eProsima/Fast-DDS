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

#include <fastdds/dds/core/conditions/ReadCondition.hpp>

using namespace eprosima::fastdds::dds;

ReadCondition::ReadCondition(
        DataReader* reader,
        const ::dds::sub::status::SampleState& sample_mask,
        const ::dds::sub::status::ViewState& view_mask,
        const ::dds::sub::status::InstanceState& instance_mask)
    : reader_(reader)
    , sample_state_mask_(sample_mask)
    , view_state_mask_(view_mask)
    , instance_state_mask_(instance_mask)
{}

DataReader* ReadCondition::get_datareader()
{
    return reader_;
}

::dds::sub::status::SampleState ReadCondition::get_sample_state_mask()
{
    return sample_state_mask_;
}

::dds::sub::status::ViewState ReadCondition::get_view_state_mask()
{
    return view_state_mask_;
}

::dds::sub::status::InstanceState ReadCondition::get_instance_state_mask()
{
    return instance_state_mask_;
}

inline bool ReadCondition::operator ==(
        ReadCondition* obj) const
{
    return (this->reader_ == obj->get_datareader()) &&
           (this->sample_state_mask_ == obj->get_sample_state_mask()) &&
           (this->view_state_mask_ == obj->get_view_state_mask()) &&
           (this->instance_state_mask_ == obj->get_instance_state_mask());
}
