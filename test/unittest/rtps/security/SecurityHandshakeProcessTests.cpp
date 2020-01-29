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
#include <fastrtps/rtps/network/NetworkFactory.h>

TEST_F(SecurityTest, discovered_participant_begin_handshake_request_fail_and_then_ok)
{
    initialization_ok();

    MockIdentityHandle remote_identity_handle;
    MockHandshakeHandle handshake_handle;
    HandshakeMessageToken handshake_message;
    CacheChange_t* change = new CacheChange_t(200);

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_, Ref(local_identity_handle_),_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(&remote_identity_handle), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_REQUEST)));
    EXPECT_CALL(*auth_plugin_, begin_handshake_request(_,_, Ref(local_identity_handle_),
                Ref(remote_identity_handle),_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_FAILED));

    ParticipantProxyData participant_data;
    fill_participant_key(participant_data.m_guid);
    ParticipantAuthenticationInfo info;
    info.status = ParticipantAuthenticationInfo::UNAUTHORIZED_PARTICIPANT;
    info.guid = participant_data.m_guid;
    EXPECT_CALL(*participant_.getListener(), onParticipantAuthentication(_, info)).Times(1);
    EXPECT_CALL(participant_, pdpsimple()).Times(1).WillOnce(Return(&pdpsimple_));
    EXPECT_CALL(pdpsimple_, get_participant_proxy_data_serialized(BIGEND)).Times(1);

    ASSERT_FALSE(manager_.discovered_participant(participant_data));

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_, Ref(local_identity_handle_),_,_,_)).Times(0);
    EXPECT_CALL(*auth_plugin_, begin_handshake_request(_,_, Ref(local_identity_handle_),
                Ref(remote_identity_handle),_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(&handshake_handle),
                    SetArgPointee<1>(&handshake_message), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));
    EXPECT_CALL(*stateless_writer_, new_change(_,_,_)).Times(1).
        WillOnce(Return(change));
    EXPECT_CALL(*stateless_writer_->history_, add_change_mock(change)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&remote_identity_handle,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(&handshake_handle,_)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(participant_, pdpsimple()).Times(1).WillOnce(Return(&pdpsimple_));
    EXPECT_CALL(pdpsimple_, get_participant_proxy_data_serialized(BIGEND)).Times(1);

    ASSERT_TRUE(manager_.discovered_participant(participant_data));

    manager_.destroy();

    delete change;
}

TEST_F(SecurityTest, discovered_participant_process_message_not_remote_participant_key)
{
    initialization_ok();

    ParticipantGenericMessage message;
    message.message_class_id("dds.sec.auth");
    CacheChange_t* change = new CacheChange_t(static_cast<uint32_t>(ParticipantGenericMessageHelper::serialized_size(message))
            + 4 /*encapsulation*/);
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

    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*stateless_reader_->history_, remove_change_mock(change)).Times(1).
        WillOnce(Return(true));

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);
}

TEST_F(SecurityTest, discovered_participant_process_message_bad_message_class_id)
{
    initialization_ok();

    MockIdentityHandle remote_identity_handle;

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_, Ref(local_identity_handle_),_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(&remote_identity_handle), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));

    ParticipantProxyData participant_data;
    fill_participant_key(participant_data.m_guid);
    ASSERT_TRUE(manager_.discovered_participant(participant_data));

    ParticipantGenericMessage message;
    CacheChange_t* change = new CacheChange_t(static_cast<uint32_t>(ParticipantGenericMessageHelper::serialized_size(message))
            + 4 /*encapsulation*/);
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

    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&remote_identity_handle,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*stateless_reader_->history_, remove_change_mock(change)).Times(1).
        WillOnce(Return(true));

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);
}

