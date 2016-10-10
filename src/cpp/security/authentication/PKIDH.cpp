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
 * @file PKIDH.cpp
 */

#include "PKIDH.h"

using namespace eprosima::fastrtps::rtps::security;

PKIDH::PKIDH(const PropertyPolicy& property_policy)
{
}

ValidationResult_t PKIDH::validate_local_identity(IdentityHandle** local_identity_handle,
        GUID_t& adjusted_participant_key,
        const uint32_t domain_id,
        const RTPSParticipantAttributes& participant_attr,
        const GUID_t& candidate_participant_key,
        SecurityException& exception)
{
    return ValidationResult_t::VALIDATION_FAILED;
}

ValidationResult_t PKIDH::validate_remote_identity(IdentityHandle** remote_identity_handle,
        const IdentityHandle& local_identity_handle,
        const IdentityToken& remote_identity_token,
        const GUID_t remote_participant_key,
        SecurityException& exception)
{
    return ValidationResult_t::VALIDATION_FAILED;
}

ValidationResult_t PKIDH::begin_handshake_request(HandshakeHandle** handshake_handle,
        HandshakeMessageToken& handshake_message,
        const IdentityHandle& initiator_identity_handle,
        const IdentityHandle& replier_identity_handle,
        SecurityException& exception)
{
    return ValidationResult_t::VALIDATION_FAILED;
}

ValidationResult_t PKIDH::begin_handshake_reply(HandshakeHandle** handshake_handle,
        HandshakeMessageToken& handshake_message_out,
        const HandshakeMessageToken& handshake_message_in,
        const IdentityHandle& initiator_identity_handle,
        const IdentityHandle& replier_identity_handle,
        SecurityException& exception)
{
    return ValidationResult_t::VALIDATION_FAILED;
}

ValidationResult_t PKIDH::process_handshake(HandshakeMessageToken& handshake_message_out,
        const HandshakeMessageToken& handshake_message_in,
        const HandshakeHandle& handshake_handle,
        SecurityException& exception)
{
    return ValidationResult_t::VALIDATION_FAILED;
}

SharedSecretHandle* PKIDH::get_shared_secret(const HandshakeHandle& handshake_handle,
        SecurityException& exception)
{
    return nullptr;
}

bool PKIDH::set_listener(AuthenticationListener* listener,
        SecurityException& exception)
{
    return false;
}

bool PKIDH::get_identity_token(IdentityToken** identity_token,
        const IdentityHandle& handle,
        SecurityException& exception)
{
    return false;
}

bool PKIDH::return_identity_token(IdentityToken* token,
        SecurityException& exception)
{
    return false;
}

bool PKIDH::return_handshake_handle(HandshakeHandle* handshake_handle,
        SecurityException& exception)
{
    return false;
}

bool PKIDH::return_identity_handle(IdentityHandle* identity_handle,
        SecurityException& exception)
{
    return false;
}

bool PKIDH::return_sharedsecret_handle(SharedSecretHandle* sharedsecret_handle,
        SecurityException& exception)
{
    return false;
}
