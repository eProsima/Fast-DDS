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

#include <fastrtps/rtps/security/authentication/Handshake.h>
#include <fastrtps/rtps/security/common/ParticipantGenericMessage.h>
#include <fastrtps/rtps/reader/ReaderListener.h>
#include <fastrtps/rtps/common/SequenceNumber.h>
#include "timedevent/HandshakeMessageTokenResent.h"
#include <fastrtps/rtps/common/SerializedPayload.h>
#include <fastrtps/rtps/builtin/data/ReaderProxyData.h>
#include <fastrtps/rtps/builtin/data/WriterProxyData.h>
#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>

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

namespace security {

class Authentication;
class Cryptography;

class SecurityManager
{
    friend class HandshakeMessageTokenResent;

    public:

        SecurityManager(RTPSParticipantImpl* participant);

        ~SecurityManager();

        bool init();

        void destroy();

        bool discovered_participant(const ParticipantProxyData& participant_data);

        void remove_participant(const ParticipantProxyData& participant_data);

        bool register_local_writer(const GUID_t& writer_guid, const PropertySeq& writer_properties);

        bool unregister_local_writer(const GUID_t& writer_guid);

        bool register_local_reader(const GUID_t& reader_guid, const PropertySeq& reader_properties);

        bool unregister_local_reader(const GUID_t& reader_guid);

        bool discovered_reader(const GUID_t& writer_guid, const GUID_t& remote_participant,
                ReaderProxyData& remote_reader_data);

        void remove_reader(const GUID_t& writer_guid, const GUID_t& remote_participant,
                const GUID_t& remote_reader_guid);

        bool discovered_writer(const GUID_t& reader_guid, const GUID_t& remote_participant,
                WriterProxyData& remote_writer_guid);

        void remove_writer(const GUID_t& reader_guid, const GUID_t& remote_participant,
                const GUID_t& remote_writer_guid);

        bool get_identity_token(IdentityToken** identity_token);

        bool return_identity_token(IdentityToken* identity_token);

        uint32_t builtin_endpoints();

        RTPSParticipantImpl* participant() { return participant_; }

        bool encode_rtps_message(CDRMessage_t& message,
                const std::vector<GuidPrefix_t>& receiving_list);

        int decode_rtps_message(CDRMessage_t& message, CDRMessage_t& out_message,
                const GuidPrefix_t& sending_participant);

        bool encode_writer_submessage(CDRMessage_t& message, const GUID_t& writer_guid,
                const std::vector<GUID_t>& receiving_list);

        bool encode_reader_submessage(CDRMessage_t& message, const GUID_t& reader_guid,
                const std::vector<GUID_t>& receiving_list);

        int decode_rtps_submessage(CDRMessage_t& message, CDRMessage_t& out_message,
                const GuidPrefix_t& sending_participant);

        bool encode_serialized_payload(const SerializedPayload_t& payload, CDRMessage_t& output_message,
                const GUID_t& writer_guid);

        bool decode_serialized_payload(const SerializedPayload_t& secure_payload,
                SerializedPayload_t& payload, const GUID_t& reader_guid, const GUID_t& writer_guid);

        uint32_t calculate_extra_size_for_rtps_message();

        uint32_t calculate_extra_size_for_rtps_submessage(const GUID_t& writer_guid);

        uint32_t calculate_extra_size_for_encoded_payload(const GUID_t& writer_guid);

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

                    AuthenticationInfo(AuthenticationStatus auth_status) :
                        identity_handle_(nullptr), handshake_handle_(nullptr),
                        auth_status_(auth_status), expected_sequence_number_(0),
                        change_sequence_number_(SequenceNumber_t::unknown()),
                        event_(nullptr) {}

                    AuthenticationInfo(AuthenticationInfo&& auth) :
                        identity_handle_(std::move(auth.identity_handle_)),
                        handshake_handle_(std::move(auth.handshake_handle_)),
                        auth_status_(auth.auth_status_),
                        expected_sequence_number_(auth.expected_sequence_number_),
                        change_sequence_number_(std::move(auth.change_sequence_number_)),
                        event_(std::move(auth.event_)) {}

