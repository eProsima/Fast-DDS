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
 * @file IConnectionsListener.hpp
 */

#ifndef RTPS_MONITOR_ICONNECTIONSLISTENER_HPP
#define RTPS_MONITOR_ICONNECTIONSLISTENER_HPP

namespace eprosima {
namespace fastrtps {
namespace rtps {

struct GUID_t;

class IConnectionsListener
{

public:

    IConnectionsListener() = default;

    virtual ~IConnectionsListener() = default;

    /**
     *
     *
     *
     * @param guid
     *
     * @return true if the entity connections changed.
     */
    virtual bool on_local_entity_connections_change(
            const GUID_t& guid) = 0;

};

}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

#endif  // RTPS_MONITOR_ICONNECTIONSLISTENER_HPP
