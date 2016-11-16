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

#include <fastrtps/rtps/security/common/Handle.h>
#include <MockAuthenticationPlugin.h>
#include <rtps/participant/RTPSParticipantImpl.h>
#include <fastrtps/rtps/writer/StatelessWriter.h>
#include <fastrtps/rtps/history/WriterHistory.h>
#include <fastrtps/rtps/reader/StatelessReader.h>
#include <fastrtps/rtps/history/ReaderHistory.h>
#include <SecurityPluginFactory.h>
#include <SecurityManager.h>

#include <gtest/gtest.h>

using namespace eprosima::fastrtps::rtps;
using namespace ::security;
using namespace ::testing;

using ::testing::DefaultValue;

class MockIdentity
{
    public:

        static const char* const class_id_;
};

const char* const MockIdentity::class_id_ = "MockIdentityHandle";

typedef HandleImpl<MockIdentity> MockIdentityHandle;

class MockHandshake
{
    public:

        static const char* const class_id_;
};

const char* const MockHandshake::class_id_ = "MockHandshakeHandle";

typedef HandleImpl<MockHandshake> MockHandshakeHandle;

// Default Values
RTPSParticipantAttributes pattr;
GUID_t guid;

void fill_participant_key(GUID_t& participant_key)
{
    participant_key.guidPrefix.value[0] = 1;
    participant_key.guidPrefix.value[1] = 2;
    participant_key.guidPrefix.value[2] = 3;
    participant_key.guidPrefix.value[3] = 4;
    participant_key.guidPrefix.value[4] = 5;
    participant_key.guidPrefix.value[5] = 6;
    participant_key.guidPrefix.value[6] = 7;
    participant_key.guidPrefix.value[7] = 8;
    participant_key.guidPrefix.value[8] = 9;
    participant_key.guidPrefix.value[9] = 10;
    participant_key.guidPrefix.value[10] = 11;
    participant_key.guidPrefix.value[11] = 12;
    participant_key.entityId.value[0] = 13;
    participant_key.entityId.value[1] = 14;
    participant_key.entityId.value[2] = 15;
    participant_key.entityId.value[3] = 16;
}

class SecurityAuthenticationTest : public ::testing::Test
{
    protected:

        virtual void SetUp()
        {
            SecurityPluginFactory::set_auth_plugin(auth_plugin_);
            fill_participant_key(guid);
        }

        virtual void TearDown()
        {
            SecurityPluginFactory::release_auth_plugin();
        }

        void initialization_ok()
        {
            DefaultValue<const RTPSParticipantAttributes&>::Set(pattr);
            DefaultValue<const GUID_t&>::Set(guid);
            stateless_writer_ = new NiceMock<StatelessWriter>();
            stateless_reader_ = new NiceMock<StatelessReader>();
            MockIdentityHandle identity_handle;
            MockIdentityHandle* p_identity_handle = &identity_handle;

            EXPECT_CALL(*auth_plugin_, validate_local_identity(_,_,_,_,_,_)).Times(1).
                WillOnce(DoAll(SetArgPointee<0>(p_identity_handle), Return(ValidationResult_t::VALIDATION_OK)));
            EXPECT_CALL(participant_, createWriter_mock(_,_,_,_,_,_)).Times(1).
                WillOnce(DoAll(SetArgPointee<0>(stateless_writer_), Return(true)));
            EXPECT_CALL(participant_, createReader_mock(_,_,_,_,_,_,_)).Times(1).
                WillOnce(DoAll(SetArgPointee<0>(stateless_reader_), Return(true)));

            ASSERT_TRUE(manager_.init());
        }

        void request_process_ok()
        {
            initialization_ok();

            MockIdentityHandle identity_handle;
            MockIdentityHandle* p_identity_handle = &identity_handle;
            MockHandshakeHandle handshake_handle;
            MockHandshakeHandle* p_handshake_handle = &handshake_handle;
            HandshakeMessageToken* p_handshake_message = new HandshakeMessageToken();
            CacheChange_t* change = new CacheChange_t(200);

            EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_,_,_,_,_)).Times(1).
                WillOnce(DoAll(SetArgPointee<0>(p_identity_handle), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_REQUEST)));
            EXPECT_CALL(*auth_plugin_, begin_handshake_request(_,_,_,_,_)).Times(1).
                WillOnce(DoAll(SetArgPointee<0>(p_handshake_handle), 
                            SetArgPointee<1>(p_handshake_message), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));
            EXPECT_CALL(*stateless_writer_, new_change(_,_,_)).Times(1).
                WillOnce(Return(change));
            EXPECT_CALL(*stateless_writer_->history_, add_change(change)).Times(1).
                WillOnce(Return(true));

            GUID_t remote_participant_key;
            fill_participant_key(remote_participant_key);
            ASSERT_TRUE(manager_.discovered_participant(IdentityToken(), remote_participant_key));

            delete change;
        }

        void reply_process_ok()
        {
            initialization_ok();

            MockIdentityHandle identity_handle;
            MockIdentityHandle* p_identity_handle = &identity_handle;

            EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_,_,_,_,_)).Times(1).
                WillOnce(DoAll(SetArgPointee<0>(p_identity_handle), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));

            GUID_t remote_participant_key;
            fill_participant_key(remote_participant_key);
            ASSERT_TRUE(manager_.discovered_participant(IdentityToken(), remote_participant_key));

            ParticipantGenericMessage message;
            message.message_identity().source_guid(remote_participant_key);
            message.destination_participant_key(remote_participant_key);
            message.message_class_id("dds.sec.auth");
            HandshakeMessageToken token;
            message.message_data().push_back(token);
            CacheChange_t* change = new CacheChange_t(ParticipantGenericMessageHelper::serialized_size(message));
            CDRMessage_t aux_msg(0);
            aux_msg.wraps = true;
            aux_msg.buffer = change->serializedPayload.data;
            aux_msg.max_size = change->serializedPayload.max_size;
            aux_msg.msg_endian = change->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND : LITTLEEND;
            ASSERT_TRUE(CDRMessage::addParticipantGenericMessage(&aux_msg, message));
            change->serializedPayload.length = aux_msg.length;

            MockHandshakeHandle handshake_handle;
            MockHandshakeHandle* p_handshake_handle = &handshake_handle;
            HandshakeMessageToken* p_handshake_message = new HandshakeMessageToken();
            CacheChange_t* change2 = new CacheChange_t(200);

            EXPECT_CALL(*auth_plugin_, begin_handshake_reply_rvr(_,_,_,_,_,_)).Times(1).
                WillOnce(DoAll(SetArgPointee<0>(p_handshake_handle), 
                            SetArgPointee<1>(p_handshake_message), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));
            EXPECT_CALL(*stateless_writer_, new_change(_,_,_)).Times(1).
                WillOnce(Return(change2));
            EXPECT_CALL(*stateless_writer_->history_, add_change(change2)).Times(1).
                WillOnce(Return(true));
            EXPECT_CALL(*stateless_reader_->history_, remove_change(change)).Times(1).
                WillOnce(Return(true));

            stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);

            delete change2;
        }

    public:

        SecurityAuthenticationTest() : auth_plugin_(new MockAuthenticationPlugin()),
        stateless_writer_(nullptr), stateless_reader_(nullptr), manager_(&participant_) {}

        ~SecurityAuthenticationTest()
        {
        }

        MockAuthenticationPlugin* auth_plugin_;
        NiceMock<RTPSParticipantImpl> participant_;
        NiceMock<StatelessWriter>* stateless_writer_;
        NiceMock<StatelessReader>* stateless_reader_;
        SecurityManager manager_;
};

