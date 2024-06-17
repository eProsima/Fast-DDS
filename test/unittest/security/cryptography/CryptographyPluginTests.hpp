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

#ifndef _UNITTEST_SECURITY_CRYPTOGRAPHY_CRYPTOGRAPHYPLUGINTESTS_HPP_
#define _UNITTEST_SECURITY_CRYPTOGRAPHY_CRYPTOGRAPHYPLUGINTESTS_HPP_

#include <cstdlib>
#include <cstring>

#include <gtest/gtest.h>
#include <openssl/rand.h>

#include <fastdds/rtps/common/CDRMessage_t.hpp>

#include <security/accesscontrol/AccessPermissionsHandle.h>
#include <security/authentication/PKIIdentityHandle.h>
#include <security/cryptography/AESGCMGMAC.h>
#include <security/MockAccessControlPlugin.h>
#include <security/MockAuthenticationPlugin.h>

class CryptographyPluginTest : public ::testing::Test
{
protected:

    // Mock the handles to avoid cast issues
    using SharedSecretHandle = eprosima::fastdds::rtps::security::MockAuthenticationPlugin::SharedSecretHandle;
    using PKIIdentityHandle = eprosima::fastdds::rtps::security::MockAuthenticationPlugin::PKIIdentityHandle;
    using AccessPermissionsHandle =
            eprosima::fastdds::rtps::security::MockAccessControlPlugin::AccessPermissionsHandle;

    virtual void SetUp()
    {
        using namespace eprosima::fastdds::rtps::security;

        eprosima::fastdds::rtps::PropertyPolicy m_propertypolicy;

        CryptoPlugin = new eprosima::fastdds::rtps::security::AESGCMGMAC();

        // Delegate SharedSecret creation to an actual implementation
        ON_CALL(auth_plugin, get_shared_secret)
                .WillByDefault([this](
                    const HandshakeHandle&,
                    SecurityException&)
                {
                    return auth_plugin.get_dummy_shared_secret();
                });

        // Delegate SharedSecret disposal to an actual implementation
        ON_CALL(auth_plugin, return_sharedsecret_handle)
                .WillByDefault([this](
                    std::shared_ptr<SecretHandle>& sh,
                    SecurityException&)
                {
                    return auth_plugin.return_dummy_sharedsecret(sh);
                });

        // Delegate identity handle creation to an actual implementation
        ON_CALL(auth_plugin, get_identity_handle)
                .WillByDefault([this](SecurityException&)
                {
                    return auth_plugin.get_dummy_identity_handle();
                });

        // Delegate identity handle disposal to an actual implementation
        ON_CALL(auth_plugin, return_identity_handle)
                .WillByDefault([this](
                    IdentityHandle* ih,
                    SecurityException&)
                {
                    return auth_plugin.return_dummy_identity_handle(ih);
                });
    }

    virtual void TearDown()
    {
        delete CryptoPlugin;
    }

public:

    CryptographyPluginTest()
        : CryptoPlugin(nullptr)
    {
    }

    eprosima::fastdds::rtps::security::AESGCMGMAC* CryptoPlugin;
    ::testing::NiceMock<eprosima::fastdds::rtps::security::MockAuthenticationPlugin> auth_plugin;
    ::testing::NiceMock<eprosima::fastdds::rtps::security::MockAccessControlPlugin> access_plugin;
};

TEST_F(CryptographyPluginTest, factory_CreateLocalParticipantHandle)
{
    using namespace eprosima::fastdds::rtps::security;

    SecurityException exception;

    PKIIdentityHandle& i_handle =
            PKIIdentityHandle::narrow(*auth_plugin.get_identity_handle(exception));

    AccessPermissionsHandle& perm_handle =
            AccessPermissionsHandle::narrow(*access_plugin.get_permissions_handle(exception));

    eprosima::fastdds::rtps::PropertySeq prop_handle;
    ParticipantSecurityAttributes part_sec_attr;

    part_sec_attr.is_rtps_protected = true;
    part_sec_attr.plugin_participant_attributes = PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ENCRYPTED |
            PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ORIGIN_AUTHENTICATED;

    std::shared_ptr<ParticipantCryptoHandle> target =
            CryptoPlugin->keyfactory()->register_local_participant(i_handle, perm_handle, prop_handle, part_sec_attr,
                    exception);
    AESGCMGMAC_ParticipantCryptoHandle& local_participant = AESGCMGMAC_ParticipantCryptoHandle::narrow(*target);

    ASSERT_FALSE(local_participant.nil());

    ASSERT_GT(local_participant->Participant2ParticipantKeyMaterial.size(), 0ul);
    ASSERT_GT(local_participant->Participant2ParticipantKxKeyMaterial.size(), 0ul);

    ASSERT_TRUE((local_participant->ParticipantKeyMaterial.transformation_kind ==
            c_transfrom_kind_aes256_gcm));
    ASSERT_TRUE((local_participant->Participant2ParticipantKeyMaterial.at(0).transformation_kind ==
            c_transfrom_kind_aes256_gcm));
    ASSERT_TRUE((local_participant->Participant2ParticipantKxKeyMaterial.at(0).transformation_kind ==
            c_transfrom_kind_aes256_gcm));

    ASSERT_FALSE( std::all_of(local_participant->ParticipantKeyMaterial.master_salt.begin(),
            local_participant->ParticipantKeyMaterial.master_salt.end(), [](uint8_t i)
            {
                return i == 0;
            }));
    ASSERT_FALSE( std::all_of(local_participant->Participant2ParticipantKeyMaterial.at(0).master_salt.begin(),
            local_participant->Participant2ParticipantKeyMaterial.at(0).master_salt.end(), [](uint8_t i)
            {
                return i == 0;
            }));
    ASSERT_FALSE( std::all_of(local_participant->Participant2ParticipantKxKeyMaterial.at(0).master_salt.begin(),
            local_participant->Participant2ParticipantKxKeyMaterial.at(0).master_salt.end(), [](uint8_t i)
            {
                return i == 0;
            }));

    ASSERT_FALSE( std::all_of(local_participant->ParticipantKeyMaterial.master_sender_key.begin(),
            local_participant->ParticipantKeyMaterial.master_sender_key.end(), [](uint8_t i)
            {
                return i == 0;
            }));
    ASSERT_FALSE( std::all_of(local_participant->Participant2ParticipantKeyMaterial.at(0).master_sender_key.begin(),
            local_participant->Participant2ParticipantKeyMaterial.at(0).master_sender_key.end(), [](uint8_t i)
            {
                return i == 0;
            }));
    ASSERT_FALSE( std::all_of(local_participant->Participant2ParticipantKxKeyMaterial.at(0).master_sender_key.begin(),
            local_participant->Participant2ParticipantKxKeyMaterial.at(0).master_sender_key.end(), [](uint8_t i)
            {
                return i == 0;
            }));

    ASSERT_FALSE( std::any_of(local_participant->ParticipantKeyMaterial.receiver_specific_key_id.begin(),
            local_participant->ParticipantKeyMaterial.receiver_specific_key_id.end(), [](uint8_t i)
            {
                return i != 0;
            }));
    ASSERT_FALSE( std::all_of(local_participant->Participant2ParticipantKeyMaterial.at(0).receiver_specific_key_id.begin(),
            local_participant->Participant2ParticipantKeyMaterial.at(0).receiver_specific_key_id.end(), [](uint8_t i)
            {
                return i == 0;
            }));
    ASSERT_FALSE( std::all_of(local_participant->Participant2ParticipantKxKeyMaterial.at(0).receiver_specific_key_id.
                    begin(),
            local_participant->Participant2ParticipantKxKeyMaterial.at(0).receiver_specific_key_id.end(),
            [](uint8_t i)
            {
                return i == 0;
            }));

    //Release resources and check the handle is indeed empty
    auth_plugin.return_identity_handle(&i_handle, exception);
    access_plugin.return_permissions_handle(&perm_handle, exception);

    CryptoPlugin->keyfactory()->unregister_participant(target, exception);
}


TEST_F(CryptographyPluginTest, factory_RegisterRemoteParticipant)
{
    using namespace eprosima::fastdds::rtps::security;

    SecurityException exception;

    PKIIdentityHandle& i_handle =
            PKIIdentityHandle::narrow(*auth_plugin.get_identity_handle(exception));

    AccessPermissionsHandle& perm_handle =
            AccessPermissionsHandle::narrow(*access_plugin.get_permissions_handle(exception));

    eprosima::fastdds::rtps::PropertySeq prop_handle;
    ParticipantSecurityAttributes part_sec_attr;

    std::shared_ptr<SecretHandle> secret =
            auth_plugin.get_shared_secret(SharedSecretHandle::nil_handle, exception);

    std::shared_ptr<SharedSecretHandle> shared_secret = std::dynamic_pointer_cast<SharedSecretHandle>(secret);

    part_sec_attr.is_rtps_protected = true;
    part_sec_attr.plugin_participant_attributes = PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ENCRYPTED |
            PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ORIGIN_AUTHENTICATED;

    std::shared_ptr<ParticipantCryptoHandle> local =
            CryptoPlugin->keyfactory()->register_local_participant(i_handle, perm_handle, prop_handle, part_sec_attr,
                    exception);

    ASSERT_TRUE(local);

    //Dissect results to check correct creation

    //Fill shared secret with dummy values
    std::vector<uint8_t> dummy_data, challenge_1, challenge_2;
    SharedSecret::BinaryData binary_data;
    challenge_1.resize(32);
    challenge_2.resize(32);

    RAND_bytes(challenge_1.data(), 32);
    binary_data.name("Challenge1");
    binary_data.value(challenge_1);
    (*shared_secret)->data_.push_back(binary_data);

    RAND_bytes(challenge_2.data(), 32);
    binary_data.name("Challenge2");
    binary_data.value(challenge_2);
    (*shared_secret)->data_.push_back(binary_data);

    dummy_data.resize(32);
    RAND_bytes(dummy_data.data(), 32);
    binary_data.name("SharedSecret");
    binary_data.value(dummy_data);
    (*shared_secret)->data_.push_back(binary_data);

    std::shared_ptr<ParticipantCryptoHandle> remote_A =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*local, i_handle, perm_handle,
                    *shared_secret, exception);
    std::shared_ptr<ParticipantCryptoHandle> remote_B =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*local, i_handle, perm_handle,
                    *shared_secret, exception);

    ASSERT_TRUE(remote_A);
    ASSERT_TRUE(remote_B);

    AESGCMGMAC_ParticipantCryptoHandle& remote_participant_A =
            AESGCMGMAC_ParticipantCryptoHandle::narrow(*remote_A);
    AESGCMGMAC_ParticipantCryptoHandle& remote_participant_B =
            AESGCMGMAC_ParticipantCryptoHandle::narrow(*remote_B);

    //Check the presence of both remote P2PKeyMaterial and P2PKxKeyMaterial
    ASSERT_TRUE(remote_participant_A->Participant2ParticipantKeyMaterial.size() == 1);
    ASSERT_TRUE(remote_participant_B->Participant2ParticipantKeyMaterial.size() == 1);
    ASSERT_TRUE(remote_participant_A->Participant2ParticipantKxKeyMaterial.size() == 1);
    ASSERT_TRUE(remote_participant_B->Participant2ParticipantKxKeyMaterial.size() == 1);
    //Check that both remoteKeysMaterials have unique IDS (keys are the same since they use the same source material
    ASSERT_TRUE(remote_participant_A->Participant2ParticipantKeyMaterial.at(
                0).sender_key_id == remote_participant_B->Participant2ParticipantKeyMaterial.at(0).sender_key_id);
    //KxKeys should be the same since they derive from the same Shared Secret although Keys should not
    ASSERT_TRUE(remote_participant_A->Participant2ParticipantKeyMaterial.at(
                0).master_receiver_specific_key != remote_participant_B->Participant2ParticipantKeyMaterial.at(
                0).master_receiver_specific_key);
    ASSERT_TRUE(remote_participant_A->Participant2ParticipantKxKeyMaterial.at(
                0).master_sender_key == remote_participant_B->Participant2ParticipantKxKeyMaterial.at(
                0).master_sender_key);

    CryptoPlugin->keyfactory()->unregister_participant(remote_A, exception);
    CryptoPlugin->keyfactory()->unregister_participant(remote_B, exception);
    CryptoPlugin->keyfactory()->unregister_participant(local, exception);

    auth_plugin.return_identity_handle(&i_handle, exception);
    auth_plugin.return_sharedsecret_handle(secret, exception);
    access_plugin.return_permissions_handle(&perm_handle, exception);
}

