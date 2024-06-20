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

#ifndef FASTDDS_RTPS_SECURITY__MOCKAUTHENTICATIONPLUGIN_H
#define FASTDDS_RTPS_SECURITY__MOCKAUTHENTICATIONPLUGIN_H

// TODO(Ricardo) Change when GMock supports r-values.

#include <gmock/gmock.h>

#include <rtps/security/authentication/Authentication.h>

#include <security/authentication/PKIIdentityHandle.h>

#pragma warning(push)
#pragma warning(disable : 4373)

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

class MockAuthenticationPlugin : public Authentication
{
public:

    using PKIIdentityHandle = HandleImpl<PKIIdentity, MockAuthenticationPlugin>;
    using SharedSecretHandle = HandleImpl<SharedSecret, MockAuthenticationPlugin>;

    MOCK_METHOD(ValidationResult_t, validate_local_identity, (IdentityHandle * *local_identity_handle,
            GUID_t & adjusted_participant_key,
            const uint32_t domain_id,
            const RTPSParticipantAttributes& participant_attr,
            const GUID_t& candidate_participant_key,
            SecurityException & exception), (override));

    MOCK_METHOD5(validate_remote_identity_rvr, ValidationResult_t(IdentityHandle * *remote_identity_handle,
            const IdentityHandle& local_identity_handle,
            IdentityToken remote_identity_token,
            const GUID_t& remote_participant_key,
            SecurityException & exception));

    MOCK_METHOD(ValidationResult_t, begin_handshake_request, (HandshakeHandle * *handshake_handle,
            HandshakeMessageToken * *handshake_message,
            const IdentityHandle& initiator_identity_handle,
            IdentityHandle & replier_identity_handle,
            const CDRMessage_t& cdr_participant_data,
            SecurityException & exception), (override));

    MOCK_METHOD7(begin_handshake_reply_rvr, ValidationResult_t(HandshakeHandle * *handshake_handle,
            HandshakeMessageToken * *handshake_message_out,
            HandshakeMessageToken handshake_message_in,
            IdentityHandle & initiator_identity_handle,
            const IdentityHandle& replier_identity_handle,
            const CDRMessage_t& cdr_participant_data,
            SecurityException & exception));

    MOCK_METHOD4(process_handshake_rvr, ValidationResult_t(HandshakeMessageToken * *handshake_message_out,
            HandshakeMessageToken handshake_message_in,
            HandshakeHandle & handshake_handle,
            SecurityException & exception));

    MOCK_METHOD(bool, set_listener, (AuthenticationListener * listener,
            SecurityException & exception), (override));

    MOCK_METHOD(bool, get_identity_token, (IdentityToken * *identity_token,
            const IdentityHandle& handle,
            SecurityException & exception), (override));

    MOCK_METHOD(bool, return_identity_token, (IdentityToken * token,
            SecurityException & exception), (override));

    MOCK_METHOD(bool, return_handshake_handle, (HandshakeHandle * handshake_handle,
            SecurityException & exception), (override));

    MOCK_METHOD(bool, set_permissions_credential_and_token, (IdentityHandle & identity_handle,
            PermissionsCredentialToken & permissions_credential_token,
            SecurityException & ex), (override));

    MOCK_METHOD(bool, get_authenticated_peer_credential_token, (PermissionsCredentialToken * *token,
            const IdentityHandle& identity_handle, SecurityException & exception), (override));

    MOCK_METHOD(bool, return_authenticated_peer_credential_token, (PermissionsCredentialToken * token,
            SecurityException & ex), (override));

    ValidationResult_t validate_remote_identity(
            IdentityHandle** remote_identity_handle,
            const IdentityHandle& local_identity_handle,
            const IdentityToken& remote_identity_token,
            const GUID_t& remote_participant_key,
            SecurityException& exception) override
    {
        return validate_remote_identity_rvr(remote_identity_handle, local_identity_handle,
                       remote_identity_token, remote_participant_key, exception);
    }

    ValidationResult_t begin_handshake_reply(
            HandshakeHandle** handshake_handle,
            HandshakeMessageToken** handshake_message_out,
            HandshakeMessageToken&& handshake_message_in,
            IdentityHandle& initiator_identity_handle,
            const IdentityHandle& replier_identity_handle,
            const CDRMessage_t& cdr_participant_data,
            SecurityException& exception) override
    {
        return begin_handshake_reply_rvr(handshake_handle, handshake_message_out, handshake_message_in,
                       initiator_identity_handle, replier_identity_handle, cdr_participant_data, exception);
    }

    ValidationResult_t process_handshake(
            HandshakeMessageToken** handshake_message_out,
            HandshakeMessageToken&& handshake_message_in,
            HandshakeHandle& handshake_handle,
            SecurityException& exception) override
    {
        return process_handshake_rvr(handshake_message_out, handshake_message_in,
                       handshake_handle, exception);
    }

    MOCK_METHOD(IdentityHandle*, get_identity_handle, (
                SecurityException &), (override));

    IdentityHandle* get_dummy_identity_handle()
    {
        return new (std::nothrow) PKIIdentityHandle();
    }

    MOCK_METHOD(bool, return_identity_handle, (
                IdentityHandle * identity_handle,
                SecurityException &), (override));

    bool return_dummy_identity_handle(
            IdentityHandle* identity_handle)
    {
        delete dynamic_cast<PKIIdentityHandle*>(identity_handle);
        return true;
    }

    MOCK_METHOD(std::shared_ptr<SecretHandle>, get_shared_secret, (
                const HandshakeHandle&,
                SecurityException &), (const, override));

    std::shared_ptr<SecretHandle> get_dummy_shared_secret() const
    {
        // create ad hoc deleter because this object can only be created/release from the friend factory
        auto p = new (std::nothrow) SharedSecretHandle;
        return std::dynamic_pointer_cast<SecretHandle>(std::shared_ptr<SharedSecretHandle>(p,
                       [](SharedSecretHandle* p)
                       {
                           delete p;
                       }));
    }

    MOCK_METHOD(bool, return_sharedsecret_handle, (
                std::shared_ptr<SecretHandle>& sharedsecret_handle,
                SecurityException &), (const, override));

    bool return_dummy_sharedsecret(
            std::shared_ptr<SecretHandle>& sharedsecret_handle) const
    {
        sharedsecret_handle.reset();
        return true;
    }

};

} // namespace security
} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#pragma warning(pop)

#endif // FASTDDS_RTPS_SECURITY__MOCKAUTHENTICATIONPLUGIN_H
