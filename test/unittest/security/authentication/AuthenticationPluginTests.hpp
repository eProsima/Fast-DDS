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

#ifndef _UNITTEST_SECURITY_AUTHENTICATION_AUTHENTICATIONPLUGINTESTS_HPP_
#define _UNITTEST_SECURITY_AUTHENTICATION_AUTHENTICATIONPLUGINTESTS_HPP_

#include "../../../../src/cpp/rtps/security/SecurityPluginFactory.h"

#include <gtest/gtest.h>

using namespace eprosima::fastrtps::rtps;
using namespace ::security;

class AuthenticationPluginTest : public ::testing::Test
{
    protected:

        virtual void SetUp()
        {
            plugin = factory.create_authentication_plugin(AuthenticationPluginTest::get_valid_policy());
        }

        virtual void TearDown()
        {
            delete plugin;
        }

    public:

        AuthenticationPluginTest() : plugin(nullptr){}

        static PropertyPolicy get_valid_policy();
        static PropertyPolicy get_wrong_policy();
        static void check_local_identity_handle(const IdentityHandle& handle);
        static void check_remote_identity_handle(const IdentityHandle& handle);
        static void check_handshake_request_message(const HandshakeHandle& handle, const HandshakeMessageToken& message);
        static void check_handshake_reply_message(const HandshakeHandle& handle, const HandshakeMessageToken& message,
                const HandshakeMessageToken& request_message);
        static void check_handshake_final_message(const HandshakeHandle& handle, const HandshakeMessageToken& message,
                const HandshakeMessageToken& reply_message);
        static void check_shared_secrets(const SharedSecretHandle& sharedsecret1,
                const SharedSecretHandle& sharedsecret2);

        SecurityPluginFactory factory;
        Authentication* plugin;
};

void fill_candidate_participant_key(GUID_t& candidate_participant_key)
{
    candidate_participant_key.guidPrefix.value[0] = 1;
    candidate_participant_key.guidPrefix.value[1] = 2;
    candidate_participant_key.guidPrefix.value[2] = 3;
    candidate_participant_key.guidPrefix.value[3] = 4;
    candidate_participant_key.guidPrefix.value[4] = 5;
    candidate_participant_key.guidPrefix.value[5] = 6;
    candidate_participant_key.guidPrefix.value[6] = 7;
    candidate_participant_key.guidPrefix.value[7] = 8;
    candidate_participant_key.guidPrefix.value[8] = 9;
    candidate_participant_key.guidPrefix.value[9] = 10;
    candidate_participant_key.guidPrefix.value[10] = 11;
    candidate_participant_key.guidPrefix.value[11] = 12;
    candidate_participant_key.entityId.value[0] = 13;
    candidate_participant_key.entityId.value[1] = 14;
    candidate_participant_key.entityId.value[2] = 15;
    candidate_participant_key.entityId.value[3] = 16;
}

TEST_F(AuthenticationPluginTest, validate_local_identity_validation_ok)
{
    ASSERT_TRUE(plugin != nullptr);

    IdentityHandle* local_identity_handle = nullptr;
    GUID_t adjusted_participant_key;
    uint32_t domain_id = 0;
    RTPSParticipantAttributes participant_attr;
    GUID_t candidate_participant_key;
    SecurityException exception;
    ValidationResult_t result= ValidationResult_t::VALIDATION_FAILED;

    fill_candidate_participant_key(candidate_participant_key);
    participant_attr.properties = AuthenticationPluginTest::get_valid_policy();

    result = plugin->validate_local_identity(&local_identity_handle,
            adjusted_participant_key,
            domain_id,
            participant_attr,
            candidate_participant_key,
            exception);

    ASSERT_TRUE(result == ValidationResult_t::VALIDATION_OK);
    ASSERT_TRUE(local_identity_handle != nullptr);
    AuthenticationPluginTest::check_local_identity_handle(*local_identity_handle);
    ASSERT_TRUE(adjusted_participant_key != GUID_t::unknown());
}