TEST_F(CryptographyPluginTest, exchange_CDRSerializenDeserialize){

    using namespace eprosima::fastdds::rtps::security;

    SecurityException exception;

    PKIIdentityHandle& i_handle =
            PKIIdentityHandle::narrow(*auth_plugin.get_identity_handle(exception));

    AccessPermissionsHandle& perm_handle =
            AccessPermissionsHandle::narrow(*access_plugin.get_permissions_handle(exception));

    eprosima::fastdds::rtps::PropertySeq prop_handle;
    ParticipantSecurityAttributes part_sec_attr;

    part_sec_attr.is_rtps_protected = true;
    part_sec_attr.plugin_participant_attributes = PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ENCRYPTED |
            PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ORIGIN_AUTHENTICATED;

    std::shared_ptr<ParticipantCryptoHandle> ParticipantA =
            CryptoPlugin->keyfactory()->register_local_participant(i_handle, perm_handle, prop_handle, part_sec_attr,
                    exception);

    AESGCMGMAC_ParticipantCryptoHandle& Participant_A =
            AESGCMGMAC_ParticipantCryptoHandle::narrow(*ParticipantA);

    KeyMaterial_AES_GCM_GMAC base = Participant_A->ParticipantKeyMaterial;

    std::vector<uint8_t> serialized = CryptoPlugin->keyexchange()->KeyMaterialCDRSerialize(base);
    KeyMaterial_AES_GCM_GMAC result;
    CryptoPlugin->keyexchange()->KeyMaterialCDRDeserialize(result, &serialized);
    ASSERT_TRUE(
        (base.transformation_kind == result.transformation_kind) &&
        (base.master_salt == result.master_salt) &&
        (base.sender_key_id == result.sender_key_id) &&
        (base.master_sender_key == result.master_sender_key) &&
        (base.receiver_specific_key_id == result.receiver_specific_key_id) &&
        (base.master_receiver_specific_key == result.master_receiver_specific_key)
        );

    CryptoPlugin->keyfactory()->unregister_participant(ParticipantA, exception);

    auth_plugin.return_identity_handle(&i_handle, exception);
    access_plugin.return_permissions_handle(&perm_handle, exception);
}

TEST_F(CryptographyPluginTest, exchange_ParticipantCryptoTokens)
{
    using namespace eprosima::fastdds::rtps::security;

    SecurityException exception;

    PKIIdentityHandle& i_handle =
            PKIIdentityHandle::narrow(*auth_plugin.get_identity_handle(exception));

    AccessPermissionsHandle& perm_handle =
            AccessPermissionsHandle::narrow(*access_plugin.get_permissions_handle(exception));

    eprosima::fastdds::rtps::PropertySeq prop_handle;
    ParticipantSecurityAttributes part_sec_attr;

    std::shared_ptr<SecretHandle> secret =
            auth_plugin.get_shared_secret(SharedSecretHandle::nil_handle, exception);

    std::shared_ptr<SharedSecretHandle> shared_secret = std::dynamic_pointer_cast<SharedSecretHandle>(secret);

    part_sec_attr.is_rtps_protected = true;
    part_sec_attr.plugin_participant_attributes = PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ENCRYPTED |
            PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ORIGIN_AUTHENTICATED;

    //Fill shared secret with dummy values
    std::vector<uint8_t> dummy_data, challenge_1, challenge_2;
    SharedSecret::BinaryData binary_data;
    challenge_1.resize(32);
    challenge_2.resize(32);

    RAND_bytes(challenge_1.data(), 32);
    binary_data.name("Challenge1");
    binary_data.value(challenge_1);
    (*shared_secret)->data_.push_back(binary_data);

    RAND_bytes(challenge_2.data(), 32);
    binary_data.name("Challenge2");
    binary_data.value(challenge_2);
    (*shared_secret)->data_.push_back(binary_data);

    dummy_data.resize(32);
    RAND_bytes(dummy_data.data(), 32);
    binary_data.name("SharedSecret");
    binary_data.value(dummy_data);
    (*shared_secret)->data_.push_back(binary_data);

    //Create ParticipantA and ParticipantB
    std::shared_ptr<ParticipantCryptoHandle> ParticipantA =
            CryptoPlugin->keyfactory()->register_local_participant(i_handle, perm_handle, prop_handle, part_sec_attr,
                    exception);
    std::shared_ptr<ParticipantCryptoHandle> ParticipantB =
            CryptoPlugin->keyfactory()->register_local_participant(i_handle, perm_handle, prop_handle, part_sec_attr,
                    exception);

    ASSERT_TRUE(ParticipantA && ParticipantB);

    //Register a remote for both Participants
    std::shared_ptr<ParticipantCryptoHandle> ParticipantA_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*ParticipantA, i_handle, perm_handle,
                    *shared_secret, exception);
    std::shared_ptr<ParticipantCryptoHandle> ParticipantB_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*ParticipantB, i_handle, perm_handle,
                    *shared_secret, exception);

    //Create CryptoTokens for both Participants
    ParticipantCryptoTokenSeq ParticipantA_CryptoTokens, ParticipantB_CryptoTokens;

    ASSERT_TRUE(
        CryptoPlugin->keyexchange()->create_local_participant_crypto_tokens(ParticipantA_CryptoTokens, *ParticipantA,
        *ParticipantA_remote, exception)
        );
    ASSERT_TRUE(
        CryptoPlugin->keyexchange()->create_local_participant_crypto_tokens(ParticipantB_CryptoTokens, *ParticipantB,
        *ParticipantB_remote, exception)
        );

    //Set ParticipantA token into ParticipantB and viceversa
    ASSERT_TRUE(
        CryptoPlugin->keyexchange()->set_remote_participant_crypto_tokens(*ParticipantA, *ParticipantA_remote,
        ParticipantB_CryptoTokens, exception)
        );
    ASSERT_TRUE(
        CryptoPlugin->keyexchange()->set_remote_participant_crypto_tokens(*ParticipantB, *ParticipantB_remote,
        ParticipantA_CryptoTokens, exception)
        );

    //Check that ParticipantB's KeyMaterial is congruent with ParticipantA and viceversa
    AESGCMGMAC_ParticipantCryptoHandle& Participant_A_remote =
            AESGCMGMAC_ParticipantCryptoHandle::narrow(*ParticipantA_remote);
    AESGCMGMAC_ParticipantCryptoHandle& Participant_B_remote =
            AESGCMGMAC_ParticipantCryptoHandle::narrow(*ParticipantB_remote);

    ASSERT_TRUE(Participant_A_remote->RemoteParticipant2ParticipantKeyMaterial.size() == 1);
    ASSERT_TRUE(Participant_B_remote->RemoteParticipant2ParticipantKeyMaterial.size() == 1);
    ASSERT_TRUE(Participant_A_remote->Participant2ParticipantKeyMaterial.at(
                0).master_sender_key == Participant_B_remote->RemoteParticipant2ParticipantKeyMaterial.at(
                0).master_sender_key);
    ASSERT_TRUE(Participant_B_remote->Participant2ParticipantKeyMaterial.at(
                0).master_sender_key == Participant_A_remote->RemoteParticipant2ParticipantKeyMaterial.at(
                0).master_sender_key);

    CryptoPlugin->keyfactory()->unregister_participant(ParticipantA, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantB, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantA_remote, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantB_remote, exception);

    auth_plugin.return_identity_handle(&i_handle, exception);
    auth_plugin.return_sharedsecret_handle(secret, exception);
    access_plugin.return_permissions_handle(&perm_handle, exception);
}

