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

void SecurityTest::initialization_ok()
{
    ::testing::DefaultValue<const GUID_t&>::Set(guid);
    ::testing::DefaultValue<CDRMessage_t>::Set(default_cdr_message);
    ::testing::DefaultValue<const ParticipantSecurityAttributes&>::Set(security_attributes_);
    stateless_writer_ = new ::testing::NiceMock<StatelessWriter>(&participant_);
    stateless_reader_ = new ::testing::NiceMock<StatelessReader>();
    volatile_writer_ = new ::testing::StrictMock<StatefulWriter>(&participant_);
    volatile_reader_ = new ::testing::NiceMock<StatefulReader>();

    EXPECT_CALL(*auth_plugin_, validate_local_identity(_, _, _, _, _, _)).Times(1).
            WillOnce(DoAll(SetArgPointee<0>(&local_identity_handle_), Return(ValidationResult_t::VALIDATION_OK)));
    EXPECT_CALL(crypto_plugin_->cryptokeyfactory_,
            register_local_participant(Ref(local_identity_handle_), _, _, _, _)).Times(1).
            WillOnce(Return(local_participant_crypto_handle_));
    EXPECT_CALL(crypto_plugin_->cryptokeyfactory_,
            unregister_participant(local_participant_crypto_handle_, _)).Times(1).
            WillOnce(Return(true));
    EXPECT_CALL(participant_, createWriter_mock(_, _, _, _, _, _)).Times(2).
            WillOnce(DoAll(SetArgPointee<0>(stateless_writer_), Return(true))).
            WillOnce(DoAll(SaveArg<3>(&volatile_writer_->listener_), SetArgPointee<0>(volatile_writer_), Return(true)));
    EXPECT_CALL(participant_, createReader_mock(_, _, _, _, _, _, _)).Times(2).
            WillOnce(DoAll(SetArgPointee<0>(stateless_reader_), Return(true))).
            WillOnce(DoAll(SetArgPointee<0>(volatile_reader_), Return(true)));

    security_activated_ = manager_.init(security_attributes_, participant_properties_);
    ASSERT_TRUE(security_activated_);
    ASSERT_TRUE(manager_.is_security_initialized());
    ASSERT_TRUE(manager_.create_entities());
    ASSERT_TRUE(volatile_writer_->listener_ != nullptr);
}

void SecurityTest::initialization_auth_ok()
{
    SecurityPluginFactory::release_crypto_plugin();

    ::testing::DefaultValue<const GUID_t&>::Set(guid);
    ::testing::DefaultValue<CDRMessage_t>::Set(default_cdr_message);
    ::testing::DefaultValue<const ParticipantSecurityAttributes&>::Set(security_attributes_);
    stateless_writer_ = new ::testing::NiceMock<StatelessWriter>(&participant_);
    stateless_reader_ = new ::testing::NiceMock<StatelessReader>();

    EXPECT_CALL(*auth_plugin_, validate_local_identity(_, _, _, _, _, _)).Times(1).
            WillOnce(DoAll(SetArgPointee<0>(&local_identity_handle_), Return(ValidationResult_t::VALIDATION_OK)));
    EXPECT_CALL(participant_, createWriter_mock(_, _, _, _, _, _)).Times(1).
            WillOnce(DoAll(SetArgPointee<0>(stateless_writer_), Return(true)));
    EXPECT_CALL(participant_, createReader_mock(_, _, _, _, _, _, _)).Times(1).
            WillOnce(DoAll(SetArgPointee<0>(stateless_reader_), Return(true)));

    security_activated_ = manager_.init(security_attributes_, participant_properties_);
    ASSERT_TRUE(security_activated_);
    ASSERT_TRUE(manager_.is_security_initialized());
    ASSERT_TRUE(manager_.create_entities());
}

void SecurityTest::request_process_ok(
        CacheChange_t** request_message_change)
{
    initialization_ok();

    HandshakeMessageToken handshake_message;
    CacheChange_t* change = new CacheChange_t(200);

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_, Ref(local_identity_handle_), _, _, _)).Times(1).
            WillOnce(DoAll(SetArgPointee<0>(&remote_identity_handle_),
            Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_REQUEST)));
    EXPECT_CALL(*auth_plugin_, begin_handshake_request(_, _, Ref(local_identity_handle_),
            Ref(remote_identity_handle_), _, _)).Times(1).
            WillOnce(DoAll(SetArgPointee<0>(&handshake_handle_),
            SetArgPointee<1>(&handshake_message), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));
    EXPECT_CALL(*stateless_writer_->history_, create_change(_, _, _)).Times(1).
            WillOnce(Return(change));
    EXPECT_CALL(*stateless_writer_->history_, add_change_mock(change)).Times(1).
            WillOnce(Return(true));
    EXPECT_CALL(participant_, pdp()).Times(1).WillOnce(Return(&pdp_));
    EXPECT_CALL(pdp_, get_participant_proxy_data_serialized(BIGEND)).Times(1);

    fill_participant_key(participant_data_.guid);
    ASSERT_TRUE(manager_.discovered_participant(participant_data_));

    if (request_message_change != nullptr)
    {
        *request_message_change = change;
    }
    else
    {
        delete change;
    }
}

