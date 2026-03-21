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
 * @file PKIDH.h
 */

#ifndef _SECURITY_AUTHENTICATION_PKIDH_H_
#define _SECURITY_AUTHENTICATION_PKIDH_H_

#include <fastdds/rtps/attributes/PropertyPolicy.hpp>
#include <rtps/security/authentication/Authentication.h>
#include <security/artifact_providers/Pkcs11Provider.hpp>
#include <security/authentication/PKIHandshakeHandle.h>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

class PKIDH : public Authentication
{
public:

    ValidationResult_t validate_local_identity(
            IdentityHandle** local_identity_handle,
            GUID_t& adjusted_participant_key,
            const uint32_t domain_id,
            const RTPSParticipantAttributes& participant_attr,
            const GUID_t& candidate_participant_key,
            SecurityException& exception) override;

    ValidationResult_t validate_remote_identity(
            IdentityHandle** remote_identity_handle,
            const IdentityHandle& local_identity_handle,
            const IdentityToken& remote_identity_token,
            const GUID_t& remote_participant_key,
            SecurityException& exception) override;

    ValidationResult_t begin_handshake_request(
            HandshakeHandle** handshake_handle,
            HandshakeMessageToken** handshake_message,
            const IdentityHandle& initiator_identity_handle,
            IdentityHandle& replier_identity_handle,
            const CDRMessage_t& cdr_participant_data,
            SecurityException& exception) override;

    ValidationResult_t begin_handshake_reply(
            HandshakeHandle** handshake_handle,
            HandshakeMessageToken** handshake_message_out,
            HandshakeMessageToken&& handshake_message_in,
            IdentityHandle& initiator_identity_handle,
            const IdentityHandle& replier_identity_handle,
            const CDRMessage_t& cdr_participant_data,
            SecurityException& exception) override;

    ValidationResult_t process_handshake(
            HandshakeMessageToken** handshake_message_out,
            HandshakeMessageToken&& handshake_message_in,
            HandshakeHandle& handshake_handle,
            SecurityException& exception) override;

    std::shared_ptr<SecretHandle> get_shared_secret(
            const HandshakeHandle& handshake_handle,
            SecurityException& exception) const override;

    bool set_listener(
            AuthenticationListener* listener,
            SecurityException& exception) override;

    bool get_identity_token(
            IdentityToken** identity_token,
            const IdentityHandle& handle,
            SecurityException& exception) override;

    bool return_identity_token(
            IdentityToken* token,
            SecurityException& exception) override;

    bool return_handshake_handle(
            HandshakeHandle* handshake_handle,
            SecurityException& exception) override;

    IdentityHandle* get_identity_handle(
            SecurityException& exception) override;

    bool return_identity_handle(
            IdentityHandle* identity_handle,
            SecurityException& exception) override;

    bool return_sharedsecret_handle(
            std::shared_ptr<SecretHandle>& sharedsecret_handle,
            SecurityException& exception) const override;

    bool set_permissions_credential_and_token(
            IdentityHandle& identity_handle,
            PermissionsCredentialToken& permissions_credential_token,
            SecurityException& ex) override;

    bool get_authenticated_peer_credential_token(
            PermissionsCredentialToken** token,
            const IdentityHandle& identity_handle,
            SecurityException& exception) override;

    bool return_authenticated_peer_credential_token(
            PermissionsCredentialToken* token,
            SecurityException& ex) override;

    bool check_guid_comes_from(
            IdentityHandle* identity_handle,
            const GUID_t& adjusted,
            const GUID_t& original) override;

    std::unique_ptr<detail::Pkcs11Provider> pkcs11_provider;

private:

    ValidationResult_t process_handshake_request(
            HandshakeMessageToken** handshake_message_out,
            HandshakeMessageToken&& handshake_message_in,
            PKIHandshakeHandle& handshake_handle,
            SecurityException& exception);

    ValidationResult_t process_handshake_reply(
            HandshakeMessageToken** handshake_message_out,
            HandshakeMessageToken&& handshake_message_in,
            PKIHandshakeHandle& handshake_handle,
            SecurityException& exception);

    std::shared_ptr<SecretHandle> generate_sharedsecret(
            EVP_PKEY* private_key,
            EVP_PKEY* public_key,
            SecurityException& exception) const;
};

} //namespace security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima

#endif // _SECURITY_AUTHENTICATION_PKIDH_H_
