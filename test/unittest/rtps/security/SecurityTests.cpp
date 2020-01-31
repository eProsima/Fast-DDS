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
    volatile_writer_ = new ::testing::NiceMock<StatefulWriter>(&participant_);
    volatile_reader_ = new ::testing::NiceMock<StatefulReader>();

    EXPECT_CALL(*auth_plugin_, validate_local_identity(_, _, _, _, _, _)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(&local_identity_handle_), Return(ValidationResult_t::VALIDATION_OK)));
    EXPECT_CALL(crypto_plugin_->cryptokeyfactory_, register_local_participant(Ref(local_identity_handle_), _, _, _, _)).Times(1).
        WillOnce(Return(&local_participant_crypto_handle_));
    EXPECT_CALL(crypto_plugin_->cryptokeyfactory_, unregister_participant(&local_participant_crypto_handle_, _)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(participant_, createWriter_mock(_, _, _, _, _, _)).Times(2).
        WillOnce(DoAll(SetArgPointee<0>(stateless_writer_), Return(true))).
        WillOnce(DoAll(SetArgPointee<0>(volatile_writer_), Return(true)));
    EXPECT_CALL(participant_, createReader_mock(_, _, _, _, _, _, _)).Times(2).
        WillOnce(DoAll(SetArgPointee<0>(stateless_reader_), Return(true))).
        WillOnce(DoAll(SetArgPointee<0>(volatile_reader_), Return(true)));

    ASSERT_TRUE(manager_.init(security_attributes_, participant_properties_, security_activated_));
    ASSERT_TRUE(!security_activated_ || manager_.create_entities());
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

    ASSERT_TRUE(manager_.init(security_attributes_, participant_properties_, security_activated_));
    ASSERT_TRUE(!security_activated_ || manager_.create_entities());
}

void SecurityTest::request_process_ok(CacheChange_t** request_message_change)
{
    initialization_ok();

    HandshakeMessageToken handshake_message;
    CacheChange_t* change = new CacheChange_t(200);

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_, Ref(local_identity_handle_), _, _, _)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(&remote_identity_handle_), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_REQUEST)));
    EXPECT_CALL(*auth_plugin_, begin_handshake_request(_, _, Ref(local_identity_handle_),
        Ref(remote_identity_handle_), _, _)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(&handshake_handle_),
            SetArgPointee<1>(&handshake_message), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));
    EXPECT_CALL(*stateless_writer_, new_change(_, _, _)).Times(1).
        WillOnce(Return(change));
    EXPECT_CALL(*stateless_writer_->history_, add_change_mock(change)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(participant_, pdpsimple()).Times(1).WillOnce(Return(&pdpsimple_));
    EXPECT_CALL(pdpsimple_, get_participant_proxy_data_serialized(BIGEND)).Times(1);

    fill_participant_key(participant_data_.m_guid);
    ASSERT_TRUE(manager_.discovered_participant(participant_data_));

    if (request_message_change != nullptr)
        *request_message_change = change;
    else
        delete change;
}

void SecurityTest::reply_process_ok(CacheChange_t** reply_message_change)
{
    initialization_ok();

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_, Ref(local_identity_handle_), _, _, _)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(&remote_identity_handle_), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));

    fill_participant_key(participant_data_.m_guid);
    ASSERT_TRUE(manager_.discovered_participant(participant_data_));

    ParticipantGenericMessage message;
    message.message_identity().source_guid(participant_data_.m_guid);
    message.destination_participant_key(participant_data_.m_guid);
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
#if __BIG_ENDIAN__
    aux_msg.msg_endian = BIGEND;
    change->serializedPayload.encapsulation = PL_CDR_BE;
    CDRMessage::addOctet(&aux_msg, CDR_BE);
#else
    aux_msg.msg_endian = LITTLEEND;
    change->serializedPayload.encapsulation = PL_CDR_LE;
    CDRMessage::addOctet(&aux_msg, CDR_LE);
