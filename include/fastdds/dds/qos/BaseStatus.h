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
 * @file BaseStatus.h
*/

#ifndef _FASTRTPS_BASE_STATUS_H_
#define _FASTRTPS_BASE_STATUS_H_

#include <cstdint>

namespace eprosima {
namespace fastdds {
namespace dds{

//! @brief A struct storing the base status
struct BaseStatus
{
    int32_t total_count = 0;

    int32_t total_count_change = 0;
};

using SampleLostStatus = BaseStatus;
using LivelinessLostStatus = BaseStatus;
using InconsistentTopicStatus = BaseStatus;

} //end of anamespace dds
} //end of namespace fastdds
} //end of namespace eprosima

#endif // _FASTRTPS_BASE_STATUS_H_
