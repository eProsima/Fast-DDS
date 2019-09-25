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
 * @file ParticipantProxyData.h
 *
 */

#ifndef _RTPS_BUILTIN_DATA_PARTICIPANTPROXYDATA_H_
#define _RTPS_BUILTIN_DATA_PARTICIPANTPROXYDATA_H_

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include "../../../qos/ParameterList.h"

#include "../../attributes/WriterAttributes.h"
#include "../../attributes/ReaderAttributes.h"
#include "../../common/Token.h"

#if HAVE_SECURITY
#include "../../security/accesscontrol/ParticipantSecurityAttributes.h"
#endif

#define DISCOVERY_PARTICIPANT_DATA_MAX_SIZE 5000
#define DISCOVERY_TOPIC_DATA_MAX_SIZE 500
#define DISCOVERY_PUBLICATION_DATA_MAX_SIZE 5000
#define DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE 5000
#define BUILTIN_PARTICIPANT_DATA_MAX_SIZE 100

#define DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER              (0x00000001 << 0)
#define DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR               (0x00000001 << 1)
#define DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER              (0x00000001 << 2)
#define DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR               (0x00000001 << 3)
#define DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER             (0x00000001 << 4)
#define DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR              (0x00000001 << 5)
#define DISC_BUILTIN_ENDPOINT_PARTICIPANT_PROXY_ANNOUNCER        (0x00000001 << 6)
#define DISC_BUILTIN_ENDPOINT_PARTICIPANT_PROXY_DETECTOR         (0x00000001 << 7)
#define DISC_BUILTIN_ENDPOINT_PARTICIPANT_STATE_ANNOUNCER        (0x00000001 << 8)
#define DISC_BUILTIN_ENDPOINT_PARTICIPANT_STATE_DETECTOR         (0x00000001 << 9)
#define BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER         (0x00000001 << 10)
#define BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER         (0x00000001 << 11)
#define DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_ANNOUNCER       (0x00000001 << 16)
#define DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_DETECTOR        (0x00000001 << 17)
#define DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_ANNOUNCER      (0x00000001 << 18)
#define DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_DETECTOR       (0x00000001 << 19)
#define BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_SECURE_DATA_WRITER  (0x00000001 << 20)
#define BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_SECURE_DATA_READER  (0x00000001 << 21)
#define DISC_BUILTIN_ENDPOINT_PARTICIPANT_SECURE_ANNOUNCER       (0x00000001 << 26)
#define DISC_BUILTIN_ENDPOINT_PARTICIPANT_SECURE_DETECTOR        (0x00000001 << 27)

namespace eprosima {
namespace fastrtps{
namespace rtps {

struct CDRMessage_t;
class PDPSimple;
class RemoteParticipantLeaseDuration;
class RTPSParticipantImpl;
class ReaderProxyData;
class WriterProxyData;

/**
* ParticipantProxyData class is used to store and convert the information Participants send to each other during the PDP phase.
*@ingroup BUILTIN_MODULE
*/
class ParticipantProxyData
{
    public:

        ParticipantProxyData();

        ParticipantProxyData(const ParticipantProxyData& pdata);

        virtual ~ParticipantProxyData();

        //!Protocol version
        ProtocolVersion_t m_protocolVersion;
        //!GUID
        GUID_t m_guid;
        //!Vendor ID
        VendorId_t m_VendorId;
        //!Expects Inline QOS.
        bool m_expectsInlineQos;
        //!Available builtin endpoints
        BuiltinEndpointSet_t m_availableBuiltinEndpoints;
        //!Metatraffic unicast locator list
        LocatorList_t m_metatrafficUnicastLocatorList;
        //!Metatraffic multicast locator list
        LocatorList_t m_metatrafficMulticastLocatorList;
        //!Default unicast locator list
        LocatorList_t m_defaultUnicastLocatorList;
        //!Default multicast locator list
        LocatorList_t m_defaultMulticastLocatorList;
        //!Manual liveliness count
        Count_t m_manualLivelinessCount; // TODO(MiguelC): remove when safe to change ABI
        //!Participant name
        string_255 m_participantName;
        //!
        InstanceHandle_t m_key;
        //!
        Duration_t m_leaseDuration;
#if HAVE_SECURITY
        //!
        IdentityToken identity_token_;
        //!
        PermissionsToken permissions_token_;
        //!
        security::ParticipantSecurityAttributesMask security_attributes_;
        //!
        security::PluginParticipantSecurityAttributesMask plugin_security_attributes_;
#endif
        //!
        bool isAlive;
        //!
        ParameterPropertyList_t m_properties;
        //!
        std::vector<octet> m_userData;
        //!
        RemoteParticipantLeaseDuration* mp_leaseDurationTimer;
        //!
        std::vector<ReaderProxyData*> m_readers;
        //!
        std::vector<WriterProxyData*> m_writers;

        /**
         * Update the data.
         * @param pdata Object to copy the data from
         * @return True on success
         */
        bool updateData(ParticipantProxyData& pdata);

        /**
         * Write as a parameter list on a CDRMessage_t
         * @return True on success
         */
        bool writeToCDRMessage(CDRMessage_t* msg, bool write_encapsulation);

        /**
         * Read the parameter list from a recevied CDRMessage_t
         * @return True on success
         */
        bool readFromCDRMessage(CDRMessage_t* msg, bool use_encapsulation=true);

        //!Clear the data (restore to default state.)
        void clear();

        /**
         * Copy the data from another object.
         * @param pdata Object to copy the data from
         */
        void copy(ParticipantProxyData& pdata);
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif

#endif // _RTPS_BUILTIN_DATA_PARTICIPANTPROXYDATA_H_
