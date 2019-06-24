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
 * @file MockAuthenticationPlugin.h
 */

#ifndef _SECURITY_MOCKAUTHENTICATIONPLUGIN_H_
#define _SECURITY_MOCKAUTHENTICATIONPLUGIN_H_

// TODO(Ricardo) Change when GMock supports r-values.

#include <fastrtps/rtps/security/authentication/Authentication.h>
#include <gmock/gmock.h>

#pragma warning(push)
#pragma warning(disable : 4373)

namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace security {

class MockAuthenticationPlugin : public Authentication
{
    public:

        MOCK_METHOD6(validate_local_identity, ValidationResult_t(IdentityHandle** local_identity_handle,
                GUID_t& adjusted_participant_key,
                const uint32_t domain_id,
                const RTPSParticipantAttributes& participant_attr,
                const GUID_t& candidate_participant_key,
                SecurityException& exception));

        MOCK_METHOD5(validate_remote_identity_rvr, ValidationResult_t(IdentityHandle** remote_identity_handle,
                const IdentityHandle& local_identity_handle,
                IdentityToken remote_identity_token,
                const GUID_t& remote_participant_key,
                SecurityException& exception));

        MOCK_METHOD6(begin_handshake_request, ValidationResult_t(HandshakeHandle** handshake_handle,
                HandshakeMessageToken** handshake_message,
                const IdentityHandle& initiator_identity_handle,
                IdentityHandle& replier_identity_handle,
                const CDRMessage_t& cdr_participant_data,
                SecurityException& exception));

        MOCK_METHOD7(begin_handshake_reply_rvr, ValidationResult_t(HandshakeHandle** handshake_handle,
                HandshakeMessageToken** handshake_message_out,
                HandshakeMessageToken handshake_message_in,
                IdentityHandle& initiator_identity_handle,
                const IdentityHandle& replier_identity_handle,
                const CDRMessage_t& cdr_participant_data,
                SecurityException& exception));

        MOCK_METHOD4(process_handshake_rvr, ValidationResult_t(HandshakeMessageToken** handshake_message_out,
                HandshakeMessageToken handshake_message_in,
                HandshakeHandle& handshake_handle,
                SecurityException& exception));

        MOCK_METHOD2(get_shared_secret, SharedSecretHandle*(const HandshakeHandle& handshake_handle,
                SecurityException& exception));

        MOCK_METHOD2(set_listener, bool(AuthenticationListener* listener,
                SecurityException& exception));

        MOCK_METHOD3(get_identity_token, bool(IdentityToken** identity_token,
                const IdentityHandle& handle,
                SecurityException& exception));

        MOCK_METHOD2(return_identity_token, bool(IdentityToken* token,
                SecurityException& exception));

        MOCK_METHOD2(return_handshake_handle, bool(HandshakeHandle* handshake_handle,
                SecurityException& exception));

        MOCK_METHOD2(return_identity_handle, bool(IdentityHandle* identity_handle,
                SecurityException& exception));

        MOCK_METHOD2(return_sharedsecret_handle, bool(SharedSecretHandle* sharedsecret_handle,
                SecurityException& exception));

        MOCK_METHOD3(set_permissions_credential_and_token, bool(IdentityHandle& identity_handle,
                PermissionsCredentialToken& permissions_credential_token,
                SecurityException& ex));

        MOCK_METHOD3(get_authenticated_peer_credential_token, bool(PermissionsCredentialToken **token,
                const IdentityHandle& identity_handle, SecurityException& exception));

        MOCK_METHOD2(return_authenticated_peer_credential_token, bool(PermissionsCredentialToken* token,
                SecurityException& ex));

        ValidationResult_t validate_remote_identity(IdentityHandle** remote_identity_handle,
                const IdentityHandle& local_identity_handle,
                const IdentityToken& remote_identity_token,
                const GUID_t& remote_participant_key,
                SecurityException& exception)
        {
            return validate_remote_identity_rvr(remote_identity_handle, local_identity_handle,
                    remote_identity_token, remote_participant_key, exception);
        }

        ValidationResult_t begin_handshake_reply(HandshakeHandle** handshake_handle,
                HandshakeMessageToken** handshake_message_out,
                HandshakeMessageToken&& handshake_message_in,
                IdentityHandle& initiator_identity_handle,
                const IdentityHandle& replier_identity_handle,
                const CDRMessage_t& cdr_participant_data,
                SecurityException& exception)
        {
            return begin_handshake_reply_rvr(handshake_handle, handshake_message_out, handshake_message_in,
                    initiator_identity_handle, replier_identity_handle, cdr_participant_data, exception);
        }

        ValidationResult_t process_handshake(HandshakeMessageToken** handshake_message_out,
                HandshakeMessageToken&& handshake_message_in,
                HandshakeHandle& handshake_handle,
                SecurityException& exception)
        {
            return process_handshake_rvr(handshake_message_out, handshake_message_in,
                    handshake_handle, exception);
        }
};

} // namespace security
} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#pragma warning(pop)

#endif // _SECURITY_MOCKAUTHENTICATIONPLUGIN_H_
