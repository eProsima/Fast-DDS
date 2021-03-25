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
 * @file StatisticsCommon.hpp
 */

#ifndef _FASTDDS_STATISTICS_RTPS_STATISTICSCOMMON_HPP_
#define _FASTDDS_STATISTICS_RTPS_STATISTICSCOMMON_HPP_

#include <fastdds/statistics/IListeners.hpp>

#include <memory>

namespace eprosima {
namespace fastdds {
namespace statistics {

// Members are private details
struct StatisticsAncillary;

class StatisticsListenersImpl
{
    std::unique_ptr<StatisticsAncillary> members_;

protected:

    bool add_statistics_listener_impl(
            std::shared_ptr<fastdds::statistics::IListener> listener);
    bool remove_statistics_listener_impl(
            std::shared_ptr<fastdds::statistics::IListener> listener);

    // lambda function to traverse the listener collection
    template<class Function>
    Function for_each_listener(
            Function f);
};

class StatisticsWriterImpl
    : protected StatisticsListenersImpl
{
protected:

    // TODO: methods for listeners callbacks
};

class StatisticsReaderImpl
    : protected StatisticsListenersImpl
{
protected:

    // TODO: methods for listeners callbacks
};

} // namespace statistics
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_STATISTICS_RTPS_STATISTICSCOMMON_HPP_