void SecurityTest::reply_process_ok(
        CacheChange_t** reply_message_change)
{
    initialization_ok();

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_, Ref(local_identity_handle_), _, _, _)).Times(1).
            WillOnce(DoAll(SetArgPointee<0>(&remote_identity_handle_),
            Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));

    fill_participant_key(participant_data_.guid);
    ASSERT_TRUE(manager_.discovered_participant(participant_data_));

    ParticipantGenericMessage message;
    message.message_identity().source_guid(participant_data_.guid);
    message.destination_participant_key(participant_data_.guid);
    message.message_class_id("dds.sec.auth");
    HandshakeMessageToken token;
    message.message_data().push_back(token);
    CacheChange_t* change =
            new CacheChange_t(static_cast<uint32_t>(ParticipantGenericMessageHelper::serialized_size(message))
                    + 4 /*encapsulation*/); //TODO(Ricardo) Think casting
    CDRMessage_t aux_msg(0);
    aux_msg.wraps = true;
    aux_msg.buffer = change->serializedPayload.data;
    aux_msg.max_size = change->serializedPayload.max_size;

    // Serialize encapsulation
    CDRMessage::addOctet(&aux_msg, 0);
    aux_msg.msg_endian = DEFAULT_ENDIAN;
    change->serializedPayload.encapsulation = PL_DEFAULT_ENCAPSULATION;
    CDRMessage::addOctet(&aux_msg, DEFAULT_ENCAPSULATION);
    CDRMessage::addUInt16(&aux_msg, 0);

    ASSERT_TRUE(CDRMessage::addParticipantGenericMessage(&aux_msg, message));
    change->serializedPayload.length = aux_msg.length;

    HandshakeMessageToken handshake_message;
    CacheChange_t* change2 = new CacheChange_t(200);

    EXPECT_CALL(*auth_plugin_, begin_handshake_reply_rvr(_, _, _, Ref(remote_identity_handle_),
            Ref(local_identity_handle_), _, _)).Times(1).
            WillOnce(DoAll(SetArgPointee<0>(&handshake_handle_),
            SetArgPointee<1>(&handshake_message), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));
    EXPECT_CALL(*stateless_writer_->history_, create_change(_, _, _)).Times(1).
            WillOnce(Return(change2));
    EXPECT_CALL(*stateless_writer_->history_, add_change_mock(change2)).Times(1).
            WillOnce(Return(true));
    EXPECT_CALL(*stateless_reader_->history_, remove_change_mock(change)).Times(1).
            WillOnce(Return(true));
    EXPECT_CALL(participant_, pdp()).Times(1).WillOnce(Return(&pdp_));
    EXPECT_CALL(pdp_, get_participant_proxy_data_serialized(BIGEND)).Times(1);

    stateless_reader_->listener_->on_new_cache_change_added(stateless_reader_, change);

    if (reply_message_change != nullptr)
    {
        *reply_message_change = change2;
    }
    else
    {
        delete change2;
    }
}

