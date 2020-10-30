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

/*!
 * @file SecurityManager.h
 */
#ifndef _RTPS_SECURITY_SECURITYMANAGER_H_
#define _RTPS_SECURITY_SECURITYMANAGER_H_

#include <rtps/security/SecurityPluginFactory.h>

#include <fastdds/rtps/security/authentication/Handshake.h>
#include <fastdds/rtps/security/common/ParticipantGenericMessage.h>
#include <fastdds/rtps/reader/ReaderListener.h>
#include <fastdds/rtps/common/SequenceNumber.h>
#include <fastdds/rtps/common/SerializedPayload.h>
#include <fastdds/rtps/builtin/data/ReaderProxyData.h>
#include <fastdds/rtps/builtin/data/WriterProxyData.h>
#include <fastdds/rtps/builtin/data/ParticipantProxyData.h>
#include <fastdds/rtps/attributes/HistoryAttributes.h>
#include <fastdds/rtps/resources/TimedEvent.h>

#include <map>
#include <mutex>
#include <atomic>
#include <memory>
#include <list>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class RTPSParticipantImpl;
class StatelessWriter;
class StatelessReader;
class StatefulWriter;
class StatefulReader;
class WriterHistory;
class ReaderHistory;
class ITopicPayloadPool;

namespace security {

class Authentication;
class AccessControl;
class Cryptography;
struct ParticipantSecurityAttributes;
struct EndpointSecurityAttributes;

class SecurityManager
{
public:

    SecurityManager(
            RTPSParticipantImpl* participant);

    ~SecurityManager();

    bool init(
            ParticipantSecurityAttributes& attributes,
            const PropertyPolicy& participant_properties,
            bool& security_activated);

    bool create_entities();

    void destroy();

    bool discovered_participant(
            const ParticipantProxyData& participant_data);

    void remove_participant(
            const ParticipantProxyData& participant_data);

    bool register_local_writer(
            const GUID_t& writer_guid,
            const PropertyPolicy& writer_properties,
            EndpointSecurityAttributes& security_attributes);

    bool register_local_builtin_writer(
            const GUID_t& writer_guid,
            EndpointSecurityAttributes& security_attributes);

    bool unregister_local_writer(
            const GUID_t& writer_guid);

    bool register_local_reader(
            const GUID_t& reader_guid,
            const PropertyPolicy& reader_properties,
            EndpointSecurityAttributes& security_attributes);

    bool register_local_builtin_reader(
            const GUID_t& reader_guid,
            EndpointSecurityAttributes& security_attributes);

    bool unregister_local_reader(
            const GUID_t& reader_guid);

    bool discovered_reader(
            const GUID_t& writer_guid,
            const GUID_t& remote_participant,
            ReaderProxyData& remote_reader_data,
            const EndpointSecurityAttributes& security_attributes);

    void remove_reader(
            const GUID_t& writer_guid,
            const GUID_t& remote_participant,
            const GUID_t& remote_reader_guid);

    bool discovered_builtin_reader(
            const GUID_t& writer_guid,
            const GUID_t& remote_participant,
            ReaderProxyData& remote_reader_data,
            const EndpointSecurityAttributes& security_attributes);

    bool discovered_writer(
            const GUID_t& reader_guid,
            const GUID_t& remote_participant,
            WriterProxyData& remote_writer_guid,
            const EndpointSecurityAttributes& security_attributes);

    void remove_writer(
            const GUID_t& reader_guid,
            const GUID_t& remote_participant,
            const GUID_t& remote_writer_guid);

    bool discovered_builtin_writer(
            const GUID_t& reader_guid,
            const GUID_t& remote_participant,
            WriterProxyData& remote_writer_guid,
            const EndpointSecurityAttributes& security_attributes);

    bool get_identity_token(
            IdentityToken** identity_token);

    bool return_identity_token(
            IdentityToken* identity_token);

    bool get_permissions_token(
            PermissionsToken** permissions_token);

    bool return_permissions_token(
            PermissionsToken* permissions_token);

    uint32_t builtin_endpoints();

    RTPSParticipantImpl* participant()
    {
        return participant_;
    }

    bool encode_rtps_message(
            const CDRMessage_t& input_message,
            CDRMessage_t& output_message,
            const std::vector<GuidPrefix_t>& receiving_list);

    int decode_rtps_message(
            const CDRMessage_t& message,
            CDRMessage_t& out_message,
            const GuidPrefix_t& sending_participant);

