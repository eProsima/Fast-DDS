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
 * @file Endpoint.hpp
 */

#ifndef FASTDDS_RTPS__ENDPOINT_HPP
#define FASTDDS_RTPS__ENDPOINT_HPP

#include <fastdds/utils/TimedMutex.hpp>
#include <fastdds/rtps/attributes/EndpointAttributes.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class RTPSParticipantImpl;

class Endpoint
{
public:

    virtual ~Endpoint() = default;

    const GUID_t& getGuid()
    {
        return m_guid;
    }

    inline RecursiveTimedMutex& getMutex()
    {
        return mp_mutex;
    }

    EndpointAttributes& getAttributes()
    {
        return m_att;
    }

#if HAVE_SECURITY
    bool supports_rtps_protection_;
#endif // HAVE_SECURITY

    RTPSParticipantImpl* mp_RTPSParticipant;
    GUID_t m_guid;
    EndpointAttributes m_att;
    mutable RecursiveTimedMutex mp_mutex;

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS__ENDPOINT_HPP
