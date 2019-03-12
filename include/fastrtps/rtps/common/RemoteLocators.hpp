// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file RemoteLocators.h
 */

#ifndef FASTRTPS_RTPS_COMMON_REMOTELOCATORS_HPP_
#define FASTRTPS_RTPS_COMMON_REMOTELOCATORS_HPP_

#include "./Locator.h"
#include "../../utils/collections/ResourceLimitedVector.hpp"

namespace eprosima {
namespace fastrtps {
namespace rtps {

struct RemoteLocatorList
{
    RemoteLocatorList(
            size_t max_unicast_locators,
            size_t max_multicast_locators)
        : unicast(ResourceLimitedContainerConfig::fixed_size_configuration(max_unicast_locators))
        , multicast(ResourceLimitedContainerConfig::fixed_size_configuration(max_multicast_locators))
    {
    }

    RemoteLocatorList(const RemoteLocatorList& other)
    {
        *this = other;
    }

    RemoteLocatorList& operator = (const RemoteLocatorList& other)
    {
        unicast.clear();
        for (const Locator_t& locator : other.unicast)
        {
            unicast.push_back(locator);
        }

        multicast.clear();
        for (const Locator_t& locator : other.multicast)
        {
            multicast.push_back(locator);
        }

        return *this;
    }

    void add_unicast_locator(const Locator_t& locator)
    {
        for (const Locator_t& loc : unicast)
        {
            if (loc == locator)
            {
                return;
            }
        }

        unicast.push_back(locator);
    }

    void add_multicast_locator(const Locator_t& locator)
    {
        for (const Locator_t& loc : multicast)
        {
            if (loc == locator)
            {
                return;
            }
        }

        multicast.push_back(locator);
    }

    ResourceLimitedVector<Locator_t> unicast;
    ResourceLimitedVector<Locator_t> multicast;
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* FASTRTPS_RTPS_COMMON_REMOTELOCATORS_HPP_ */