TEST_F(SecurityTest, discovered_participant_process_message_not_expecting_request)
{
    initialization_auth_ok();

    MockIdentityHandle remote_identity_handle;

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_, Ref(local_identity_handle_),_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(&remote_identity_handle), Return(ValidationResult_t::VALIDATION_OK)));

    ParticipantProxyData participant_data;
    fill_participant_key(participant_data.m_guid);
    EXPECT_CALL(participant_, pdpsimple()).Times(1).WillOnce(Return(&pdpsimple_));
    EXPECT_CALL(pdpsimple_, notifyAboveRemoteEndpoints(_)).Times(1);

    ParticipantAuthenticationInfo info;
    info.status = ParticipantAuthenticationInfo::AUTHORIZED_PARTICIPANT;
    info.guid = participant_data.m_guid;
    EXPECT_CALL(*participant_.getListener(), onParticipantAuthentication(_, info)).Times(1);

    ASSERT_TRUE(manager_.discovered_participant(participant_data));

    ParticipantGenericMessage message;
    message.message_identity().source_guid(participant_data.m_guid);
    message.destination_participant_key(participant_data.m_guid);
    message.message_class_id("dds.sec.auth");
    HandshakeMessageToken token;
    message.message_data().push_back(token);
    CacheChange_t* change = new CacheChange_t(static_cast<uint32_t>(ParticipantGenericMessageHelper::serialized_size(message))
            + 4 /*encapsulation*/);
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

    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&remote_identity_handle,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*stateless_reader_->history_, remove_change_mock(change)).Times(1).
        WillOnce(Return(true));

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);
}

TEST_F(SecurityTest, discovered_participant_process_message_fail_begin_handshake_reply)
{
    initialization_ok();

    MockIdentityHandle remote_identity_handle;

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_, Ref(local_identity_handle_),_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(&remote_identity_handle), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));

    ParticipantProxyData participant_data;
    fill_participant_key(participant_data.m_guid);
    ASSERT_TRUE(manager_.discovered_participant(participant_data));

    ParticipantGenericMessage message;
    message.message_identity().source_guid(participant_data.m_guid);
    message.destination_participant_key(participant_data.m_guid);
    message.message_class_id("dds.sec.auth");
    HandshakeMessageToken token;
    message.message_data().push_back(token);
    CacheChange_t* change = new CacheChange_t(static_cast<uint32_t>(ParticipantGenericMessageHelper::serialized_size(message))
            + 4 /*encapsulation*/);
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

    EXPECT_CALL(*auth_plugin_, begin_handshake_reply_rvr(_,_,_, Ref(remote_identity_handle),
                Ref(local_identity_handle_),_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_FAILED));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&remote_identity_handle,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*stateless_reader_->history_, remove_change_mock(change)).Times(1).
        WillOnce(Return(true));
    ParticipantAuthenticationInfo info;
    info.status = ParticipantAuthenticationInfo::UNAUTHORIZED_PARTICIPANT;
    info.guid = participant_data.m_guid;
    EXPECT_CALL(*participant_.getListener(), onParticipantAuthentication(_, info)).Times(1);
    EXPECT_CALL(participant_, pdpsimple()).Times(1).WillOnce(Return(&pdpsimple_));
    EXPECT_CALL(pdpsimple_, get_participant_proxy_data_serialized(BIGEND)).Times(1);

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);
}