TEST_F(SecurityAuthenticationTest, initialization_auth_nullptr)
{
    SecurityPluginFactory::release_auth_plugin();
    DefaultValue<const RTPSParticipantAttributes&>::Set(pattr);
    DefaultValue<const GUID_t&>::Set(guid);

    ASSERT_FALSE(manager_.init());
}

TEST_F(SecurityAuthenticationTest, initialization_auth_failed)
{
    DefaultValue<const RTPSParticipantAttributes&>::Set(pattr);
    DefaultValue<const GUID_t&>::Set(guid);

    EXPECT_CALL(*auth_plugin_, validate_local_identity(_,_,_,_,_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_FAILED));

    ASSERT_FALSE(manager_.init());
}

TEST_F(SecurityAuthenticationTest, initialization_fail_participant_stateless_message_writer)
{
    DefaultValue<const RTPSParticipantAttributes&>::Set(pattr);
    DefaultValue<const GUID_t&>::Set(guid);
    MockIdentityHandle identity_handle;
    MockIdentityHandle* p_identity_handle = &identity_handle;

    EXPECT_CALL(*auth_plugin_, validate_local_identity(_,_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_identity_handle), Return(ValidationResult_t::VALIDATION_OK)));
    EXPECT_CALL(participant_, createWriter_mock(_,_,_,_,_,_)).Times(1).
        WillOnce(Return(false));

    ASSERT_FALSE(manager_.init());
}

TEST_F(SecurityAuthenticationTest, initialization_fail_participant_stateless_message_reader)
{
    DefaultValue<const RTPSParticipantAttributes&>::Set(pattr);
    DefaultValue<const GUID_t&>::Set(guid);
    NiceMock<StatelessWriter>* stateless_writer = new NiceMock<StatelessWriter>();
    MockIdentityHandle identity_handle;
    MockIdentityHandle* p_identity_handle = &identity_handle;

    EXPECT_CALL(*auth_plugin_, validate_local_identity(_,_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_identity_handle), Return(ValidationResult_t::VALIDATION_OK)));
    EXPECT_CALL(participant_, createWriter_mock(_,_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(stateless_writer), Return(true)));
    EXPECT_CALL(participant_, createReader_mock(_,_,_,_,_,_,_)).Times(1).
        WillOnce(Return(false));

    ASSERT_FALSE(manager_.init());
}

TEST_F(SecurityAuthenticationTest, initialization_auth_retry)
{
    DefaultValue<const RTPSParticipantAttributes&>::Set(pattr);
    DefaultValue<const GUID_t&>::Set(guid);
    NiceMock<StatelessWriter>* stateless_writer = new NiceMock<StatelessWriter>();
    NiceMock<StatelessReader>* stateless_reader = new NiceMock<StatelessReader>();
    MockIdentityHandle identity_handle;
    MockIdentityHandle* p_identity_handle = &identity_handle;

    EXPECT_CALL(*auth_plugin_, validate_local_identity(_,_,_,_,_,_)).Times(2).
        WillOnce(Return(ValidationResult_t::VALIDATION_PENDING_RETRY)).
        WillOnce(DoAll(SetArgPointee<0>(p_identity_handle), Return(ValidationResult_t::VALIDATION_OK)));
    EXPECT_CALL(participant_, createWriter_mock(_,_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(stateless_writer), Return(true)));
    EXPECT_CALL(participant_, createReader_mock(_,_,_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(stateless_reader), Return(true)));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_OK));

    ASSERT_TRUE(manager_.init());
}


TEST_F(SecurityAuthenticationTest, initialization_ok)
{
    EXPECT_CALL(*auth_plugin_, return_identity_handle(_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_OK));

    initialization_ok();
}

TEST_F(SecurityAuthenticationTest, discovered_participant_validation_remote_identity_fail)
{
    initialization_ok();

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_,_,_,_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_FAILED));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_OK));

    GUID_t remote_participant_key;
    fill_participant_key(remote_participant_key);
    ASSERT_FALSE(manager_.discovered_participant(IdentityToken(), remote_participant_key));
}

