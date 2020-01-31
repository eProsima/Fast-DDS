// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "SecurityTests.hpp"

const char* const MockIdentity::class_id_ = "MockIdentityHandle";
const char* const MockHandshake::class_id_ = "MockHandshakeHandle";
const char* const SharedSecret::class_id_ = "SharedSecretHandle";
const char* const MockParticipantCrypto::class_id_ = "MockParticipantCryptoHandle";

TEST_F(SecurityTest, initialization_auth_nullptr)
{
    SecurityPluginFactory::release_auth_plugin();
    DefaultValue<const GUID_t&>::Set(guid);

    ASSERT_TRUE(manager_.init(security_attributes_, participant_properties_, security_activated_));
    ASSERT_TRUE(!security_activated_ || manager_.create_entities());
}

TEST_F(SecurityTest, initialization_auth_failed)
{
    DefaultValue<const GUID_t&>::Set(guid);

    EXPECT_CALL(*auth_plugin_, validate_local_identity(_,_,_,_,_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_FAILED));

    ASSERT_FALSE(manager_.init(security_attributes_, participant_properties_, security_activated_));
}

TEST_F(SecurityTest, initialization_register_local_participant_error)
{
    DefaultValue<const GUID_t&>::Set(guid);
    DefaultValue<const ParticipantSecurityAttributes&>::Set(security_attributes_);

    EXPECT_CALL(*auth_plugin_, validate_local_identity(_,_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(&local_identity_handle_), Return(ValidationResult_t::VALIDATION_OK)));
    EXPECT_CALL(crypto_plugin_->cryptokeyfactory_, register_local_participant(Ref(local_identity_handle_),_,_,_,_)).Times(1).
        WillOnce(Return(nullptr));

    ASSERT_FALSE(manager_.init(security_attributes_, participant_properties_, security_activated_));
}

