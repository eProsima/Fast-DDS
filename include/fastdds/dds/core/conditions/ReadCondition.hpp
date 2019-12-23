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
 * @file ReadCondition.hpp
 *
 */

#ifndef _FASTDDS_READCONDITION_HPP_
#define _FASTDDS_READCONDITION_HPP_

#include <fastrtps/fastrtps_all.h>
#include <fastdds/dds/core/conditions/Condition.hpp>
#include <fastrtps/rtps/common/Types.h>
#include <fastdds/dds/topic/DataReader.hpp>
#include <dds/sub/status/DataState.hpp>

using eprosima::fastrtps::types::ReturnCode_t;

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * @brief The ReadCondition class
 */
class RTPS_DllAPI ReadCondition : public Condition
{
public:

        ReadCondition(
                DataReader* reader,
                const ::dds::sub::status::SampleState& sample_mask = ::dds::sub::status::SampleState::any(),
                const ::dds::sub::status::ViewState& view_mask = ::dds::sub::status::ViewState::any(),
                const ::dds::sub::status::InstanceState& instance_mask = ::dds::sub::status::InstanceState::any());

        DataReader* get_datareader();

        ::dds::sub::status::SampleState get_sample_state_mask();

        ::dds::sub::status::ViewState get_view_state_mask();

        ::dds::sub::status::InstanceState get_instance_state_mask();

        inline bool operator ==(
                ReadCondition* obj) const;

private:

        DataReader* reader_;

        ::dds::sub::status::SampleState sample_state_mask_;

        ::dds::sub::status::ViewState view_state_mask_;

        ::dds::sub::status::InstanceState instance_state_mask_;

};

} //namespace dds
} //namespace fastdds
} //namespace eprosima

#endif /* _FASTDDS_READCONDITION_HPP_ */