TEST_F(AuthenticationPluginTest, validate_local_identity_wrong_validation)
{
    ASSERT_TRUE(plugin != nullptr);

    IdentityHandle* local_identity_handle = nullptr;
    GUID_t adjusted_participant_key;
    uint32_t domain_id = 0;
    RTPSParticipantAttributes participant_attr;
    GUID_t candidate_participant_key;
    SecurityException exception;
    ValidationResult_t result= ValidationResult_t::VALIDATION_FAILED;

    fill_candidate_participant_key(candidate_participant_key);
    participant_attr.properties = get_wrong_policy();

    result = plugin->validate_local_identity(&local_identity_handle,
            adjusted_participant_key,
            domain_id,
            participant_attr,
            candidate_participant_key,
            exception);

    ASSERT_TRUE(result == ValidationResult_t::VALIDATION_FAILED);
    ASSERT_TRUE(local_identity_handle == nullptr);
    ASSERT_TRUE(adjusted_participant_key == GUID_t::unknown());
}

TEST_F(AuthenticationPluginTest, handshake_process_ok)
{
    ASSERT_TRUE(plugin != nullptr);

    IdentityHandle* local_identity_handle = nullptr;
    GUID_t adjusted_participant_key;
    uint32_t domain_id = 0;
    RTPSParticipantAttributes participant_attr;
    GUID_t candidate_participant_key;
    SecurityException exception;
    ValidationResult_t result= ValidationResult_t::VALIDATION_FAILED;

    participant_attr.properties = get_valid_policy();

    result = plugin->validate_local_identity(&local_identity_handle,
            adjusted_participant_key,
            domain_id,
            participant_attr,
            candidate_participant_key,
            exception);

    ASSERT_TRUE(result == ValidationResult_t::VALIDATION_OK);
    ASSERT_TRUE(local_identity_handle != nullptr);
    AuthenticationPluginTest::check_local_identity_handle(*local_identity_handle);

    IdentityHandle* remote_identity_handle = nullptr;
    IdentityToken remote_identity_token;
    GUID_t remote_participant_key;

    result = plugin->validate_remote_identity(&remote_identity_handle,
            *local_identity_handle,
            remote_identity_token,
            remote_participant_key,
            exception);

    ASSERT_TRUE(remote_identity_handle != nullptr);
    AuthenticationPluginTest::check_remote_identity_handle(*remote_identity_handle);

    HandshakeHandle* handshake_handle = nullptr;
    HandshakeMessageToken *handshake_message = nullptr;

    result = plugin->begin_handshake_request(&handshake_handle,
            &handshake_message,
            *local_identity_handle,
            *remote_identity_handle,
            exception);

    ASSERT_TRUE(result == ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE);
    ASSERT_TRUE(handshake_handle != nullptr);
    ASSERT_TRUE(handshake_message != nullptr);
    check_handshake_request_message(*handshake_handle, *handshake_message);

    HandshakeHandle* handshake_handle_reply = nullptr;
    HandshakeMessageToken* handshake_message_reply = nullptr;

    result = plugin->begin_handshake_reply(&handshake_handle_reply,
            &handshake_message_reply,
            HandshakeMessageToken(*handshake_message),
            *remote_identity_handle,
            *local_identity_handle,
            exception);

    ASSERT_TRUE(result == ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE);
    ASSERT_TRUE(handshake_handle_reply != nullptr);
    ASSERT_TRUE(handshake_message_reply != nullptr);
    check_handshake_reply_message(*handshake_handle_reply, *handshake_message_reply, *handshake_message);

    HandshakeMessageToken* handshake_message_final = nullptr;

    result = plugin->process_handshake(&handshake_message_final,
            HandshakeMessageToken(*handshake_message_reply),
            *handshake_handle,
            exception);
    
    ASSERT_TRUE(result == ValidationResult_t::VALIDATION_OK_WITH_FINAL_MESSAGE);
    ASSERT_TRUE(handshake_message_final != nullptr);
    check_handshake_final_message(*handshake_handle, *handshake_message_final, *handshake_message_reply);

    HandshakeMessageToken* handshake_message_aux = nullptr;

    result = plugin->process_handshake(&handshake_message_aux,
            HandshakeMessageToken(*handshake_message_final),
            *handshake_handle_reply,
            exception);

    ASSERT_TRUE(result == ValidationResult_t::VALIDATION_OK);

    SharedSecretHandle* sharedsecret1 = plugin->get_shared_secret(*handshake_handle, exception);
    ASSERT_TRUE(sharedsecret1 != nullptr);

    SharedSecretHandle* sharedsecret2 = plugin->get_shared_secret(*handshake_handle_reply, exception);
    ASSERT_TRUE(sharedsecret2 != nullptr);
    check_shared_secrets(*sharedsecret1, *sharedsecret2);
}

#endif // _UNITTEST_SECURITY_AUTHENTICATION_AUTHENTICATIONPLUGINTESTS_HPP_
