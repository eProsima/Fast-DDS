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

class SecurityAuthenticationTest : public ::testing::Test
{
    protected:

        virtual void SetUp()
        {
            SecurityPluginFactory::set_auth_plugin(auth_plugin_);
        }

        virtual void TearDown()
        {
            SecurityPluginFactory::release_auth_plugin();
        }

        void initialization_ok()
        {
            DefaultValue<const RTPSParticipantAttributes&>::Set(pattr);
            DefaultValue<const GUID_t&>::Set(guid);
            MockIdentityHandle identity_handle;
            MockIdentityHandle* p_identity_handle = &identity_handle;

            EXPECT_CALL(*auth_plugin_, validate_local_identity(_,_,_,_,_,_)).Times(1).
                WillOnce(DoAll(SetArgPointee<0>(p_identity_handle), Return(ValidationResult_t::VALIDATION_OK)));
            EXPECT_CALL(participant_, createWriter(_,_,_,_,_,_)).Times(1).
                WillOnce(Return(true));
            EXPECT_CALL(participant_, createReader(_,_,_,_,_,_,_)).Times(1).
                WillOnce(Return(true));

            ASSERT_TRUE(manager_.init());
        }


    public:

        SecurityAuthenticationTest() : auth_plugin_(new MockAuthenticationPlugin()), manager_(&participant_) {}
        MockAuthenticationPlugin* auth_plugin_;
        NiceMock<RTPSParticipantImpl> participant_;
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
    EXPECT_CALL(participant_, createWriter(_,_,_,_,_,_)).Times(1).
        WillOnce(Return(false));

    ASSERT_FALSE(manager_.init());
}

TEST_F(SecurityAuthenticationTest, initialization_fail_participant_stateless_message_reader)
{
    DefaultValue<const RTPSParticipantAttributes&>::Set(pattr);
    DefaultValue<const GUID_t&>::Set(guid);
    MockIdentityHandle identity_handle;
    MockIdentityHandle* p_identity_handle = &identity_handle;

    EXPECT_CALL(*auth_plugin_, validate_local_identity(_,_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_identity_handle), Return(ValidationResult_t::VALIDATION_OK)));
    EXPECT_CALL(participant_, createWriter(_,_,_,_,_,_)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(participant_, createReader(_,_,_,_,_,_,_)).Times(1).
        WillOnce(Return(false));

    ASSERT_FALSE(manager_.init());
}

TEST_F(SecurityAuthenticationTest, initialization_auth_retry)
{
    DefaultValue<const RTPSParticipantAttributes&>::Set(pattr);
    DefaultValue<const GUID_t&>::Set(guid);
    MockIdentityHandle identity_handle;
    MockIdentityHandle* p_identity_handle = &identity_handle;

    EXPECT_CALL(*auth_plugin_, validate_local_identity(_,_,_,_,_,_)).Times(2).
        WillOnce(Return(ValidationResult_t::VALIDATION_PENDING_RETRY)).
        WillOnce(DoAll(SetArgPointee<0>(p_identity_handle), Return(ValidationResult_t::VALIDATION_OK)));
    EXPECT_CALL(participant_, createWriter(_,_,_,_,_,_)).Times(1).
        WillOnce(Return(true));
    EXPECT_CALL(participant_, createReader(_,_,_,_,_,_,_)).Times(1).
        WillOnce(Return(true));

    ASSERT_TRUE(manager_.init());
}


TEST_F(SecurityAuthenticationTest, initialization_ok)
{
    initialization_ok();
}

TEST_F(SecurityAuthenticationTest, discovered_participant_validation_remote_identity_fail)
{
    initialization_ok();

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_,_,_,_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_FAILED));

    GUID_t remote_participant_key;
    ASSERT_FALSE(manager_.discovered_participant(IdentityToken(), remote_participant_key));
}

TEST_F(SecurityAuthenticationTest, discovered_participant_validation_remote_identity_ok)
{
    initialization_ok();

    MockIdentityHandle identity_handle;
    MockIdentityHandle* p_identity_handle = &identity_handle;

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_identity_handle), Return(ValidationResult_t::VALIDATION_OK)));

    GUID_t remote_participant_key;
    ASSERT_TRUE(manager_.discovered_participant(IdentityToken(), remote_participant_key));
}

TEST_F(SecurityAuthenticationTest, discovered_participant_validation_remote_identity_pending_handshake_message)
{
    initialization_ok();

    MockIdentityHandle identity_handle;
    MockIdentityHandle* p_identity_handle = &identity_handle;

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_identity_handle), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));

    GUID_t remote_participant_key;
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

    GUID_t remote_participant_key;
    ASSERT_FALSE(manager_.discovered_participant(IdentityToken(), remote_participant_key));
}

TEST_F(SecurityAuthenticationTest, discovered_participant_validation_remote_identity_pending_handshake_request_ok)
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
                    SetArgPointee<1>(p_handshake_message), Return(ValidationResult_t::VALIDATION_OK)));

    GUID_t remote_participant_key;
    ASSERT_TRUE(manager_.discovered_participant(IdentityToken(), remote_participant_key));
}

TEST_F(SecurityAuthenticationTest, discovered_participant_validation_remote_identity_pending_handshake_request_ok_with_final_message)
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
                    SetArgPointee<1>(p_handshake_message), Return(ValidationResult_t::VALIDATION_OK_WITH_FINAL_MESSAGE)));

    GUID_t remote_participant_key;
    ASSERT_TRUE(manager_.discovered_participant(IdentityToken(), remote_participant_key));
}

TEST_F(SecurityAuthenticationTest, discovered_participant_ok)
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

    GUID_t remote_participant_key;
    ASSERT_TRUE(manager_.discovered_participant(IdentityToken(), remote_participant_key));
}

TEST_F(SecurityAuthenticationTest, discovered_participant_validate_remote_fail_and_then_ok)
{
    initialization_ok();

    MockIdentityHandle identity_handle;
    MockIdentityHandle* p_identity_handle = &identity_handle;
    MockHandshakeHandle handshake_handle;
    MockHandshakeHandle* p_handshake_handle = &handshake_handle;
    HandshakeMessageToken* p_handshake_message = new HandshakeMessageToken();

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_,_,_,_,_)).Times(1).
        WillOnce(Return(ValidationResult_t::VALIDATION_FAILED));

    GUID_t remote_participant_key;
    ASSERT_FALSE(manager_.discovered_participant(IdentityToken(), remote_participant_key));

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_identity_handle), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_REQUEST)));
    EXPECT_CALL(*auth_plugin_, begin_handshake_request(_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_handshake_handle), 
                    SetArgPointee<1>(p_handshake_message), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));

    ASSERT_TRUE(manager_.discovered_participant(IdentityToken(), remote_participant_key));
}

TEST_F(SecurityAuthenticationTest, discovered_participant_begin_handshake_request_fail_and_then_ok)
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
        WillOnce(Return(ValidationResult_t::VALIDATION_FAILED));

    GUID_t remote_participant_key;
    ASSERT_FALSE(manager_.discovered_participant(IdentityToken(), remote_participant_key));

    EXPECT_CALL(*auth_plugin_, validate_remote_identity_rvr(_,_,_,_,_)).Times(0);
    EXPECT_CALL(*auth_plugin_, begin_handshake_request(_,_,_,_,_)).Times(1).
        WillOnce(DoAll(SetArgPointee<0>(p_handshake_handle), 
                    SetArgPointee<1>(p_handshake_message), Return(ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE)));

    ASSERT_TRUE(manager_.discovered_participant(IdentityToken(), remote_participant_key));
}

int main(int argc, char **argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
