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
#ifndef _FASTDDS_RTPS_SECURITY_ACCESSCONTROL_ENDPOINTSECURITYATTRIBUTES_H_
#define _FASTDDS_RTPS_SECURITY_ACCESSCONTROL_ENDPOINTSECURITYATTRIBUTES_H_

#include <fastdds/rtps/security/accesscontrol/SecurityMaskUtilities.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace security {

typedef uint32_t PluginEndpointSecurityAttributesMask;

#define PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED            (0x00000001UL << 0)
#define PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_PAYLOAD_ENCRYPTED               (0x00000001UL << 1)
#define PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED (0x00000001UL << 2)
#define PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_VALID                           (0x00000001UL << 31)

#define PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_MASK_DEFAULT PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_VALID

struct PluginEndpointSecurityAttributes
{
    PluginEndpointSecurityAttributes() : 
        is_submessage_encrypted(false), is_submessage_origin_authenticated(false), is_payload_encrypted(false)
    { }

    explicit PluginEndpointSecurityAttributes(const PluginEndpointSecurityAttributesMask mask) :
        is_submessage_encrypted((mask & PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED) != 0),
        is_submessage_origin_authenticated((mask & PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED) != 0),
        is_payload_encrypted((mask & PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_PAYLOAD_ENCRYPTED) != 0)
    {
    }

    bool is_submessage_encrypted;
    bool is_submessage_origin_authenticated;
    bool is_payload_encrypted;

    inline PluginEndpointSecurityAttributesMask mask() const
    {
        PluginEndpointSecurityAttributesMask rv = PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_VALID;
        if (is_submessage_encrypted) rv |= PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED;
        if (is_submessage_origin_authenticated) rv |= PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED;
        if (is_payload_encrypted) rv |= PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_PAYLOAD_ENCRYPTED;
        return rv;
    }
};

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
        is_submessage_protected(false), is_payload_protected(false), is_key_protected(false),
        plugin_endpoint_attributes(0UL)
    {}

    explicit EndpointSecurityAttributes(const EndpointSecurityAttributesMask mask) :
        is_read_protected((mask & ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_READ_PROTECTED) != 0),
        is_write_protected((mask & ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_WRITE_PROTECTED) != 0),
        is_discovery_protected((mask & ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_DISCOVERY_PROTECTED) != 0),
        is_liveliness_protected((mask & ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_LIVELINESS_PROTECTED) != 0),
        is_submessage_protected((mask & ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_PROTECTED) != 0),
        is_payload_protected((mask & ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_PAYLOAD_PROTECTED) != 0),
        is_key_protected((mask & ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_KEY_PROTECTED) != 0),
        plugin_endpoint_attributes(0UL)
    {}

    bool is_read_protected;

    bool is_write_protected;

    bool is_discovery_protected;

    bool is_liveliness_protected;

    bool is_submessage_protected;

    bool is_payload_protected;

    bool is_key_protected;

    PluginEndpointSecurityAttributesMask plugin_endpoint_attributes;

    inline EndpointSecurityAttributesMask mask() const
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

    inline bool match(const EndpointSecurityAttributesMask remoteMask,
        const PluginEndpointSecurityAttributesMask remotePluginMask) const
    {
        return security_mask_matches(mask(), remoteMask) &&
            security_mask_matches(plugin_endpoint_attributes, remotePluginMask);
    }
};

}
}
}
}

#endif // _FASTDDS_RTPS_SECURITY_ACCESSCONTROL_ENDPOINTSECURITYATTRIBUTES_H_