TEST_F(SecurityTest, discovered_participant_process_message_ok_begin_handshake_reply)
{
    initialization_ok();

    MockIdentityHandle remote_identity_handle;

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_, Ref(local_identity_handle_),_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(&remote_identity_handle), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));

    ParticipantProxyData participant_data;
    fill_participant_key(participant_data.m_guid);
    ASSERT_TRUE(manager_.discovered_participant(participant_data));

    ParticipantGenericMessage message;
    message.message_identity().source_guid(participant_data.m_guid);
    message.destination_participant_key(participant_data.m_guid);
    message.message_class_id("dds.sec.auth");
    HandshakeMessageToken token;
    message.message_data().push_back(token);
    CacheChange_t* change = new CacheChange_t(static_cast<uint32_t>(ParticipantGenericMessageHelper::serialized_size(message))
            + 4 /*encapsulation*/);
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

    MockHandshakeHandle handshake_handle;
    MockSharedSecretHandle shared_secret_handle;
    MockParticipantCryptoHandle participant_crypto_handle;

    EXPECT_CALL(*auth_plugin_, begin_handshake_reply_rvr(_,_,_, Ref(remote_identity_handle),
                Ref(local_identity_handle_),_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(&handshake_handle),
                    Return(ValidationResult_t::VALIDATION_OK)));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&remote_identity_handle,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(&handshake_handle,_)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(*stateless_reader_->history_, remove_change_mock(change)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(participant_, pdpsimple()).Times(2).WillRepeatedly(Return(&pdpsimple_));
    EXPECT_CALL(pdpsimple_, notifyAboveRemoteEndpoints(_)).Times(1);
    EXPECT_CALL(pdpsimple_, get_participant_proxy_data_serialized(BIGEND)).Times(1);
    EXPECT_CALL(*auth_plugin_, get_shared_secret(Ref(handshake_handle),_)).Times(1).
        WillOnce(Return(&shared_secret_handle));
    EXPECT_CALL(*auth_plugin_, return_sharedsecret_handle(&shared_secret_handle,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(crypto_plugin_->cryptokeyfactory_, register_matched_remote_participant(Ref(local_participant_crypto_handle_),
                Ref(remote_identity_handle),_,Ref(shared_secret_handle),_)).Times(1).
        WillOnce(Return(&participant_crypto_handle));
    EXPECT_CALL(crypto_plugin_->cryptokeyexchange_, create_local_participant_crypto_tokens(_,
                Ref(local_participant_crypto_handle_), Ref(participant_crypto_handle),_)).Times(1).
           WillOnce(Return(true));
    EXPECT_CALL(crypto_plugin_->cryptokeyfactory_, unregister_participant(&participant_crypto_handle,_)).Times(1).
        WillOnce(Return(true));

    ParticipantAuthenticationInfo info;
    info.status = ParticipantAuthenticationInfo::AUTHORIZED_PARTICIPANT;
    info.guid = participant_data.m_guid;
    EXPECT_CALL(*participant_.getListener(), onParticipantAuthentication(_, info)).Times(1);

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);
}

TEST_F(SecurityTest, discovered_participant_process_message_new_change_fail)
{
    initialization_ok();

    MockIdentityHandle remote_identity_handle;

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_, Ref(local_identity_handle_),_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(&remote_identity_handle), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));

    ParticipantProxyData participant_data;
    fill_participant_key(participant_data.m_guid);
    ASSERT_TRUE(manager_.discovered_participant(participant_data));

    ParticipantGenericMessage message;
    message.message_identity().source_guid(participant_data.m_guid);
    message.destination_participant_key(participant_data.m_guid);
    message.message_class_id("dds.sec.auth");
    HandshakeMessageToken token;
    message.message_data().push_back(token);
    CacheChange_t* change = new CacheChange_t(static_cast<uint32_t>(ParticipantGenericMessageHelper::serialized_size(message))
            + 4 /*encapsulation*/);
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

    MockHandshakeHandle handshake_handle;
    HandshakeMessageToken handshake_message;

    EXPECT_CALL(*auth_plugin_, begin_handshake_reply_rvr(_,_,_, Ref(remote_identity_handle),
                Ref(local_identity_handle_),_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(&handshake_handle),
                    SetArgPointee<1>(&handshake_message), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&remote_identity_handle,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*stateless_writer_, new_change(_,_,_)).Times(1).
        WillOnce(Return(nullptr));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(&handshake_handle,_)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(*stateless_reader_->history_, remove_change_mock(change)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(participant_, pdpsimple()).Times(1).WillOnce(Return(&pdpsimple_));
    EXPECT_CALL(pdpsimple_, get_participant_proxy_data_serialized(BIGEND)).Times(1);

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);
}

