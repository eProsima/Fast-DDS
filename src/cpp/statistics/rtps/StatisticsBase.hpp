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
 * @file StatisticsBase.hpp
 */

#ifndef _STATISTICS_RTPS_STATISTICSBASE_HPP_
#define _STATISTICS_RTPS_STATISTICSBASE_HPP_

#include <fastdds/statistics/rtps/StatisticsCommon.hpp>

#include <set>

namespace eprosima {
namespace fastdds {
namespace statistics {

struct StatisticsAncillary
{
    std::set<std::shared_ptr<IListener>> listeners;
};

} // namespace statistics
} // namespace fastdds
} // namespace eprosima

#endif // _STATISTICS_RTPS_STATISTICSBASE_HPP_
