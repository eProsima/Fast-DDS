// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file ServerAttributes.hpp
 *
 */

#ifndef FASTDDS_RTPS_ATTRIBUTES__SERVERATTRIBUTES_HPP
#define FASTDDS_RTPS_ATTRIBUTES__SERVERATTRIBUTES_HPP

#include <list>

#include <fastdds/rtps/common/LocatorList.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class RemoteServerAttributes
{
public:

    bool operator ==(
            const RemoteServerAttributes&) const
    {
        return true;
    }

    template<int> bool requires_transport() const
    {
        return true;
    }

    bool ReadguidPrefix(
            const char*)
    {
        return true;
    }

    LocatorList metatrafficMulticastLocatorList;
    LocatorList metatrafficUnicastLocatorList;

};

typedef std::list<RemoteServerAttributes> RemoteServerList_t;

static inline bool load_environment_server_info(
        RemoteServerList_t&)
{
    return true;
}

static inline bool ros_super_client_env()
{
    return false;
}

template<class charT>
std::basic_ostream<charT>& operator <<(
        std::basic_ostream<charT>& output,
        const RemoteServerList_t& /*list*/)
{
    return output;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_ATTRIBUTES__SERVERATTRIBUTES_HPP
