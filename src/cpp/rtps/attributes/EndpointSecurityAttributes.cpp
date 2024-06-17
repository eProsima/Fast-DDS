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
 * @file EndpointSecurityAttributes.cpp
 */

#include <fastdds/rtps/attributes/EndpointSecurityAttributes.hpp>

#include <rtps/security/accesscontrol/SecurityMaskUtilities.h>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

typedef uint32_t PluginEndpointSecurityAttributesMask;

PluginEndpointSecurityAttributesMask PluginEndpointSecurityAttributes::mask() const
{
    PluginEndpointSecurityAttributesMask rv = PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_VALID;
    if (is_submessage_encrypted)
    {
        rv |= PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED;
    }
    if (is_submessage_origin_authenticated)
    {
        rv |= PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED;
    }
    if (is_payload_encrypted)
    {
        rv |= PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_PAYLOAD_ENCRYPTED;
    }
    return rv;
}

typedef uint32_t EndpointSecurityAttributesMask;


EndpointSecurityAttributesMask EndpointSecurityAttributes::mask() const
{
    EndpointSecurityAttributesMask rv = ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_VALID;
    if (is_read_protected)
    {
        rv |= ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_READ_PROTECTED;
    }
    if (is_write_protected)
    {
        rv |= ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_WRITE_PROTECTED;
    }
    if (is_discovery_protected)
    {
        rv |= ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_DISCOVERY_PROTECTED;
    }
    if (is_liveliness_protected)
    {
        rv |= ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_LIVELINESS_PROTECTED;
    }
    if (is_submessage_protected)
    {
        rv |= ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_PROTECTED;
    }
    if (is_payload_protected)
    {
        rv |= ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_PAYLOAD_PROTECTED;
    }
    if (is_key_protected)
    {
        rv |= ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_KEY_PROTECTED;
    }
    return rv;
}

bool EndpointSecurityAttributes::match(
        const EndpointSecurityAttributesMask remoteMask,
        const PluginEndpointSecurityAttributesMask remotePluginMask) const
{
    return security_mask_matches(mask(), remoteMask) &&
           security_mask_matches(plugin_endpoint_attributes, remotePluginMask);
}

} // namespace security
} // namespace rtps
} // namespace fastdds
} // namespace eprosima