    bool encode_writer_submessage(
            const CDRMessage_t& input_message,
            CDRMessage_t& output_message,
            const GUID_t& writer_guid,
            const std::vector<GUID_t>& receiving_list);

    bool encode_reader_submessage(
            const CDRMessage_t& input_message,
            CDRMessage_t& output_message,
            const GUID_t& reader_guid,
            const std::vector<GUID_t>& receiving_list);

    int decode_rtps_submessage(
            CDRMessage_t& message,
            CDRMessage_t& out_message,
            const GuidPrefix_t& sending_participant);

    bool encode_serialized_payload(
            const SerializedPayload_t& payload,
            SerializedPayload_t& output_payload,
            const GUID_t& writer_guid);

    bool decode_serialized_payload(
            const SerializedPayload_t& secure_payload,
            SerializedPayload_t& payload,
            const GUID_t& reader_guid,
            const GUID_t& writer_guid);

    uint32_t calculate_extra_size_for_rtps_message();

    uint32_t calculate_extra_size_for_rtps_submessage(
            const GUID_t& writer_guid);

    uint32_t calculate_extra_size_for_encoded_payload(
            const GUID_t& writer_guid);

private:

    enum AuthenticationStatus : uint32_t
    {
        AUTHENTICATION_OK = 0,
        AUTHENTICATION_FAILED,
        AUTHENTICATION_INIT,
        AUTHENTICATION_REQUEST_NOT_SEND,
        AUTHENTICATION_WAITING_REQUEST,
        AUTHENTICATION_WAITING_REPLY,
        AUTHENTICATION_WAITING_FINAL
    };

    class DiscoveredParticipantInfo
    {
        struct AuthenticationInfo
        {
        public:

            AuthenticationInfo(
                    AuthenticationStatus auth_status)
                : identity_handle_(nullptr)
                , handshake_handle_(nullptr)
                , auth_status_(auth_status)
                , expected_sequence_number_(0)
                , change_sequence_number_(SequenceNumber_t::unknown())
                , event_(nullptr)
            {
            }

            AuthenticationInfo(
                    AuthenticationInfo&& auth)
                : identity_handle_(std::move(auth.identity_handle_))
                , handshake_handle_(std::move(auth.handshake_handle_))
                , auth_status_(auth.auth_status_)
                , expected_sequence_number_(auth.expected_sequence_number_)
                , change_sequence_number_(std::move(auth.change_sequence_number_))
                , event_(std::move(auth.event_))
            {
            }

            IdentityHandle* identity_handle_;

            HandshakeHandle* handshake_handle_;

            AuthenticationStatus auth_status_;

            int64_t expected_sequence_number_;

            SequenceNumber_t change_sequence_number_;

            TimedEvent* event_;

        private:

            AuthenticationInfo(
                    const AuthenticationInfo& auth) = delete;
        };

    public:

        typedef std::unique_ptr<AuthenticationInfo> AuthUniquePtr;

        DiscoveredParticipantInfo(
                AuthenticationStatus auth_status,
                const ParticipantProxyData& participant_data)
            : auth_(new AuthenticationInfo(auth_status))
            , shared_secret_handle_(nullptr)
            , permissions_handle_(nullptr)
            , participant_crypto_(nullptr)
            , participant_data_(participant_data)
        {
        }

        DiscoveredParticipantInfo(
                DiscoveredParticipantInfo&& info)
            : auth_(std::move(info.auth_))
            , shared_secret_handle_(std::move(info.shared_secret_handle_))
            , permissions_handle_(std::move(info.permissions_handle_))
            , participant_crypto_(info.participant_crypto_)
            , participant_data_(std::move(info.participant_data_))
        {
        }

        AuthUniquePtr get_auth()
        {
            return std::move(auth_);
        }

        void set_auth(
                AuthUniquePtr& auth)
        {
            auth_ = std::move(auth);
        }

        void set_shared_secret(
                SharedSecretHandle* shared_secret)
        {
            shared_secret_handle_ = shared_secret;
        }

        SharedSecretHandle* get_shared_secret()
        {
            return shared_secret_handle_;
        }

        void set_permissions_handle(
                PermissionsHandle* handle)
        {
            permissions_handle_ = handle;
        }

        PermissionsHandle* get_permissions_handle()
        {
            return permissions_handle_;
        }

        const PermissionsHandle* get_permissions_handle() const
        {
            return permissions_handle_;
        }

        void set_participant_crypto(
                ParticipantCryptoHandle* participant_crypto)
        {
            participant_crypto_ = participant_crypto;
        }

        ParticipantCryptoHandle* get_participant_crypto()
        {
            return participant_crypto_;
        }