TEST_F(CryptographyPluginTest, transform_RTPSMessage)
{
    using namespace eprosima::fastdds::rtps::security;

    SecurityException exception;

    PKIIdentityHandle& i_handle =
            PKIIdentityHandle::narrow(*auth_plugin.get_identity_handle(exception));

    AccessPermissionsHandle& perm_handle =
            AccessPermissionsHandle::narrow(*access_plugin.get_permissions_handle(exception));

    eprosima::fastdds::rtps::PropertySeq prop_handle;
    ParticipantSecurityAttributes part_sec_attr;

    std::shared_ptr<SecretHandle> secret =
            auth_plugin.get_shared_secret(SharedSecretHandle::nil_handle, exception);

    std::shared_ptr<SharedSecretHandle> shared_secret = std::dynamic_pointer_cast<SharedSecretHandle>(secret);

    part_sec_attr.is_rtps_protected = true;
    part_sec_attr.plugin_participant_attributes = PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ENCRYPTED |
            PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ORIGIN_AUTHENTICATED;

    //Fill shared secret with dummy values
    std::vector<uint8_t> dummy_data, challenge_1, challenge_2;
    SharedSecret::BinaryData binary_data;
    challenge_1.resize(32);
    challenge_2.resize(32);

    RAND_bytes(challenge_1.data(), 32);
    binary_data.name("Challenge1");
    binary_data.value(challenge_1);
    (*shared_secret)->data_.push_back(binary_data);

    RAND_bytes(challenge_2.data(), 32);
    binary_data.name("Challenge2");
    binary_data.value(challenge_2);
    (*shared_secret)->data_.push_back(binary_data);

    dummy_data.resize(32);
    RAND_bytes(dummy_data.data(), 32);
    binary_data.name("SharedSecret");
    binary_data.value(dummy_data);
    (*shared_secret)->data_.push_back(binary_data);

    //Create ParticipantA and ParticipantB
    std::shared_ptr<ParticipantCryptoHandle> ParticipantA =
            CryptoPlugin->keyfactory()->register_local_participant(i_handle, perm_handle, prop_handle, part_sec_attr,
                    exception);
    std::shared_ptr<ParticipantCryptoHandle> ParticipantB =
            CryptoPlugin->keyfactory()->register_local_participant(i_handle, perm_handle, prop_handle, part_sec_attr,
                    exception);

    ASSERT_TRUE(ParticipantA && ParticipantB);

    //Register a remote for both Participants
    std::shared_ptr<ParticipantCryptoHandle> ParticipantA_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*ParticipantA, i_handle, perm_handle,
                    *shared_secret, exception);
    std::shared_ptr<ParticipantCryptoHandle> ParticipantB_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*ParticipantB, i_handle, perm_handle,
                    *shared_secret, exception);

    //Create CryptoTokens for both Participants
    ParticipantCryptoTokenSeq ParticipantA_CryptoTokens, ParticipantB_CryptoTokens;

    CryptoPlugin->keyexchange()->create_local_participant_crypto_tokens(ParticipantA_CryptoTokens, *ParticipantA,
            *ParticipantA_remote, exception);
    CryptoPlugin->keyexchange()->create_local_participant_crypto_tokens(ParticipantB_CryptoTokens, *ParticipantB,
            *ParticipantB_remote, exception);

    //Set ParticipantA token into ParticipantB and viceversa
    CryptoPlugin->keyexchange()->set_remote_participant_crypto_tokens(*ParticipantA, *ParticipantA_remote,
            ParticipantB_CryptoTokens, exception);
    CryptoPlugin->keyexchange()->set_remote_participant_crypto_tokens(*ParticipantB, *ParticipantB_remote,
            ParticipantA_CryptoTokens, exception);

    //Perform sample message exchange
    eprosima::fastdds::rtps::CDRMessage_t plain_rtps_message(RTPSMESSAGE_DEFAULT_SIZE);
    eprosima::fastdds::rtps::CDRMessage_t encoded_rtps_message(RTPSMESSAGE_DEFAULT_SIZE);
    eprosima::fastdds::rtps::CDRMessage_t decoded_rtps_message(RTPSMESSAGE_DEFAULT_SIZE);

    char message[] = "RPTSMessage"; //Length 11
    memcpy(plain_rtps_message.buffer, message, 11);
    plain_rtps_message.length = 11;

    std::shared_ptr<ParticipantCryptoHandle> unintended_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*ParticipantA, i_handle, perm_handle,
                    *shared_secret, exception);
    std::vector<std::shared_ptr<ParticipantCryptoHandle>> receivers;

    //Send message to intended participant
    receivers.push_back(ParticipantA_remote);
    receivers.push_back(unintended_remote);
    for (int i = 0; i < 50; i++)
    {
        ASSERT_TRUE(CryptoPlugin->cryptotransform()->encode_rtps_message(encoded_rtps_message, plain_rtps_message,
                *ParticipantA, receivers, exception));
        encoded_rtps_message.pos = 0;
        ASSERT_TRUE(CryptoPlugin->cryptotransform()->decode_rtps_message(decoded_rtps_message, encoded_rtps_message,
                *ParticipantB, *ParticipantB_remote, exception));
        ASSERT_TRUE(plain_rtps_message.length == decoded_rtps_message.length);
        ASSERT_TRUE(memcmp(plain_rtps_message.buffer, decoded_rtps_message.buffer, decoded_rtps_message.length) == 0);
        plain_rtps_message.pos = 0;
        encoded_rtps_message.pos = 0;
        encoded_rtps_message.length = 0;
        decoded_rtps_message.pos = 0;
        decoded_rtps_message.length = 0;
    }
    //Send message to unintended participant

    receivers.clear();
    receivers.push_back(unintended_remote);
    ASSERT_TRUE(CryptoPlugin->cryptotransform()->encode_rtps_message(encoded_rtps_message, plain_rtps_message,
            *ParticipantA, receivers, exception));
    encoded_rtps_message.pos = 0;
    ASSERT_FALSE(CryptoPlugin->cryptotransform()->decode_rtps_message(decoded_rtps_message, encoded_rtps_message,
            *ParticipantB, *ParticipantB_remote, exception));
    plain_rtps_message.pos = 0;
    encoded_rtps_message.pos = 0;
    encoded_rtps_message.length = 0;
    decoded_rtps_message.pos = 0;
    decoded_rtps_message.length = 0;


    CryptoPlugin->keyfactory()->unregister_participant(ParticipantA, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantB, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantA_remote, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantB_remote, exception);
    CryptoPlugin->keyfactory()->unregister_participant(unintended_remote, exception);

    //Now lets do the same with 128GCM
    //Fill prop_handle with info about the new mode we want
    eprosima::fastdds::rtps::Property prop1;
    prop1.name("dds.sec.crypto.keysize");
    prop1.value("128");
    prop_handle.push_back(prop1);
    eprosima::fastdds::rtps::Property prop2;
    prop2.name("dds.sec.crypto.maxblockspersession");
    prop2.value("16");
    prop_handle.push_back(prop2);
    //Create ParticipantA and ParticipantB
    ParticipantA = CryptoPlugin->keyfactory()->register_local_participant(i_handle, perm_handle, prop_handle,
                    part_sec_attr, exception);
    ParticipantB = CryptoPlugin->keyfactory()->register_local_participant(i_handle, perm_handle, prop_handle,
                    part_sec_attr, exception);

    //Register a remote for both Participants
    ParticipantA_remote = CryptoPlugin->keyfactory()->register_matched_remote_participant(*ParticipantA, i_handle,
                    perm_handle, *shared_secret,
                    exception);
    ParticipantB_remote = CryptoPlugin->keyfactory()->register_matched_remote_participant(*ParticipantB, i_handle,
                    perm_handle, *shared_secret,
                    exception);

    //Create CryptoTokens for both Participants
    CryptoPlugin->keyexchange()->create_local_participant_crypto_tokens(ParticipantA_CryptoTokens, *ParticipantA,
            *ParticipantA_remote, exception);
    CryptoPlugin->keyexchange()->create_local_participant_crypto_tokens(ParticipantB_CryptoTokens, *ParticipantB,
            *ParticipantB_remote, exception);

    //Set ParticipantA token into ParticipantB and viceversa
    CryptoPlugin->keyexchange()->set_remote_participant_crypto_tokens(*ParticipantA, *ParticipantA_remote,
            ParticipantB_CryptoTokens, exception);
    CryptoPlugin->keyexchange()->set_remote_participant_crypto_tokens(*ParticipantB, *ParticipantB_remote,
            ParticipantA_CryptoTokens, exception);

    unintended_remote = CryptoPlugin->keyfactory()->register_matched_remote_participant(*ParticipantA, i_handle,
                    perm_handle, *shared_secret,
                    exception);

    //Perform sample message exchange
    receivers.clear();

    //Send message to intended participant
    receivers.push_back(ParticipantA_remote);
    receivers.push_back(unintended_remote);
    ASSERT_TRUE(CryptoPlugin->cryptotransform()->encode_rtps_message(encoded_rtps_message, plain_rtps_message,
            *ParticipantA, receivers, exception));
    encoded_rtps_message.pos = 0;
    ASSERT_TRUE(CryptoPlugin->cryptotransform()->decode_rtps_message(decoded_rtps_message, encoded_rtps_message,
            *ParticipantB, *ParticipantB_remote, exception));
    ASSERT_TRUE(plain_rtps_message.length == decoded_rtps_message.length);
    ASSERT_TRUE(memcmp(plain_rtps_message.buffer, decoded_rtps_message.buffer, decoded_rtps_message.length) == 0);


    CryptoPlugin->keyfactory()->unregister_participant(unintended_remote, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantA, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantB, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantA_remote, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantB_remote, exception);

    auth_plugin.return_identity_handle(&i_handle, exception);
    auth_plugin.return_sharedsecret_handle(secret, exception);
    access_plugin.return_permissions_handle(&perm_handle, exception);
}

TEST_F(CryptographyPluginTest, factory_CreateLocalWriterHandle)
{
    using namespace eprosima::fastdds::rtps::security;

    SecurityException exception;

    PKIIdentityHandle& i_handle =
            PKIIdentityHandle::narrow(*auth_plugin.get_identity_handle(exception));

    AccessPermissionsHandle& perm_handle =
            AccessPermissionsHandle::narrow(*access_plugin.get_permissions_handle(exception));

    eprosima::fastdds::rtps::PropertySeq prop_handle;
    ParticipantSecurityAttributes part_sec_attr;

    EndpointSecurityAttributes sec_attrs;

    std::shared_ptr<SecretHandle> secret =
            auth_plugin.get_shared_secret(SharedSecretHandle::nil_handle, exception);

    std::shared_ptr<SharedSecretHandle> shared_secret = std::dynamic_pointer_cast<SharedSecretHandle>(secret);

    part_sec_attr.is_rtps_protected = true;
    part_sec_attr.plugin_participant_attributes = PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ENCRYPTED |
            PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ORIGIN_AUTHENTICATED;

    sec_attrs.is_submessage_protected = true;
    sec_attrs.is_payload_protected = false;
    sec_attrs.is_key_protected = false;
    sec_attrs.plugin_endpoint_attributes = PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED |
            PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED;

    std::shared_ptr<ParticipantCryptoHandle> participant =
            CryptoPlugin->keyfactory()->register_local_participant(i_handle, perm_handle, prop_handle, part_sec_attr,
                    exception);
    DatawriterCryptoHandle* target =
            CryptoPlugin->keyfactory()->register_local_datawriter(*participant, prop_handle, sec_attrs, exception);
    ASSERT_TRUE(target != nullptr);

    AESGCMGMAC_WriterCryptoHandle& local_writer = AESGCMGMAC_WriterCryptoHandle::narrow(*target);
    ASSERT_TRUE(!local_writer.nil());

    ASSERT_TRUE(local_writer->Entity2RemoteKeyMaterial.empty());
    ASSERT_TRUE((local_writer->EntityKeyMaterial.at(0).transformation_kind == c_transfrom_kind_aes256_gcm));

    ASSERT_FALSE( std::all_of(local_writer->EntityKeyMaterial.at(0).master_salt.begin(),
            local_writer->EntityKeyMaterial.at(0).master_salt.end(), [](uint8_t i)
            {
                return i == 0;
            }));

    ASSERT_FALSE( std::all_of(local_writer->EntityKeyMaterial.at(0).master_sender_key.begin(),
            local_writer->EntityKeyMaterial.at(0).master_sender_key.end(), [](uint8_t i)
            {
                return i == 0;
            }));

    ASSERT_FALSE( std::any_of(local_writer->EntityKeyMaterial.at(0).receiver_specific_key_id.begin(),
            local_writer->EntityKeyMaterial.at(0).receiver_specific_key_id.end(), [](uint8_t i)
            {
                return i != 0;
            }));

    ASSERT_FALSE( std::any_of(local_writer->EntityKeyMaterial.at(0).master_receiver_specific_key.begin(),
            local_writer->EntityKeyMaterial.at(0).master_receiver_specific_key.end(), [](uint8_t i)
            {
                return i != 0;
            }));

    //Release resources and check the handle is indeed empty
    auth_plugin.return_identity_handle(&i_handle, exception);
    auth_plugin.return_sharedsecret_handle(secret, exception);
    access_plugin.return_permissions_handle(&perm_handle, exception);

    CryptoPlugin->keyfactory()->unregister_datawriter(target, exception);
    CryptoPlugin->keyfactory()->unregister_participant(participant, exception);
}

