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
 * @file ParticipantProxyData.hpp
 *
 */

#ifndef RTPS_BUILTIN_DATA__PARTICIPANTPROXYDATA_HPP
#define RTPS_BUILTIN_DATA__PARTICIPANTPROXYDATA_HPP

#include <chrono>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/core/Types.hpp>
#include <fastdds/rtps/attributes/ReaderAttributes.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAllocationAttributes.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/builtin/data/BuiltinEndpoints.hpp>
#include <fastdds/rtps/builtin/data/ParticipantBuiltinTopicData.hpp>
#include <fastdds/rtps/common/ProductVersion_t.hpp>
#include <fastdds/rtps/common/RemoteLocators.hpp>
#include <fastdds/rtps/common/Token.hpp>
#include <fastdds/rtps/common/VendorId_t.hpp>

#include <rtps/network/NetworkFactory.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

struct CDRMessage_t;
class PDPSimple;
class TimedEvent;
class RTPSParticipantImpl;
class ReaderProxyData;
class WriterProxyData;
class NetworkFactory;

#if HAVE_SECURITY
namespace security {
struct ParticipantSecurityAttributes;
} // namespace security
#endif // if HAVE_SECURITY

// proxy specific declarations
template<class Proxy>
class ProxyHashTable;

/**
 * ParticipantProxyData class is used to store and convert the information Participants send to each other during the PDP phase.
 *@ingroup BUILTIN_MODULE
 */
class ParticipantProxyData : public ParticipantBuiltinTopicData
{
public:

    ParticipantProxyData(
            const RTPSParticipantAllocationAttributes& allocation);

    ParticipantProxyData(
            const ParticipantProxyData& pdata);

    virtual ~ParticipantProxyData();

    //!Protocol version
    ProtocolVersion_t m_protocol_version;
    //!Machine ID
    fastcdr::string_255 machine_id;
    //!Expects Inline QOS.
    bool m_expects_inline_qos;
    //!Available builtin endpoints
    BuiltinEndpointSet_t m_available_builtin_endpoints;
    //!Network configuration
    NetworkConfigSet_t m_network_configuration;
    //!Manual liveliness count
    Count_t m_manual_liveliness_count;
    //!
    InstanceHandle_t m_key;
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
    bool is_alive;
    //!
    TimedEvent* lease_duration_event;
    //!
    bool should_check_lease_duration;
    //!
    ProxyHashTable<ReaderProxyData>* m_readers = nullptr;
    //!
    ProxyHashTable<WriterProxyData>* m_writers = nullptr;

    SampleIdentity m_sample_identity;

    /**
     * Update the data.
     * @param pdata Object to copy the data from
     * @return True on success
     */
    bool update_data(
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
    bool write_to_cdr_message(
            CDRMessage_t* msg,
            bool write_encapsulation);

    /**
     * Read the parameter list from a received CDRMessage_t
     * @return True on success
     */
    bool read_from_cdr_message(
            CDRMessage_t* msg,
            bool use_encapsulation,
            NetworkFactory& network,
            bool should_filter_locators,
            fastdds::rtps::VendorId_t source_vendor_id = c_VendorId_eProsima);

    /**
     * Check if the host where the current process is running is the same as the one that sent the data.
     * It tries to use the machine_id. If it is not available, it will compare GUIDs.
     * @return True if the host is the same
     */
    bool is_from_this_host() const;

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
            const GUID_t& ps_guid);

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

private:

    //! Store the last timestamp it was received a RTPS message from the remote participant.
    std::chrono::steady_clock::time_point last_received_message_tm_;

    //! Remote participant lease duration in microseconds.
    std::chrono::microseconds lease_duration_;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // RTPS_BUILTIN_DATA__PARTICIPANTPROXYDATA_HPP