        const ParticipantProxyData& participant_data() const
        {
            return participant_data_;
        }

    private:

        DiscoveredParticipantInfo(
                const DiscoveredParticipantInfo& info) = delete;

        AuthUniquePtr auth_;

        SharedSecretHandle* shared_secret_handle_;

        PermissionsHandle* permissions_handle_;

        ParticipantCryptoHandle* participant_crypto_;

        ParticipantProxyData participant_data_;

    };

    class ParticipantStatelessMessageListener : public eprosima::fastrtps::rtps::ReaderListener
    {
    public:

        ParticipantStatelessMessageListener(
                SecurityManager& manager)
            : manager_(manager)
        {
        }

        ~ParticipantStatelessMessageListener()
        {
        }

        void onNewCacheChangeAdded(
                RTPSReader* reader,
                const CacheChange_t* const change) override;

    private:

        ParticipantStatelessMessageListener& operator =(
                const ParticipantStatelessMessageListener&) = delete;

        SecurityManager& manager_;
    }
    participant_stateless_message_listener_;

    class ParticipantVolatileMessageListener : public eprosima::fastrtps::rtps::ReaderListener
    {
    public:

        ParticipantVolatileMessageListener(
                SecurityManager& manager)
            : manager_(manager)
        {
        }

        ~ParticipantVolatileMessageListener()
        {
        }

        void onNewCacheChangeAdded(
                RTPSReader* reader,
                const CacheChange_t* const change) override;

    private:

        ParticipantVolatileMessageListener& operator =(
                const ParticipantVolatileMessageListener&) = delete;

        SecurityManager& manager_;
    }
    participant_volatile_message_secure_listener_;

    void cancel_init();

    void remove_discovered_participant_info(
            DiscoveredParticipantInfo::AuthUniquePtr&& auth_ptr);

    bool restore_discovered_participant_info(
            const GUID_t& remote_participant_key,
            DiscoveredParticipantInfo::AuthUniquePtr& auth_ptr);

    void delete_entities();
    bool create_participant_stateless_message_entities();
    void delete_participant_stateless_message_entities();
    void create_participant_stateless_message_pool();
    void delete_participant_stateless_message_pool();
    bool create_participant_stateless_message_writer();
    void delete_participant_stateless_message_writer();
    bool create_participant_stateless_message_reader();
    void delete_participant_stateless_message_reader();
    bool create_participant_volatile_message_secure_entities();
    void delete_participant_volatile_message_secure_entities();
    void create_participant_volatile_message_secure_pool();
    void delete_participant_volatile_message_secure_pool();
    bool create_participant_volatile_message_secure_writer();
    void delete_participant_volatile_message_secure_writer();
    bool create_participant_volatile_message_secure_reader();
    void delete_participant_volatile_message_secure_reader();

    bool discovered_reader(
            const GUID_t& writer_guid,
            const GUID_t& remote_participant,
            ReaderProxyData& remote_reader_data,
            const EndpointSecurityAttributes& security_attributes,
            bool is_builtin);

    bool discovered_writer(
            const GUID_t& reader_guid,
            const GUID_t& remote_participant,
            WriterProxyData& remote_writer_guid,
            const EndpointSecurityAttributes& security_attributes,
            bool is_builtin);

    void match_builtin_endpoints(
            const ParticipantProxyData& participant_data);

    void match_builtin_key_exchange_endpoints(
            const ParticipantProxyData& participant_data);

    void unmatch_builtin_endpoints(
            const ParticipantProxyData& participant_data);

    ParticipantCryptoHandle* register_and_match_crypto_endpoint(
            IdentityHandle& remote_participant_identity,
            SharedSecretHandle& shared_secret);

    void exchange_participant_crypto(
            ParticipantCryptoHandle* remote_participant_crypto,
            const GUID_t& remote_participant_guid);

    void process_participant_stateless_message(
            const CacheChange_t* const change);

    void process_participant_volatile_message_secure(
            const CacheChange_t* const change);

    bool on_process_handshake(
            const ParticipantProxyData& participant_data,
            DiscoveredParticipantInfo::AuthUniquePtr& remote_participant_info,
            MessageIdentity&& message_identity,
            HandshakeMessageToken&& message);

    ParticipantGenericMessage generate_authentication_message(
            const MessageIdentity& related_message_identity,
            const GUID_t& destination_participant_key,
            HandshakeMessageToken& handshake_message);

    ParticipantGenericMessage generate_participant_crypto_token_message(
            const GUID_t& destination_participant_key,
            ParticipantCryptoTokenSeq& crypto_tokens);