TEST_F(CryptographyPluginTest, factory_CreateLocalReaderHandle)
{
    using namespace eprosima::fastdds::rtps::security;

    SecurityException exception;

    PKIIdentityHandle& i_handle =
            PKIIdentityHandle::narrow(*auth_plugin.get_identity_handle(exception));

    AccessPermissionsHandle& perm_handle =
            AccessPermissionsHandle::narrow(*access_plugin.get_permissions_handle(exception));

    eprosima::fastdds::rtps::PropertySeq prop_handle;
    ParticipantSecurityAttributes part_sec_attr;

    EndpointSecurityAttributes sec_attrs;

    std::shared_ptr<SecretHandle> secret =
            auth_plugin.get_shared_secret(SharedSecretHandle::nil_handle, exception);

    std::shared_ptr<SharedSecretHandle> shared_secret = std::dynamic_pointer_cast<SharedSecretHandle>(secret);

    part_sec_attr.is_rtps_protected = true;
    part_sec_attr.plugin_participant_attributes = PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ENCRYPTED |
            PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ORIGIN_AUTHENTICATED;

    sec_attrs.is_submessage_protected = true;
    sec_attrs.is_payload_protected = false;
    sec_attrs.is_key_protected = false;
    sec_attrs.plugin_endpoint_attributes = PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED |
            PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED;

    std::shared_ptr<ParticipantCryptoHandle> participant =
            CryptoPlugin->keyfactory()->register_local_participant(i_handle, perm_handle, prop_handle, part_sec_attr,
                    exception);
    DatareaderCryptoHandle* target =
            CryptoPlugin->keyfactory()->register_local_datareader(*participant, prop_handle, sec_attrs, exception);
    ASSERT_TRUE(target != nullptr);

    AESGCMGMAC_ReaderCryptoHandle& local_reader = AESGCMGMAC_ReaderCryptoHandle::narrow(*target);
    ASSERT_TRUE(!local_reader.nil());

    ASSERT_TRUE(local_reader->Entity2RemoteKeyMaterial.empty());
    ASSERT_TRUE((local_reader->EntityKeyMaterial.at(0).transformation_kind == c_transfrom_kind_aes256_gcm));

    ASSERT_FALSE( std::all_of(local_reader->EntityKeyMaterial.at(0).master_salt.begin(),
            local_reader->EntityKeyMaterial.at(0).master_salt.end(), [](uint8_t i)
            {
                return i == 0;
            }));

    ASSERT_FALSE( std::all_of(local_reader->EntityKeyMaterial.at(0).master_sender_key.begin(),
            local_reader->EntityKeyMaterial.at(0).master_sender_key.end(), [](uint8_t i)
            {
                return i == 0;
            }));

    ASSERT_FALSE( std::any_of(local_reader->EntityKeyMaterial.at(0).receiver_specific_key_id.begin(),
            local_reader->EntityKeyMaterial.at(0).receiver_specific_key_id.end(), [](uint8_t i)
            {
                return i != 0;
            }));

    ASSERT_FALSE( std::any_of(local_reader->EntityKeyMaterial.at(0).master_receiver_specific_key.begin(),
            local_reader->EntityKeyMaterial.at(0).master_receiver_specific_key.end(), [](uint8_t i)
            {
                return i != 0;
            }));

    //Release resources and check the handle is indeed empty
    auth_plugin.return_identity_handle(&i_handle, exception);
    auth_plugin.return_sharedsecret_handle(secret, exception);
    access_plugin.return_permissions_handle(&perm_handle, exception);

    CryptoPlugin->keyfactory()->unregister_datareader(target, exception);
    CryptoPlugin->keyfactory()->unregister_participant(participant, exception);
}

TEST_F(CryptographyPluginTest, factory_RegisterRemoteReaderWriter)
{
    using namespace eprosima::fastdds::rtps::security;

    SecurityException exception;

    PKIIdentityHandle& i_handle =
            PKIIdentityHandle::narrow(*auth_plugin.get_identity_handle(exception));

    AccessPermissionsHandle& perm_handle =
            AccessPermissionsHandle::narrow(*access_plugin.get_permissions_handle(exception));

    eprosima::fastdds::rtps::PropertySeq prop_handle;
    ParticipantSecurityAttributes part_sec_attr;

    EndpointSecurityAttributes sec_attrs;

    std::shared_ptr<SecretHandle> secret =
            auth_plugin.get_shared_secret(SharedSecretHandle::nil_handle, exception);

    std::shared_ptr<SharedSecretHandle> shared_secret = std::dynamic_pointer_cast<SharedSecretHandle>(secret);

    part_sec_attr.is_rtps_protected = true;
    part_sec_attr.plugin_participant_attributes = PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ENCRYPTED |
            PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ORIGIN_AUTHENTICATED;

    sec_attrs.is_submessage_protected = true;
    sec_attrs.is_payload_protected = false;
    sec_attrs.is_key_protected = false;
    sec_attrs.plugin_endpoint_attributes = PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED |
            PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED;

    std::shared_ptr<ParticipantCryptoHandle> participant_A =
            CryptoPlugin->keyfactory()->register_local_participant(i_handle, perm_handle, prop_handle, part_sec_attr,
                    exception);
    std::shared_ptr<ParticipantCryptoHandle> participant_B =
            CryptoPlugin->keyfactory()->register_local_participant(i_handle, perm_handle, prop_handle, part_sec_attr,
                    exception);

    DatareaderCryptoHandle* reader =
            CryptoPlugin->keyfactory()->register_local_datareader(*participant_A, prop_handle, sec_attrs, exception);
    DatareaderCryptoHandle* writer =
            CryptoPlugin->keyfactory()->register_local_datawriter(*participant_B, prop_handle, sec_attrs, exception);

    //Fill shared secret with dummy values
    std::vector<uint8_t> dummy_data, challenge_1, challenge_2;
    SharedSecret::BinaryData binary_data;
    challenge_1.resize(32);
    challenge_2.resize(32);

    RAND_bytes(challenge_1.data(), 32);
    binary_data.name("Challenge1");
    binary_data.value(challenge_1);
    (*shared_secret)->data_.push_back(binary_data);

    RAND_bytes(challenge_2.data(), 32);
    binary_data.name("Challenge2");
    binary_data.value(challenge_2);
    (*shared_secret)->data_.push_back(binary_data);

    dummy_data.resize(32);
    RAND_bytes(dummy_data.data(), 32);
    binary_data.name("SharedSecret");
    binary_data.value(dummy_data);
    (*shared_secret)->data_.push_back(binary_data);

    //Register a remote for both Participants
    std::shared_ptr<ParticipantCryptoHandle> ParticipantA_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_A, i_handle, perm_handle,
                    *shared_secret, exception);
    std::shared_ptr<ParticipantCryptoHandle> ParticipantB_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_B, i_handle, perm_handle,
                    *shared_secret, exception);

    //Register DataReader with DataWriter
    DatareaderCryptoHandle* remote_reader =
            CryptoPlugin->keyfactory()->register_matched_remote_datareader(*writer, *ParticipantB_remote,
                    *shared_secret, false, exception);
    ASSERT_TRUE(remote_reader != nullptr);


    //Register DataWriter with DataReader
    DatawriterCryptoHandle* remote_writer =
            CryptoPlugin->keyfactory()->register_matched_remote_datawriter(*reader, *ParticipantA_remote,
                    *shared_secret, exception);
    ASSERT_TRUE(remote_writer != nullptr);

    auth_plugin.return_identity_handle(&i_handle, exception);
    auth_plugin.return_sharedsecret_handle(secret, exception);
    access_plugin.return_permissions_handle(&perm_handle, exception);

    CryptoPlugin->keyfactory()->unregister_datawriter(writer, exception);
    CryptoPlugin->keyfactory()->unregister_datareader(reader, exception);

    CryptoPlugin->keyfactory()->unregister_datareader(remote_reader, exception);
    CryptoPlugin->keyfactory()->unregister_datawriter(remote_writer, exception);

    CryptoPlugin->keyfactory()->unregister_participant(participant_A, exception);
    CryptoPlugin->keyfactory()->unregister_participant(participant_B, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantA_remote, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantB_remote, exception);
}

TEST_F(CryptographyPluginTest, exchange_ReaderWriterCryptoTokens)
{
    using namespace eprosima::fastdds::rtps::security;

    // Participant A owns Writer
    // Participant B owns Reader

    SecurityException exception;

    PKIIdentityHandle& i_handle =
            PKIIdentityHandle::narrow(*auth_plugin.get_identity_handle(exception));

    AccessPermissionsHandle& perm_handle =
            AccessPermissionsHandle::narrow(*access_plugin.get_permissions_handle(exception));

    eprosima::fastdds::rtps::PropertySeq prop_handle;
    ParticipantSecurityAttributes part_sec_attr;

    EndpointSecurityAttributes sec_attrs;

    std::shared_ptr<SecretHandle> secret =
            auth_plugin.get_shared_secret(SharedSecretHandle::nil_handle, exception);

    std::shared_ptr<SharedSecretHandle> shared_secret = std::dynamic_pointer_cast<SharedSecretHandle>(secret);

    part_sec_attr.is_rtps_protected = true;
    part_sec_attr.plugin_participant_attributes = PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ENCRYPTED |
            PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ORIGIN_AUTHENTICATED;

    sec_attrs.is_submessage_protected = true;
    sec_attrs.is_payload_protected = false;
    sec_attrs.is_key_protected = false;
    sec_attrs.plugin_endpoint_attributes = PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED |
            PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED;

    std::shared_ptr<ParticipantCryptoHandle> participant_A =
            CryptoPlugin->keyfactory()->register_local_participant(i_handle, perm_handle, prop_handle, part_sec_attr,
                    exception);
    std::shared_ptr<ParticipantCryptoHandle> participant_B =
            CryptoPlugin->keyfactory()->register_local_participant(i_handle, perm_handle, prop_handle, part_sec_attr,
                    exception);

    DatareaderCryptoHandle* reader =
            CryptoPlugin->keyfactory()->register_local_datareader(*participant_A, prop_handle, sec_attrs, exception);
    DatareaderCryptoHandle* writer =
            CryptoPlugin->keyfactory()->register_local_datawriter(*participant_B, prop_handle, sec_attrs, exception);

    //Fill shared secret with dummy values
    std::vector<uint8_t> dummy_data, challenge_1, challenge_2;
    SharedSecret::BinaryData binary_data;
    challenge_1.resize(32);
    challenge_2.resize(32);

    RAND_bytes(challenge_1.data(), 32);
    binary_data.name("Challenge1");
    binary_data.value(challenge_1);
    (*shared_secret)->data_.push_back(binary_data);

    RAND_bytes(challenge_2.data(), 32);
    binary_data.name("Challenge2");
    binary_data.value(challenge_2);
    (*shared_secret)->data_.push_back(binary_data);

    dummy_data.resize(32);
    RAND_bytes(dummy_data.data(), 32);
    binary_data.name("SharedSecret");
    binary_data.value(dummy_data);
    (*shared_secret)->data_.push_back(binary_data);

    //Register a remote for both Participants
    std::shared_ptr<ParticipantCryptoHandle> ParticipantA_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_A, i_handle, perm_handle,
                    *shared_secret, exception);
    std::shared_ptr<ParticipantCryptoHandle> ParticipantB_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_B, i_handle, perm_handle,
                    *shared_secret, exception);

    //Register DataReader with DataWriter
    DatareaderCryptoHandle* remote_reader =
            CryptoPlugin->keyfactory()->register_matched_remote_datareader(*writer, *ParticipantB_remote,
                    *shared_secret, false, exception);

    //Register DataWriter with DataReader
    DatawriterCryptoHandle* remote_writer =
            CryptoPlugin->keyfactory()->register_matched_remote_datawriter(*reader, *ParticipantA_remote,
                    *shared_secret, exception);

    //Create CryptoTokens for both Participants
    ParticipantCryptoTokenSeq ParticipantA_CryptoTokens, ParticipantB_CryptoTokens;

    CryptoPlugin->keyexchange()->create_local_participant_crypto_tokens(ParticipantA_CryptoTokens, *participant_A,
            *ParticipantA_remote, exception);
    CryptoPlugin->keyexchange()->create_local_participant_crypto_tokens(ParticipantB_CryptoTokens, *participant_B,
            *ParticipantB_remote, exception);

    //Set ParticipantA token into ParticipantB and viceversa
    CryptoPlugin->keyexchange()->set_remote_participant_crypto_tokens(*participant_A, *ParticipantA_remote,
            ParticipantB_CryptoTokens, exception);
    CryptoPlugin->keyexchange()->set_remote_participant_crypto_tokens(*participant_B, *ParticipantB_remote,
            ParticipantA_CryptoTokens, exception);

    //Create CryptoTokens for the DataWriter and DataReader
    DatawriterCryptoTokenSeq Writer_CryptoTokens, Reader_CryptoTokens;

    ASSERT_TRUE(
        CryptoPlugin->keyexchange()->create_local_datawriter_crypto_tokens(Writer_CryptoTokens, *writer, *remote_reader,
        exception)
        );
    ASSERT_TRUE(
        CryptoPlugin->keyexchange()->create_local_datareader_crypto_tokens(Reader_CryptoTokens, *reader, *remote_writer,
        exception)
        );

    //Exchange Datareader and Datawriter Cryptotokens

    ASSERT_TRUE(
        CryptoPlugin->keyexchange()->set_remote_datareader_crypto_tokens(*writer, *remote_reader, Reader_CryptoTokens,
        exception)
        );
    ASSERT_TRUE(
        CryptoPlugin->keyexchange()->set_remote_datawriter_crypto_tokens(*reader, *remote_writer, Writer_CryptoTokens,
        exception)
        );

    //Check contents
    AESGCMGMAC_WriterCryptoHandle& WriterH = AESGCMGMAC_WriterCryptoHandle::narrow(*writer);
    AESGCMGMAC_ReaderCryptoHandle& ReaderH = AESGCMGMAC_ReaderCryptoHandle::narrow(*reader);

    ASSERT_TRUE(WriterH->Remote2EntityKeyMaterial.size() == 1);
    ASSERT_TRUE(ReaderH->Remote2EntityKeyMaterial.size() == 1);
    ASSERT_TRUE(WriterH->Entity2RemoteKeyMaterial.at(0).master_sender_key ==
            ReaderH->Remote2EntityKeyMaterial.at(0).master_sender_key);
    ASSERT_TRUE(ReaderH->Entity2RemoteKeyMaterial.at(0).master_sender_key ==
            WriterH->Remote2EntityKeyMaterial.at(0).master_sender_key);

    //Release resources and check the handle is indeed empty
    auth_plugin.return_identity_handle(&i_handle, exception);
    auth_plugin.return_sharedsecret_handle(secret, exception);
    access_plugin.return_permissions_handle(&perm_handle, exception);

    CryptoPlugin->keyfactory()->unregister_datawriter(writer, exception);
    CryptoPlugin->keyfactory()->unregister_datawriter(remote_writer, exception);
    CryptoPlugin->keyfactory()->unregister_datareader(reader, exception);
    CryptoPlugin->keyfactory()->unregister_datareader(remote_reader, exception);

    CryptoPlugin->keyfactory()->unregister_participant(participant_A, exception);
    CryptoPlugin->keyfactory()->unregister_participant(participant_B, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantA_remote, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantB_remote, exception);
}