TEST_F(SecurityTest, discovered_participant_process_message_add_change_fail)
{
    initialization_ok();

    MockIdentityHandle remote_identity_handle;

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_, Ref(local_identity_handle_),_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(&remote_identity_handle), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));

    ParticipantProxyData participant_data;
    fill_participant_key(participant_data.m_guid);
    ASSERT_TRUE(manager_.discovered_participant(participant_data));

    ParticipantGenericMessage message;
    message.message_identity().source_guid(participant_data.m_guid);
    message.destination_participant_key(participant_data.m_guid);
    message.message_class_id("dds.sec.auth");
    HandshakeMessageToken token;
    message.message_data().push_back(token);
    CacheChange_t* change = new CacheChange_t(static_cast<uint32_t>(ParticipantGenericMessageHelper::serialized_size(message))
            + 4 /*encapsulation*/);
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

    MockHandshakeHandle handshake_handle;
    HandshakeMessageToken handshake_message;
    CacheChange_t* change2 = new CacheChange_t(200);

    EXPECT_CALL(*auth_plugin_, begin_handshake_reply_rvr(_,_,_, Ref(remote_identity_handle),
                Ref(local_identity_handle_),_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(&handshake_handle),
                    SetArgPointee<1>(&handshake_message), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&remote_identity_handle,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*stateless_writer_, new_change(_,_,_)).Times(1).
        WillOnce(Return(change2));
    EXPECT_CALL(*stateless_writer_->history_, add_change_mock(change2)).Times(1).
        WillOnce(Return(false));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(&handshake_handle,_)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(*stateless_reader_->history_, remove_change_mock(change)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(participant_, pdpsimple()).Times(1).WillOnce(Return(&pdpsimple_));
    EXPECT_CALL(pdpsimple_, get_participant_proxy_data_serialized(BIGEND)).Times(1);

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);

    manager_.destroy();

    delete change2;
}

TEST_F(SecurityTest, discovered_participant_process_message_pending_handshake_reply_pending_message)
{
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&remote_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(&handshake_handle_,_)).Times(1).
        WillOnce(Return(true));

    reply_process_ok();
}

TEST_F(SecurityTest, discovered_participant_process_message_pending_handshake_reply_pending_message_resent)
{
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&remote_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(&handshake_handle_,_)).Times(1).
        WillOnce(Return(true));

    CacheChange_t* reply_message_change = nullptr;
    reply_process_ok(&reply_message_change);

    EXPECT_CALL(*stateless_writer_->history_, remove_change_and_reuse(reply_message_change->sequenceNumber)).Times(1).
        WillOnce(Return(reply_message_change));
    EXPECT_CALL(*stateless_writer_->history_, add_change_mock(reply_message_change)).Times(1).
        WillOnce(Return(true));
    stateless_writer_->history_->wait_for_more_samples_than(1);

    manager_.destroy();

    delete reply_message_change;
}

