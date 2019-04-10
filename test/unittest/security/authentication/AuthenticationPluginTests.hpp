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

#include "../../../../src/cpp/security/authentication/PKIDH.h"

// Include first necessary mocks
#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>
#include <fastrtps/rtps/builtin/data/WriterProxyData.h>
#include <fastrtps/rtps/builtin/data/ReaderProxyData.h>

#include <gtest/gtest.h>

class AuthenticationPluginTest : public ::testing::Test
{
    protected:

        virtual void SetUp()
        {
        }

        virtual void TearDown()
        {
        }

    public:

        AuthenticationPluginTest() {}

        static eprosima::fastrtps::rtps::PropertyPolicy get_valid_policy();
        static eprosima::fastrtps::rtps::PropertyPolicy get_wrong_policy();
        static eprosima::fastrtps::rtps::IdentityToken generate_remote_identity_token_ok(const eprosima::fastrtps::rtps::security::IdentityHandle& local_identity_handle);
        static void check_local_identity_handle(const eprosima::fastrtps::rtps::security::IdentityHandle& handle);
        static void check_remote_identity_handle(const eprosima::fastrtps::rtps::security::IdentityHandle& handle);
        static void check_handshake_request_message(const eprosima::fastrtps::rtps::security::HandshakeHandle& handle, const eprosima::fastrtps::rtps::security::HandshakeMessageToken& message);
        static void check_handshake_reply_message(const eprosima::fastrtps::rtps::security::HandshakeHandle& handle, const eprosima::fastrtps::rtps::security::HandshakeMessageToken& message,
                const eprosima::fastrtps::rtps::security::HandshakeMessageToken& request_message);
        static void check_handshake_final_message(const eprosima::fastrtps::rtps::security::HandshakeHandle& handle, const eprosima::fastrtps::rtps::security::HandshakeMessageToken& message,
                const eprosima::fastrtps::rtps::security::HandshakeMessageToken& reply_message);
        static void check_shared_secrets(const eprosima::fastrtps::rtps::security::SharedSecretHandle& sharedsecret1,
                const eprosima::fastrtps::rtps::security::SharedSecretHandle& sharedsecret2);

        eprosima::fastrtps::rtps::security::PKIDH plugin;
};

void fill_candidate_participant_key(eprosima::fastrtps::rtps::GUID_t& candidate_participant_key)
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
    candidate_participant_key.entityId.value[0] = 0x0;
    candidate_participant_key.entityId.value[1] = 0x0;
    candidate_participant_key.entityId.value[2] = 0x1;
    candidate_participant_key.entityId.value[3] = 0xc1;
}

TEST_F(AuthenticationPluginTest, validate_local_identity_validation_ok)
{
    eprosima::fastrtps::rtps::security::IdentityHandle* local_identity_handle = nullptr;
    eprosima::fastrtps::rtps::GUID_t adjusted_participant_key;
    uint32_t domain_id = 0;
    eprosima::fastrtps::rtps::RTPSParticipantAttributes participant_attr;
    eprosima::fastrtps::rtps::GUID_t candidate_participant_key;
    eprosima::fastrtps::rtps::security::SecurityException exception;
    eprosima::fastrtps::rtps::security::ValidationResult_t result= eprosima::fastrtps::rtps::security::ValidationResult_t::VALIDATION_FAILED;

    fill_candidate_participant_key(candidate_participant_key);
    participant_attr.properties = AuthenticationPluginTest::get_valid_policy();

    result = plugin.validate_local_identity(&local_identity_handle,
            adjusted_participant_key,
            domain_id,
            participant_attr,
            candidate_participant_key,
            exception);

    ASSERT_TRUE(result == eprosima::fastrtps::rtps::security::ValidationResult_t::VALIDATION_OK);
    ASSERT_TRUE(local_identity_handle != nullptr);
    AuthenticationPluginTest::check_local_identity_handle(*local_identity_handle);
    ASSERT_TRUE(adjusted_participant_key != eprosima::fastrtps::rtps::GUID_t::unknown());

    ASSERT_TRUE(plugin.return_identity_handle(local_identity_handle, exception));
}

