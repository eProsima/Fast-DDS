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
 * @file ParticipantSecurityAttributes.h
 */
#ifndef _FASTDDS_RTPS_SECURITY_ACCESSCONTROL_PARTICIPANTSECURITYATTRIBUTES_H_
#define _FASTDDS_RTPS_SECURITY_ACCESSCONTROL_PARTICIPANTSECURITYATTRIBUTES_H_

#include <fastdds/rtps/security/accesscontrol/SecurityMaskUtilities.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace security {

typedef uint32_t PluginParticipantSecurityAttributesMask;

#define PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ENCRYPTED                  (0x00000001UL << 0)
#define PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_BUILTIN_IS_DISCOVERY_ENCRYPTED     (0x00000001UL << 1)
#define PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_LIVELINESS_ENCRYPTED            (0x00000001UL << 2)
#define PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ORIGIN_AUTHENTICATED       (0x00000001UL << 3)
#define PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_DISCOVERY_ORIGIN_AUTHENTICATED  (0x00000001UL << 4)
#define PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_LIVELINESS_ORIGIN_AUTHENTICATED (0x00000001UL << 5)
#define PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_VALID                           (0x00000001UL << 31)

struct PluginParticipantSecurityAttributes
{
    PluginParticipantSecurityAttributes() :
        is_rtps_encrypted(false), is_discovery_encrypted(false), is_liveliness_encrypted(false),
        is_rtps_origin_authenticated(false), is_discovery_origin_authenticated(false),
        is_liveliness_origin_authenticated(false)
    {}

    explicit PluginParticipantSecurityAttributes(const PluginParticipantSecurityAttributesMask mask) :
        is_rtps_encrypted((mask & PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ENCRYPTED) != 0),
        is_discovery_encrypted((mask & PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_BUILTIN_IS_DISCOVERY_ENCRYPTED) != 0),
        is_liveliness_encrypted((mask & PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_LIVELINESS_ENCRYPTED) != 0),
        is_rtps_origin_authenticated((mask & PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ORIGIN_AUTHENTICATED) != 0),
        is_discovery_origin_authenticated((mask & PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_DISCOVERY_ORIGIN_AUTHENTICATED) != 0),
        is_liveliness_origin_authenticated((mask & PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_LIVELINESS_ORIGIN_AUTHENTICATED) != 0)
    {}

    bool is_rtps_encrypted;

    bool is_discovery_encrypted;

    bool is_liveliness_encrypted;

    bool is_rtps_origin_authenticated;

    bool is_discovery_origin_authenticated;

    bool is_liveliness_origin_authenticated;

    inline PluginParticipantSecurityAttributesMask mask() const
    {
        PluginParticipantSecurityAttributesMask rv = PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_VALID;
        if (is_rtps_encrypted) rv |= PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ENCRYPTED;
        if (is_discovery_encrypted) rv |= PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_BUILTIN_IS_DISCOVERY_ENCRYPTED;
        if (is_liveliness_encrypted) rv |= PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_LIVELINESS_ENCRYPTED;
        if (is_rtps_origin_authenticated) rv |= PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ORIGIN_AUTHENTICATED;
        if (is_discovery_origin_authenticated) rv |= PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_DISCOVERY_ORIGIN_AUTHENTICATED;
        if (is_liveliness_origin_authenticated) rv |= PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_LIVELINESS_ORIGIN_AUTHENTICATED;
        return rv;
    }
};

typedef uint32_t ParticipantSecurityAttributesMask;

#define PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_PROTECTED       (0x00000001UL << 0)
#define PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_DISCOVERY_PROTECTED  (0x00000001UL << 1)
#define PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_LIVELINESS_PROTECTED (0x00000001UL << 2)
#define PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_VALID                (0x00000001UL << 31)

struct ParticipantSecurityAttributes
{
    ParticipantSecurityAttributes() :
        allow_unauthenticated_participants(false), is_access_protected(true),
        is_rtps_protected(false), is_discovery_protected(false), is_liveliness_protected(false),
        plugin_participant_attributes (0UL)
    {}

    explicit ParticipantSecurityAttributes(const ParticipantSecurityAttributesMask mask) :
        allow_unauthenticated_participants(false), is_access_protected(true),
        is_rtps_protected((mask & PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_PROTECTED) != 0),
        is_discovery_protected((mask & PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_DISCOVERY_PROTECTED) != 0),
        is_liveliness_protected((mask & PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_LIVELINESS_PROTECTED) != 0),
        plugin_participant_attributes(0UL)
    {}

    bool allow_unauthenticated_participants;

    bool is_access_protected;

    bool is_rtps_protected;

    bool is_discovery_protected;

    bool is_liveliness_protected;

    PluginParticipantSecurityAttributesMask plugin_participant_attributes;

    inline ParticipantSecurityAttributesMask mask() const
    {
        ParticipantSecurityAttributesMask rv = PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_VALID;
        if (is_rtps_protected) rv |= PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_PROTECTED;
        if (is_discovery_protected) rv |= PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_DISCOVERY_PROTECTED;
        if (is_liveliness_protected) rv |= PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_LIVELINESS_PROTECTED;
        return rv;
    }

    inline bool match(const ParticipantSecurityAttributesMask remoteMask,
        const PluginParticipantSecurityAttributesMask remotePluginMask) const
    {
        return security_mask_matches(mask(), remoteMask) &&
            security_mask_matches(plugin_participant_attributes, remotePluginMask);
    }
};

}
}
}
}

#endif // _FASTDDS_RTPS_SECURITY_ACCESSCONTROL_PARTICIPANTSECURITYATTRIBUTES_H_