TEST_F(SecurityTest, discovered_participant_process_message_pending_handshake_reply_ok_with_final_message)
{
    initialization_ok();

    MockIdentityHandle remote_identity_handle;

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_, Ref(local_identity_handle_),_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(&remote_identity_handle), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));

    ParticipantProxyData participant_data;
    fill_participant_key(participant_data.m_guid);
    ASSERT_TRUE(manager_.discovered_participant(participant_data));

    ParticipantGenericMessage message;
    message.message_identity().source_guid(participant_data.m_guid);
    message.destination_participant_key(participant_data.m_guid);
    message.message_class_id("dds.sec.auth");
    HandshakeMessageToken token;
    message.message_data().push_back(token);
    CacheChange_t* change = new CacheChange_t(static_cast<uint32_t>(ParticipantGenericMessageHelper::serialized_size(message))
            + 4 /*encapsulation*/);
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

    MockHandshakeHandle handshake_handle;
    HandshakeMessageToken handshake_message;
    CacheChange_t* change2 = new CacheChange_t(200);
    MockSharedSecretHandle shared_secret_handle;
    MockParticipantCryptoHandle participant_crypto_handle;

    EXPECT_CALL(*auth_plugin_, begin_handshake_reply_rvr(_,_,_, Ref(remote_identity_handle),
                Ref(local_identity_handle_),_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(&handshake_handle),
                    SetArgPointee<1>(&handshake_message), Return(ValidationResult_t::VALIDATION_OK_WITH_FINAL_MESSAGE)));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&remote_identity_handle,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*stateless_writer_, new_change(_,_,_)).Times(1).
        WillOnce(Return(change2));
    EXPECT_CALL(*stateless_writer_->history_, add_change_mock(change2)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(&handshake_handle,_)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(*stateless_reader_->history_, remove_change_mock(change)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(participant_, pdpsimple()).Times(2).WillRepeatedly(Return(&pdpsimple_));
    EXPECT_CALL(pdpsimple_, notifyAboveRemoteEndpoints(_)).Times(1);
    EXPECT_CALL(pdpsimple_, get_participant_proxy_data_serialized(BIGEND)).Times(1);
    EXPECT_CALL(*auth_plugin_, get_shared_secret(Ref(handshake_handle),_)).Times(1).
        WillOnce(Return(&shared_secret_handle));
    EXPECT_CALL(*auth_plugin_, return_sharedsecret_handle(&shared_secret_handle,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(crypto_plugin_->cryptokeyfactory_, register_matched_remote_participant(Ref(local_participant_crypto_handle_),
                Ref(remote_identity_handle),_,Ref(shared_secret_handle),_)).Times(1).
        WillOnce(Return(&participant_crypto_handle));
    EXPECT_CALL(crypto_plugin_->cryptokeyexchange_, create_local_participant_crypto_tokens(_,
                Ref(local_participant_crypto_handle_), Ref(participant_crypto_handle),_)).Times(1).
           WillOnce(Return(true));
    EXPECT_CALL(crypto_plugin_->cryptokeyfactory_, unregister_participant(&participant_crypto_handle,_)).Times(1).
        WillOnce(Return(true));

    ParticipantAuthenticationInfo info;
    info.status = ParticipantAuthenticationInfo::AUTHORIZED_PARTICIPANT;
    info.guid = participant_data.m_guid;
    EXPECT_CALL(*participant_.getListener(), onParticipantAuthentication(_, info)).Times(1);

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);

    manager_.destroy();

    delete change2;
}

TEST_F(SecurityTest, discovered_participant_process_message_fail_process_handshake_reply)
{
    request_process_ok();

    GUID_t remote_participant_key(participant_data_.m_guid);

    ParticipantGenericMessage message;
    message.message_identity().source_guid(remote_participant_key);
    message.related_message_identity().source_guid(remote_participant_key);
    message.related_message_identity().sequence_number(1);
    message.destination_participant_key(remote_participant_key);
    message.message_class_id("dds.sec.auth");
    HandshakeMessageToken token;
    message.message_data().push_back(token);
    CacheChange_t* change = new CacheChange_t(static_cast<uint32_t>(ParticipantGenericMessageHelper::serialized_size(message))
            + 4 /*encapsulation*/);
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

    EXPECT_CALL(*auth_plugin_, process_handshake_rvr(_,_, Ref(handshake_handle_),_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_FAILED));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&remote_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(&handshake_handle_,_)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(*stateless_reader_->history_, remove_change_mock(change)).Times(1).
        WillOnce(Return(true));
    ParticipantAuthenticationInfo info;
    info.status = ParticipantAuthenticationInfo::UNAUTHORIZED_PARTICIPANT;
    info.guid = remote_participant_key;
    EXPECT_CALL(*participant_.getListener(), onParticipantAuthentication(_, info)).Times(1);

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);
}

TEST_F(SecurityTest, discovered_participant_process_message_ok_process_handshake_reply)
{
    request_process_ok();

    EXPECT_CALL(*stateless_writer_->history_, remove_change(SequenceNumber_t{0,1})).Times(1).
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
    CacheChange_t* change = new CacheChange_t(static_cast<uint32_t>(ParticipantGenericMessageHelper::serialized_size(message))
            + 4 /*encapsulation*/);
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

    MockSharedSecretHandle shared_secret_handle;
    MockParticipantCryptoHandle participant_crypto_handle;

    EXPECT_CALL(*auth_plugin_, process_handshake_rvr(_,_, Ref(handshake_handle_),_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&remote_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(&handshake_handle_,_)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(*stateless_reader_->history_, remove_change_mock(change)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(participant_, pdpsimple()).Times(1).WillOnce(Return(&pdpsimple_));
    EXPECT_CALL(pdpsimple_, notifyAboveRemoteEndpoints(_)).Times(1);
    EXPECT_CALL(*auth_plugin_, get_shared_secret(Ref(handshake_handle_),_)).Times(1).
        WillOnce(Return(&shared_secret_handle));
    EXPECT_CALL(*auth_plugin_, return_sharedsecret_handle(&shared_secret_handle,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(crypto_plugin_->cryptokeyfactory_, register_matched_remote_participant(Ref(local_participant_crypto_handle_),
                Ref(remote_identity_handle_),_,Ref(shared_secret_handle),_)).Times(1).
        WillOnce(Return(&participant_crypto_handle));
    EXPECT_CALL(crypto_plugin_->cryptokeyexchange_, create_local_participant_crypto_tokens(_,
                Ref(local_participant_crypto_handle_), Ref(participant_crypto_handle),_)).Times(1).
           WillOnce(Return(true));
    EXPECT_CALL(crypto_plugin_->cryptokeyfactory_, unregister_participant(&participant_crypto_handle,_)).Times(1).
        WillOnce(Return(true));

    ParticipantAuthenticationInfo info;
    info.status = ParticipantAuthenticationInfo::AUTHORIZED_PARTICIPANT;
    info.guid = remote_participant_key;
    EXPECT_CALL(*participant_.getListener(), onParticipantAuthentication(_, info)).Times(1);

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);
}

TEST_F(SecurityTest, discovered_participant_process_message_process_handshake_reply_new_change_fail)
{
    request_process_ok();

    EXPECT_CALL(*stateless_writer_->history_, remove_change(SequenceNumber_t{0,1})).Times(1).
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
    CacheChange_t* change = new CacheChange_t(static_cast<uint32_t>(ParticipantGenericMessageHelper::serialized_size(message))
            + 4 /*encapsulation*/);
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

    EXPECT_CALL(*auth_plugin_, process_handshake_rvr(_,_, Ref(handshake_handle_),_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(&handshake_message), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&remote_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*stateless_writer_, new_change(_,_,_)).Times(1).
        WillOnce(Return(nullptr));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(&handshake_handle_,_)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(*stateless_reader_->history_, remove_change_mock(change)).Times(1).
        WillOnce(Return(true));

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);
}

TEST_F(SecurityTest, discovered_participant_process_message_process_handshake_reply_add_change_fail)
{
    request_process_ok();

    EXPECT_CALL(*stateless_writer_->history_, remove_change(SequenceNumber_t{0,1})).Times(1).
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
    CacheChange_t* change = new CacheChange_t(static_cast<uint32_t>(ParticipantGenericMessageHelper::serialized_size(message))
            + 4 /*encapsulation*/);
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

    EXPECT_CALL(*auth_plugin_, process_handshake_rvr(_,_, Ref(handshake_handle_),_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(&handshake_message), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&remote_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*stateless_writer_, new_change(_,_,_)).Times(1).
        WillOnce(Return(change2));
    EXPECT_CALL(*stateless_writer_->history_, add_change_mock(change2)).Times(1).
        WillOnce(Return(false));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(&handshake_handle_,_)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(*stateless_reader_->history_, remove_change_mock(change)).Times(1).
        WillOnce(Return(true));

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);

    manager_.destroy();

    delete change2;
}

TEST_F(SecurityTest, discovered_participant_process_message_process_handshake_reply_ok_with_final_message)
{
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&remote_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(&handshake_handle_,_)).Times(1).
        WillOnce(Return(true));

    final_message_process_ok();
}

TEST_F(SecurityTest, discovered_participant_process_message_process_handshake_reply_ok_with_final_message_resent)
{
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&remote_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(&handshake_handle_,_)).Times(1).
        WillOnce(Return(true));

    CacheChange_t* final_message_change = nullptr;
    final_message_process_ok(&final_message_change);

    GUID_t remote_participant_key(participant_data_.m_guid);

    ParticipantGenericMessage message;
    message.message_identity().source_guid(remote_participant_key);
    message.related_message_identity().source_guid(remote_participant_key);
    message.related_message_identity().sequence_number(1);
    message.destination_participant_key(remote_participant_key);
    message.message_class_id("dds.sec.auth");
    HandshakeMessageToken token;
    message.message_data().push_back(token);
    CacheChange_t* change = new CacheChange_t(static_cast<uint32_t>(ParticipantGenericMessageHelper::serialized_size(message))
            + 4 /*encapsulation*/);
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

    EXPECT_CALL(*stateless_writer_->history_, remove_change_and_reuse(final_message_change->sequenceNumber)).Times(1).
        WillOnce(Return(final_message_change));
    EXPECT_CALL(*stateless_writer_->history_, add_change_mock(final_message_change)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(*stateless_reader_->history_, remove_change_mock(change)).Times(1).
        WillOnce(Return(true));

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);

    manager_.destroy();

    delete final_message_change;
}

TEST_F(SecurityTest, discovered_participant_process_message_bad_related_guid)
{
    reply_process_ok();

    GUID_t remote_participant_key(participant_data_.m_guid);
    remote_participant_key.guidPrefix.value[0] = 0xFF;

    ParticipantGenericMessage message;
    message.message_identity().source_guid(remote_participant_key);
    message.related_message_identity().source_guid(remote_participant_key);
    message.related_message_identity().sequence_number(1);
    message.destination_participant_key(remote_participant_key);
    message.message_class_id("dds.sec.auth");
    HandshakeMessageToken token;
    message.message_data().push_back(token);
    CacheChange_t* change = new CacheChange_t(static_cast<uint32_t>(ParticipantGenericMessageHelper::serialized_size(message))
            + 4 /*encapsulation*/);
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

    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&remote_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(&handshake_handle_,_)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(*stateless_reader_->history_, remove_change_mock(change)).Times(1).
        WillOnce(Return(true));

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);
}

TEST_F(SecurityTest, discovered_participant_process_message_bad_related_sequence_number)
{
    reply_process_ok();

    GUID_t remote_participant_key(participant_data_.m_guid);

    ParticipantGenericMessage message;
    message.message_identity().source_guid(remote_participant_key);
    message.related_message_identity().source_guid(remote_participant_key);
    message.related_message_identity().sequence_number(10);
    message.destination_participant_key(remote_participant_key);
    message.message_class_id("dds.sec.auth");
    HandshakeMessageToken token;
    message.message_data().push_back(token);
    CacheChange_t* change = new CacheChange_t(static_cast<uint32_t>(ParticipantGenericMessageHelper::serialized_size(message))
            + 4 /*encapsulation*/);
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

    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&remote_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(&handshake_handle_,_)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(*stateless_reader_->history_, remove_change_mock(change)).Times(1).
        WillOnce(Return(true));

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);
}

TEST_F(SecurityTest, discovered_participant_process_message_fail_process_handshake_final)
{
    reply_process_ok();

    GUID_t remote_participant_key(participant_data_.m_guid);

    ParticipantGenericMessage message;
    message.message_identity().source_guid(remote_participant_key);
    message.related_message_identity().source_guid(remote_participant_key);
    message.related_message_identity().sequence_number(1);
    message.destination_participant_key(remote_participant_key);
    message.message_class_id("dds.sec.auth");
    HandshakeMessageToken token;
    message.message_data().push_back(token);
    CacheChange_t* change = new CacheChange_t(static_cast<uint32_t>(ParticipantGenericMessageHelper::serialized_size(message))
            + 4 /*encapsulation*/);
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

    EXPECT_CALL(*auth_plugin_, process_handshake_rvr(_,_, Ref(handshake_handle_),_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_FAILED));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&remote_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(&handshake_handle_,_)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(*stateless_reader_->history_, remove_change_mock(change)).Times(1).
        WillOnce(Return(true));
    ParticipantAuthenticationInfo info;
    info.status = ParticipantAuthenticationInfo::UNAUTHORIZED_PARTICIPANT;
    info.guid = remote_participant_key;
    EXPECT_CALL(*participant_.getListener(), onParticipantAuthentication(_, info)).Times(1);

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);
}

