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

#ifndef _FASTDDS_RTPS_BUILTIN_DATA_PARTICIPANTPROXYDATA_H_
#define _FASTDDS_RTPS_BUILTIN_DATA_PARTICIPANTPROXYDATA_H_

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include <fastrtps/qos/QosPolicies.h>

#include <fastdds/rtps/attributes/RTPSParticipantAllocationAttributes.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.h>
#include <fastdds/rtps/attributes/ReaderAttributes.h>
#include <fastdds/rtps/common/Token.h>
#include <fastdds/rtps/common/RemoteLocators.hpp>

#if HAVE_SECURITY
#include <fastdds/rtps/security/accesscontrol/ParticipantSecurityAttributes.h>
#endif // if HAVE_SECURITY

#include <chrono>

#define BUILTIN_PARTICIPANT_DATA_MAX_SIZE 100
#define TYPELOOKUP_DATA_MAX_SIZE 5000

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
#define BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REQUEST_DATA_WRITER  (0x00000001 << 12)
#define BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REQUEST_DATA_READER  (0x00000001 << 13)
#define BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REPLY_DATA_WRITER    (0x00000001 << 14)
#define BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REPLY_DATA_READER    (0x00000001 << 15)
#define DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_ANNOUNCER       (0x00000001 << 16)
#define DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_DETECTOR        (0x00000001 << 17)
#define DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_ANNOUNCER      (0x00000001 << 18)
#define DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_DETECTOR       (0x00000001 << 19)
#define BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_SECURE_DATA_WRITER  (0x00000001 << 20)
#define BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_SECURE_DATA_READER  (0x00000001 << 21)
#define DISC_BUILTIN_ENDPOINT_PARTICIPANT_SECURE_ANNOUNCER       (0x00000001 << 26)
#define DISC_BUILTIN_ENDPOINT_PARTICIPANT_SECURE_DETECTOR        (0x00000001 << 27)

namespace eprosima {
namespace fastrtps {
namespace rtps {

struct CDRMessage_t;
class PDPSimple;
class TimedEvent;
class RTPSParticipantImpl;
class ReaderProxyData;
class WriterProxyData;
class NetworkFactory;

// proxy specific declarations
template<class Proxy>
class ProxyHashTable;

/**
 * ParticipantProxyData class is used to store and convert the information Participants send to each other during the PDP phase.
 *@ingroup BUILTIN_MODULE
 */
class ParticipantProxyData
{
public:

    ParticipantProxyData(
            const RTPSParticipantAllocationAttributes& allocation);

    ParticipantProxyData(
            const ParticipantProxyData& pdata);

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
    //!Metatraffic locators
    RemoteLocatorList metatraffic_locators;
    //!Default locators
    RemoteLocatorList default_locators;
    //!Manual liveliness count
    Count_t m_manualLivelinessCount;
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
#endif // if HAVE_SECURITY
    //!
    bool isAlive;
    //!
    ParameterPropertyList_t m_properties;
    //!
    UserDataQosPolicy m_userData;
    //!
    TimedEvent* lease_duration_event;
    //!
    bool should_check_lease_duration;
    //!
    ProxyHashTable<ReaderProxyData>* m_readers = nullptr;
    //!
    ProxyHashTable<WriterProxyData>* m_writers = nullptr;

    /**
     * Update the data.
     * @param pdata Object to copy the data from
     * @return True on success
     */
    bool updateData(
            ParticipantProxyData& pdata);

    /**
     * Get the size in bytes of the CDR serialization of this object.
     * @param include_encapsulation Whether to include the size of the encapsulation info.
     * @return size in bytes of the CDR serialization.
     */
    uint32_t get_serialized_size(
            bool include_encapsulation) const;

    /**
     * Write as a parameter list on a CDRMessage_t
     * @return True on success
     */
    bool writeToCDRMessage(
            CDRMessage_t* msg,
            bool write_encapsulation);

    /**
     * Read the parameter list from a recevied CDRMessage_t
     * @return True on success
     */
    bool readFromCDRMessage(
            CDRMessage_t* msg,
            bool use_encapsulation,
            const NetworkFactory& network,
            bool is_shm_transport_available);

    //! Clear the data (restore to default state).
    void clear();

    /**
     * Copy the data from another object.
     * @param pdata Object to copy the data from
     */
    void copy(
            const ParticipantProxyData& pdata);

    /**
     * Set participant persistent GUID_t
     * @param guid valid GUID_t
     */
    void set_persistence_guid(
            const GUID_t& guid);

    /**
     * Retrieve participant persistent GUID_t
     * @return guid persistent GUID_t or c_Guid_Unknown
     */
    GUID_t get_persistence_guid() const;

    /**
     * Set participant client server sample identity
     * @param sid valid SampleIdentity
     */
    void set_sample_identity(
            const SampleIdentity& sid);

    /**
     * Retrieve participant SampleIdentity
     * @return SampleIdentity
     */
    SampleIdentity get_sample_identity() const;

    /**
     * Identifies the participant as client of the given server
     * @param guid valid backup server GUID
     */
    void set_backup_stamp(
            const GUID_t& guid);

    /**
     * Retrieves BACKUP server stamp. On deserialization hints if lease duration must be enforced
     * @return GUID
     */
    GUID_t get_backup_stamp() const;

    void assert_liveliness();

    const std::chrono::steady_clock::time_point& last_received_message_tm() const
    {
        return last_received_message_tm_;
    }

    const std::chrono::microseconds& lease_duration() const
    {
        return lease_duration_;
    }

private:

    //! Store the last timestamp it was received a RTPS message from the remote participant.
    std::chrono::steady_clock::time_point last_received_message_tm_;

    //! Remote participant lease duration in microseconds.
    std::chrono::microseconds lease_duration_;
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#endif // _FASTDDS_RTPS_BUILTIN_DATA_PARTICIPANTPROXYDATA_H_
