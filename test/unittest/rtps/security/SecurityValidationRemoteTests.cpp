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

#include <rtps/builtin/data/ParticipantProxyData.hpp>

TEST_F(SecurityTest, discovered_participant_validation_remote_identity_fail)
{
    initialization_ok();

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_, Ref(local_identity_handle_), _, _, _)).Times(1).
            WillOnce(Return(ValidationResult_t::VALIDATION_FAILED));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_, _)).Times(1).
            WillOnce(Return(true));

    ParticipantProxyData participant_data;
    fill_participant_key(participant_data.guid);
    ParticipantAuthenticationInfo info;
    info.status = ParticipantAuthenticationInfo::UNAUTHORIZED_PARTICIPANT;
    info.guid = participant_data.guid;
    EXPECT_CALL(*participant_.getListener(), onParticipantAuthentication(_, info)).Times(1);

    ASSERT_FALSE(manager_.discovered_participant(participant_data));
}

TEST_F(SecurityTest, discovered_participant_validation_remote_identity_ok)
{
    initialization_auth_ok();

    auto& remote_identity_handle = get_handle<MockIdentityHandle>();
    ParticipantProxyData participant_data;
    fill_participant_key(participant_data.guid);

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_, Ref(local_identity_handle_), _, _, _)).Times(1).
            WillOnce(DoAll(SetArgPointee<0>(&remote_identity_handle), Return(ValidationResult_t::VALIDATION_OK)));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_, _)).Times(1).
            WillOnce(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&remote_identity_handle, _)).Times(1).
            WillOnce(Return(true));
    EXPECT_CALL(participant_, pdp()).Times(1).WillOnce(Return(&pdp_));
    EXPECT_CALL(pdp_, notifyAboveRemoteEndpoints(_, true)).Times(1);

    ParticipantAuthenticationInfo info;
    info.status = ParticipantAuthenticationInfo::AUTHORIZED_PARTICIPANT;
    info.guid = participant_data.guid;
    EXPECT_CALL(*participant_.getListener(), onParticipantAuthentication(_, info)).Times(1);

    ASSERT_TRUE(manager_.discovered_participant(participant_data));

    return_handle(remote_identity_handle);
}

TEST_F(SecurityTest, discovered_participant_validation_remote_identity_pending_handshake_message)
{
    initialization_ok();

    auto& remote_identity_handle = get_handle<MockIdentityHandle>();

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_, Ref(local_identity_handle_), _, _, _)).Times(1).
            WillOnce(DoAll(SetArgPointee<0>(&remote_identity_handle),
            Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_, _)).Times(1).
            WillOnce(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&remote_identity_handle, _)).Times(1).
            WillOnce(Return(true));

    ParticipantProxyData participant_data;
    fill_participant_key(participant_data.guid);
    ASSERT_TRUE(manager_.discovered_participant(participant_data));

    return_handle(remote_identity_handle);
}

TEST_F(SecurityTest, discovered_participant_validation_remote_identity_pending_handshake_request_fail)
{
    initialization_ok();

    auto& remote_identity_handle = get_handle<MockIdentityHandle>();

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_, Ref(local_identity_handle_), _, _, _)).Times(1).
            WillOnce(DoAll(SetArgPointee<0>(&remote_identity_handle),
            Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_REQUEST)));
    EXPECT_CALL(*auth_plugin_, begin_handshake_request(_, _, Ref(local_identity_handle_),
            Ref(remote_identity_handle), _, _)).Times(1).
            WillOnce(Return(ValidationResult_t::VALIDATION_FAILED));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_, _)).Times(1).
            WillOnce(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&remote_identity_handle, _)).Times(1).
            WillOnce(Return(true));
    EXPECT_CALL(participant_, pdp()).Times(1).WillOnce(Return(&pdp_));
    EXPECT_CALL(pdp_, get_participant_proxy_data_serialized(BIGEND)).Times(1);

    ParticipantProxyData participant_data;
    fill_participant_key(participant_data.guid);
    ParticipantAuthenticationInfo info;
    info.status = ParticipantAuthenticationInfo::UNAUTHORIZED_PARTICIPANT;
    info.guid = participant_data.guid;
    EXPECT_CALL(*participant_.getListener(), onParticipantAuthentication(_, info)).Times(1);

    ASSERT_FALSE(manager_.discovered_participant(participant_data));

    return_handle(remote_identity_handle);
}