TEST_F(SecurityAuthenticationTest, discovered_participant_validation_remote_identity_ok)
{
    initialization_ok();

    MockIdentityHandle identity_handle;
    MockIdentityHandle* p_identity_handle = &identity_handle;

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_identity_handle), Return(ValidationResult_t::VALIDATION_OK)));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(_,_)).Times(2).
        WillRepeatedly(Return(ValidationResult_t::VALIDATION_OK));

    GUID_t remote_participant_key;
    fill_participant_key(remote_participant_key);
    ASSERT_TRUE(manager_.discovered_participant(IdentityToken(), remote_participant_key));
}

TEST_F(SecurityAuthenticationTest, discovered_participant_validation_remote_identity_pending_handshake_message)
{
    initialization_ok();

    MockIdentityHandle identity_handle;
    MockIdentityHandle* p_identity_handle = &identity_handle;

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_identity_handle), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(_,_)).Times(2).
        WillRepeatedly(Return(ValidationResult_t::VALIDATION_OK));

    GUID_t remote_participant_key;
    fill_participant_key(remote_participant_key);
    ASSERT_TRUE(manager_.discovered_participant(IdentityToken(), remote_participant_key));
}

TEST_F(SecurityAuthenticationTest, discovered_participant_validation_remote_identity_pending_handshake_request_fail)
{
    initialization_ok();

    MockIdentityHandle identity_handle;
    MockIdentityHandle* p_identity_handle = &identity_handle;

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_identity_handle), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_REQUEST)));
    EXPECT_CALL(*auth_plugin_, begin_handshake_request(_,_,_,_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_FAILED));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(_,_)).Times(2).
        WillRepeatedly(Return(ValidationResult_t::VALIDATION_OK));

    GUID_t remote_participant_key;
    fill_participant_key(remote_participant_key);
    ASSERT_FALSE(manager_.discovered_participant(IdentityToken(), remote_participant_key));
}

TEST_F(SecurityAuthenticationTest, discovered_participant_validation_remote_identity_pending_handshake_request_ok)
{
    initialization_ok();

    MockIdentityHandle identity_handle;
    MockIdentityHandle* p_identity_handle = &identity_handle;
    MockHandshakeHandle handshake_handle;
    MockHandshakeHandle* p_handshake_handle = &handshake_handle;

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_identity_handle), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_REQUEST)));
    EXPECT_CALL(*auth_plugin_, begin_handshake_request(_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_handshake_handle), 
                    Return(ValidationResult_t::VALIDATION_OK)));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(_,_)).Times(2).
        WillRepeatedly(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(_,_)).Times(1).
        WillRepeatedly(Return(ValidationResult_t::VALIDATION_OK));

    GUID_t remote_participant_key;
    fill_participant_key(remote_participant_key);
    ASSERT_TRUE(manager_.discovered_participant(IdentityToken(), remote_participant_key));
}

TEST_F(SecurityAuthenticationTest, discovered_participant_validation_remote_identity_new_change_fail)
{
    initialization_ok();

    MockIdentityHandle identity_handle;
    MockIdentityHandle* p_identity_handle = &identity_handle;
    MockHandshakeHandle handshake_handle;
    MockHandshakeHandle* p_handshake_handle = &handshake_handle;
    HandshakeMessageToken* p_handshake_message = new HandshakeMessageToken();

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_identity_handle), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_REQUEST)));
    EXPECT_CALL(*auth_plugin_, begin_handshake_request(_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_handshake_handle), 
                    SetArgPointee<1>(p_handshake_message), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));
    EXPECT_CALL(*stateless_writer_, new_change(_,_,_)).Times(1).
        WillOnce(Return(nullptr));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(_,_)).Times(2).
        WillRepeatedly(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_OK));

    GUID_t remote_participant_key;
    fill_participant_key(remote_participant_key);
    ASSERT_FALSE(manager_.discovered_participant(IdentityToken(), remote_participant_key));
}

TEST_F(SecurityAuthenticationTest, discovered_participant_validation_remote_identity_add_change_fail)
{
    initialization_ok();

    MockIdentityHandle identity_handle;
    MockIdentityHandle* p_identity_handle = &identity_handle;
    MockHandshakeHandle handshake_handle;
    MockHandshakeHandle* p_handshake_handle = &handshake_handle;
    HandshakeMessageToken* p_handshake_message = new HandshakeMessageToken();
    CacheChange_t* change = new CacheChange_t(200);

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_identity_handle), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_REQUEST)));
    EXPECT_CALL(*auth_plugin_, begin_handshake_request(_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_handshake_handle), 
                    SetArgPointee<1>(p_handshake_message), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));
    EXPECT_CALL(*stateless_writer_, new_change(_,_,_)).Times(1).
        WillOnce(Return(change));
    EXPECT_CALL(*stateless_writer_->history_, add_change(change)).Times(1).
        WillOnce(Return(false));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(_,_)).Times(2).
        WillRepeatedly(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_OK));

    GUID_t remote_participant_key;
    fill_participant_key(remote_participant_key);
    ASSERT_FALSE(manager_.discovered_participant(IdentityToken(), remote_participant_key));

    delete change;
}

TEST_F(SecurityAuthenticationTest, discovered_participant_validation_remote_identity_pending_handshake_request_pending_message)
{
    EXPECT_CALL(*auth_plugin_, return_identity_handle(_,_)).Times(2).
        WillRepeatedly(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_OK));

    request_process_ok();
}

