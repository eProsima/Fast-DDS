// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file StateFilter.hpp
 */

#ifndef _FASTDDS_SUBSCRIBER_DATAREADERIMPL_STATEFILTER_HPP_
#define _FASTDDS_SUBSCRIBER_DATAREADERIMPL_STATEFILTER_HPP_

#include <fastdds/dds/subscriber/InstanceState.hpp>
#include <fastdds/dds/subscriber/SampleState.hpp>
#include <fastdds/dds/subscriber/ViewState.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

struct StateFilter
{
    SampleStateMask sample_states;
    ViewStateMask view_states;
    InstanceStateMask instance_states;
};

} /* namespace detail */
} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif  // _FASTDDS_SUBSCRIBER_DATAREADERIMPL_STATEFILTER_HPP_
