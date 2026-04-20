// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <rtps/transport/MulticastTransportInterface.h>

#include <fastdds/utils/IPLocator.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

bool MulticastTransportInterface::fillMulticastLocator(
        Locator& locator,
        uint32_t well_known_port) const
{
    LocatorList defaults;
    getDefaultMulticastLocators(defaults, well_known_port);
    if (!defaults.empty())
    {
        const Locator& default_loc = *defaults.begin();
        if (locator.port == 0)
        {
            locator.port = default_loc.port;
        }
        if (!IsAddressDefined(locator))
        {
            IPLocator::copy_address(default_loc, locator);
        }
    }

    return true;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