TEST_F(CryptographyPluginTest, transform_SerializedPayload)
{
    using namespace eprosima::fastdds::rtps::security;

    // Participant A owns Writer
    // Participant B owns Reader

    SecurityException exception;

    PKIIdentityHandle& i_handle =
            PKIIdentityHandle::narrow(*auth_plugin.get_identity_handle(exception));

    AccessPermissionsHandle& perm_handle =
            AccessPermissionsHandle::narrow(*access_plugin.get_permissions_handle(exception));

    eprosima::fastdds::rtps::PropertySeq prop_handle;
    ParticipantSecurityAttributes part_sec_attr;

    EndpointSecurityAttributes sec_attrs;

    std::shared_ptr<SecretHandle> secret =
            auth_plugin.get_shared_secret(SharedSecretHandle::nil_handle, exception);

    std::shared_ptr<SharedSecretHandle> shared_secret = std::dynamic_pointer_cast<SharedSecretHandle>(secret);

    part_sec_attr.is_rtps_protected = true;
    part_sec_attr.plugin_participant_attributes = PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ENCRYPTED |
            PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ORIGIN_AUTHENTICATED;

    sec_attrs.is_submessage_protected = true;
    sec_attrs.is_payload_protected = true;
    sec_attrs.is_key_protected = true;
    sec_attrs.plugin_endpoint_attributes = PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED |
            PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED |
            PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_PAYLOAD_ENCRYPTED;

    std::shared_ptr<ParticipantCryptoHandle> participant_A =
            CryptoPlugin->keyfactory()->register_local_participant(i_handle, perm_handle, prop_handle, part_sec_attr,
                    exception);
    std::shared_ptr<ParticipantCryptoHandle> participant_B =
            CryptoPlugin->keyfactory()->register_local_participant(i_handle, perm_handle, prop_handle, part_sec_attr,
                    exception);

    DatareaderCryptoHandle* reader =
            CryptoPlugin->keyfactory()->register_local_datareader(*participant_A, prop_handle, sec_attrs, exception);
    DatareaderCryptoHandle* writer =
            CryptoPlugin->keyfactory()->register_local_datawriter(*participant_B, prop_handle, sec_attrs, exception);

    //Fill shared secret with dummy values
    std::vector<uint8_t> dummy_data, challenge_1, challenge_2;
    SharedSecret::BinaryData binary_data;
    challenge_1.resize(32);
    challenge_2.resize(32);

    RAND_bytes(challenge_1.data(), 32);
    binary_data.name("Challenge1");
    binary_data.value(challenge_1);
    (*shared_secret)->data_.push_back(binary_data);

    RAND_bytes(challenge_2.data(), 32);
    binary_data.name("Challenge2");
    binary_data.value(challenge_2);
    (*shared_secret)->data_.push_back(binary_data);

    dummy_data.resize(32);
    RAND_bytes(dummy_data.data(), 32);
    binary_data.name("SharedSecret");
    binary_data.value(dummy_data);
    (*shared_secret)->data_.push_back(binary_data);

    //Register a remote for both Participants
    std::shared_ptr<ParticipantCryptoHandle> ParticipantA_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_A, i_handle, perm_handle,
                    *shared_secret, exception);
    std::shared_ptr<ParticipantCryptoHandle> ParticipantB_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_B, i_handle, perm_handle,
                    *shared_secret, exception);

    //Register DataReader with DataWriter
    DatareaderCryptoHandle* remote_reader =
            CryptoPlugin->keyfactory()->register_matched_remote_datareader(*writer, *ParticipantB_remote,
                    *shared_secret, false, exception);

    //Register DataWriter with DataReader
    DatawriterCryptoHandle* remote_writer =
            CryptoPlugin->keyfactory()->register_matched_remote_datawriter(*reader, *ParticipantA_remote,
                    *shared_secret, exception);

    //Create CryptoTokens for both Participants
    ParticipantCryptoTokenSeq ParticipantA_CryptoTokens, ParticipantB_CryptoTokens;

    CryptoPlugin->keyexchange()->create_local_participant_crypto_tokens(ParticipantA_CryptoTokens, *participant_A,
            *ParticipantA_remote, exception);
    CryptoPlugin->keyexchange()->create_local_participant_crypto_tokens(ParticipantB_CryptoTokens, *participant_B,
            *ParticipantB_remote, exception);

    //Set ParticipantA token into ParticipantB and viceversa
    CryptoPlugin->keyexchange()->set_remote_participant_crypto_tokens(*participant_A, *ParticipantA_remote,
            ParticipantB_CryptoTokens, exception);
    CryptoPlugin->keyexchange()->set_remote_participant_crypto_tokens(*participant_B, *ParticipantB_remote,
            ParticipantA_CryptoTokens, exception);

    //Create CryptoTokens for the DataWriter and DataReader
    DatawriterCryptoTokenSeq Writer_CryptoTokens, Reader_CryptoTokens;

    CryptoPlugin->keyexchange()->create_local_datawriter_crypto_tokens(Writer_CryptoTokens, *writer, *remote_reader,
            exception);
    CryptoPlugin->keyexchange()->create_local_datareader_crypto_tokens(Reader_CryptoTokens, *reader, *remote_writer,
            exception);

    //Exchange Datareader and Datawriter Cryptotokens
    CryptoPlugin->keyexchange()->set_remote_datareader_crypto_tokens(*writer, *remote_reader, Reader_CryptoTokens,
            exception);
    CryptoPlugin->keyexchange()->set_remote_datawriter_crypto_tokens(*reader, *remote_writer, Writer_CryptoTokens,
            exception);

    //Perform sample message exchange
    eprosima::fastdds::rtps::SerializedPayload_t plain_payload(18); // Message will have 18 length.
    eprosima::fastdds::rtps::SerializedPayload_t encoded_payload(100);
    // Message will have 18 length + cipher block size.
    eprosima::fastdds::rtps::SerializedPayload_t decoded_payload(18 + 32);

    char message[] = "My goose is cooked"; //Length 18
    memcpy(plain_payload.data, message, 18);
    plain_payload.length = 18;

    std::vector<uint8_t> inline_qos;

    //Send message to intended participant
    ASSERT_TRUE(CryptoPlugin->cryptotransform()->encode_serialized_payload(encoded_payload, inline_qos, plain_payload,
            *writer, exception));
    encoded_payload.pos = 0;
    ASSERT_TRUE(CryptoPlugin->cryptotransform()->decode_serialized_payload(decoded_payload, encoded_payload, inline_qos,
            *reader, *remote_writer, exception));
    ASSERT_TRUE(memcmp(plain_payload.data, decoded_payload.data, 18) == 0);
    plain_payload.pos = 0;
    encoded_payload.pos = 0;
    encoded_payload.length = 0;
    decoded_payload.pos = 0;
    decoded_payload.length = 0;

    CryptoPlugin->keyfactory()->unregister_datawriter(writer, exception);
    CryptoPlugin->keyfactory()->unregister_datawriter(remote_writer, exception);

    CryptoPlugin->keyfactory()->unregister_datareader(reader, exception);
    CryptoPlugin->keyfactory()->unregister_datareader(remote_reader, exception);

    CryptoPlugin->keyfactory()->unregister_participant(participant_A, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantA_remote, exception);
    CryptoPlugin->keyfactory()->unregister_participant(participant_B, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantB_remote, exception);

    //Lets do it with the 128 version
    eprosima::fastdds::rtps::Property prop1;
    prop1.name("dds.sec.crypto.keysize");
    prop1.value("128");
    prop_handle.push_back(prop1);
    eprosima::fastdds::rtps::Property prop2;
    prop2.name("dds.sec.crypto.maxblockspersession");
    prop2.value("16");
    prop_handle.push_back(prop2);

    participant_A = CryptoPlugin->keyfactory()->register_local_participant(i_handle, perm_handle, prop_handle,
                    part_sec_attr, exception);
    participant_B = CryptoPlugin->keyfactory()->register_local_participant(i_handle, perm_handle, prop_handle,
                    part_sec_attr, exception);

    reader = CryptoPlugin->keyfactory()->register_local_datareader(*participant_A, prop_handle, sec_attrs, exception);
    writer = CryptoPlugin->keyfactory()->register_local_datawriter(*participant_B, prop_handle, sec_attrs, exception);

    //Register a remote for both Participants
    ParticipantA_remote = CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_A, i_handle,
                    perm_handle, *shared_secret,
                    exception);
    ParticipantB_remote = CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_B, i_handle,
                    perm_handle, *shared_secret,
                    exception);
    //Register DataReader with DataWriter
    remote_reader = CryptoPlugin->keyfactory()->register_matched_remote_datareader(*writer, *ParticipantB_remote,
                    *shared_secret, false, exception);
    //Register DataWriter with DataReader
    remote_writer = CryptoPlugin->keyfactory()->register_matched_remote_datawriter(*reader, *ParticipantA_remote,
                    *shared_secret, exception);

    CryptoPlugin->keyexchange()->create_local_participant_crypto_tokens(ParticipantA_CryptoTokens, *participant_A,
            *ParticipantA_remote, exception);
    CryptoPlugin->keyexchange()->create_local_participant_crypto_tokens(ParticipantB_CryptoTokens, *participant_B,
            *ParticipantB_remote, exception);

    //Set ParticipantA token into ParticipantB and viceversa
    CryptoPlugin->keyexchange()->set_remote_participant_crypto_tokens(*participant_A, *ParticipantA_remote,
            ParticipantB_CryptoTokens, exception);
    CryptoPlugin->keyexchange()->set_remote_participant_crypto_tokens(*participant_B, *ParticipantB_remote,
            ParticipantA_CryptoTokens, exception);

    //Create CryptoTokens for the DataWriter and DataReader
    CryptoPlugin->keyexchange()->create_local_datawriter_crypto_tokens(Writer_CryptoTokens, *writer, *remote_reader,
            exception);
    CryptoPlugin->keyexchange()->create_local_datareader_crypto_tokens(Reader_CryptoTokens, *reader, *remote_writer,
            exception);

    //Exchange Datareader and Datawriter Cryptotokens
    CryptoPlugin->keyexchange()->set_remote_datareader_crypto_tokens(*writer, *remote_reader, Reader_CryptoTokens,
            exception);
    CryptoPlugin->keyexchange()->set_remote_datawriter_crypto_tokens(*reader, *remote_writer, Writer_CryptoTokens,
            exception);
    //Perform sample message exchange

    //Send message to intended participant
    ASSERT_TRUE(CryptoPlugin->cryptotransform()->encode_serialized_payload(encoded_payload, inline_qos, plain_payload,
            *writer, exception));
    encoded_payload.pos = 0;
    ASSERT_TRUE(CryptoPlugin->cryptotransform()->decode_serialized_payload(decoded_payload, encoded_payload, inline_qos,
            *reader, *remote_writer, exception));
    ASSERT_TRUE(memcmp(plain_payload.data, decoded_payload.data, 18) == 0);

    CryptoPlugin->keyfactory()->unregister_datawriter(writer, exception);
    CryptoPlugin->keyfactory()->unregister_datawriter(remote_writer, exception);

    CryptoPlugin->keyfactory()->unregister_datareader(reader, exception);
    CryptoPlugin->keyfactory()->unregister_datareader(remote_reader, exception);

    CryptoPlugin->keyfactory()->unregister_participant(participant_A, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantA_remote, exception);
    CryptoPlugin->keyfactory()->unregister_participant(participant_B, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantB_remote, exception);

    auth_plugin.return_identity_handle(&i_handle, exception);
    auth_plugin.return_sharedsecret_handle(secret, exception);
    access_plugin.return_permissions_handle(&perm_handle, exception);
}

