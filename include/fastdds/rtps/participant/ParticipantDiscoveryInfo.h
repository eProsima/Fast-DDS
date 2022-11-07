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
 * @file ParticipantDiscoveryInfo.h
 *
 */

#ifndef _FASTDDS_RTPS_PARTICIPANT_PARTICIPANTDISCOVERYINFO_H__
#define _FASTDDS_RTPS_PARTICIPANT_PARTICIPANTDISCOVERYINFO_H__

#include <fastrtps/fastrtps_dll.h>
#include <fastdds/rtps/builtin/data/ParticipantProxyData.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

/**
 * Class ParticipantDiscoveryInfo with discovery information of the Participant.
 * @ingroup RTPS_MODULE
 */
struct ParticipantDiscoveryInfo
{
    //!Enum DISCOVERY_STATUS, four different status for discovered participants.
    //!@ingroup RTPS_MODULE
#if defined(_WIN32)
    enum RTPS_DllAPI DISCOVERY_STATUS
#else
    enum DISCOVERY_STATUS
#endif // if defined(_WIN32)
    {
        DISCOVERED_PARTICIPANT,
        CHANGED_QOS_PARTICIPANT,
        REMOVED_PARTICIPANT,
        DROPPED_PARTICIPANT
    };

    ParticipantDiscoveryInfo(
            const ParticipantProxyData& data)
        : status(DISCOVERED_PARTICIPANT)
        , info(data)
    {
    }

    virtual ~ParticipantDiscoveryInfo()
    {
    }

    //! Status
    DISCOVERY_STATUS status;

    /**
     * @brief Participant discovery info
     *
     * @todo This is a reference to an object that could be deleted, thus it should not be a reference
     * (intraprocess case -> BlackboxTests_DDS_PIM.DDSDiscovery.ParticipantProxyPhysicalData).
     */
    const ParticipantProxyData& info;
};

#if HAVE_SECURITY
struct ParticipantAuthenticationInfo
{
    enum RTPS_DllAPI AUTHENTICATION_STATUS
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
} // namespace fastrtps
} // namespace eprosima

#endif // _FASTDDS_RTPS_PARTICIPANT_PARTICIPANTDISCOVERYINFO_H__
