// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file StatisticsBase.hpp
 */

#ifndef _STATISTICS_RTPS_STATISTICSBASE_HPP_
#define _STATISTICS_RTPS_STATISTICSBASE_HPP_

#include <atomic>
#include <cstdint>
#include <mutex>
#include <set>

#include <fastrtps/config.h>


#include <statistics/types/types.h>


namespace eprosima {
namespace fastdds {
namespace statistics {

#ifdef FASTDDS_STATISTICS

// auxiliary conversion functions
detail::Locator_s to_statistics_type(
        fastrtps::rtps::Locator_t locator)
{
    return *reinterpret_cast<detail::Locator_s*>(&locator);
}

fastrtps::rtps::Locator_t to_fastdds_type(
        detail::Locator_s locator)
{
    return *reinterpret_cast<fastrtps::rtps::Locator_t*>(&locator);
}

detail::GUID_s to_statistics_type(
        fastrtps::rtps::GUID_t guid)
{
    return *reinterpret_cast<detail::GUID_s*>(&guid);
}

fastrtps::rtps::GUID_t to_fastdds_type(
        detail::GUID_s guid)
{
    return *reinterpret_cast<fastrtps::rtps::GUID_t*>(&guid);
}

#endif // FASTDDS_STATISTICS

} // namespace statistics
} // namespace fastdds
} // namespace eprosima

#endif // _STATISTICS_RTPS_STATISTICSBASE_HPP_
