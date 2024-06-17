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
 * @file ChangeComparison.hpp
 */

#ifndef _RTPS_COMMON_CHANGECOMPARISON_HPP
#define _RTPS_COMMON_CHANGECOMPARISON_HPP

#include <fastdds/rtps/common/CacheChange.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

inline bool history_order_cmp(
        const eprosima::fastdds::rtps::CacheChange_t* lhs,
        const eprosima::fastdds::rtps::CacheChange_t* rhs)
{
    return lhs->writerGUID == rhs->writerGUID ?
           lhs->sequenceNumber < rhs->sequenceNumber :
           lhs->sourceTimestamp < rhs->sourceTimestamp;
}

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif  // _RTPS_COMMON_CHANGECOMPARISON_HPP