#endif
    CDRMessage::addUInt16(&aux_msg, 0);

    ASSERT_TRUE(CDRMessage::addParticipantGenericMessage(&aux_msg, message));
    change->serializedPayload.length = aux_msg.length;

    HandshakeMessageToken handshake_message;
    CacheChange_t* change2 = new CacheChange_t(200);

    EXPECT_CALL(*auth_plugin_, begin_handshake_reply_rvr(_, _, _, Ref(remote_identity_handle_),
        Ref(local_identity_handle_), _, _)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(&handshake_handle_),
            SetArgPointee<1>(&handshake_message), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));
    EXPECT_CALL(*stateless_writer_, new_change(_, _, _)).Times(1).
        WillOnce(Return(change2));
    EXPECT_CALL(*stateless_writer_->history_, add_change_mock(change2)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(*stateless_reader_->history_, remove_change_mock(change)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(participant_, pdpsimple()).Times(1).WillOnce(Return(&pdpsimple_));
    EXPECT_CALL(pdpsimple_, get_participant_proxy_data_serialized(BIGEND)).Times(1);

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);

    if (reply_message_change != nullptr)
        *reply_message_change = change2;
    else
        delete change2;
}

void SecurityTest::final_message_process_ok(CacheChange_t** final_message_change)
{
    request_process_ok();

    EXPECT_CALL(*stateless_writer_->history_, remove_change(SequenceNumber_t{ 0,1 })).Times(1).
        WillOnce(Return(true));

    GUID_t remote_participant_key(participant_data_.m_guid);

    ParticipantGenericMessage message;
    message.message_identity().source_guid(remote_participant_key);
    message.related_message_identity().source_guid(remote_participant_key);
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
#if __BIG_ENDIAN__
    aux_msg.msg_endian = BIGEND;
    change->serializedPayload.encapsulation = PL_CDR_BE;
    CDRMessage::addOctet(&aux_msg, CDR_BE);
#else
    aux_msg.msg_endian = LITTLEEND;
    change->serializedPayload.encapsulation = PL_CDR_LE;
    CDRMessage::addOctet(&aux_msg, CDR_LE);
#endif
    CDRMessage::addUInt16(&aux_msg, 0);

    ASSERT_TRUE(CDRMessage::addParticipantGenericMessage(&aux_msg, message));
    change->serializedPayload.length = aux_msg.length;

    HandshakeMessageToken handshake_message;
    CacheChange_t* change2 = new CacheChange_t(200);
    MockSharedSecretHandle shared_secret_handle;
    MockParticipantCryptoHandle participant_crypto_handle;

    EXPECT_CALL(*auth_plugin_, process_handshake_rvr(_, _, Ref(handshake_handle_), _)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(&handshake_message), Return(ValidationResult_t::VALIDATION_OK_WITH_FINAL_MESSAGE)));
    EXPECT_CALL(*stateless_writer_, new_change(_, _, _)).Times(1).
        WillOnce(Return(change2));
    EXPECT_CALL(*stateless_writer_->history_, add_change_mock(change2)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(*stateless_reader_->history_, remove_change_mock(change)).Times(1).
        WillOnce(Return(true));
    //TODO(Ricardo) Verify parameter passed to notifyAboveRemoteEndpoints
    EXPECT_CALL(participant_, pdpsimple()).Times(1).WillOnce(Return(&pdpsimple_));
    EXPECT_CALL(pdpsimple_, notifyAboveRemoteEndpoints(_)).Times(1);
    EXPECT_CALL(*auth_plugin_, get_shared_secret(Ref(handshake_handle_), _)).Times(1).
        WillOnce(Return(&shared_secret_handle));
    EXPECT_CALL(*auth_plugin_, return_sharedsecret_handle(&shared_secret_handle, _)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(crypto_plugin_->cryptokeyfactory_, register_matched_remote_participant(Ref(local_participant_crypto_handle_),
        Ref(remote_identity_handle_), _, Ref(shared_secret_handle), _)).Times(1).
        WillOnce(Return(&participant_crypto_handle));
    EXPECT_CALL(crypto_plugin_->cryptokeyexchange_, create_local_participant_crypto_tokens(_,
        Ref(local_participant_crypto_handle_), Ref(participant_crypto_handle), _)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(crypto_plugin_->cryptokeyfactory_, unregister_participant(&participant_crypto_handle, _)).Times(1).
        WillOnce(Return(true));

    ParticipantAuthenticationInfo info;
    info.status = ParticipantAuthenticationInfo::AUTHORIZED_PARTICIPANT;
    info.guid = remote_participant_key;
    EXPECT_CALL(*participant_.getListener(), onParticipantAuthentication(_, info)).Times(1);

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);

    if (final_message_change == nullptr)
        delete change2;
    else
        *final_message_change = change2;
}
