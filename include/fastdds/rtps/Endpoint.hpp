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

#include <fastdds/rtps/attributes/EndpointAttributes.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/common/Types.hpp>
#include <fastdds/rtps/history/IChangePool.hpp>
#include <fastdds/rtps/history/IPayloadPool.hpp>
#include <fastdds/utils/TimedMutex.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class RTPSParticipantImpl;
class ResourceEvent;


/**
 * Class Endpoint, all entities of the RTPS network derive from this class.
 * Although the RTPSParticipant is also defined as an endpoint in the RTPS specification, in this implementation
 * the RTPSParticipant class **does not** inherit from the endpoint class. Each Endpoint object owns a pointer to the
 * RTPSParticipant it belongs to.
 * @ingroup COMMON_MODULE
 */
class Endpoint
{
    friend class RTPSParticipantImpl;

protected:

    Endpoint() = default;

    Endpoint(
            RTPSParticipantImpl* pimpl,
            const GUID_t& guid,
            const EndpointAttributes& att)
        : mp_RTPSParticipant(pimpl)
        , m_guid(guid)
        , m_att(att)
    {
    }

    virtual ~Endpoint()
    {
    }

public:

    /**
     * Get associated GUID
     * @return Associated GUID
     */
    FASTDDS_EXPORTED_API inline const GUID_t& getGuid() const
    {
        return m_guid;
    }

    /**
     * Get mutex
     * @return Associated Mutex
     */
    FASTDDS_EXPORTED_API inline RecursiveTimedMutex& getMutex()
    {
        return mp_mutex;
    }

    /**
     * Get associated attributes
     * @return Endpoint attributes
     */
    FASTDDS_EXPORTED_API inline EndpointAttributes& getAttributes()
    {
        return m_att;
    }

#if HAVE_SECURITY
    bool supports_rtps_protection()
    {
        return supports_rtps_protection_;
    }

#endif // if HAVE_SECURITY

protected:

    //!Pointer to the RTPSParticipant containing this endpoint.
    RTPSParticipantImpl* mp_RTPSParticipant;

    //!Endpoint GUID
    const GUID_t m_guid;

    //!Endpoint Attributes
    EndpointAttributes m_att;

    //!Endpoint Mutex
    mutable RecursiveTimedMutex mp_mutex;

    //!Fixed size of payloads
    uint32_t fixed_payload_size_ = 0;

private:

    Endpoint& operator =(
            const Endpoint&) = delete;

#if HAVE_SECURITY
    bool supports_rtps_protection_ = true;
#endif // if HAVE_SECURITY
};


} // namespace rtps
} // namespace rtps
} // namespace eprosima

#endif //FASTDDS_RTPS__ENDPOINT_HPP
