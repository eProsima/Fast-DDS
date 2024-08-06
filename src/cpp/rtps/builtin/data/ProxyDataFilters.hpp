// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_RTPS_BUILTIN_DATA__PROXYDATAFILTERS_HPP
#define FASTDDS_RTPS_BUILTIN_DATA__PROXYDATAFILTERS_HPP

#include <fastdds/rtps/common/RemoteLocators.hpp>

#include <rtps/network/NetworkFactory.hpp>
#include <rtps/transport/shared_mem/SHMLocator.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Contains filtering functions for ProxyData structures
 */
class ProxyDataFilters
{
public:

    /**
     * @brief This function filters out unreachable locators.
     *
     * @param [in] network_factory Reference to the @ref NetworkFactory
     * @param [in,out] target_locators_list List where parsed locators are stored
     * @param [in] temp_locator New locator to parse
     * @param [in] is_unicast true if temp_locator is unicast, false if it is multicast
     */
    static void filter_locators(
            NetworkFactory& network_factory,
            RemoteLocatorList& target_locators_list,
            const Locator_t& temp_locator,
            bool is_unicast)
    {
        if (network_factory.is_locator_reachable(temp_locator))
        {
            if (is_unicast)
            {
                target_locators_list.add_unicast_locator(temp_locator);
            }
            else
            {
                target_locators_list.add_multicast_locator(temp_locator);
            }
        }
    }

};

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif // FASTDDS_RTPS_BUILTIN_DATA__PROXYDATAFILTERS_HPP
