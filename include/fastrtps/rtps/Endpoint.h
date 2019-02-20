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
 * @file Endpoint.h
 */



#ifndef ENDPOINT_H_
#define ENDPOINT_H_
#include <mutex>
#include "common/Types.h"
#include "common/Locator.h"
#include "common/Guid.h"
#include "attributes/EndpointAttributes.h"

#include <mutex>

namespace eprosima {
namespace fastrtps{
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

    Endpoint(
            RTPSParticipantImpl* pimpl,
            const GUID_t& guid,
            const EndpointAttributes& att)
        : mp_RTPSParticipant(pimpl)
          , m_guid(guid)
          , m_att(att)
#if HAVE_SECURITY
          ,supports_rtps_protection_(true)
#endif
    {
    }

    virtual ~Endpoint() = default;

    public:
    /**
     * Get associated GUID
     * @return Associated GUID
     */
    RTPS_DllAPI inline const GUID_t& getGuid() const { return m_guid; }

    /**
     * Get mutex
     * @return Associated Mutex
     */
    RTPS_DllAPI inline std::recursive_timed_mutex& getMutex() { return mp_mutex; }

    /**
     * Get associated attributes
     * @return Endpoint attributes
     */
    RTPS_DllAPI inline EndpointAttributes& getAttributes() { return m_att; }

#if HAVE_SECURITY
    bool supports_rtps_protection() { return supports_rtps_protection_; }
#endif

    protected:

    //!Pointer to the RTPSParticipant containing this endpoint.
    RTPSParticipantImpl* mp_RTPSParticipant;

    //!Endpoint GUID
    const GUID_t m_guid;

    //!Endpoint Attributes
    EndpointAttributes m_att;

    //!Endpoint Mutex
    std::recursive_timed_mutex mp_mutex;

    private:

    Endpoint& operator=(const Endpoint&) = delete;

#if HAVE_SECURITY
    bool supports_rtps_protection_;
#endif
};


}
} /* namespace rtps */
} /* namespace eprosima */

#endif /* ENDPOINT_H_ */