                    IdentityHandle* identity_handle_;

                    HandshakeHandle* handshake_handle_;

                    AuthenticationStatus auth_status_;

                    int64_t expected_sequence_number_;

                    SequenceNumber_t change_sequence_number_;

                    HandshakeMessageTokenResent* event_;

                private:

                    AuthenticationInfo(const AuthenticationInfo& auth) = delete;
            };

            struct EmptyDelete
            {
                void operator()(AuthenticationInfo*) {};
            };

            public:

                typedef std::unique_ptr<AuthenticationInfo, EmptyDelete> AuthUniquePtr;

                DiscoveredParticipantInfo(AuthenticationStatus auth_status) :
                    auth_(auth_status), auth_ptr_(&auth_),
                    shared_secret_handle_(nullptr),
                    participant_crypto_(nullptr) {}

                DiscoveredParticipantInfo(DiscoveredParticipantInfo&& info) :
                    auth_(std::move(info.auth_)),  auth_ptr_(&auth_),
                    shared_secret_handle_(std::move(info.shared_secret_handle_)),
                    participant_crypto_(info.participant_crypto_) {}

                AuthUniquePtr get_auth() { return std::move(auth_ptr_); }

                void set_auth(AuthUniquePtr& auth)
                {
                    assert(auth.get() == &auth_);
                    auth_ptr_ = std::move(auth); 
                }

                void set_shared_secret(SharedSecretHandle* shared_secret)
                {
                    shared_secret_handle_ = shared_secret;
                }

                SharedSecretHandle* get_shared_secret()
                {
                    return shared_secret_handle_;
                }

                void set_participant_crypto(ParticipantCryptoHandle* participant_crypto)
                {
                    participant_crypto_ = participant_crypto;
                }

                ParticipantCryptoHandle* get_participant_crypto()
                {
                    return participant_crypto_;
                }

            private:

                DiscoveredParticipantInfo(const DiscoveredParticipantInfo& info) = delete;

                AuthenticationInfo auth_;

                AuthUniquePtr auth_ptr_;

                SharedSecretHandle* shared_secret_handle_;