TEST_F(SecurityTest, discovered_participant_process_message_ok_process_handshake_final)
{
    reply_process_ok();

    EXPECT_CALL(*stateless_writer_->history_, remove_change(SequenceNumber_t{0,1})).Times(1).
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
    CacheChange_t* change = new CacheChange_t(static_cast<uint32_t>(ParticipantGenericMessageHelper::serialized_size(message))
            + 4 /*encapsulation*/);
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

    MockSharedSecretHandle shared_secret_handle;
    MockParticipantCryptoHandle participant_crypto_handle;

    EXPECT_CALL(*auth_plugin_, process_handshake_rvr(_,_, Ref(handshake_handle_),_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&remote_identity_handle_,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(&handshake_handle_,_)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(*stateless_reader_->history_, remove_change_mock(change)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(participant_, pdpsimple()).Times(1).WillOnce(Return(&pdpsimple_));
    EXPECT_CALL(pdpsimple_, notifyAboveRemoteEndpoints(_)).Times(1);
    EXPECT_CALL(*auth_plugin_, get_shared_secret(Ref(handshake_handle_),_)).Times(1).
        WillOnce(Return(&shared_secret_handle));
    EXPECT_CALL(*auth_plugin_, return_sharedsecret_handle(&shared_secret_handle,_)).Times(1).
        WillRepeatedly(Return(true));
    EXPECT_CALL(crypto_plugin_->cryptokeyfactory_, register_matched_remote_participant(Ref(local_participant_crypto_handle_),
                Ref(remote_identity_handle_),_,Ref(shared_secret_handle),_)).Times(1).
        WillOnce(Return(&participant_crypto_handle));
    EXPECT_CALL(crypto_plugin_->cryptokeyexchange_, create_local_participant_crypto_tokens(_,
                Ref(local_participant_crypto_handle_), Ref(participant_crypto_handle),_)).Times(1).
           WillOnce(Return(true));
    EXPECT_CALL(crypto_plugin_->cryptokeyfactory_, unregister_participant(&participant_crypto_handle,_)).Times(1).
        WillOnce(Return(true));

    ParticipantAuthenticationInfo info;
    info.status = ParticipantAuthenticationInfo::AUTHORIZED_PARTICIPANT;
    info.guid = remote_participant_key;
    EXPECT_CALL(*participant_.getListener(), onParticipantAuthentication(_, info)).Times(1);

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);
}

int main(int argc, char **argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