TEST_F(SecurityAuthenticationTest, discovered_participant_validation_remote_identity_pending_handshake_request_ok_with_final_message)
{
    initialization_ok();

    MockIdentityHandle identity_handle;
    MockIdentityHandle* p_identity_handle = &identity_handle;
    MockHandshakeHandle handshake_handle;
    MockHandshakeHandle* p_handshake_handle = &handshake_handle;
    HandshakeMessageToken* p_handshake_message = new HandshakeMessageToken();
    CacheChange_t* change = new CacheChange_t(200);

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_identity_handle), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_REQUEST)));
    EXPECT_CALL(*auth_plugin_, begin_handshake_request(_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_handshake_handle), 
                    SetArgPointee<1>(p_handshake_message), Return(ValidationResult_t::VALIDATION_OK_WITH_FINAL_MESSAGE)));
    EXPECT_CALL(*stateless_writer_, new_change(_,_,_)).Times(1).
        WillOnce(Return(change));
    EXPECT_CALL(*stateless_writer_->history_, add_change(change)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(_,_)).Times(2).
        WillRepeatedly(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_OK));

    GUID_t remote_participant_key;
    fill_participant_key(remote_participant_key);
    ASSERT_TRUE(manager_.discovered_participant(IdentityToken(), remote_participant_key));

    delete change;
}

TEST_F(SecurityAuthenticationTest, discovered_participant_ok)
{
    initialization_ok();

    MockIdentityHandle identity_handle;
    MockIdentityHandle* p_identity_handle = &identity_handle;
    MockHandshakeHandle handshake_handle;
    MockHandshakeHandle* p_handshake_handle = &handshake_handle;
    HandshakeMessageToken* p_handshake_message = new HandshakeMessageToken();
    CacheChange_t* change = new CacheChange_t(200);

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_identity_handle), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_REQUEST)));
    EXPECT_CALL(*auth_plugin_, begin_handshake_request(_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_handshake_handle), 
                    SetArgPointee<1>(p_handshake_message), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));
    EXPECT_CALL(*stateless_writer_, new_change(_,_,_)).Times(1).
        WillOnce(Return(change));
    EXPECT_CALL(*stateless_writer_->history_, add_change(change)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(_,_)).Times(2).
        WillRepeatedly(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_OK));

    GUID_t remote_participant_key;
    fill_participant_key(remote_participant_key);
    ASSERT_TRUE(manager_.discovered_participant(IdentityToken(), remote_participant_key));

    delete change;
}

TEST_F(SecurityAuthenticationTest, discovered_participant_validate_remote_fail_and_then_ok)
{
    initialization_ok();

    MockIdentityHandle identity_handle;
    MockIdentityHandle* p_identity_handle = &identity_handle;
    MockHandshakeHandle handshake_handle;
    MockHandshakeHandle* p_handshake_handle = &handshake_handle;
    HandshakeMessageToken* p_handshake_message = new HandshakeMessageToken();
    CacheChange_t* change = new CacheChange_t(200);

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_,_,_,_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_FAILED));

    GUID_t remote_participant_key;
    fill_participant_key(remote_participant_key);
    ASSERT_FALSE(manager_.discovered_participant(IdentityToken(), remote_participant_key));

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_identity_handle), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_REQUEST)));
    EXPECT_CALL(*auth_plugin_, begin_handshake_request(_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_handshake_handle), 
                    SetArgPointee<1>(p_handshake_message), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));
    EXPECT_CALL(*stateless_writer_, new_change(_,_,_)).Times(1).
        WillOnce(Return(change));
    EXPECT_CALL(*stateless_writer_->history_, add_change(change)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(_,_)).Times(2).
        WillRepeatedly(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_OK));

    ASSERT_TRUE(manager_.discovered_participant(IdentityToken(), remote_participant_key));

    delete change;
}

TEST_F(SecurityAuthenticationTest, discovered_participant_begin_handshake_request_fail_and_then_ok)
{
    initialization_ok();

    MockIdentityHandle identity_handle;
    MockIdentityHandle* p_identity_handle = &identity_handle;
    MockHandshakeHandle handshake_handle;
    MockHandshakeHandle* p_handshake_handle = &handshake_handle;
    HandshakeMessageToken* p_handshake_message = new HandshakeMessageToken();
    CacheChange_t* change = new CacheChange_t(200);

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_identity_handle), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_REQUEST)));
    EXPECT_CALL(*auth_plugin_, begin_handshake_request(_,_,_,_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_FAILED));

    GUID_t remote_participant_key;
    fill_participant_key(remote_participant_key);
    ASSERT_FALSE(manager_.discovered_participant(IdentityToken(), remote_participant_key));

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_,_,_,_,_)).Times(0);
    EXPECT_CALL(*auth_plugin_, begin_handshake_request(_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_handshake_handle), 
                    SetArgPointee<1>(p_handshake_message), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));
    EXPECT_CALL(*stateless_writer_, new_change(_,_,_)).Times(1).
        WillOnce(Return(change));
    EXPECT_CALL(*stateless_writer_->history_, add_change(change)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(_,_)).Times(2).
        WillRepeatedly(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_OK));

    ASSERT_TRUE(manager_.discovered_participant(IdentityToken(), remote_participant_key));

    delete change;
}

TEST_F(SecurityAuthenticationTest, discovered_participant_process_message_not_remote_participant_key)
{
    initialization_ok();

    ParticipantGenericMessage message;
    message.message_class_id("dds.sec.auth");
    CacheChange_t* change = new CacheChange_t(ParticipantGenericMessageHelper::serialized_size(message));
    CDRMessage_t aux_msg(0);
    aux_msg.wraps = true;
    aux_msg.buffer = change->serializedPayload.data;
    aux_msg.max_size = change->serializedPayload.max_size;
    aux_msg.msg_endian = change->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND : LITTLEEND;
    ASSERT_TRUE(CDRMessage::addParticipantGenericMessage(&aux_msg, message));
    change->serializedPayload.length = aux_msg.length;

    EXPECT_CALL(*auth_plugin_, return_identity_handle(_,_)).Times(1).
        WillRepeatedly(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*stateless_reader_->history_, remove_change(change)).Times(1).
        WillOnce(Return(true));

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);
}

