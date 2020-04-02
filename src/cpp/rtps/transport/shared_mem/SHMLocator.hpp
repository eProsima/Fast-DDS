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

#ifndef _FASTDDS_SHMLOCATOR_H_
#define _FASTDDS_SHMLOCATOR_H_

#include <fastdds/rtps/common/Locator.h>
#include <utils/Host.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Define operations for shared-memory locators
 */
class SHMLocator
{
public:

    enum class Type 
    {
        UNICAST, 
        MULTICAST
    };

    /**
     * Generate a shared-memory locator for the local host.
     * @param port Locator's shared-memory port.
     * @param type Indicates whether the locator is unicast or multicast.
     * @return The created shared-memory locator.
     */
    static fastrtps::rtps::Locator_t create_locator(
        uint32_t port,
        Type type)
    {
        using namespace fastrtps::rtps;
        
        Locator_t locator(LOCATOR_KIND_SHM, port);
        
        locator.get_address()[0] = (type == Type::UNICAST) ? 'U' : 'M';

        auto host_id = Host::get().id();
        locator.get_address()[1] = octet(host_id);
        locator.get_address()[2] = octet(host_id >> 8);

        return locator;
    }

    /**
     * Check whether a given locator is shared-memory kind and belongs to this host
     * @param locator Locator to check
     * @return boolean 
     */
    static bool is_shm_and_from_this_host(const fastrtps::rtps::Locator_t& locator)
    {
        using namespace fastrtps::rtps;

        if(locator.kind == LOCATOR_KIND_SHM)
        {
            auto host_id = Host::get().id();
     
            return locator.address[1] == octet(host_id) && locator.address[2] == octet(host_id >> 8);
        }

        return false;
    }
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_SHMLOCATOR_H_
