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
 * @file BaseStatus.hpp
 */

#ifndef FASTDDS_DDS_CORE_STATUS__BASESTATUS_HPP
#define FASTDDS_DDS_CORE_STATUS__BASESTATUS_HPP

#include <cstdint>

namespace eprosima {
namespace fastdds {
namespace dds {

//! @brief A struct storing the base status
struct BaseStatus
{
    //!Total cumulative count
    int32_t total_count = 0;

    //!Increment since the last time the status was read
    int32_t total_count_change = 0;
};

//!Alias of BaseStatus
using SampleLostStatus = BaseStatus;
//!Alias of BaseStatus
using LivelinessLostStatus = BaseStatus;
//!Alias of BaseStatus
using InconsistentTopicStatus = BaseStatus;

} //namespace dds

using LivelinessLostStatus = fastdds::dds::BaseStatus;

} //namespace fastdds
} //namespace eprosima

#endif // FASTDDS_DDS_CORE_STATUS__BASESTATUS_HPP