TEST_F(SecurityAuthenticationTest, discovered_participant_process_message_bad_message_class_id)
{
    initialization_ok();

    MockIdentityHandle identity_handle;
    MockIdentityHandle* p_identity_handle = &identity_handle;

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_identity_handle), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));

    GUID_t remote_participant_key;
    fill_participant_key(remote_participant_key);
    ASSERT_TRUE(manager_.discovered_participant(IdentityToken(), remote_participant_key));

    ParticipantGenericMessage message;
    CacheChange_t* change = new CacheChange_t(ParticipantGenericMessageHelper::serialized_size(message));
    CDRMessage_t aux_msg(0);
    aux_msg.wraps = true;
    aux_msg.buffer = change->serializedPayload.data;
    aux_msg.max_size = change->serializedPayload.max_size;
    aux_msg.msg_endian = change->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND : LITTLEEND;
    ASSERT_TRUE(CDRMessage::addParticipantGenericMessage(&aux_msg, message));
    change->serializedPayload.length = aux_msg.length;

    EXPECT_CALL(*auth_plugin_, return_identity_handle(_,_)).Times(2).
        WillRepeatedly(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*stateless_reader_->history_, remove_change(change)).Times(1).
        WillOnce(Return(true));

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);
}

TEST_F(SecurityAuthenticationTest, discovered_participant_process_message_not_expecting_request)
{
    initialization_ok();

    MockIdentityHandle identity_handle;
    MockIdentityHandle* p_identity_handle = &identity_handle;

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_identity_handle), Return(ValidationResult_t::VALIDATION_OK)));

    GUID_t remote_participant_key;
    fill_participant_key(remote_participant_key);
    ASSERT_TRUE(manager_.discovered_participant(IdentityToken(), remote_participant_key));

    ParticipantGenericMessage message;
    message.message_identity().source_guid(remote_participant_key);
    message.destination_participant_key(remote_participant_key);
    message.message_class_id("dds.sec.auth");
    HandshakeMessageToken token;
    message.message_data().push_back(token);
    CacheChange_t* change = new CacheChange_t(ParticipantGenericMessageHelper::serialized_size(message));
    CDRMessage_t aux_msg(0);
    aux_msg.wraps = true;
    aux_msg.buffer = change->serializedPayload.data;
    aux_msg.max_size = change->serializedPayload.max_size;
    aux_msg.msg_endian = change->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND : LITTLEEND;
    ASSERT_TRUE(CDRMessage::addParticipantGenericMessage(&aux_msg, message));
    change->serializedPayload.length = aux_msg.length;

    EXPECT_CALL(*auth_plugin_, return_identity_handle(_,_)).Times(2).
        WillRepeatedly(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*stateless_reader_->history_, remove_change(change)).Times(1).
        WillOnce(Return(true));

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);
}

TEST_F(SecurityAuthenticationTest, discovered_participant_process_message_fail_begin_handshake_reply)
{
    initialization_ok();

    MockIdentityHandle identity_handle;
    MockIdentityHandle* p_identity_handle = &identity_handle;

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_identity_handle), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));

    GUID_t remote_participant_key;
    fill_participant_key(remote_participant_key);
    ASSERT_TRUE(manager_.discovered_participant(IdentityToken(), remote_participant_key));

    ParticipantGenericMessage message;
    message.message_identity().source_guid(remote_participant_key);
    message.destination_participant_key(remote_participant_key);
    message.message_class_id("dds.sec.auth");
    HandshakeMessageToken token;
    message.message_data().push_back(token);
    CacheChange_t* change = new CacheChange_t(ParticipantGenericMessageHelper::serialized_size(message));
    CDRMessage_t aux_msg(0);
    aux_msg.wraps = true;
    aux_msg.buffer = change->serializedPayload.data;
    aux_msg.max_size = change->serializedPayload.max_size;
    aux_msg.msg_endian = change->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND : LITTLEEND;
    ASSERT_TRUE(CDRMessage::addParticipantGenericMessage(&aux_msg, message));
    change->serializedPayload.length = aux_msg.length;

    EXPECT_CALL(*auth_plugin_, begin_handshake_reply_rvr(_,_,_,_,_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_FAILED));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(_,_)).Times(2).
        WillRepeatedly(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*stateless_reader_->history_, remove_change(change)).Times(1).
        WillOnce(Return(true));

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);
}

TEST_F(SecurityAuthenticationTest, discovered_participant_process_message_ok_begin_handshake_reply)
{
    initialization_ok();

    MockIdentityHandle identity_handle;
    MockIdentityHandle* p_identity_handle = &identity_handle;

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_identity_handle), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));

    GUID_t remote_participant_key;
    fill_participant_key(remote_participant_key);
    ASSERT_TRUE(manager_.discovered_participant(IdentityToken(), remote_participant_key));

    ParticipantGenericMessage message;
    message.message_identity().source_guid(remote_participant_key);
    message.destination_participant_key(remote_participant_key);
    message.message_class_id("dds.sec.auth");
    HandshakeMessageToken token;
    message.message_data().push_back(token);
    CacheChange_t* change = new CacheChange_t(ParticipantGenericMessageHelper::serialized_size(message));
    CDRMessage_t aux_msg(0);
    aux_msg.wraps = true;
    aux_msg.buffer = change->serializedPayload.data;
    aux_msg.max_size = change->serializedPayload.max_size;
    aux_msg.msg_endian = change->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND : LITTLEEND;
    ASSERT_TRUE(CDRMessage::addParticipantGenericMessage(&aux_msg, message));
    change->serializedPayload.length = aux_msg.length;

    MockHandshakeHandle handshake_handle;
    MockHandshakeHandle* p_handshake_handle = &handshake_handle;

    EXPECT_CALL(*auth_plugin_, begin_handshake_reply_rvr(_,_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_handshake_handle), 
                    Return(ValidationResult_t::VALIDATION_OK)));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(_,_)).Times(2).
        WillRepeatedly(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*stateless_reader_->history_, remove_change(change)).Times(1).
        WillOnce(Return(true));

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);
}

