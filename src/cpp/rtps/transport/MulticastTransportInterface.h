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

#ifndef _FASTDDS_MULTICAST_TRANSPORT_INTERFACE_H_
#define _FASTDDS_MULTICAST_TRANSPORT_INTERFACE_H_

#include <cstdint>

#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/common/LocatorList.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class MulticastTransportInterface
{
public:

    virtual ~MulticastTransportInterface() = default;

    virtual bool getDefaultMulticastLocators(
            LocatorList& locators,
            uint32_t multicast_port) const = 0;

    virtual bool fillMulticastLocator(
            Locator& locator,
            uint32_t well_known_port) const;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_MULTICAST_TRANSPORT_INTERFACE_H_