void SecurityTest::final_message_process_ok(
        CacheChange_t** final_message_change)
{
    request_process_ok();

    EXPECT_CALL(*stateless_writer_->history_, remove_change(SequenceNumber_t{ 0, 1 })).Times(1).
            WillOnce(Return(true));

    GUID_t remote_participant_key(participant_data_.guid);

    ParticipantGenericMessage message;
    message.message_identity().source_guid(remote_participant_key);
    message.related_message_identity().source_guid(stateless_writer_->getGuid());
    message.related_message_identity().sequence_number(1);
    message.destination_participant_key(remote_participant_key);
    message.message_class_id("dds.sec.auth");
    HandshakeMessageToken token;
    message.message_data().push_back(token);
    CacheChange_t* change =
            new CacheChange_t(static_cast<uint32_t>(ParticipantGenericMessageHelper::serialized_size(message))
                    + 4 /*encapsulation*/);
    CDRMessage_t aux_msg(0);
    aux_msg.wraps = true;
    aux_msg.buffer = change->serializedPayload.data;
    aux_msg.max_size = change->serializedPayload.max_size;
    //
    // Serialize encapsulation
    CDRMessage::addOctet(&aux_msg, 0);
    aux_msg.msg_endian = DEFAULT_ENDIAN;
    change->serializedPayload.encapsulation = PL_DEFAULT_ENCAPSULATION;
    CDRMessage::addOctet(&aux_msg, DEFAULT_ENCAPSULATION);
    CDRMessage::addUInt16(&aux_msg, 0);

    ASSERT_TRUE(CDRMessage::addParticipantGenericMessage(&aux_msg, message));
    change->serializedPayload.length = aux_msg.length;

    HandshakeMessageToken handshake_message;
    CacheChange_t* change2 = new CacheChange_t(200);
    auto shared_secret_handle = auth_plugin_->get_dummy_shared_secret();

    auto mock_crypto_factory = dynamic_cast<MockCryptoKeyFactory*>(crypto_plugin_->cryptokeyfactory());
    assert(mock_crypto_factory != nullptr);
    auto participant_crypto_handle = mock_crypto_factory->get_dummy_participant_handle();

    EXPECT_CALL(*auth_plugin_, process_handshake_rvr(_, _, Ref(handshake_handle_), _)).Times(1).
            WillOnce(DoAll(SetArgPointee<0>(&handshake_message),
            Return(ValidationResult_t::VALIDATION_OK_WITH_FINAL_MESSAGE)));
    EXPECT_CALL(*stateless_writer_->history_, create_change(_, _, _)).Times(1).
            WillOnce(Return(change2));
    EXPECT_CALL(*stateless_writer_->history_, add_change_mock(change2)).Times(1).
            WillOnce(Return(true));
    EXPECT_CALL(*stateless_reader_->history_, remove_change_mock(change)).Times(1).
            WillOnce(Return(true));
    //TODO(Ricardo) Verify parameter passed to notifyAboveRemoteEndpoints
    EXPECT_CALL(participant_, pdp()).Times(1).WillOnce(Return(&pdp_));
    EXPECT_CALL(pdp_, notifyAboveRemoteEndpoints(_, true)).Times(1);
    EXPECT_CALL(*auth_plugin_, get_shared_secret(Ref(handshake_handle_), _)).Times(1).
            WillOnce(Return(shared_secret_handle));
    EXPECT_CALL(*auth_plugin_, return_sharedsecret_handle(shared_secret_handle, _)).Times(1).
            WillRepeatedly(Return(true));
    EXPECT_CALL(crypto_plugin_->cryptokeyfactory_,
            register_matched_remote_participant(Ref(*local_participant_crypto_handle_),
            Ref(remote_identity_handle_), _, Ref(*shared_secret_handle), _)).Times(1).
            WillOnce(Return(participant_crypto_handle));
    EXPECT_CALL(crypto_plugin_->cryptokeyexchange_, create_local_participant_crypto_tokens(_,
            Ref(*local_participant_crypto_handle_), Ref(*participant_crypto_handle), _)).Times(1).
            WillOnce(Return(true));
    EXPECT_CALL(crypto_plugin_->cryptokeyfactory_, unregister_participant(participant_crypto_handle, _)).Times(1).
            WillOnce(Return(true));

    ParticipantAuthenticationInfo info;
    info.status = ParticipantAuthenticationInfo::AUTHORIZED_PARTICIPANT;
    info.guid = remote_participant_key;
    EXPECT_CALL(*participant_.getListener(), onParticipantAuthentication(_, info)).Times(1);

    CacheChange_t kx_change_to_add;
    CacheChange_t* kx_change_to_remove = new CacheChange_t(200);
    expect_kx_exchange(kx_change_to_add, kx_change_to_remove);

    stateless_reader_->listener_->on_new_cache_change_added(stateless_reader_, change);

    volatile_writer_->listener_->on_writer_change_received_by_all(volatile_writer_, kx_change_to_remove);

    if (final_message_change == nullptr)
    {
        delete change2;
    }
    else
    {
        *final_message_change = change2;
    }
}

void SecurityTest::expect_kx_exchange(
        CacheChange_t& kx_change_to_add,
        CacheChange_t* kx_change_to_remove)
{
    EXPECT_CALL(*volatile_writer_->history_, create_change(_, _, _)).Times(1).WillOnce(
        DoAll(Invoke([&kx_change_to_add](uint32_t cdr_size, ChangeKind_t, InstanceHandle_t)
        {
            kx_change_to_add.serializedPayload.reserve(cdr_size);
        }),
        Return(&kx_change_to_add)));
    EXPECT_CALL(*volatile_writer_->history_, add_change_mock(&kx_change_to_add)).Times(1).
            WillOnce(Return(true));
    EXPECT_CALL(*volatile_writer_->history_, remove_change_mock(kx_change_to_remove)).Times(1).
            WillOnce(Return(true));
}

void SecurityTest::destroy_manager_and_change(
        CacheChange_t*& change,
        bool was_added)
{
    if (was_added)
    {
        EXPECT_CALL(*stateless_writer_->history_,
                remove_change(change->sequenceNumber)).Times(1).WillOnce(Return(true));
    }

    manager_.destroy();
    delete change;
    change = nullptr;
}