TEST_F(SecurityAuthenticationTest, discovered_participant_process_message_new_change_fail)
{
    initialization_ok();

    MockIdentityHandle identity_handle;
    MockIdentityHandle* p_identity_handle = &identity_handle;

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_identity_handle), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));

    GUID_t remote_participant_key;
    fill_participant_key(remote_participant_key);
    ASSERT_TRUE(manager_.discovered_participant(IdentityToken(), remote_participant_key));

    ParticipantGenericMessage message;
    message.message_identity().source_guid(remote_participant_key);
    message.destination_participant_key(remote_participant_key);
    message.message_class_id("dds.sec.auth");
    HandshakeMessageToken token;
    message.message_data().push_back(token);
    CacheChange_t* change = new CacheChange_t(ParticipantGenericMessageHelper::serialized_size(message));
    CDRMessage_t aux_msg(0);
    aux_msg.wraps = true;
    aux_msg.buffer = change->serializedPayload.data;
    aux_msg.max_size = change->serializedPayload.max_size;
    aux_msg.msg_endian = change->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND : LITTLEEND;
    ASSERT_TRUE(CDRMessage::addParticipantGenericMessage(&aux_msg, message));
    change->serializedPayload.length = aux_msg.length;

    MockHandshakeHandle handshake_handle;
    MockHandshakeHandle* p_handshake_handle = &handshake_handle;
    HandshakeMessageToken* p_handshake_message = new HandshakeMessageToken();

    EXPECT_CALL(*auth_plugin_, begin_handshake_reply_rvr(_,_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_handshake_handle), 
                    SetArgPointee<1>(p_handshake_message), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(_,_)).Times(2).
        WillRepeatedly(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*stateless_writer_, new_change(_,_,_)).Times(1).
        WillOnce(Return(nullptr));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*stateless_reader_->history_, remove_change(change)).Times(1).
        WillOnce(Return(true));

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);
}

TEST_F(SecurityAuthenticationTest, discovered_participant_process_message_add_change_fail)
{
    initialization_ok();

    MockIdentityHandle identity_handle;
    MockIdentityHandle* p_identity_handle = &identity_handle;

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_identity_handle), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));

    GUID_t remote_participant_key;
    fill_participant_key(remote_participant_key);
    ASSERT_TRUE(manager_.discovered_participant(IdentityToken(), remote_participant_key));

    ParticipantGenericMessage message;
    message.message_identity().source_guid(remote_participant_key);
    message.destination_participant_key(remote_participant_key);
    message.message_class_id("dds.sec.auth");
    HandshakeMessageToken token;
    message.message_data().push_back(token);
    CacheChange_t* change = new CacheChange_t(ParticipantGenericMessageHelper::serialized_size(message));
    CDRMessage_t aux_msg(0);
    aux_msg.wraps = true;
    aux_msg.buffer = change->serializedPayload.data;
    aux_msg.max_size = change->serializedPayload.max_size;
    aux_msg.msg_endian = change->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND : LITTLEEND;
    ASSERT_TRUE(CDRMessage::addParticipantGenericMessage(&aux_msg, message));
    change->serializedPayload.length = aux_msg.length;

    MockHandshakeHandle handshake_handle;
    MockHandshakeHandle* p_handshake_handle = &handshake_handle;
    HandshakeMessageToken* p_handshake_message = new HandshakeMessageToken();
    CacheChange_t* change2 = new CacheChange_t(200);

    EXPECT_CALL(*auth_plugin_, begin_handshake_reply_rvr(_,_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_handshake_handle), 
                    SetArgPointee<1>(p_handshake_message), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(_,_)).Times(2).
        WillRepeatedly(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*stateless_writer_, new_change(_,_,_)).Times(1).
        WillOnce(Return(change2));
    EXPECT_CALL(*stateless_writer_->history_, add_change(change2)).Times(1).
        WillOnce(Return(false));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*stateless_reader_->history_, remove_change(change)).Times(1).
        WillOnce(Return(true));

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);

    delete change2;
}

TEST_F(SecurityAuthenticationTest, discovered_participant_process_message_pending_handshake_reply_pending_message)
{
    EXPECT_CALL(*auth_plugin_, return_identity_handle(_,_)).Times(2).
        WillRepeatedly(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_OK));

    reply_process_ok();
}

TEST_F(SecurityAuthenticationTest, discovered_participant_process_message_pending_handshake_reply_ok_with_final_message)
{
    initialization_ok();

    MockIdentityHandle identity_handle;
    MockIdentityHandle* p_identity_handle = &identity_handle;

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_identity_handle), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));

    GUID_t remote_participant_key;
    fill_participant_key(remote_participant_key);
    ASSERT_TRUE(manager_.discovered_participant(IdentityToken(), remote_participant_key));

    ParticipantGenericMessage message;
    message.message_identity().source_guid(remote_participant_key);
    message.destination_participant_key(remote_participant_key);
    message.message_class_id("dds.sec.auth");
    HandshakeMessageToken token;
    message.message_data().push_back(token);
    CacheChange_t* change = new CacheChange_t(ParticipantGenericMessageHelper::serialized_size(message));
    CDRMessage_t aux_msg(0);
    aux_msg.wraps = true;
    aux_msg.buffer = change->serializedPayload.data;
    aux_msg.max_size = change->serializedPayload.max_size;
    aux_msg.msg_endian = change->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND : LITTLEEND;
    ASSERT_TRUE(CDRMessage::addParticipantGenericMessage(&aux_msg, message));
    change->serializedPayload.length = aux_msg.length;

    MockHandshakeHandle handshake_handle;
    MockHandshakeHandle* p_handshake_handle = &handshake_handle;
    HandshakeMessageToken* p_handshake_message = new HandshakeMessageToken();
    CacheChange_t* change2 = new CacheChange_t(200);

    EXPECT_CALL(*auth_plugin_, begin_handshake_reply_rvr(_,_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_handshake_handle), 
                    SetArgPointee<1>(p_handshake_message), Return(ValidationResult_t::VALIDATION_OK_WITH_FINAL_MESSAGE)));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(_,_)).Times(2).
        WillRepeatedly(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*stateless_writer_, new_change(_,_,_)).Times(1).
        WillOnce(Return(change2));
    EXPECT_CALL(*stateless_writer_->history_, add_change(change2)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*stateless_reader_->history_, remove_change(change)).Times(1).
        WillOnce(Return(true));

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);

    delete change2;
}

