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

#include "SecurityPluginFactory.h"

#include <fastrtps/rtps/security/authentication/Handshake.h>
#include <fastrtps/rtps/security/common/ParticipantGenericMessage.h>
#include <fastrtps/rtps/reader/ReaderListener.h>

#include <map>
#include <mutex>
#include <atomic>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class RTPSParticipantImpl;
class StatelessWriter;
class StatelessReader;
class WriterHistory;
class ReaderHistory;
class ParticipantProxyData;

namespace security {

class Authentication;

class SecurityManager
{
    public:

        SecurityManager(RTPSParticipantImpl* participant);

        ~SecurityManager();

        bool init();

        bool discovered_participant(ParticipantProxyData* participant_data);

        bool get_identity_token(IdentityToken** identity_token);

        bool return_identity_token(IdentityToken* identity_token);

        uint32_t builtin_endpoints();

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
            public:

                DiscoveredParticipantInfo(ParticipantProxyData* participant_data, AuthenticationStatus auth_status) :
                    identity_handle_(nullptr), handshake_handle_(nullptr),
                    auth_status_(auth_status), expected_sequence_number_(0), participant_data_(participant_data)
                {}

                bool is_identity_handle_null()
                {
                    return identity_handle_ == nullptr;
                }

                bool set_identity_handle(IdentityHandle* identity_handle)
                {
                    if(identity_handle_ == nullptr)
                    {
                        identity_handle_ = identity_handle;
                        return true;
                    }

                    return false;
                }

                IdentityHandle* get_identity_handle()
                {
                    IdentityHandle* handle = identity_handle_;
                    identity_handle_ = nullptr;
                    return handle;
                }

                bool is_handshake_handle_null()
                {
                    return handshake_handle_ == nullptr;
                }

                bool set_handshake_handle(HandshakeHandle* handshake_handle)
                {
                    if(handshake_handle_ == nullptr)
                    {
                        handshake_handle_ = handshake_handle;
                        return true;
                    }

                    return false;
                }

                HandshakeHandle* get_handshake_handle()
                {
                    HandshakeHandle* handle = handshake_handle_;
                    handshake_handle_ = nullptr;
                    return handle;
                }

                AuthenticationStatus get_auth_status()
                {
                    return auth_status_;
                }

                void set_auth_status(AuthenticationStatus auth_status)
                {
                    auth_status_ = auth_status;
                }

                void set_expected_sequence_number(int64_t sequence_number)
                {
                    expected_sequence_number_ = sequence_number;
                }

                int64_t get_expected_sequence_number()
                {
                    return expected_sequence_number_;
                }

                ParticipantProxyData* get_participant_data()
                {
                    return participant_data_;
                }

            private:

                DiscoveredParticipantInfo(const DiscoveredParticipantInfo& info) = delete;

                IdentityHandle* identity_handle_;

                HandshakeHandle* handshake_handle_;

                AuthenticationStatus auth_status_;

                int64_t expected_sequence_number_;

                ParticipantProxyData* participant_data_;
        };

        class ParticipantStatelessMessageListener: public eprosima::fastrtps::rtps::ReaderListener
        {
            public:
                ParticipantStatelessMessageListener(SecurityManager &manager) : manager_(manager) {};

                ~ParticipantStatelessMessageListener(){};

                void onNewCacheChangeAdded(RTPSReader* reader, const CacheChange_t* const change);

            private:

                ParticipantStatelessMessageListener& operator=(const ParticipantStatelessMessageListener&) NON_COPYABLE_CXX11;

                SecurityManager &manager_;
        } participant_stateless_message_listener_;

        void remove_discovered_participant_info(const GUID_t remote_participant_key);
        void restore_remote_identity_handle(const GUID_t& remote_participant_key,
                IdentityHandle* remote_identity_handle,
                HandshakeHandle* handshake_handle = nullptr);

        bool create_entities();
        void delete_entities();
        bool create_participant_stateless_message_entities();
        void delete_participant_stateless_message_entities();
        bool create_participant_stateless_message_writer();
        void delete_participant_stateless_message_writer();
        bool create_participant_stateless_message_reader();
        void delete_participant_stateless_message_reader();

        void match_builtin_endpoints(ParticipantProxyData* participant_data);

        void process_participant_stateless_message(const CacheChange_t* const change);

        bool on_process_handshake(const GUID_t& remote_participant_key,
                AuthenticationStatus pre_auth_status,
                MessageIdentity&& message_identity,
                HandshakeMessageToken&& message,
                IdentityHandle* remote_identity_handle,
                HandshakeHandle* handshake_handle);

        ParticipantGenericMessage generate_authentication_message(const MessageIdentity& related_message_identity,
                const GUID_t& destination_participant_key,
                HandshakeMessageToken& handshake_message);

        RTPSParticipantImpl* participant_;
        StatelessWriter* participant_stateless_message_writer_;
        WriterHistory* participant_stateless_message_writer_history_;
        StatelessReader* participant_stateless_message_reader_;
        ReaderHistory* participant_stateless_message_reader_history_;
        SecurityPluginFactory factory_;

        Authentication* authentication_plugin_;

        IdentityHandle* local_identity_handle_;

        std::map<GUID_t, DiscoveredParticipantInfo> discovered_participants_;

        GUID_t auth_source_guid;

        std::mutex mutex_;

        std::atomic<int64_t> last_sequence_number_;
};

} //namespace security
} //namespace rtps
} //namespace fastrtps
} //namespace eprosima

#endif // _RTPS_SECURITY_SECURITYMANAGER_H_
