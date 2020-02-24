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

#ifndef _FASTDDS_RTPS_BUILTIN_DATA_PROXYDATAFILTERS_H_
#define _FASTDDS_RTPS_BUILTIN_DATA_PROXYDATAFILTERS_H_

#include <fastdds/rtps/common/RemoteLocators.hpp>
#include <rtps/transport/shared_mem/SHMLocator.hpp>

namespace eprosima {
namespace fastrtps {
namespace rtps {

/**
 * Contains filtering functions for ProxyData structures
 */
class ProxyDataFilters
{
public:

    /**
     * As locator are parsed, when a CDR encapsulated proxydata message is received,
     * this function decides wether SHM communication is possible, in that case only
     * SHM locators are stored in the target_locator_list. If SHM communication is
     * not possible SHM locators are not stored in the list.
     * @param[in] is_shm_transport_available Indicates wether the participant has SHM transport enabled.
     * @param[in/out] is_shm_transport_possible Is true when at least a SHM locator from the local host has
     * been parsed
     * @param[in/out] are_shm_locators_present True when SHM locators has been parsed
     * @param[in/out] target_locators_list List where parsed locators are stored
     * @param[in] temp_locator New locator to parse
     * @param[in] is_unicast true if temp_locator is unicast, false if it is multicast
     */
    static void filter_locators(
            bool is_shm_transport_available,
            bool* is_shm_transport_possible,
            bool* are_shm_locators_present,
            RemoteLocatorList* target_locators_list,
            const Locator_t& temp_locator,
            bool is_unicast)
    {
        using SHMLocator = eprosima::fastdds::rtps::SHMLocator;

        if (is_shm_transport_available && !(*is_shm_transport_possible) )
        {
            *is_shm_transport_possible = SHMLocator::is_shm_and_from_this_host(temp_locator);
        }

        if (*is_shm_transport_possible)
        {
            if (temp_locator.kind == LOCATOR_KIND_SHM)
            {
                // First SHM locator
                if (!(*are_shm_locators_present))
                {
                    // Remove previously added locators from other transports
                    target_locators_list->unicast.clear();
                    target_locators_list->multicast.clear();
                    *are_shm_locators_present = true;
                }

                if (is_unicast)
                {
                    target_locators_list->add_unicast_locator(temp_locator);
                }
                else
                {
                    target_locators_list->add_multicast_locator(temp_locator);
                }
            }
            else if (!(*are_shm_locators_present))
            {
                if (is_unicast)
                {
                    target_locators_list->add_unicast_locator(temp_locator);
                }
                else
                {
                    target_locators_list->add_multicast_locator(temp_locator);
                }
            }
        }
        else
        {
            if (temp_locator.kind != LOCATOR_KIND_SHM)
            {
                if (is_unicast)
                {
                    target_locators_list->add_unicast_locator(temp_locator);
                }
                else
                {
                    target_locators_list->add_multicast_locator(temp_locator);
                }
            }
        }
    }
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif // _FASTDDS_RTPS_BUILTIN_DATA_PROXYDATAFILTERS_H_