TEST_F(CryptographyPluginTest, transform_Writer_Submesage)
{
    using namespace eprosima::fastdds::rtps::security;

    SecurityException exception;

    PKIIdentityHandle& i_handle =
            PKIIdentityHandle::narrow(*auth_plugin.get_identity_handle(exception));

    AccessPermissionsHandle& perm_handle =
            AccessPermissionsHandle::narrow(*access_plugin.get_permissions_handle(exception));

    eprosima::fastdds::rtps::PropertySeq prop_handle;
    ParticipantSecurityAttributes part_sec_attr;

    EndpointSecurityAttributes sec_attrs;

    std::shared_ptr<SecretHandle> secret =
            auth_plugin.get_shared_secret(SharedSecretHandle::nil_handle, exception);

    std::shared_ptr<SharedSecretHandle> shared_secret = std::dynamic_pointer_cast<SharedSecretHandle>(secret);

    part_sec_attr.is_rtps_protected = true;
    part_sec_attr.plugin_participant_attributes = PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ENCRYPTED |
            PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ORIGIN_AUTHENTICATED;

    sec_attrs.is_submessage_protected = true;
    sec_attrs.is_payload_protected = false;
    sec_attrs.is_key_protected = false;
    sec_attrs.plugin_endpoint_attributes = PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED |
            PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED;

    std::shared_ptr<ParticipantCryptoHandle> participant_A =
            CryptoPlugin->keyfactory()->register_local_participant(i_handle, perm_handle, prop_handle, part_sec_attr,
                    exception);
    std::shared_ptr<ParticipantCryptoHandle> participant_B =
            CryptoPlugin->keyfactory()->register_local_participant(i_handle, perm_handle, prop_handle, part_sec_attr,
                    exception);

    DatareaderCryptoHandle* reader =
            CryptoPlugin->keyfactory()->register_local_datareader(*participant_A, prop_handle, sec_attrs, exception);
    DatareaderCryptoHandle* writer =
            CryptoPlugin->keyfactory()->register_local_datawriter(*participant_B, prop_handle, sec_attrs, exception);

    //Fill shared secret with dummy values
    std::vector<uint8_t> dummy_data, challenge_1, challenge_2;
    SharedSecret::BinaryData binary_data;
    challenge_1.resize(32);
    challenge_2.resize(32);

    RAND_bytes(challenge_1.data(), 32);
    binary_data.name("Challenge1");
    binary_data.value(challenge_1);
    (*shared_secret)->data_.push_back(binary_data);

    RAND_bytes(challenge_2.data(), 32);
    binary_data.name("Challenge2");
    binary_data.value(challenge_2);
    (*shared_secret)->data_.push_back(binary_data);

    dummy_data.resize(32);
    RAND_bytes(dummy_data.data(), 32);
    binary_data.name("SharedSecret");
    binary_data.value(dummy_data);
    (*shared_secret)->data_.push_back(binary_data);

    //Register a remote for both Participants
    std::shared_ptr<ParticipantCryptoHandle> ParticipantA_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_A, i_handle, perm_handle,
                    *shared_secret, exception);
    std::shared_ptr<ParticipantCryptoHandle> ParticipantB_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_B, i_handle, perm_handle,
                    *shared_secret, exception);

    //Register DataReader with DataWriter
    DatareaderCryptoHandle* remote_reader =
            CryptoPlugin->keyfactory()->register_matched_remote_datareader(*writer, *ParticipantB_remote,
                    *shared_secret, false, exception);

    //Register DataWriter with DataReader
    DatawriterCryptoHandle* remote_writer =
            CryptoPlugin->keyfactory()->register_matched_remote_datawriter(*reader, *ParticipantA_remote,
                    *shared_secret, exception);

    //Create CryptoTokens for both Participants
    ParticipantCryptoTokenSeq ParticipantA_CryptoTokens, ParticipantB_CryptoTokens;

    CryptoPlugin->keyexchange()->create_local_participant_crypto_tokens(ParticipantA_CryptoTokens, *participant_A,
            *ParticipantA_remote, exception);
    CryptoPlugin->keyexchange()->create_local_participant_crypto_tokens(ParticipantB_CryptoTokens, *participant_B,
            *ParticipantB_remote, exception);

    //Set ParticipantA token into ParticipantB and viceversa
    CryptoPlugin->keyexchange()->set_remote_participant_crypto_tokens(*participant_A, *ParticipantA_remote,
            ParticipantB_CryptoTokens, exception);
    CryptoPlugin->keyexchange()->set_remote_participant_crypto_tokens(*participant_B, *ParticipantB_remote,
            ParticipantA_CryptoTokens, exception);

    //Create CryptoTokens for the DataWriter and DataReader
    DatawriterCryptoTokenSeq Writer_CryptoTokens, Reader_CryptoTokens;

    CryptoPlugin->keyexchange()->create_local_datawriter_crypto_tokens(Writer_CryptoTokens, *writer, *remote_reader,
            exception);
    CryptoPlugin->keyexchange()->create_local_datareader_crypto_tokens(Reader_CryptoTokens, *reader, *remote_writer,
            exception);

    //Exchange Datareader and Datawriter Cryptotokens
    CryptoPlugin->keyexchange()->set_remote_datareader_crypto_tokens(*writer, *remote_reader, Reader_CryptoTokens,
            exception);
    CryptoPlugin->keyexchange()->set_remote_datawriter_crypto_tokens(*reader, *remote_writer, Writer_CryptoTokens,
            exception);

    //Perform sample message exchange
    std::vector<uint8_t> plain_payload;
    std::vector<uint8_t> encoded_payload;
    std::vector<uint8_t> decoded_payload;

    char message[] = "My goose is cooked"; //Length 18
    plain_payload.resize(18);
    memcpy(plain_payload.data(), message, 18);

    std::vector<uint8_t> inline_qos;
    std::vector<DatareaderCryptoHandle*> receivers;
    receivers.push_back(remote_reader);

    //TODO(Ricardo) Fix
    //Send message to intended participant
    /*
       ASSERT_TRUE(CryptoPlugin->cryptotransform()->encode_datawriter_submessage(encoded_payload,
               plain_payload, *writer, receivers, exception));
       ASSERT_TRUE(CryptoPlugin->cryptotransform()->decode_datawriter_submessage(decoded_payload,
               encoded_payload, *reader, *remote_writer, exception));
       ASSERT_TRUE(plain_payload == decoded_payload);
     */

    CryptoPlugin->keyfactory()->unregister_datawriter(writer, exception);
    CryptoPlugin->keyfactory()->unregister_datawriter(remote_writer, exception);

    CryptoPlugin->keyfactory()->unregister_datareader(reader, exception);
    CryptoPlugin->keyfactory()->unregister_datareader(remote_reader, exception);

    CryptoPlugin->keyfactory()->unregister_participant(participant_A, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantA_remote, exception);
    CryptoPlugin->keyfactory()->unregister_participant(participant_B, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantB_remote, exception);

    //Test the GCM128 version
    eprosima::fastdds::rtps::Property prop1;
    prop1.name("dds.sec.crypto.keysize");
    prop1.value("128");
    prop_handle.push_back(prop1);
    eprosima::fastdds::rtps::Property prop2;
    prop2.name("dds.sec.crypto.maxblockspersession");
    prop2.value("16");
    prop_handle.push_back(prop2);

    participant_A = CryptoPlugin->keyfactory()->register_local_participant(i_handle, perm_handle, prop_handle,
                    part_sec_attr, exception);
    participant_B = CryptoPlugin->keyfactory()->register_local_participant(i_handle, perm_handle, prop_handle,
                    part_sec_attr, exception);

    reader = CryptoPlugin->keyfactory()->register_local_datareader(*participant_A, prop_handle, sec_attrs, exception);
    writer = CryptoPlugin->keyfactory()->register_local_datawriter(*participant_B, prop_handle, sec_attrs, exception);

    //Register a remote for both Participants
    ParticipantA_remote = CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_A, i_handle,
                    perm_handle, *shared_secret,
                    exception);
    ParticipantB_remote = CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_B, i_handle,
                    perm_handle, *shared_secret,
                    exception);

    //Register DataReader with DataWriter
    remote_reader = CryptoPlugin->keyfactory()->register_matched_remote_datareader(*writer, *ParticipantB_remote,
                    *shared_secret, false, exception);

    //Register DataWriter with DataReader
    remote_writer = CryptoPlugin->keyfactory()->register_matched_remote_datawriter(*reader, *ParticipantA_remote,
                    *shared_secret, exception);

    CryptoPlugin->keyexchange()->create_local_participant_crypto_tokens(ParticipantA_CryptoTokens, *participant_A,
            *ParticipantA_remote, exception);
    CryptoPlugin->keyexchange()->create_local_participant_crypto_tokens(ParticipantB_CryptoTokens, *participant_B,
            *ParticipantB_remote, exception);

    //Set ParticipantA token into ParticipantB and viceversa
    CryptoPlugin->keyexchange()->set_remote_participant_crypto_tokens(*participant_A, *ParticipantA_remote,
            ParticipantB_CryptoTokens, exception);
    CryptoPlugin->keyexchange()->set_remote_participant_crypto_tokens(*participant_B, *ParticipantB_remote,
            ParticipantA_CryptoTokens, exception);

    //Create CryptoTokens for the DataWriter and DataReader
    CryptoPlugin->keyexchange()->create_local_datawriter_crypto_tokens(Writer_CryptoTokens, *writer, *remote_reader,
            exception);
    CryptoPlugin->keyexchange()->create_local_datareader_crypto_tokens(Reader_CryptoTokens, *reader, *remote_writer,
            exception);

    //Exchange Datareader and Datawriter Cryptotokens
    CryptoPlugin->keyexchange()->set_remote_datareader_crypto_tokens(*writer, *remote_reader, Reader_CryptoTokens,
            exception);
    CryptoPlugin->keyexchange()->set_remote_datawriter_crypto_tokens(*reader, *remote_writer, Writer_CryptoTokens,
            exception);

    receivers.clear();
    receivers.push_back(remote_reader);

    //TODO(Ricardo) Fix
    //Send message to intended participant
    /*
       ASSERT_TRUE(CryptoPlugin->cryptotransform()->encode_datawriter_submessage(encoded_payload,
               plain_payload, *writer, receivers, exception));
       ASSERT_TRUE(CryptoPlugin->cryptotransform()->decode_datawriter_submessage(decoded_payload,
               encoded_payload, *reader, *remote_writer, exception));
       ASSERT_TRUE(plain_payload == decoded_payload);
     */

    auth_plugin.return_sharedsecret_handle(secret, exception);
    auth_plugin.return_identity_handle(&i_handle, exception);
    access_plugin.return_permissions_handle(&perm_handle, exception);

    CryptoPlugin->keyfactory()->unregister_datawriter(writer, exception);
    CryptoPlugin->keyfactory()->unregister_datawriter(remote_writer, exception);

    CryptoPlugin->keyfactory()->unregister_datareader(reader, exception);
    CryptoPlugin->keyfactory()->unregister_datareader(remote_reader, exception);

    CryptoPlugin->keyfactory()->unregister_participant(participant_A, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantA_remote, exception);
    CryptoPlugin->keyfactory()->unregister_participant(participant_B, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantB_remote, exception);

}

