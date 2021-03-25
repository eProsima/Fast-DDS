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
 * @file StatisticsBase.cpp
 */

#include "StatisticsBase.hpp"

using namespace eprosima::fastdds::statistics;

bool StatisticsListenersImpl::add_statistics_listener_impl(
        std::shared_ptr<fastdds::statistics::IListener> listener)
{
    if(!listener)
    {
        // avoid nullptr
        return false;
    }

    // if the collection is not initialized do it
    if (!members_)
    {
        members_.reset(new StatisticsAncillary());
    }

    // add the new listener
    return members_->listeners.insert(listener).second;
}

bool StatisticsListenersImpl::remove_statistics_listener_impl(
        std::shared_ptr<fastdds::statistics::IListener> listener)
{
    if(!members_ || !listener)
    {
        // avoid nullptr
        return false;
    }

    return 1 == members_->listeners.erase(listener);
}

template<class Function>
Function StatisticsListenersImpl::for_each_listener(
        Function f)
{
    // If the collection is not initialized ignore it
    if (members_)
    {
        for(auto& listener : members_->listeners)
        {
            f(listener);
        }
    }

    return f;
}
