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
 * @file ParticipantDiscoveryInfo.hpp
 *
 */

#ifndef FASTDDS_RTPS_PARTICIPANT__PARTICIPANTDISCOVERYINFO_HPP
#define FASTDDS_RTPS_PARTICIPANT__PARTICIPANTDISCOVERYINFO_HPP

#include <fastdds/fastdds_dll.hpp>
#include <fastdds/rtps/common/Guid.hpp>
namespace eprosima {
namespace fastdds {
namespace rtps {

//! Enum ParticipantDiscoveryStatus, four different status for discovered participants.
// *INDENT-OFF* eduponz: Does not understand the #if correctly and ends up removing the ;
//                       at the end of the enum, which does not build.
#if defined(_WIN32)
enum class FASTDDS_EXPORTED_API ParticipantDiscoveryStatus
#else
enum class ParticipantDiscoveryStatus
#endif // if defined(_WIN32)
{
    DISCOVERED_PARTICIPANT,
    CHANGED_QOS_PARTICIPANT,
    REMOVED_PARTICIPANT,
    DROPPED_PARTICIPANT,
    IGNORED_PARTICIPANT
};
// *INDENT-ON*

#if HAVE_SECURITY
struct ParticipantAuthenticationInfo
{
    enum FASTDDS_EXPORTED_API AUTHENTICATION_STATUS
    {
        AUTHORIZED_PARTICIPANT,
        UNAUTHORIZED_PARTICIPANT
    };

    ParticipantAuthenticationInfo()
        : status(UNAUTHORIZED_PARTICIPANT)
    {
    }

    ~ParticipantAuthenticationInfo()
    {
    }

    //! Status
    AUTHENTICATION_STATUS status;

    //! Associated GUID
    GUID_t guid;
};

inline bool operator ==(
        const ParticipantAuthenticationInfo& l,
        const ParticipantAuthenticationInfo& r)
{
    return l.status == r.status &&
           l.guid == r.guid;
}

#endif // if HAVE_SECURITY

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_PARTICIPANT__PARTICIPANTDISCOVERYINFO_HPP