TEST_F(CryptographyPluginTest, transform_Reader_Submessage)
{
    using namespace eprosima::fastdds::rtps::security;

    SecurityException exception;

    PKIIdentityHandle& i_handle =
            PKIIdentityHandle::narrow(*auth_plugin.get_identity_handle(exception));

    AccessPermissionsHandle& perm_handle =
            AccessPermissionsHandle::narrow(*access_plugin.get_permissions_handle(exception));

    eprosima::fastdds::rtps::PropertySeq prop_handle;
    ParticipantSecurityAttributes part_sec_attr;

    EndpointSecurityAttributes sec_attrs;

    std::shared_ptr<SecretHandle> secret =
            auth_plugin.get_shared_secret(SharedSecretHandle::nil_handle, exception);

    std::shared_ptr<SharedSecretHandle> shared_secret = std::dynamic_pointer_cast<SharedSecretHandle>(secret);

    part_sec_attr.is_rtps_protected = true;
    part_sec_attr.plugin_participant_attributes = PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ENCRYPTED |
            PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ORIGIN_AUTHENTICATED;

    sec_attrs.is_submessage_protected = true;
    sec_attrs.is_payload_protected = false;
    sec_attrs.is_key_protected = false;
    sec_attrs.plugin_endpoint_attributes = PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED |
            PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED;

    std::shared_ptr<ParticipantCryptoHandle> participant_A =
            CryptoPlugin->keyfactory()->register_local_participant(i_handle, perm_handle, prop_handle, part_sec_attr,
                    exception);
    std::shared_ptr<ParticipantCryptoHandle> participant_B =
            CryptoPlugin->keyfactory()->register_local_participant(i_handle, perm_handle, prop_handle, part_sec_attr,
                    exception);

    DatareaderCryptoHandle* reader =
            CryptoPlugin->keyfactory()->register_local_datareader(*participant_A, prop_handle, sec_attrs, exception);
    DatareaderCryptoHandle* writer =
            CryptoPlugin->keyfactory()->register_local_datawriter(*participant_B, prop_handle, sec_attrs, exception);

    //Fill shared secret with dummy values
    std::vector<uint8_t> dummy_data, challenge_1, challenge_2;
    SharedSecret::BinaryData binary_data;
    challenge_1.resize(32);
    challenge_2.resize(32);

    RAND_bytes(challenge_1.data(), 32);
    binary_data.name("Challenge1");
    binary_data.value(challenge_1);
    (*shared_secret)->data_.push_back(binary_data);

    RAND_bytes(challenge_2.data(), 32);
    binary_data.name("Challenge2");
    binary_data.value(challenge_2);
    (*shared_secret)->data_.push_back(binary_data);

    dummy_data.resize(32);
    RAND_bytes(dummy_data.data(), 32);
    binary_data.name("SharedSecret");
    binary_data.value(dummy_data);
    (*shared_secret)->data_.push_back(binary_data);

    //Register a remote for both Participants
    std::shared_ptr<ParticipantCryptoHandle> ParticipantA_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_A, i_handle, perm_handle,
                    *shared_secret, exception);
    std::shared_ptr<ParticipantCryptoHandle> ParticipantB_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_B, i_handle, perm_handle,
                    *shared_secret, exception);

    //Register DataReader with DataWriter
    DatareaderCryptoHandle* remote_reader =
            CryptoPlugin->keyfactory()->register_matched_remote_datareader(*writer, *ParticipantB_remote,
                    *shared_secret, false, exception);

    //Register DataWriter with DataReader
    DatawriterCryptoHandle* remote_writer =
            CryptoPlugin->keyfactory()->register_matched_remote_datawriter(*reader, *ParticipantA_remote,
                    *shared_secret, exception);

    //Create CryptoTokens for both Participants
    ParticipantCryptoTokenSeq ParticipantA_CryptoTokens, ParticipantB_CryptoTokens;

    CryptoPlugin->keyexchange()->create_local_participant_crypto_tokens(ParticipantA_CryptoTokens, *participant_A,
            *ParticipantA_remote, exception);
    CryptoPlugin->keyexchange()->create_local_participant_crypto_tokens(ParticipantB_CryptoTokens, *participant_B,
            *ParticipantB_remote, exception);

    //Set ParticipantA token into ParticipantB and viceversa
    CryptoPlugin->keyexchange()->set_remote_participant_crypto_tokens(*participant_A, *ParticipantA_remote,
            ParticipantB_CryptoTokens, exception);
    CryptoPlugin->keyexchange()->set_remote_participant_crypto_tokens(*participant_B, *ParticipantB_remote,
            ParticipantA_CryptoTokens, exception);

    //Create CryptoTokens for the DataWriter and DataReader
    DatawriterCryptoTokenSeq Writer_CryptoTokens, Reader_CryptoTokens;

    CryptoPlugin->keyexchange()->create_local_datawriter_crypto_tokens(Writer_CryptoTokens, *writer, *remote_reader,
            exception);
    CryptoPlugin->keyexchange()->create_local_datareader_crypto_tokens(Reader_CryptoTokens, *reader, *remote_writer,
            exception);

    //Exchange Datareader and Datawriter Cryptotokens
    CryptoPlugin->keyexchange()->set_remote_datareader_crypto_tokens(*writer, *remote_reader, Reader_CryptoTokens,
            exception);
    CryptoPlugin->keyexchange()->set_remote_datawriter_crypto_tokens(*reader, *remote_writer, Writer_CryptoTokens,
            exception);

    //Perform sample message exchange
    std::vector<uint8_t> plain_payload;
    std::vector<uint8_t> encoded_payload;
    std::vector<uint8_t> decoded_payload;

    char message[] = "My goose is cooked"; //Length 18
    plain_payload.resize(18);
    memcpy(plain_payload.data(), message, 18);

    std::vector<DatawriterCryptoHandle*> receivers;
    receivers.push_back(remote_writer);

    //TODO(Ricardo) Fix
    //Send message to intended participant
    /*
       ASSERT_TRUE(CryptoPlugin->cryptotransform()->encode_datareader_submessage(encoded_payload,
               plain_payload, *reader, receivers, exception));
       ASSERT_TRUE(CryptoPlugin->cryptotransform()->decode_datareader_submessage(decoded_payload,
               encoded_payload, *writer, *remote_reader, exception));
       ASSERT_TRUE(plain_payload == decoded_payload);
     */

    CryptoPlugin->keyfactory()->unregister_datawriter(writer, exception);
    CryptoPlugin->keyfactory()->unregister_datawriter(remote_writer, exception);

    CryptoPlugin->keyfactory()->unregister_datareader(reader, exception);
    CryptoPlugin->keyfactory()->unregister_datareader(remote_reader, exception);

    CryptoPlugin->keyfactory()->unregister_participant(participant_A, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantA_remote, exception);
    CryptoPlugin->keyfactory()->unregister_participant(participant_B, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantB_remote, exception);

    //Test the GCM128 version
    eprosima::fastdds::rtps::Property prop1;
    prop1.name("dds.sec.crypto.keysize");
    prop1.value("128");
    prop_handle.push_back(prop1);
    eprosima::fastdds::rtps::Property prop2;
    prop2.name("dds.sec.crypto.maxblockspersession");
    prop2.value("16");
    prop_handle.push_back(prop2);

    participant_A = CryptoPlugin->keyfactory()->register_local_participant(i_handle, perm_handle, prop_handle,
                    part_sec_attr, exception);
    participant_B = CryptoPlugin->keyfactory()->register_local_participant(i_handle, perm_handle, prop_handle,
                    part_sec_attr, exception);

    reader = CryptoPlugin->keyfactory()->register_local_datareader(*participant_A, prop_handle, sec_attrs, exception);
    writer = CryptoPlugin->keyfactory()->register_local_datawriter(*participant_B, prop_handle, sec_attrs, exception);

    //Register a remote for both Participants
    ParticipantA_remote = CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_A, i_handle,
                    perm_handle, *shared_secret,
                    exception);
    ParticipantB_remote = CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_B, i_handle,
                    perm_handle, *shared_secret,
                    exception);

    //Register DataReader with DataWriter
    remote_reader = CryptoPlugin->keyfactory()->register_matched_remote_datareader(*writer, *ParticipantB_remote,
                    *shared_secret, false, exception);

    //Register DataWriter with DataReader
    remote_writer = CryptoPlugin->keyfactory()->register_matched_remote_datawriter(*reader, *ParticipantA_remote,
                    *shared_secret, exception);

    //Create CryptoTokens for both Participants
    CryptoPlugin->keyexchange()->create_local_participant_crypto_tokens(ParticipantA_CryptoTokens, *participant_A,
            *ParticipantA_remote, exception);
    CryptoPlugin->keyexchange()->create_local_participant_crypto_tokens(ParticipantB_CryptoTokens, *participant_B,
            *ParticipantB_remote, exception);

    //Set ParticipantA token into ParticipantB and viceversa
    CryptoPlugin->keyexchange()->set_remote_participant_crypto_tokens(*participant_A, *ParticipantA_remote,
            ParticipantB_CryptoTokens, exception);
    CryptoPlugin->keyexchange()->set_remote_participant_crypto_tokens(*participant_B, *ParticipantB_remote,
            ParticipantA_CryptoTokens, exception);

    //Create CryptoTokens for the DataWriter and DataReader
    CryptoPlugin->keyexchange()->create_local_datawriter_crypto_tokens(Writer_CryptoTokens, *writer, *remote_reader,
            exception);
    CryptoPlugin->keyexchange()->create_local_datareader_crypto_tokens(Reader_CryptoTokens, *reader, *remote_writer,
            exception);

    //Exchange Datareader and Datawriter Cryptotokens
    CryptoPlugin->keyexchange()->set_remote_datareader_crypto_tokens(*writer, *remote_reader, Reader_CryptoTokens,
            exception);
    CryptoPlugin->keyexchange()->set_remote_datawriter_crypto_tokens(*reader, *remote_writer, Writer_CryptoTokens,
            exception);

    //Perform sample message exchange
    receivers.clear();
    receivers.push_back(remote_writer);

    //TODO(Ricardo)Fix
    //Send message to intended participant
    /*
       ASSERT_TRUE(CryptoPlugin->cryptotransform()->encode_datareader_submessage(encoded_payload,
               plain_payload, *reader, receivers, exception));
       ASSERT_TRUE(CryptoPlugin->cryptotransform()->decode_datareader_submessage(decoded_payload,
               encoded_payload, *writer, *remote_reader, exception));
       ASSERT_TRUE(plain_payload == decoded_payload);
     */

    CryptoPlugin->keyfactory()->unregister_datawriter(writer, exception);
    CryptoPlugin->keyfactory()->unregister_datawriter(remote_writer, exception);

    CryptoPlugin->keyfactory()->unregister_datareader(reader, exception);
    CryptoPlugin->keyfactory()->unregister_datareader(remote_reader, exception);

    CryptoPlugin->keyfactory()->unregister_participant(participant_A, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantA_remote, exception);
    CryptoPlugin->keyfactory()->unregister_participant(participant_B, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantB_remote, exception);

    auth_plugin.return_sharedsecret_handle(secret, exception);
    auth_plugin.return_identity_handle(&i_handle, exception);
    access_plugin.return_permissions_handle(&perm_handle, exception);
}

