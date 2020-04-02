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

#ifndef __TEST_UNITTEST_RTPS_SECURITY_SECURITYTESTS_HPP__
#define __TEST_UNITTEST_RTPS_SECURITY_SECURITYTESTS_HPP__

#include <rtps/participant/RTPSParticipantImpl.h>
#include <fastrtps/rtps/security/common/Handle.h>
#include <rtps/security/MockAuthenticationPlugin.h>
#include <rtps/security/MockCryptographyPlugin.h>
#include <fastrtps/rtps/writer/StatelessWriter.h>
#include <fastrtps/rtps/writer/StatefulWriter.h>
#include <fastrtps/rtps/history/WriterHistory.h>
#include <fastrtps/rtps/reader/StatelessReader.h>
#include <fastrtps/rtps/reader/StatefulReader.h>
#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>
#include <rtps/security/SecurityPluginFactory.h>
#include <rtps/security/SecurityManager.h>
#include <fastrtps/rtps/security/accesscontrol/ParticipantSecurityAttributes.h>
#include <fastrtps/rtps/builtin/discovery/participant/PDPSimple.h>

#include <gtest/gtest.h>

using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::rtps::security;
using namespace ::testing;

class MockIdentity
{
    public:

        static const char* const class_id_;
};

typedef HandleImpl<MockIdentity> MockIdentityHandle;

class MockHandshake
{
    public:

        static const char* const class_id_;
};

typedef HandleImpl<MockHandshake> MockHandshakeHandle;

typedef HandleImpl<SharedSecret> MockSharedSecretHandle;

class MockParticipantCrypto
{
    public:

        static const char* const class_id_;
};

typedef HandleImpl<MockParticipantCrypto> MockParticipantCryptoHandle;

class SecurityTest : public ::testing::Test
{
    protected:

        virtual void SetUp()
        {
            ::testing::DefaultValue<const NetworkFactory&>::Set(network);
            SecurityPluginFactory::set_auth_plugin(auth_plugin_);
            SecurityPluginFactory::set_crypto_plugin(crypto_plugin_);
            fill_participant_key(guid);
        }

        virtual void TearDown()
        {
            SecurityPluginFactory::release_auth_plugin();
            SecurityPluginFactory::release_crypto_plugin();

            ::testing::DefaultValue<const GUID_t&>::Clear();
            ::testing::DefaultValue<CDRMessage_t>::Clear();
            ::testing::DefaultValue<const ParticipantSecurityAttributes&>::Clear();
        }

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
            participant_key.entityId.value[0] = 0x0;
            participant_key.entityId.value[1] = 0x0;
            participant_key.entityId.value[2] = 0x1;
            participant_key.entityId.value[3] = 0xc1;
        }

        void initialization_ok();

        void initialization_auth_ok();

        void request_process_ok(CacheChange_t** request_message_change = nullptr);

        void reply_process_ok(CacheChange_t** reply_message_change = nullptr);

        void final_message_process_ok(CacheChange_t** final_message_change = nullptr);

    public:

        SecurityTest() : auth_plugin_(new MockAuthenticationPlugin()),
        crypto_plugin_(new MockCryptographyPlugin()),
        stateless_writer_(nullptr), stateless_reader_(nullptr),
        volatile_writer_(nullptr), volatile_reader_(nullptr),
        manager_(&participant_), participant_data_(c_default_RTPSParticipantAllocationAttributes),
        default_cdr_message(RTPSMESSAGE_DEFAULT_SIZE){}

        ~SecurityTest()
        {
        }

        MockAuthenticationPlugin* auth_plugin_;
        MockCryptographyPlugin* crypto_plugin_;
        ::testing::NiceMock<RTPSParticipantImpl> participant_;
        ::testing::NiceMock<StatelessWriter>* stateless_writer_;
        ::testing::NiceMock<StatelessReader>* stateless_reader_;
        ::testing::NiceMock<StatefulWriter>* volatile_writer_;
        ::testing::NiceMock<StatefulReader>* volatile_reader_;
        PDPSimple pdpsimple_;
        SecurityManager manager_;

        MockIdentityHandle local_identity_handle_;
        MockIdentityHandle remote_identity_handle_;
        MockHandshakeHandle handshake_handle_;
        MockParticipantCryptoHandle local_participant_crypto_handle_;
        ParticipantProxyData participant_data_;
        ParticipantSecurityAttributes security_attributes_;
        PropertyPolicy participant_properties_;
        bool security_activated_;


        // Default Values
        NetworkFactory network;
        GUID_t guid;
        CDRMessage_t default_cdr_message;
};

struct SecurityTestsGlobalDefaultValues
{
    // Default Values
    RTPSParticipantAttributes pattr;

    SecurityTestsGlobalDefaultValues()
    {
        ::testing::DefaultValue<const RTPSParticipantAttributes&>::Set(pattr);
    }
};

static SecurityTestsGlobalDefaultValues g_security_default_values_;

#endif // __TEST_UNITTEST_RTPS_SECURITY_SECURITYTESTS_HPP__