TEST_F(SecurityTest, discovered_participant_validation_remote_identity_pending_handshake_request_ok)
{
    initialization_ok();

    auto& remote_identity_handle = get_handle<MockIdentityHandle>();
    auto& handshake_handle = get_handle<MockHandshakeHandle>();
    auto shared_secret_handle = get_sh_ptr<MockSharedSecretHandle>();
    auto participant_crypto_handle = get_sh_ptr<MockParticipantCryptoHandle>();
    ParticipantProxyData participant_data;
    fill_participant_key(participant_data.guid);

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_, Ref(local_identity_handle_), _, _, _)).Times(1).
            WillOnce(DoAll(SetArgPointee<0>(&remote_identity_handle),
            Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_REQUEST)));
    EXPECT_CALL(*auth_plugin_, begin_handshake_request(_, _, Ref(local_identity_handle_),
            Ref(remote_identity_handle), _, _)).Times(1).
            WillOnce(DoAll(SetArgPointee<0>(&handshake_handle),
            Return(ValidationResult_t::VALIDATION_OK)));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_, _)).Times(1).
            WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&remote_identity_handle, _)).Times(1).
            WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(&handshake_handle, _)).Times(1).
            WillRepeatedly(Return(true));
    EXPECT_CALL(participant_, pdp()).Times(2).WillRepeatedly(Return(&pdp_));
    EXPECT_CALL(pdp_, notifyAboveRemoteEndpoints(_, true)).Times(1);
    EXPECT_CALL(pdp_, get_participant_proxy_data_serialized(BIGEND)).Times(1);
    EXPECT_CALL(*auth_plugin_, get_shared_secret(Ref(handshake_handle), _)).Times(1).
            WillOnce(Return(shared_secret_handle));
    EXPECT_CALL(*auth_plugin_, return_sharedsecret_handle(shared_secret_handle, _)).Times(1).
            WillRepeatedly(Return(true));
    EXPECT_CALL(crypto_plugin_->cryptokeyfactory_,
            register_matched_remote_participant(Ref(*local_participant_crypto_handle_),
            Ref(remote_identity_handle), _, Ref(*shared_secret_handle), _)).Times(1).
            WillOnce(Return(participant_crypto_handle));
    EXPECT_CALL(crypto_plugin_->cryptokeyexchange_, create_local_participant_crypto_tokens(_,
            Ref(*local_participant_crypto_handle_), Ref(*participant_crypto_handle), _)).Times(1).
            WillOnce(Return(true));
    EXPECT_CALL(crypto_plugin_->cryptokeyfactory_, unregister_participant(participant_crypto_handle, _)).Times(1).
            WillOnce(Return(true));

    ParticipantAuthenticationInfo info;
    info.status = ParticipantAuthenticationInfo::AUTHORIZED_PARTICIPANT;
    info.guid = participant_data.guid;
    EXPECT_CALL(*participant_.getListener(), onParticipantAuthentication(_, info)).Times(1);

    CacheChange_t kx_change_to_add;
    CacheChange_t* kx_change_to_remove = new CacheChange_t(500);
    expect_kx_exchange(kx_change_to_add, kx_change_to_remove);

    ASSERT_TRUE(manager_.discovered_participant(participant_data));

    volatile_writer_->listener_->on_writer_change_received_by_all(volatile_writer_, kx_change_to_remove);

    return_handle(remote_identity_handle);
    return_handle(handshake_handle);
}