                ParticipantCryptoHandle* participant_crypto_;

        };

        class ParticipantStatelessMessageListener: public eprosima::fastrtps::rtps::ReaderListener
        {
            public:
                ParticipantStatelessMessageListener(SecurityManager &manager) : manager_(manager) {};

                ~ParticipantStatelessMessageListener(){};

                void onNewCacheChangeAdded(RTPSReader* reader, const CacheChange_t* const change);

            private:

                ParticipantStatelessMessageListener& operator=(const ParticipantStatelessMessageListener&) = delete;

                SecurityManager &manager_;
        } participant_stateless_message_listener_;

        class ParticipantVolatileMessageListener: public eprosima::fastrtps::rtps::ReaderListener
        {
            public:
                ParticipantVolatileMessageListener(SecurityManager &manager) : manager_(manager) {};

                ~ParticipantVolatileMessageListener(){};

                void onNewCacheChangeAdded(RTPSReader* reader, const CacheChange_t* const change);

            private:

                ParticipantVolatileMessageListener& operator=(const ParticipantVolatileMessageListener&) = delete;

                SecurityManager &manager_;
        } participant_volatile_message_secure_listener_;

        void remove_discovered_participant_info(DiscoveredParticipantInfo::AuthUniquePtr& auth_ptr);
        bool restore_discovered_participant_info(const GUID_t& remote_participant_key,
                DiscoveredParticipantInfo::AuthUniquePtr& auth_ptr);

        bool create_entities();
        void delete_entities();
        bool create_participant_stateless_message_entities();
        void delete_participant_stateless_message_entities();
        bool create_participant_stateless_message_writer();
        void delete_participant_stateless_message_writer();
        bool create_participant_stateless_message_reader();
        void delete_participant_stateless_message_reader();
        bool create_participant_volatile_message_secure_entities();
        void delete_participant_volatile_message_secure_entities();
        bool create_participant_volatile_message_secure_writer();
        void delete_participant_volatile_message_secure_writer();
        bool create_participant_volatile_message_secure_reader();
        void delete_participant_volatile_message_secure_reader();

        void match_builtin_endpoints(const ParticipantProxyData& participant_data);

        void unmatch_builtin_endpoints(const ParticipantProxyData& participant_data);

        ParticipantCryptoHandle* register_and_match_crypto_endpoint(const GUID_t& remote_participant_guid, IdentityHandle& remote_participant_identity,
                SharedSecretHandle& shared_secret);

        void process_participant_stateless_message(const CacheChange_t* const change);

        void process_participant_volatile_message_secure(const CacheChange_t* const change);

        bool on_process_handshake(const GUID_t& remote_participant_guid,
                DiscoveredParticipantInfo::AuthUniquePtr& remote_participant_info,
                MessageIdentity&& message_identity,
                HandshakeMessageToken&& message);

        ParticipantGenericMessage generate_authentication_message(const MessageIdentity& related_message_identity,
                const GUID_t& destination_participant_key,
                HandshakeMessageToken& handshake_message);

        ParticipantGenericMessage generate_participant_crypto_token_message(const GUID_t& destination_participant_key,
                ParticipantCryptoTokenSeq& crypto_tokens);

        ParticipantGenericMessage generate_writer_crypto_token_message(const GUID_t& destination_participant_key,
                const GUID_t& destination_endpoint_key, const GUID_t& source_endpoint_key,
                ParticipantCryptoTokenSeq& crypto_tokens);

        ParticipantGenericMessage generate_reader_crypto_token_message(const GUID_t& destination_participant_key,
                const GUID_t& destination_endpoint_key, const GUID_t& source_endpoint_key,
                ParticipantCryptoTokenSeq& crypto_tokens);

        bool participant_authorized(const GUID_t& remote_participant_guid,
                const DiscoveredParticipantInfo::AuthUniquePtr& remote_participant_info,
                SharedSecretHandle* shared_secret_handle);

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

        Authentication* authentication_plugin_;

        Cryptography* crypto_plugin_;

        IdentityHandle* local_identity_handle_;

        ParticipantCryptoHandle* local_participant_crypto_handle_;

        std::map<GUID_t, DiscoveredParticipantInfo> discovered_participants_;

        GUID_t auth_source_guid;

        std::mutex mutex_;

        std::atomic<int64_t> auth_last_sequence_number_;

        std::atomic<int64_t> crypto_last_sequence_number_;

        struct DatawriterAssociations
        {
            DatawriterAssociations(DatawriterCryptoHandle* wh) : writer_handle(wh) {}

            DatawriterCryptoHandle* writer_handle;

            std::map<GUID_t, std::tuple<ReaderProxyData, DatareaderCryptoHandle*>> associated_readers;
        };

        struct DatareaderAssociations
        {
            DatareaderAssociations(DatareaderCryptoHandle* rh) : reader_handle(rh) {}

            DatareaderCryptoHandle* reader_handle;

            std::map<GUID_t, std::tuple<WriterProxyData, DatawriterCryptoHandle*>> associated_writers;
        };

        // TODO(Ricardo) Temporal. Store individual in FastRTPS code.
        std::map<GUID_t, DatawriterAssociations> writer_handles_;
        std::map<GUID_t, DatareaderAssociations> reader_handles_;

        std::map<GUID_t, DataHolderSeq> remote_participant_pending_messages_;
        std::map<GUID_t, DataHolderSeq> remote_writer_pending_messages_;
        std::map<GUID_t, DataHolderSeq> remote_reader_pending_messages_;
        std::list<std::tuple<ReaderProxyData, GUID_t, GUID_t>> remote_reader_pending_discovery_messages_;
        std::list<std::tuple<WriterProxyData, GUID_t, GUID_t>> remote_writer_pending_discovery_messages_;
};

} //namespace security
} //namespace rtps
} //namespace fastrtps
} //namespace eprosima

#endif // _RTPS_SECURITY_SECURITYMANAGER_H_