TEST_F(AuthenticationPluginTest, validate_local_identity_wrong_validation)
{
    eprosima::fastrtps::rtps::security::IdentityHandle* local_identity_handle = nullptr;
    eprosima::fastrtps::rtps::GUID_t adjusted_participant_key;
    uint32_t domain_id = 0;
    eprosima::fastrtps::rtps::RTPSParticipantAttributes participant_attr;
    eprosima::fastrtps::rtps::GUID_t candidate_participant_key;
    eprosima::fastrtps::rtps::security::SecurityException exception;
    eprosima::fastrtps::rtps::security::ValidationResult_t result= eprosima::fastrtps::rtps::security::ValidationResult_t::VALIDATION_FAILED;

    fill_candidate_participant_key(candidate_participant_key);
    participant_attr.properties = get_wrong_policy();

    result = plugin.validate_local_identity(&local_identity_handle,
            adjusted_participant_key,
            domain_id,
            participant_attr,
            candidate_participant_key,
            exception);

    ASSERT_TRUE(result == eprosima::fastrtps::rtps::security::ValidationResult_t::VALIDATION_FAILED);
    ASSERT_TRUE(local_identity_handle == nullptr);
    ASSERT_TRUE(adjusted_participant_key == eprosima::fastrtps::rtps::GUID_t::unknown());
}