TEST_F(SecurityTest, discovered_participant_validation_remote_identity_new_change_fail)
{
    initialization_ok();

    auto& remote_identity_handle = get_handle<MockIdentityHandle>();
    auto& handshake_handle = get_handle<MockHandshakeHandle>();
    HandshakeMessageToken handshake_message;

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_, Ref(local_identity_handle_), _, _, _)).Times(1).
            WillOnce(DoAll(SetArgPointee<0>(&remote_identity_handle),
            Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_REQUEST)));
    EXPECT_CALL(*auth_plugin_, begin_handshake_request(_, _, Ref(local_identity_handle_),
            Ref(remote_identity_handle), _, _)).Times(1).
            WillOnce(DoAll(SetArgPointee<0>(&handshake_handle),
            SetArgPointee<1>(&handshake_message), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));
    EXPECT_CALL(*stateless_writer_->history_, create_change(_, _, _)).Times(1).
            WillOnce(Return(nullptr));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_, _)).Times(1).
            WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&remote_identity_handle, _)).Times(1).
            WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(&handshake_handle, _)).Times(1).
            WillOnce(Return(true));
    EXPECT_CALL(participant_, pdp()).Times(1).WillOnce(Return(&pdp_));
    EXPECT_CALL(pdp_, get_participant_proxy_data_serialized(BIGEND)).Times(1);

    ParticipantProxyData participant_data;
    fill_participant_key(participant_data.guid);
    ASSERT_FALSE(manager_.discovered_participant(participant_data));

    return_handle(remote_identity_handle);
    return_handle(handshake_handle);
}

TEST_F(SecurityTest, discovered_participant_validation_remote_identity_add_change_fail)
{
    initialization_ok();

    auto& remote_identity_handle = get_handle<MockIdentityHandle>();
    auto& handshake_handle = get_handle<MockHandshakeHandle>();
    HandshakeMessageToken handshake_message;
    CacheChange_t* change = new CacheChange_t(200);

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_, Ref(local_identity_handle_), _, _, _)).Times(1).
            WillOnce(DoAll(SetArgPointee<0>(&remote_identity_handle),
            Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_REQUEST)));
    EXPECT_CALL(*auth_plugin_, begin_handshake_request(_, _, Ref(local_identity_handle_),
            Ref(remote_identity_handle), _, _)).Times(1).
            WillOnce(DoAll(SetArgPointee<0>(&handshake_handle),
            SetArgPointee<1>(&handshake_message), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));
    EXPECT_CALL(*stateless_writer_->history_, create_change(_, _, _)).Times(1).
            WillOnce(Return(change));
    EXPECT_CALL(*stateless_writer_->history_, add_change_mock(change)).Times(1).
            WillOnce(Return(false));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_, _)).Times(1).
            WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&remote_identity_handle, _)).Times(1).
            WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(&handshake_handle, _)).Times(1).
            WillOnce(Return(true));
    EXPECT_CALL(participant_, pdp()).Times(1).WillOnce(Return(&pdp_));
    EXPECT_CALL(pdp_, get_participant_proxy_data_serialized(BIGEND)).Times(1);

    ParticipantProxyData participant_data;
    fill_participant_key(participant_data.guid);
    ASSERT_FALSE(manager_.discovered_participant(participant_data));

    destroy_manager_and_change(change, false);

    return_handle(remote_identity_handle);
    return_handle(handshake_handle);
}

TEST_F(SecurityTest, discovered_participant_validation_remote_identity_pending_handshake_request_pending_message)
{
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_, _)).Times(1).
            WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&remote_identity_handle_, _)).Times(1).
            WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(&handshake_handle_, _)).Times(1).
            WillOnce(Return(true));

    request_process_ok();

    EXPECT_CALL(*stateless_writer_->history_, remove_change(SequenceNumber_t{ 0, 1 })).Times(1).
            WillOnce(Return(true));
}

TEST_F(SecurityTest, discovered_participant_validation_remote_identity_pending_handshake_request_pending_message_resent)
{
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_, _)).Times(1).
            WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&remote_identity_handle_, _)).Times(1).
            WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(&handshake_handle_, _)).Times(1).
            WillOnce(Return(true));

    CacheChange_t* request_message_change = nullptr;
    request_process_ok(&request_message_change);

    EXPECT_CALL(*stateless_writer_->history_, remove_change_and_reuse(request_message_change->sequenceNumber)).Times(1).
            WillOnce(Return(request_message_change));
    EXPECT_CALL(*stateless_writer_->history_, add_change_mock(request_message_change)).Times(1).
            WillOnce(Return(true));
    stateless_writer_->history_->wait_for_more_samples_than(1);

    destroy_manager_and_change(request_message_change);
}