TEST_F(SecurityTest, initialization_fail_participant_stateless_message_writer)
{
    DefaultValue<const GUID_t&>::Set(guid);
    DefaultValue<const ParticipantSecurityAttributes&>::Set(security_attributes_);
    MockParticipantCryptoHandle local_participant_crypto_handle;

    EXPECT_CALL(*auth_plugin_, validate_local_identity(_,_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(&local_identity_handle_), Return(ValidationResult_t::VALIDATION_OK)));
    EXPECT_CALL(crypto_plugin_->cryptokeyfactory_, register_local_participant(Ref(local_identity_handle_),_,_,_,_)).Times(1).
        WillOnce(Return(&local_participant_crypto_handle));
    EXPECT_CALL(crypto_plugin_->cryptokeyfactory_, unregister_participant(&local_participant_crypto_handle,_)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(participant_, createWriter_mock(_,_,_,_,_,_)).Times(1).
        WillOnce(Return(false));

    ASSERT_TRUE(manager_.init(security_attributes_, participant_properties_, security_activated_));
    ASSERT_TRUE(security_activated_);
    ASSERT_FALSE(manager_.create_entities());
}

TEST_F(SecurityTest, initialization_fail_participant_stateless_message_reader)
{
    DefaultValue<const GUID_t&>::Set(guid);
    DefaultValue<const ParticipantSecurityAttributes&>::Set(security_attributes_);
    MockParticipantCryptoHandle local_participant_crypto_handle;
    NiceMock<StatelessWriter>* stateless_writer = new NiceMock<StatelessWriter>(&participant_);

    EXPECT_CALL(*auth_plugin_, validate_local_identity(_,_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(&local_identity_handle_), Return(ValidationResult_t::VALIDATION_OK)));
    EXPECT_CALL(crypto_plugin_->cryptokeyfactory_, register_local_participant(Ref(local_identity_handle_),_,_,_,_)).Times(1).
        WillOnce(Return(&local_participant_crypto_handle));
    EXPECT_CALL(crypto_plugin_->cryptokeyfactory_, unregister_participant(&local_participant_crypto_handle,_)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(participant_, createWriter_mock(_,_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(stateless_writer), Return(true)));
    EXPECT_CALL(participant_, createReader_mock(_,_,_,_,_,_,_)).Times(1).
        WillOnce(Return(false));

    ASSERT_TRUE(manager_.init(security_attributes_, participant_properties_, security_activated_));
    ASSERT_TRUE(security_activated_);
    ASSERT_FALSE(manager_.create_entities());
}

TEST_F(SecurityTest, initialization_fail_participant_volatile_message_writer)
{
    DefaultValue<const GUID_t&>::Set(guid);
    DefaultValue<const ParticipantSecurityAttributes&>::Set(security_attributes_);
    MockParticipantCryptoHandle local_participant_crypto_handle;
    NiceMock<StatelessWriter>* stateless_writer = new NiceMock<StatelessWriter>(&participant_);
    NiceMock<StatelessReader>* stateless_reader = new NiceMock<StatelessReader>();

    EXPECT_CALL(*auth_plugin_, validate_local_identity(_,_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(&local_identity_handle_), Return(ValidationResult_t::VALIDATION_OK)));
    EXPECT_CALL(crypto_plugin_->cryptokeyfactory_, register_local_participant(Ref(local_identity_handle_),_,_,_,_)).Times(1).
        WillOnce(Return(&local_participant_crypto_handle));
    EXPECT_CALL(crypto_plugin_->cryptokeyfactory_, unregister_participant(&local_participant_crypto_handle,_)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(participant_, createWriter_mock(_,_,_,_,_,_)).Times(2).
        WillOnce(DoAll(SetArgPointee<0>(stateless_writer), Return(true))).
        WillOnce(Return(false));
    EXPECT_CALL(participant_, createReader_mock(_,_,_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(stateless_reader), Return(true)));

    ASSERT_TRUE(manager_.init(security_attributes_, participant_properties_, security_activated_));
    ASSERT_TRUE(security_activated_);
    ASSERT_FALSE(manager_.create_entities());
}

TEST_F(SecurityTest, initialization_fail_participant_volatile_message_reader)
{
    DefaultValue<const GUID_t&>::Set(guid);
    DefaultValue<const ParticipantSecurityAttributes&>::Set(security_attributes_);
    MockParticipantCryptoHandle local_participant_crypto_handle;
    NiceMock<StatelessWriter>* stateless_writer = new NiceMock<StatelessWriter>(&participant_);
    NiceMock<StatelessReader>* stateless_reader = new NiceMock<StatelessReader>();
    NiceMock<StatefulWriter>* volatile_writer = new NiceMock<StatefulWriter>(&participant_);

    EXPECT_CALL(*auth_plugin_, validate_local_identity(_,_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(&local_identity_handle_), Return(ValidationResult_t::VALIDATION_OK)));
    EXPECT_CALL(crypto_plugin_->cryptokeyfactory_, register_local_participant(Ref(local_identity_handle_),_,_,_,_)).Times(1).
        WillOnce(Return(&local_participant_crypto_handle));
    EXPECT_CALL(crypto_plugin_->cryptokeyfactory_, unregister_participant(&local_participant_crypto_handle,_)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(participant_, createWriter_mock(_,_,_,_,_,_)).Times(2).
        WillOnce(DoAll(SetArgPointee<0>(stateless_writer), Return(true))).
        WillOnce(DoAll(SetArgPointee<0>(volatile_writer), Return(true)));
    EXPECT_CALL(participant_, createReader_mock(_,_,_,_,_,_,_)).Times(2).
        WillOnce(DoAll(SetArgPointee<0>(stateless_reader), Return(true))).
        WillOnce(Return(false));

    ASSERT_TRUE(manager_.init(security_attributes_, participant_properties_, security_activated_));
    ASSERT_TRUE(security_activated_);
    ASSERT_FALSE(manager_.create_entities());
}

TEST_F(SecurityTest, initialization_auth_retry)
{
    DefaultValue<const GUID_t&>::Set(guid);
    DefaultValue<const ParticipantSecurityAttributes&>::Set(security_attributes_);
    MockParticipantCryptoHandle local_participant_crypto_handle;
    NiceMock<StatelessWriter>* stateless_writer = new NiceMock<StatelessWriter>(&participant_);
    NiceMock<StatelessReader>* stateless_reader = new NiceMock<StatelessReader>();
    NiceMock<StatefulWriter>* volatile_writer = new NiceMock<StatefulWriter>(&participant_);
    NiceMock<StatefulReader>* volatile_reader = new NiceMock<StatefulReader>();

    EXPECT_CALL(*auth_plugin_, validate_local_identity(_,_,_,_,_,_)).Times(2).
        WillOnce(Return(ValidationResult_t::VALIDATION_PENDING_RETRY)).
        WillOnce(DoAll(SetArgPointee<0>(&local_identity_handle_), Return(ValidationResult_t::VALIDATION_OK)));
    EXPECT_CALL(crypto_plugin_->cryptokeyfactory_, register_local_participant(Ref(local_identity_handle_),_,_,_,_)).Times(1).
        WillOnce(Return(&local_participant_crypto_handle));
    EXPECT_CALL(crypto_plugin_->cryptokeyfactory_, unregister_participant(&local_participant_crypto_handle,_)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(participant_, createWriter_mock(_,_,_,_,_,_)).Times(2).
        WillOnce(DoAll(SetArgPointee<0>(stateless_writer), Return(true))).
        WillOnce(DoAll(SetArgPointee<0>(volatile_writer), Return(true)));
    EXPECT_CALL(participant_, createReader_mock(_,_,_,_,_,_,_)).Times(2).
        WillOnce(DoAll(SetArgPointee<0>(stateless_reader), Return(true))).
        WillOnce(DoAll(SetArgPointee<0>(volatile_reader), Return(true)));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_,_)).Times(1).
        WillOnce(Return(true));

    ASSERT_TRUE(manager_.init(security_attributes_, participant_properties_, security_activated_));
    ASSERT_TRUE(!security_activated_ || manager_.create_entities());
}


TEST_F(SecurityTest, initialization_ok)
{
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_,_)).Times(1).
        WillOnce(Return(true));

    initialization_ok();

}

