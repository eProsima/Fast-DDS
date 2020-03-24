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

#ifndef _FASTDDS_BASE_STATUS_HPP_
#define _FASTDDS_BASE_STATUS_HPP_

#include <cstdint>

namespace eprosima {
namespace fastdds {
namespace dds {

//! @brief A struct storing the base status
struct BaseStatus
{
    //! @brief Total cumulative number of times the event occurred.
    int32_t total_count = 0;

    //! @brief The change in total_count since the last time the listener was called or the status was read
    int32_t total_count_change = 0;
};

using SampleLostStatus = BaseStatus;
using LivelinessLostStatus = BaseStatus;
using InconsistentTopicStatus = BaseStatus;

} //namespace dds
} //namespace fastdds

namespace fastrtps {

using LivelinessLostStatus = fastdds::dds::BaseStatus;

}

} //namespace eprosima

#endif // _FASTDDS_BASE_STATUS_HPP_