TEST_F(SecurityAuthenticationTest, discovered_participant_process_message_fail_process_handshake_reply)
{
    request_process_ok();

    GUID_t remote_participant_key;
    fill_participant_key(remote_participant_key);

    ParticipantGenericMessage message;
    message.message_identity().source_guid(remote_participant_key);
    message.related_message_identity().source_guid(remote_participant_key);
    message.destination_participant_key(remote_participant_key);
    message.message_class_id("dds.sec.auth");
    HandshakeMessageToken token;
    message.message_data().push_back(token);
    CacheChange_t* change = new CacheChange_t(ParticipantGenericMessageHelper::serialized_size(message));
    CDRMessage_t aux_msg(0);
    aux_msg.wraps = true;
    aux_msg.buffer = change->serializedPayload.data;
    aux_msg.max_size = change->serializedPayload.max_size;
    aux_msg.msg_endian = change->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND : LITTLEEND;
    ASSERT_TRUE(CDRMessage::addParticipantGenericMessage(&aux_msg, message));
    change->serializedPayload.length = aux_msg.length;

    EXPECT_CALL(*auth_plugin_, process_handshake_rvr(_,_,_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_FAILED));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(_,_)).Times(2).
        WillRepeatedly(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*stateless_reader_->history_, remove_change(change)).Times(1).
        WillOnce(Return(true));

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);

    delete change;
}

TEST_F(SecurityAuthenticationTest, discovered_participant_process_message_ok_process_handshake_reply)
{
    request_process_ok();

    GUID_t remote_participant_key;
    fill_participant_key(remote_participant_key);

    ParticipantGenericMessage message;
    message.message_identity().source_guid(remote_participant_key);
    message.related_message_identity().source_guid(remote_participant_key);
    message.destination_participant_key(remote_participant_key);
    message.message_class_id("dds.sec.auth");
    HandshakeMessageToken token;
    message.message_data().push_back(token);
    CacheChange_t* change = new CacheChange_t(ParticipantGenericMessageHelper::serialized_size(message));
    CDRMessage_t aux_msg(0);
    aux_msg.wraps = true;
    aux_msg.buffer = change->serializedPayload.data;
    aux_msg.max_size = change->serializedPayload.max_size;
    aux_msg.msg_endian = change->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND : LITTLEEND;
    ASSERT_TRUE(CDRMessage::addParticipantGenericMessage(&aux_msg, message));
    change->serializedPayload.length = aux_msg.length;

    EXPECT_CALL(*auth_plugin_, process_handshake_rvr(_,_,_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(_,_)).Times(2).
        WillRepeatedly(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*stateless_reader_->history_, remove_change(change)).Times(1).
        WillOnce(Return(true));

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);
}

TEST_F(SecurityAuthenticationTest, discovered_participant_process_message_process_handshake_reply_new_change_fail)
{
    request_process_ok();

    GUID_t remote_participant_key;
    fill_participant_key(remote_participant_key);

    ParticipantGenericMessage message;
    message.message_identity().source_guid(remote_participant_key);
    message.related_message_identity().source_guid(remote_participant_key);
    message.destination_participant_key(remote_participant_key);
    message.message_class_id("dds.sec.auth");
    HandshakeMessageToken token;
    message.message_data().push_back(token);
    CacheChange_t* change = new CacheChange_t(ParticipantGenericMessageHelper::serialized_size(message));
    CDRMessage_t aux_msg(0);
    aux_msg.wraps = true;
    aux_msg.buffer = change->serializedPayload.data;
    aux_msg.max_size = change->serializedPayload.max_size;
    aux_msg.msg_endian = change->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND : LITTLEEND;
    ASSERT_TRUE(CDRMessage::addParticipantGenericMessage(&aux_msg, message));
    change->serializedPayload.length = aux_msg.length;

    HandshakeMessageToken* p_handshake_message = new HandshakeMessageToken();

    EXPECT_CALL(*auth_plugin_, process_handshake_rvr(_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_handshake_message), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(_,_)).Times(2).
        WillRepeatedly(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*stateless_writer_, new_change(_,_,_)).Times(1).
        WillOnce(Return(nullptr));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*stateless_reader_->history_, remove_change(change)).Times(1).
        WillOnce(Return(true));

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);
}