    ParticipantGenericMessage generate_writer_crypto_token_message(
            const GUID_t& destination_participant_key,
            const GUID_t& destination_endpoint_key,
            const GUID_t& source_endpoint_key,
            ParticipantCryptoTokenSeq& crypto_tokens);

    ParticipantGenericMessage generate_reader_crypto_token_message(
            const GUID_t& destination_participant_key,
            const GUID_t& destination_endpoint_key,
            const GUID_t& source_endpoint_key,
            ParticipantCryptoTokenSeq& crypto_tokens);

    bool participant_authorized(
            const ParticipantProxyData& participant_data,
            const DiscoveredParticipantInfo::AuthUniquePtr& remote_participant_info,
            SharedSecretHandle* shared_secret_handle);

    void resend_handshake_message_token(
            const GUID_t& remote_participant_key);

    RTPSParticipantImpl* participant_;
    StatelessWriter* participant_stateless_message_writer_;
    WriterHistory* participant_stateless_message_writer_history_;
    StatelessReader* participant_stateless_message_reader_;
    ReaderHistory* participant_stateless_message_reader_history_;
    StatefulWriter* participant_volatile_message_secure_writer_;
    WriterHistory* participant_volatile_message_secure_writer_history_;
    StatefulReader* participant_volatile_message_secure_reader_;
    ReaderHistory* participant_volatile_message_secure_reader_history_;
    SecurityPluginFactory factory_;

    Logging* logging_plugin_;

    Authentication* authentication_plugin_;

    AccessControl* access_plugin_;

    Cryptography* crypto_plugin_;

    uint32_t domain_id_;

    IdentityHandle* local_identity_handle_;

    PermissionsHandle* local_permissions_handle_;

    ParticipantCryptoHandle* local_participant_crypto_handle_;

    std::map<GUID_t, DiscoveredParticipantInfo> discovered_participants_;

    GUID_t auth_source_guid;

    std::mutex mutex_;

    std::atomic<int64_t> auth_last_sequence_number_;

    std::atomic<int64_t> crypto_last_sequence_number_;

    struct DatawriterAssociations
    {
        DatawriterAssociations(
                DatawriterCryptoHandle* wh)
            : writer_handle(wh)
        {
        }

        DatawriterCryptoHandle* writer_handle;

        std::map<GUID_t, std::tuple<ReaderProxyData, DatareaderCryptoHandle*>> associated_readers;
    };

    struct DatareaderAssociations
    {
        DatareaderAssociations(
                DatareaderCryptoHandle* rh)
            : reader_handle(rh)
        {
        }

        DatareaderCryptoHandle* reader_handle;

        std::map<GUID_t, std::tuple<WriterProxyData, DatawriterCryptoHandle*>> associated_writers;
    };

    // TODO(Ricardo) Temporal. Store individual in FastRTPS code.
    std::map<GUID_t, DatawriterAssociations> writer_handles_;
    std::map<GUID_t, DatareaderAssociations> reader_handles_;

    std::map<GUID_t, DataHolderSeq> remote_participant_pending_messages_;
    std::map<std::pair<GUID_t, GUID_t>, DataHolderSeq> remote_writer_pending_messages_;
    std::map<std::pair<GUID_t, GUID_t>, DataHolderSeq> remote_reader_pending_messages_;
    std::list<std::tuple<ReaderProxyData, GUID_t, GUID_t>> remote_reader_pending_discovery_messages_;
    std::list<std::tuple<WriterProxyData, GUID_t, GUID_t>> remote_writer_pending_discovery_messages_;

    std::mutex temp_stateless_data_lock_;
    ReaderProxyData temp_stateless_reader_proxy_data_;
    WriterProxyData temp_stateless_writer_proxy_data_;
    std::mutex temp_volatile_data_lock_;
    ReaderProxyData temp_volatile_reader_proxy_data_;
    WriterProxyData temp_volatile_writer_proxy_data_;

    HistoryAttributes participant_stateless_message_writer_hattr_;
    HistoryAttributes participant_stateless_message_reader_hattr_;
    std::shared_ptr<ITopicPayloadPool> participant_stateless_message_pool_;

    HistoryAttributes participant_volatile_message_secure_hattr_;
    std::shared_ptr<ITopicPayloadPool> participant_volatile_message_secure_pool_;
};

} //namespace security
} //namespace rtps
} //namespace fastrtps
} //namespace eprosima

#endif // _RTPS_SECURITY_SECURITYMANAGER_H_