TEST_F(AuthenticationPluginTest, handshake_process_ok)
{
    eprosima::fastrtps::rtps::security::IdentityHandle* local_identity_handle1 = nullptr;
    eprosima::fastrtps::rtps::GUID_t adjusted_participant_key1;
    eprosima::fastrtps::rtps::GUID_t adjusted_participant_key2;
    uint32_t domain_id = 0;
    eprosima::fastrtps::rtps::RTPSParticipantAttributes participant_attr;
    eprosima::fastrtps::rtps::GUID_t candidate_participant_key1;
    eprosima::fastrtps::rtps::GUID_t candidate_participant_key2;
    eprosima::fastrtps::rtps::security::SecurityException exception;
    eprosima::fastrtps::rtps::security::ValidationResult_t result= eprosima::fastrtps::rtps::security::ValidationResult_t::VALIDATION_FAILED;

    participant_attr.properties = get_valid_policy();

    result = plugin.validate_local_identity(&local_identity_handle1,
            adjusted_participant_key1,
            domain_id,
            participant_attr,
            candidate_participant_key1,
            exception);

    ASSERT_TRUE(result == eprosima::fastrtps::rtps::security::ValidationResult_t::VALIDATION_OK);
    ASSERT_TRUE(local_identity_handle1 != nullptr);
    AuthenticationPluginTest::check_local_identity_handle(*local_identity_handle1);

    eprosima::fastrtps::rtps::security::IdentityHandle* local_identity_handle2 = nullptr;
    result = plugin.validate_local_identity(&local_identity_handle2,
            adjusted_participant_key2,
            domain_id,
            participant_attr,
            candidate_participant_key2,
            exception);

    ASSERT_TRUE(result == eprosima::fastrtps::rtps::security::ValidationResult_t::VALIDATION_OK);
    ASSERT_TRUE(local_identity_handle2 != nullptr);
    AuthenticationPluginTest::check_local_identity_handle(*local_identity_handle2);

    eprosima::fastrtps::rtps::security::IdentityHandle* remote_identity_handle1 = nullptr;
    eprosima::fastrtps::rtps::IdentityToken remote_identity_token1 = generate_remote_identity_token_ok(*local_identity_handle1);
    eprosima::fastrtps::rtps::GUID_t remote_participant_key;

    result = plugin.validate_remote_identity(&remote_identity_handle1,
            *local_identity_handle1,
            std::move(remote_identity_token1),
            remote_participant_key,
            exception);

    ASSERT_TRUE(remote_identity_handle1 != nullptr);
    AuthenticationPluginTest::check_remote_identity_handle(*remote_identity_handle1);

    eprosima::fastrtps::rtps::security::IdentityHandle* remote_identity_handle2 = nullptr;
    eprosima::fastrtps::rtps::IdentityToken remote_identity_token2 = generate_remote_identity_token_ok(*local_identity_handle2);

    result = plugin.validate_remote_identity(&remote_identity_handle2,
            *local_identity_handle2,
            std::move(remote_identity_token2),
            remote_participant_key,
            exception);

    ASSERT_TRUE(remote_identity_handle2 != nullptr);
    AuthenticationPluginTest::check_remote_identity_handle(*remote_identity_handle2);

    eprosima::fastrtps::rtps::security::HandshakeHandle* handshake_handle = nullptr;
    eprosima::fastrtps::rtps::security::HandshakeMessageToken *handshake_message = nullptr;
    eprosima::fastrtps::rtps::ParticipantProxyData participant_data1;
    participant_data1.m_guid = adjusted_participant_key1;
    eprosima::fastrtps::rtps::CDRMessage_t auxMsg;
    auxMsg.msg_endian = eprosima::fastrtps::rtps::BIGEND;
    ASSERT_TRUE(participant_data1.writeToCDRMessage(&auxMsg, false));

    result = plugin.begin_handshake_request(&handshake_handle,
            &handshake_message,
            *local_identity_handle1,
            *remote_identity_handle1,
            auxMsg,
            exception);

    ASSERT_TRUE(result == eprosima::fastrtps::rtps::security::ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE);
    ASSERT_TRUE(handshake_handle != nullptr);
    ASSERT_TRUE(handshake_message != nullptr);
    check_handshake_request_message(*handshake_handle, *handshake_message);

    eprosima::fastrtps::rtps::security::HandshakeHandle* handshake_handle_reply = nullptr;
    eprosima::fastrtps::rtps::security::HandshakeMessageToken* handshake_message_reply = nullptr;
    eprosima::fastrtps::rtps::ParticipantProxyData participant_data2;
    participant_data2.m_guid = adjusted_participant_key2;

    auxMsg.length = 0;
    auxMsg.pos = 0;

    ASSERT_TRUE(participant_data2.writeToCDRMessage(&auxMsg, false));

    result = plugin.begin_handshake_reply(&handshake_handle_reply,
            &handshake_message_reply,
            eprosima::fastrtps::rtps::security::HandshakeMessageToken(*handshake_message),
            *remote_identity_handle2,
            *local_identity_handle2,
            auxMsg,
            exception);

    ASSERT_TRUE(result == eprosima::fastrtps::rtps::security::ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE);
    ASSERT_TRUE(handshake_handle_reply != nullptr);
    ASSERT_TRUE(handshake_message_reply != nullptr);
    check_handshake_reply_message(*handshake_handle_reply, *handshake_message_reply, *handshake_message);

    eprosima::fastrtps::rtps::security::HandshakeMessageToken* handshake_message_final = nullptr;

    result = plugin.process_handshake(&handshake_message_final,
            eprosima::fastrtps::rtps::security::HandshakeMessageToken(*handshake_message_reply),
            *handshake_handle,
            exception);

    ASSERT_TRUE(result == eprosima::fastrtps::rtps::security::ValidationResult_t::VALIDATION_OK_WITH_FINAL_MESSAGE);
    ASSERT_TRUE(handshake_message_final != nullptr);
    check_handshake_final_message(*handshake_handle, *handshake_message_final, *handshake_message_reply);

    eprosima::fastrtps::rtps::security::HandshakeMessageToken* handshake_message_aux = nullptr;

    result = plugin.process_handshake(&handshake_message_aux,
            eprosima::fastrtps::rtps::security::HandshakeMessageToken(*handshake_message_final),
            *handshake_handle_reply,
            exception);

    ASSERT_TRUE(result == eprosima::fastrtps::rtps::security::ValidationResult_t::VALIDATION_OK);

    eprosima::fastrtps::rtps::security::SharedSecretHandle* sharedsecret1 = plugin.get_shared_secret(*handshake_handle, exception);
    ASSERT_TRUE(sharedsecret1 != nullptr);

    eprosima::fastrtps::rtps::security::SharedSecretHandle* sharedsecret2 = plugin.get_shared_secret(*handshake_handle_reply, exception);
    ASSERT_TRUE(sharedsecret2 != nullptr);
    check_shared_secrets(*sharedsecret1, *sharedsecret2);

    ASSERT_TRUE(plugin.return_sharedsecret_handle(sharedsecret2, exception));
    ASSERT_TRUE(plugin.return_sharedsecret_handle(sharedsecret1, exception));
    ASSERT_TRUE(plugin.return_handshake_handle(handshake_handle_reply, exception));
    ASSERT_TRUE(plugin.return_handshake_handle(handshake_handle, exception));
    ASSERT_TRUE(plugin.return_identity_handle(remote_identity_handle2, exception));
    ASSERT_TRUE(plugin.return_identity_handle(remote_identity_handle1, exception));
    ASSERT_TRUE(plugin.return_identity_handle(local_identity_handle2, exception));
    ASSERT_TRUE(plugin.return_identity_handle(local_identity_handle1, exception));
}

#endif // _UNITTEST_SECURITY_AUTHENTICATION_AUTHENTICATIONPLUGINTESTS_HPP_
