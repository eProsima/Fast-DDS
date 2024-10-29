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

#include <gtest/gtest.h>

#include <fastdds/rtps/history/ReaderHistory.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>

#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/builtin/discovery/participant/PDP.h>
#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <rtps/reader/StatefulReader.hpp>
#include <rtps/reader/StatelessReader.hpp>
#include <rtps/security/accesscontrol/ParticipantSecurityAttributes.h>
#include <rtps/security/common/Handle.h>
#include <rtps/security/MockAuthenticationPlugin.h>
#include <rtps/security/MockCryptographyPlugin.h>
#include <rtps/security/SecurityManager.h>
#include <rtps/security/SecurityPluginFactory.h>
#include <rtps/writer/StatefulWriter.hpp>
#include <rtps/writer/StatelessWriter.hpp>

using namespace eprosima::fastdds::rtps;
using namespace eprosima::fastdds::rtps::security;
using namespace ::testing;

class SecurityTest;

class MockIdentity
{
public:

    static const char* const class_id_;
};

typedef HandleImpl<MockIdentity, SecurityTest> MockIdentityHandle;

class MockHandshake
{
public:

    static const char* const class_id_;
};

typedef HandleImpl<MockHandshake, SecurityTest> MockHandshakeHandle;

typedef HandleImpl<SharedSecret, SecurityTest> MockSharedSecretHandle;

class MockParticipantCrypto
{
public:

    static const char* const class_id_;
};

typedef HandleImpl<MockParticipantCrypto, SecurityTest> MockParticipantCryptoHandle;

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

    void fill_participant_key(
            GUID_t& participant_key)
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

    void request_process_ok(
            CacheChange_t** request_message_change = nullptr);

    void reply_process_ok(
            CacheChange_t** reply_message_change = nullptr);

    void final_message_process_ok(
            CacheChange_t** final_message_change = nullptr);

    void expect_kx_exchange(
            CacheChange_t& kx_change_to_add,
            CacheChange_t* kx_change_to_remove);

    void destroy_manager_and_change(
            CacheChange_t*& change,
            bool was_added = true);

public:

    SecurityTest()
        : auth_plugin_(new MockAuthenticationPlugin())
        , crypto_plugin_(new MockCryptographyPlugin())
        , stateless_writer_(nullptr)
        , stateless_reader_(nullptr)
        , volatile_writer_(nullptr)
        , volatile_reader_(nullptr)
        , manager_(&participant_, plugin_factory_)
        , participant_data_(c_default_RTPSParticipantAllocationAttributes)
        , default_cdr_message(RTPSMESSAGE_DEFAULT_SIZE)
    {
        // enforce deleter due to handle destructor protected nature
        local_participant_crypto_handle_.reset(
            new MockParticipantCryptoHandle,
            [](MockParticipantCryptoHandle* p)
            {
                delete p;
            });
    }

    ~SecurityTest()
    {
    }

    MockAuthenticationPlugin* auth_plugin_;
    MockCryptographyPlugin* crypto_plugin_;
    ::testing::NiceMock<RTPSParticipantImpl> participant_;
    ::testing::NiceMock<StatelessWriter>* stateless_writer_;
    ::testing::NiceMock<StatelessReader>* stateless_reader_;
    ::testing::StrictMock<StatefulWriter>* volatile_writer_;
    ::testing::NiceMock<StatefulReader>* volatile_reader_;
    PDP pdp_;
    SecurityPluginFactory plugin_factory_;
    SecurityManager manager_;

    // handles
    MockIdentityHandle local_identity_handle_;
    MockIdentityHandle remote_identity_handle_;
    MockHandshakeHandle handshake_handle_;

    std::shared_ptr<ParticipantCryptoHandle> local_participant_crypto_handle_;
    ParticipantProxyData participant_data_;
    ParticipantSecurityAttributes security_attributes_;
    PropertyPolicy participant_properties_;
    bool security_activated_;

    // Default Values
    NetworkFactory network{g_security_default_values_.pattr};
    GUID_t guid;
    CDRMessage_t default_cdr_message;

    // handle factory for the tests
    template<class T>
    typename std::enable_if<std::is_base_of<Handle, T>::value, T&>::type
    get_handle() const
    {
        return *new T;
    }

    // specialization for shared_ptrs doesn't need return method
    template<class T>
    typename std::enable_if<std::is_base_of<Handle, T>::value, std::shared_ptr<Handle>>::type
    get_sh_ptr() const
    {
        return std::dynamic_pointer_cast<Handle>(
            std::shared_ptr<T>(
                new T,
                [](T* p)
                {
                    delete p;
                }));
    }

    template<class T>
    typename std::enable_if<std::is_base_of<Handle, T>::value, void>::type
    return_handle(
            T& h) const
    {
        delete &h;
    }

};

#endif // __TEST_UNITTEST_RTPS_SECURITY_SECURITYTESTS_HPP__