TEST_F(SecurityTest, discovered_participant_validation_remote_identity_pending_handshake_request_ok_with_final_message)
{
    initialization_ok();

    auto& remote_identity_handle = get_handle<MockIdentityHandle>();
    auto& handshake_handle = get_handle<MockHandshakeHandle>();
    HandshakeMessageToken handshake_message;
    CacheChange_t* change = new CacheChange_t(200);
    auto shared_secret_handle = get_sh_ptr<MockSharedSecretHandle>();
    auto participant_crypto_handle = get_sh_ptr<MockParticipantCryptoHandle>();
    ParticipantProxyData participant_data;
    fill_participant_key(participant_data.guid);

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_, Ref(local_identity_handle_), _, _, _)).Times(1).
            WillOnce(DoAll(SetArgPointee<0>(&remote_identity_handle),
            Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_REQUEST)));
    EXPECT_CALL(*auth_plugin_, begin_handshake_request(_, _, Ref(local_identity_handle_),
            Ref(remote_identity_handle), _, _)).Times(1).
            WillOnce(DoAll(SetArgPointee<0>(&handshake_handle),
            SetArgPointee<1>(&handshake_message), Return(ValidationResult_t::VALIDATION_OK_WITH_FINAL_MESSAGE)));
    EXPECT_CALL(*stateless_writer_->history_, create_change(_, _, _)).Times(1).
            WillOnce(Return(change));
    EXPECT_CALL(*stateless_writer_->history_, add_change_mock(change)).Times(1).
            WillOnce(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_, _)).Times(1).
            WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&remote_identity_handle, _)).Times(1).
            WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(&handshake_handle, _)).Times(1).
            WillOnce(Return(true));
    EXPECT_CALL(participant_, pdp()).Times(2).WillRepeatedly(Return(&pdp_));
    EXPECT_CALL(pdp_, notifyAboveRemoteEndpoints(_, true)).Times(1);
    EXPECT_CALL(pdp_, get_participant_proxy_data_serialized(BIGEND)).Times(1);
    EXPECT_CALL(*auth_plugin_, get_shared_secret(Ref(handshake_handle), _)).Times(1).
            WillOnce(Return(shared_secret_handle));
    EXPECT_CALL(*auth_plugin_, return_sharedsecret_handle(shared_secret_handle, _)).Times(1).
            WillRepeatedly(Return(true));
    EXPECT_CALL(crypto_plugin_->cryptokeyfactory_,
            register_matched_remote_participant(Ref(*local_participant_crypto_handle_),
            Ref(remote_identity_handle), _, Ref(*shared_secret_handle), _)).Times(1).
            WillOnce(Return(participant_crypto_handle));
    EXPECT_CALL(crypto_plugin_->cryptokeyexchange_, create_local_participant_crypto_tokens(_,
            Ref(*local_participant_crypto_handle_), Ref(*participant_crypto_handle), _)).Times(1).
            WillOnce(Return(true));
    EXPECT_CALL(crypto_plugin_->cryptokeyfactory_, unregister_participant(participant_crypto_handle, _)).Times(1).
            WillOnce(Return(true));

    ParticipantAuthenticationInfo info;
    info.status = ParticipantAuthenticationInfo::AUTHORIZED_PARTICIPANT;
    info.guid = participant_data.guid;
    EXPECT_CALL(*participant_.getListener(), onParticipantAuthentication(_, info)).Times(1);

    CacheChange_t kx_change_to_add;
    CacheChange_t* kx_change_to_remove = new CacheChange_t(500);
    expect_kx_exchange(kx_change_to_add, kx_change_to_remove);

    ASSERT_TRUE(manager_.discovered_participant(participant_data));

    volatile_writer_->listener_->on_writer_change_received_by_all(volatile_writer_, kx_change_to_remove);

    destroy_manager_and_change(change);

    return_handle(remote_identity_handle);
    return_handle(handshake_handle);
}