TEST_F(SecurityAuthenticationTest, discovered_participant_process_message_process_handshake_reply_add_change_fail)
{
    request_process_ok();

    GUID_t remote_participant_key;
    fill_participant_key(remote_participant_key);

    ParticipantGenericMessage message;
    message.message_identity().source_guid(remote_participant_key);
    message.related_message_identity().source_guid(remote_participant_key);
    message.destination_participant_key(remote_participant_key);
    message.message_class_id("dds.sec.auth");
    HandshakeMessageToken token;
    message.message_data().push_back(token);
    CacheChange_t* change = new CacheChange_t(ParticipantGenericMessageHelper::serialized_size(message));
    CDRMessage_t aux_msg(0);
    aux_msg.wraps = true;
    aux_msg.buffer = change->serializedPayload.data;
    aux_msg.max_size = change->serializedPayload.max_size;
    aux_msg.msg_endian = change->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND : LITTLEEND;
    ASSERT_TRUE(CDRMessage::addParticipantGenericMessage(&aux_msg, message));
    change->serializedPayload.length = aux_msg.length;

    HandshakeMessageToken* p_handshake_message = new HandshakeMessageToken();
    CacheChange_t* change2 = new CacheChange_t(200);

    EXPECT_CALL(*auth_plugin_, process_handshake_rvr(_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_handshake_message), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(_,_)).Times(2).
        WillRepeatedly(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*stateless_writer_, new_change(_,_,_)).Times(1).
        WillOnce(Return(change2));
    EXPECT_CALL(*stateless_writer_->history_, add_change(change2)).Times(1).
        WillOnce(Return(false));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*stateless_reader_->history_, remove_change(change)).Times(1).
        WillOnce(Return(true));

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);

    delete change2;
}

TEST_F(SecurityAuthenticationTest, discovered_participant_process_message_process_handshake_reply_ok_with_final_message)
{
    request_process_ok();

    GUID_t remote_participant_key;
    fill_participant_key(remote_participant_key);

    ParticipantGenericMessage message;
    message.message_identity().source_guid(remote_participant_key);
    message.related_message_identity().source_guid(remote_participant_key);
    message.destination_participant_key(remote_participant_key);
    message.message_class_id("dds.sec.auth");
    HandshakeMessageToken token;
    message.message_data().push_back(token);
    CacheChange_t* change = new CacheChange_t(ParticipantGenericMessageHelper::serialized_size(message));
    CDRMessage_t aux_msg(0);
    aux_msg.wraps = true;
    aux_msg.buffer = change->serializedPayload.data;
    aux_msg.max_size = change->serializedPayload.max_size;
    aux_msg.msg_endian = change->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND : LITTLEEND;
    ASSERT_TRUE(CDRMessage::addParticipantGenericMessage(&aux_msg, message));
    change->serializedPayload.length = aux_msg.length;

    HandshakeMessageToken* p_handshake_message = new HandshakeMessageToken();
    CacheChange_t* change2 = new CacheChange_t(200);

    EXPECT_CALL(*auth_plugin_, process_handshake_rvr(_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_handshake_message), Return(ValidationResult_t::VALIDATION_OK_WITH_FINAL_MESSAGE)));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(_,_)).Times(2).
        WillRepeatedly(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*stateless_writer_, new_change(_,_,_)).Times(1).
        WillOnce(Return(change2));
    EXPECT_CALL(*stateless_writer_->history_, add_change(change2)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*stateless_reader_->history_, remove_change(change)).Times(1).
        WillOnce(Return(true));

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);

    delete change2;
}

TEST_F(SecurityAuthenticationTest, discovered_participant_process_message_fail_process_handshake_final)
{
    reply_process_ok();

    GUID_t remote_participant_key;
    fill_participant_key(remote_participant_key);

    ParticipantGenericMessage message;
    message.message_identity().source_guid(remote_participant_key);
    message.related_message_identity().source_guid(remote_participant_key);
    message.destination_participant_key(remote_participant_key);
    message.message_class_id("dds.sec.auth");
    HandshakeMessageToken token;
    message.message_data().push_back(token);
    CacheChange_t* change = new CacheChange_t(ParticipantGenericMessageHelper::serialized_size(message));
    CDRMessage_t aux_msg(0);
    aux_msg.wraps = true;
    aux_msg.buffer = change->serializedPayload.data;
    aux_msg.max_size = change->serializedPayload.max_size;
    aux_msg.msg_endian = change->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND : LITTLEEND;
    ASSERT_TRUE(CDRMessage::addParticipantGenericMessage(&aux_msg, message));
    change->serializedPayload.length = aux_msg.length;

    EXPECT_CALL(*auth_plugin_, process_handshake_rvr(_,_,_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_FAILED));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(_,_)).Times(2).
        WillRepeatedly(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*stateless_reader_->history_, remove_change(change)).Times(1).
        WillOnce(Return(true));

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);

    delete change;
}

TEST_F(SecurityAuthenticationTest, discovered_participant_process_message_ok_process_handshake_final)
{
    reply_process_ok();

    GUID_t remote_participant_key;
    fill_participant_key(remote_participant_key);

    ParticipantGenericMessage message;
    message.message_identity().source_guid(remote_participant_key);
    message.related_message_identity().source_guid(remote_participant_key);
    message.destination_participant_key(remote_participant_key);
    message.message_class_id("dds.sec.auth");
    HandshakeMessageToken token;
    message.message_data().push_back(token);
    CacheChange_t* change = new CacheChange_t(ParticipantGenericMessageHelper::serialized_size(message));
    CDRMessage_t aux_msg(0);
    aux_msg.wraps = true;
    aux_msg.buffer = change->serializedPayload.data;
    aux_msg.max_size = change->serializedPayload.max_size;
    aux_msg.msg_endian = change->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND : LITTLEEND;
    ASSERT_TRUE(CDRMessage::addParticipantGenericMessage(&aux_msg, message));
    change->serializedPayload.length = aux_msg.length;

    EXPECT_CALL(*auth_plugin_, process_handshake_rvr(_,_,_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*auth_plugin_, return_identity_handle(_,_)).Times(2).
        WillRepeatedly(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*auth_plugin_, return_handshake_handle(_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_OK));
    EXPECT_CALL(*stateless_reader_->history_, remove_change(change)).Times(1).
        WillOnce(Return(true));

    stateless_reader_->listener_->onNewCacheChangeAdded(stateless_reader_, change);
}

int main(int argc, char **argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
