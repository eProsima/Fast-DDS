// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/*!
 * @file EndpointSecurityAttributes.h
 */
#ifndef __RTPS_SECURITY_ACCESSCONTROL_ENDPOINTSECURITYATTRIBUTES_H__
#define __RTPS_SECURITY_ACCESSCONTROL_ENDPOINTSECURITYATTRIBUTES_H__

namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace security {

typedef uint32_t EndpointSecurityAttributesMask;

#define ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_READ_PROTECTED       (0x00000001UL << 0)
#define ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_WRITE_PROTECTED      (0x00000001UL << 1)
#define ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_DISCOVERY_PROTECTED  (0x00000001UL << 2)
#define ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_PROTECTED (0x00000001UL << 3)
#define ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_PAYLOAD_PROTECTED    (0x00000001UL << 4)
#define ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_KEY_PROTECTED        (0x00000001UL << 5)
#define ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_LIVELINESS_PROTECTED (0x00000001UL << 6)
#define ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_VALID                (0x00000001UL << 31)

struct EndpointSecurityAttributes
{
    EndpointSecurityAttributes() : 
        is_read_protected(true), is_write_protected(true),
        is_discovery_protected(false), is_liveliness_protected(false),
        is_submessage_protected(false), is_payload_protected(false), is_key_protected(false)
    {}

    explicit EndpointSecurityAttributes(const EndpointSecurityAttributesMask mask) :
        is_read_protected((mask & ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_READ_PROTECTED) != 0),
        is_write_protected((mask & ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_WRITE_PROTECTED) != 0),
        is_discovery_protected((mask & ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_DISCOVERY_PROTECTED) != 0),
        is_liveliness_protected((mask & ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_LIVELINESS_PROTECTED) != 0),
        is_submessage_protected((mask & ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_PROTECTED) != 0),
        is_payload_protected((mask & ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_PAYLOAD_PROTECTED) != 0),
        is_key_protected((mask & ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_KEY_PROTECTED) != 0)
    {}

    bool is_read_protected;

    bool is_write_protected;

    bool is_discovery_protected;

    bool is_liveliness_protected;

    bool is_submessage_protected;

    bool is_payload_protected;

    bool is_key_protected;

    EndpointSecurityAttributesMask mask()
    {
        EndpointSecurityAttributesMask rv = ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_VALID;
        if (is_read_protected) rv |= ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_READ_PROTECTED;
        if (is_write_protected) rv |= ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_WRITE_PROTECTED;
        if (is_discovery_protected) rv |= ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_DISCOVERY_PROTECTED;
        if (is_liveliness_protected) rv |= ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_LIVELINESS_PROTECTED;
        if (is_submessage_protected) rv |= ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_PROTECTED;
        if (is_payload_protected) rv |= ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_PAYLOAD_PROTECTED;
        if (is_key_protected) rv |= ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_KEY_PROTECTED;
        return rv;
    }
};

}
}
}
}

#endif // __RTPS_SECURITY_ACCESSCONTROL_ENDPOINTSECURITYATTRIBUTES_H__