TEST_F(SecurityTest, discovered_participant_ok)
{
    initialization_ok();

    auto& remote_identity_handle = get_handle<MockIdentityHandle>();
    auto& handshake_handle = get_handle<MockHandshakeHandle>();
    HandshakeMessageToken handshake_message;
    CacheChange_t* change = new CacheChange_t(200);

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_, Ref(local_identity_handle_), _, _, _)).Times(1).
            WillOnce(DoAll(SetArgPointee<0>(&remote_identity_handle),
            Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_REQUEST)));
    EXPECT_CALL(*auth_plugin_, begin_handshake_request(_, _, Ref(local_identity_handle_),
            Ref(remote_identity_handle), _, _)).Times(1).
            WillOnce(DoAll(SetArgPointee<0>(&handshake_handle),
            SetArgPointee<1>(&handshake_message), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));
    EXPECT_CALL(*stateless_writer_->history_, create_change(_, _, _)).Times(1).
            WillOnce(Return(change));
    EXPECT_CALL(*stateless_writer_->history_, add_change_mock(change)).Times(1).
            WillOnce(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_, _)).Times(1).
            WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&remote_identity_handle, _)).Times(1).
            WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(&handshake_handle, _)).Times(1).
            WillOnce(Return(true));
    EXPECT_CALL(participant_, pdp()).Times(1).WillOnce(Return(&pdp_));
    EXPECT_CALL(pdp_, get_participant_proxy_data_serialized(BIGEND)).Times(1);

    ParticipantProxyData participant_data;
    fill_participant_key(participant_data.guid);
    ASSERT_TRUE(manager_.discovered_participant(participant_data));

    destroy_manager_and_change(change);

    return_handle(remote_identity_handle);
    return_handle(handshake_handle);
}

TEST_F(SecurityTest, discovered_participant_validate_remote_fail_and_then_ok)
{
    initialization_ok();

    auto& remote_identity_handle = get_handle<MockIdentityHandle>();
    auto& handshake_handle = get_handle<MockHandshakeHandle>();
    HandshakeMessageToken handshake_message;
    CacheChange_t* change = new CacheChange_t(200);

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_, Ref(local_identity_handle_), _, _, _)).Times(1).
            WillOnce(Return(ValidationResult_t::VALIDATION_FAILED));

    ParticipantProxyData participant_data;
    fill_participant_key(participant_data.guid);
    ParticipantAuthenticationInfo info;
    info.status = ParticipantAuthenticationInfo::UNAUTHORIZED_PARTICIPANT;
    info.guid = participant_data.guid;
    EXPECT_CALL(*participant_.getListener(), onParticipantAuthentication(_, info)).Times(1);

    ASSERT_FALSE(manager_.discovered_participant(participant_data));

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_, Ref(local_identity_handle_), _, _, _)).Times(1).
            WillOnce(DoAll(SetArgPointee<0>(&remote_identity_handle),
            Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_REQUEST)));
    EXPECT_CALL(*auth_plugin_, begin_handshake_request(_, _, Ref(local_identity_handle_),
            Ref(remote_identity_handle), _, _)).Times(1).
            WillOnce(DoAll(SetArgPointee<0>(&handshake_handle),
            SetArgPointee<1>(&handshake_message), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));
    EXPECT_CALL(*stateless_writer_->history_, create_change(_, _, _)).Times(1).
            WillOnce(Return(change));
    EXPECT_CALL(*stateless_writer_->history_, add_change_mock(change)).Times(1).
            WillOnce(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&local_identity_handle_, _)).Times(1).
            WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(&remote_identity_handle, _)).Times(1).
            WillRepeatedly(Return(true));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(&handshake_handle, _)).Times(1).
            WillOnce(Return(true));
    EXPECT_CALL(participant_, pdp()).Times(1).WillOnce(Return(&pdp_));
    EXPECT_CALL(pdp_, get_participant_proxy_data_serialized(BIGEND)).Times(1);

    ASSERT_TRUE(manager_.discovered_participant(participant_data));

    destroy_manager_and_change(change);

    return_handle(remote_identity_handle);
    return_handle(handshake_handle);
}
