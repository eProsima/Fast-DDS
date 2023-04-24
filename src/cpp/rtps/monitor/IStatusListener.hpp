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
 * @file IStatusListener.hpp
 */

#ifndef RTPS_MONITOR_ISTATUSLISTENER_HPP
#define RTPS_MONITOR_ISTATUSLISTENER_HPP

#include <cstdint>

namespace eprosima {
namespace fastrtps {
namespace rtps {

struct GUID_t;

class IStatusListener
{

public:

    IStatusListener() = default;

    virtual ~IStatusListener() = default;

    /**
     *
     * @param guid
     * @param id
     *
     * @return true if the entity status changed.
     */
    virtual bool on_local_entity_status_change(
            const GUID_t& guid,
            const uint32_t& id) = 0;

};

}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

#endif  // RTPS_MONITOR_ISTATUSLISTENER_HPP