TEST_F(CryptographyPluginTest, transform_preprocess_secure_submessage)
{
    using namespace eprosima::fastdds::rtps::security;

    SecurityException exception;

    PKIIdentityHandle& i_handle =
            PKIIdentityHandle::narrow(*auth_plugin.get_identity_handle(exception));

    AccessPermissionsHandle& perm_handle =
            AccessPermissionsHandle::narrow(*access_plugin.get_permissions_handle(exception));

    eprosima::fastdds::rtps::PropertySeq prop_handle;
    ParticipantSecurityAttributes part_sec_attr;

    EndpointSecurityAttributes sec_attrs;

    std::shared_ptr<SecretHandle> secret =
            auth_plugin.get_shared_secret(SharedSecretHandle::nil_handle, exception);

    auto shared_secret = std::dynamic_pointer_cast<SharedSecretHandle>(secret);

    part_sec_attr.is_rtps_protected = true;
    part_sec_attr.plugin_participant_attributes = PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ENCRYPTED |
            PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ORIGIN_AUTHENTICATED;

    sec_attrs.is_submessage_protected = true;
    sec_attrs.is_payload_protected = false;
    sec_attrs.is_key_protected = false;
    sec_attrs.plugin_endpoint_attributes = PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED |
            PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED;

    auto participant_A =
            CryptoPlugin->keyfactory()->register_local_participant(i_handle, perm_handle, prop_handle, part_sec_attr,
                    exception);
    auto participant_B =
            CryptoPlugin->keyfactory()->register_local_participant(i_handle, perm_handle, prop_handle, part_sec_attr,
                    exception);

    DatareaderCryptoHandle* reader =
            CryptoPlugin->keyfactory()->register_local_datareader(*participant_B, prop_handle, sec_attrs, exception);
    EXPECT_TRUE(reader != nullptr);
    DatareaderCryptoHandle* writer =
            CryptoPlugin->keyfactory()->register_local_datawriter(*participant_A, prop_handle, sec_attrs, exception);
    EXPECT_TRUE(writer != nullptr);

    //Fill shared secret with dummy values
    std::vector<uint8_t> dummy_data, challenge_1, challenge_2;
    SharedSecret::BinaryData binary_data;
    challenge_1.resize(32);
    challenge_2.resize(32);

    RAND_bytes(challenge_1.data(), 32);
    binary_data.name("Challenge1");
    binary_data.value(challenge_1);
    (*shared_secret)->data_.push_back(binary_data);

    RAND_bytes(challenge_2.data(), 32);
    binary_data.name("Challenge2");
    binary_data.value(challenge_2);
    (*shared_secret)->data_.push_back(binary_data);

    dummy_data.resize(32);
    RAND_bytes(dummy_data.data(), 32);
    binary_data.name("SharedSecret");
    binary_data.value(dummy_data);
    (*shared_secret)->data_.push_back(binary_data);

    //Register a remote for both Participants
    std::shared_ptr<ParticipantCryptoHandle> ParticipantA_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_A, i_handle, perm_handle,
                    *shared_secret, exception);
    EXPECT_TRUE(ParticipantA_remote != nullptr);
    std::shared_ptr<ParticipantCryptoHandle> ParticipantB_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_B, i_handle, perm_handle,
                    *shared_secret, exception);
    EXPECT_TRUE(ParticipantB_remote != nullptr);

    //Register DataReader with DataWriter
    DatareaderCryptoHandle* remote_reader =
            CryptoPlugin->keyfactory()->register_matched_remote_datareader(*writer, *ParticipantA_remote,
                    *shared_secret, false, exception);
    EXPECT_TRUE(remote_reader != nullptr);

    //Register DataWriter with DataReader
    DatawriterCryptoHandle* remote_writer =
            CryptoPlugin->keyfactory()->register_matched_remote_datawriter(*reader, *ParticipantB_remote,
                    *shared_secret, exception);
    EXPECT_TRUE(remote_writer != nullptr);

    //Create CryptoTokens for both Participants
    ParticipantCryptoTokenSeq ParticipantA_CryptoTokens, ParticipantB_CryptoTokens;

    EXPECT_TRUE(CryptoPlugin->keyexchange()->create_local_participant_crypto_tokens(ParticipantA_CryptoTokens,
            *participant_A, *ParticipantA_remote, exception));
    EXPECT_TRUE(CryptoPlugin->keyexchange()->create_local_participant_crypto_tokens(ParticipantB_CryptoTokens,
            *participant_B, *ParticipantB_remote, exception));

    //Set ParticipantA token into ParticipantB and viceversa
    EXPECT_TRUE(CryptoPlugin->keyexchange()->set_remote_participant_crypto_tokens(*participant_A, *ParticipantA_remote,
            ParticipantB_CryptoTokens, exception));
    EXPECT_TRUE(CryptoPlugin->keyexchange()->set_remote_participant_crypto_tokens(*participant_B, *ParticipantB_remote,
            ParticipantA_CryptoTokens, exception));

    //Create CryptoTokens for the DataWriter and DataReader
    DatawriterCryptoTokenSeq Writer_CryptoTokens, Reader_CryptoTokens;

    EXPECT_TRUE(CryptoPlugin->keyexchange()->create_local_datawriter_crypto_tokens(Writer_CryptoTokens, *writer,
            *remote_reader, exception));
    EXPECT_TRUE(CryptoPlugin->keyexchange()->create_local_datareader_crypto_tokens(Reader_CryptoTokens, *reader,
            *remote_writer, exception));

    //Exchange Datareader and Datawriter Cryptotokens
    EXPECT_TRUE(CryptoPlugin->keyexchange()->set_remote_datareader_crypto_tokens(*writer, *remote_reader,
            Reader_CryptoTokens, exception));
    EXPECT_TRUE(CryptoPlugin->keyexchange()->set_remote_datawriter_crypto_tokens(*reader, *remote_writer,
            Writer_CryptoTokens, exception));

    //Verify each remote participant has data about the remote readers and writer
    AESGCMGMAC_ParticipantCryptoHandle& P_B =
            AESGCMGMAC_ParticipantCryptoHandle::narrow(*ParticipantB_remote);                                                                       //Owner of a Reader
    AESGCMGMAC_ParticipantCryptoHandle& P_A =
            AESGCMGMAC_ParticipantCryptoHandle::narrow(*ParticipantA_remote);                                                                       //Owner of a Writer

    ASSERT_TRUE( P_A->Readers.size() == 2);
    ASSERT_TRUE( P_A->Writers.size() == 1);
    ASSERT_TRUE( P_B->Writers.size() == 2);
    ASSERT_TRUE( P_B->Readers.size() == 1);

    //Perform sample message exchange
    eprosima::fastdds::rtps::CDRMessage_t plain_payload(RTPSMESSAGE_DEFAULT_SIZE);
    eprosima::fastdds::rtps::CDRMessage_t encoded_datareader_payload(RTPSMESSAGE_DEFAULT_SIZE);
    eprosima::fastdds::rtps::CDRMessage_t encoded_datawriter_payload(RTPSMESSAGE_DEFAULT_SIZE);

    char message[] = "My goose is cooked"; //Length 18
    memcpy(plain_payload.buffer, message, 18);
    plain_payload.length = 18;

    std::vector<std::shared_ptr<DatawriterCryptoHandle>> receivers;
    receivers.push_back(remote_writer->shared_from_this());

    EXPECT_TRUE(CryptoPlugin->cryptotransform()->encode_datareader_submessage(encoded_datareader_payload, plain_payload,
            *reader, receivers, exception));

    receivers.clear();
    receivers.push_back(remote_reader->shared_from_this());
    plain_payload.pos = 0;
    EXPECT_TRUE(CryptoPlugin->cryptotransform()->encode_datawriter_submessage(encoded_datawriter_payload, plain_payload,
            *writer, receivers, exception));

    SecureSubmessageCategory_t message_category;
    DatareaderCryptoHandle** target_reader =
            new DatareaderCryptoHandle*;
    DatawriterCryptoHandle** target_writer =
            new DatawriterCryptoHandle*;
    encoded_datareader_payload.pos = 0;
    ASSERT_TRUE(CryptoPlugin->cryptotransform()->preprocess_secure_submsg(target_writer, target_reader,
            message_category, encoded_datareader_payload, *participant_A, *ParticipantA_remote, exception));

    ASSERT_TRUE(message_category == DATAREADER_SUBMESSAGE);
    ASSERT_TRUE(*target_reader == remote_reader);
    ASSERT_TRUE(*target_writer == writer);

    encoded_datawriter_payload.pos = 0;
    ASSERT_TRUE(CryptoPlugin->cryptotransform()->preprocess_secure_submsg(target_writer, target_reader,
            message_category, encoded_datawriter_payload, *participant_B, *ParticipantB_remote, exception));
    ASSERT_TRUE(message_category == DATAWRITER_SUBMESSAGE);
    ASSERT_TRUE(*target_writer == remote_writer);
    ASSERT_TRUE(*target_reader == reader);

    delete target_reader;
    delete target_writer;

    EXPECT_TRUE(CryptoPlugin->keyfactory()->unregister_datawriter(writer, exception));
    EXPECT_TRUE(CryptoPlugin->keyfactory()->unregister_datawriter(remote_writer, exception));

    EXPECT_TRUE(CryptoPlugin->keyfactory()->unregister_datareader(reader, exception));
    EXPECT_TRUE(CryptoPlugin->keyfactory()->unregister_datareader(remote_reader, exception));

    EXPECT_TRUE(CryptoPlugin->keyfactory()->unregister_participant(participant_A, exception));
    EXPECT_TRUE(CryptoPlugin->keyfactory()->unregister_participant(ParticipantA_remote, exception));
    EXPECT_TRUE(CryptoPlugin->keyfactory()->unregister_participant(participant_B, exception));
    EXPECT_TRUE(CryptoPlugin->keyfactory()->unregister_participant(ParticipantB_remote, exception));

    auth_plugin.return_sharedsecret_handle(secret, exception);
    auth_plugin.return_identity_handle(&i_handle, exception);
    access_plugin.return_permissions_handle(&perm_handle, exception);
}

#endif // ifndef _UNITTEST_SECURITY_CRYPTOGRAPHY_CRYPTOGRAPHYPLUGINTESTS_HPP_
