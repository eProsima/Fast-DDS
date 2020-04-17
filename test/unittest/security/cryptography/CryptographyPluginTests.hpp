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

#include "../../../../src/cpp/fastrtps_deprecated/security/cryptography/AESGCMGMAC.h"
#include "../../../../src/cpp/fastrtps_deprecated/security/authentication/PKIIdentityHandle.h"
#include "../../../../src/cpp/fastrtps_deprecated/security/accesscontrol/AccessPermissionsHandle.h"
#include <fastrtps/rtps/common/CDRMessage_t.h>

#include <gtest/gtest.h>
#include <openssl/rand.h>
#include <cstdlib>
#include <cstring>

class CryptographyPluginTest : public ::testing::Test
{
protected:

    virtual void SetUp()
    {
        eprosima::fastrtps::rtps::PropertyPolicy m_propertypolicy;

        CryptoPlugin = new eprosima::fastrtps::rtps::security::AESGCMGMAC();
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

    eprosima::fastrtps::rtps::security::AESGCMGMAC* CryptoPlugin;

};

TEST_F(CryptographyPluginTest, factory_CreateLocalParticipantHandle)
{

    eprosima::fastrtps::rtps::security::PKIIdentityHandle* i_handle =
            new eprosima::fastrtps::rtps::security::PKIIdentityHandle();
    eprosima::fastrtps::rtps::security::AccessPermissionsHandle* perm_handle =
            new eprosima::fastrtps::rtps::security::AccessPermissionsHandle();
    eprosima::fastrtps::rtps::PropertySeq prop_handle;
    eprosima::fastrtps::rtps::security::ParticipantSecurityAttributes part_sec_attr;

    eprosima::fastrtps::rtps::security::SecurityException exception;

    part_sec_attr.is_rtps_protected = true;
    part_sec_attr.plugin_participant_attributes = PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ENCRYPTED |
            PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ORIGIN_AUTHENTICATED;

    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* target =
            CryptoPlugin->keyfactory()->register_local_participant(*i_handle, *perm_handle, prop_handle, part_sec_attr,
                    exception);
    ASSERT_TRUE(target != nullptr);

    eprosima::fastrtps::rtps::security::AESGCMGMAC_ParticipantCryptoHandle& local_participant =
            eprosima::fastrtps::rtps::security::AESGCMGMAC_ParticipantCryptoHandle::narrow(*target);
    ASSERT_TRUE(!local_participant.nil());

    ASSERT_GT(local_participant->Participant2ParticipantKeyMaterial.size(), 0ul);
    ASSERT_GT(local_participant->Participant2ParticipantKxKeyMaterial.size(), 0ul);

    ASSERT_TRUE( (local_participant->ParticipantKeyMaterial.transformation_kind ==
            eprosima::fastrtps::rtps::security::c_transfrom_kind_aes256_gcm) );
    ASSERT_TRUE( (local_participant->Participant2ParticipantKeyMaterial.at(0).transformation_kind ==
            eprosima::fastrtps::rtps::security::c_transfrom_kind_aes256_gcm) );
    ASSERT_TRUE( (local_participant->Participant2ParticipantKxKeyMaterial.at(0).transformation_kind ==
            eprosima::fastrtps::rtps::security::c_transfrom_kind_aes256_gcm) );

    ASSERT_FALSE( std::all_of(local_participant->ParticipantKeyMaterial.master_salt.begin(),
            local_participant->ParticipantKeyMaterial.master_salt.end(), [](uint8_t i){
        return i == 0;
    }) );
    ASSERT_FALSE( std::all_of(local_participant->Participant2ParticipantKeyMaterial.at(0).master_salt.begin(),
            local_participant->Participant2ParticipantKeyMaterial.at(0).master_salt.end(), [](uint8_t i){
        return i == 0;
    }) );
    ASSERT_FALSE( std::all_of(local_participant->Participant2ParticipantKxKeyMaterial.at(0).master_salt.begin(),
            local_participant->Participant2ParticipantKxKeyMaterial.at(0).master_salt.end(), [](uint8_t i){
        return i == 0;
    }) );

    ASSERT_FALSE( std::all_of(local_participant->ParticipantKeyMaterial.master_sender_key.begin(),
            local_participant->ParticipantKeyMaterial.master_sender_key.end(), [](uint8_t i){
        return i == 0;
    }) );
    ASSERT_FALSE( std::all_of(local_participant->Participant2ParticipantKeyMaterial.at(0).master_sender_key.begin(),
            local_participant->Participant2ParticipantKeyMaterial.at(0).master_sender_key.end(), [](uint8_t i){
        return i == 0;
    }) );
    ASSERT_FALSE( std::all_of(local_participant->Participant2ParticipantKxKeyMaterial.at(0).master_sender_key.begin(),
            local_participant->Participant2ParticipantKxKeyMaterial.at(0).master_sender_key.end(), [](uint8_t i){
        return i == 0;
    }) );

    ASSERT_FALSE( std::any_of(local_participant->ParticipantKeyMaterial.receiver_specific_key_id.begin(),
            local_participant->ParticipantKeyMaterial.receiver_specific_key_id.end(), [](uint8_t i){
        return i != 0;
    }) );
    ASSERT_FALSE( std::any_of(local_participant->Participant2ParticipantKeyMaterial.at(0).receiver_specific_key_id.begin(),
            local_participant->Participant2ParticipantKeyMaterial.at(0).receiver_specific_key_id.end(), [](uint8_t i){
        return i == 0;
    }) );
    ASSERT_FALSE( std::any_of(local_participant->Participant2ParticipantKxKeyMaterial.at(0).receiver_specific_key_id.
            begin(), local_participant->Participant2ParticipantKxKeyMaterial.at(0).receiver_specific_key_id.end(),
            [](uint8_t i){
        return i == 0;
    }) );

    delete i_handle;
    delete perm_handle;

    //Release resources and check the handle is indeed empty

    CryptoPlugin->keyfactory()->unregister_participant(target, exception);
}


TEST_F(CryptographyPluginTest, factory_RegisterRemoteParticipant)
{

    eprosima::fastrtps::rtps::security::PKIIdentityHandle* i_handle =
            new eprosima::fastrtps::rtps::security::PKIIdentityHandle();
    eprosima::fastrtps::rtps::security::AccessPermissionsHandle* perm_handle =
            new eprosima::fastrtps::rtps::security::AccessPermissionsHandle();
    eprosima::fastrtps::rtps::PropertySeq prop_handle;
    eprosima::fastrtps::rtps::security::ParticipantSecurityAttributes part_sec_attr;
    eprosima::fastrtps::rtps::security::SharedSecretHandle* shared_secret =
            new eprosima::fastrtps::rtps::security::SharedSecretHandle();

    eprosima::fastrtps::rtps::security::SecurityException exception;

    part_sec_attr.is_rtps_protected = true;
    part_sec_attr.plugin_participant_attributes = PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ENCRYPTED |
            PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ORIGIN_AUTHENTICATED;

    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* local =
            CryptoPlugin->keyfactory()->register_local_participant(*i_handle, *perm_handle, prop_handle, part_sec_attr,
                    exception);

    ASSERT_TRUE(local != nullptr);

    //Dissect results to check correct creation

    //Fill shared secret with dummy values
    std::vector<uint8_t> dummy_data, challenge_1, challenge_2;
    eprosima::fastrtps::rtps::security::SharedSecret::BinaryData binary_data;
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

    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* remote_A =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*local, *i_handle, *perm_handle,
                    *shared_secret, exception);
    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* remote_B =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*local, *i_handle, *perm_handle,
                    *shared_secret, exception);

    ASSERT_TRUE( (remote_A != nullptr) );
    ASSERT_TRUE( (remote_B != nullptr) );

    eprosima::fastrtps::rtps::security::AESGCMGMAC_ParticipantCryptoHandle& remote_participant_A =
            eprosima::fastrtps::rtps::security::AESGCMGMAC_ParticipantCryptoHandle::narrow(*remote_A);
    eprosima::fastrtps::rtps::security::AESGCMGMAC_ParticipantCryptoHandle& remote_participant_B =
            eprosima::fastrtps::rtps::security::AESGCMGMAC_ParticipantCryptoHandle::narrow(*remote_B);

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

    delete perm_handle;
    delete i_handle;
    delete shared_secret;
}

TEST_F(CryptographyPluginTest, exchange_CDRSerializenDeserialize){

    eprosima::fastrtps::rtps::security::PKIIdentityHandle* i_handle =
            new eprosima::fastrtps::rtps::security::PKIIdentityHandle();
    eprosima::fastrtps::rtps::security::AccessPermissionsHandle* perm_handle =
            new eprosima::fastrtps::rtps::security::AccessPermissionsHandle();
    eprosima::fastrtps::rtps::PropertySeq prop_handle;
    eprosima::fastrtps::rtps::security::ParticipantSecurityAttributes part_sec_attr;
    eprosima::fastrtps::rtps::security::SecurityException exception;

    part_sec_attr.is_rtps_protected = true;
    part_sec_attr.plugin_participant_attributes = PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ENCRYPTED |
            PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ORIGIN_AUTHENTICATED;

    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* ParticipantA =
            CryptoPlugin->keyfactory()->register_local_participant(*i_handle, *perm_handle, prop_handle, part_sec_attr,
                    exception);

    eprosima::fastrtps::rtps::security::AESGCMGMAC_ParticipantCryptoHandle& Participant_A =
            eprosima::fastrtps::rtps::security::AESGCMGMAC_ParticipantCryptoHandle::narrow(*ParticipantA);

    eprosima::fastrtps::rtps::security::KeyMaterial_AES_GCM_GMAC base = Participant_A->ParticipantKeyMaterial;

    std::vector<uint8_t> serialized = CryptoPlugin->keyexchange()->KeyMaterialCDRSerialize(base);
    eprosima::fastrtps::rtps::security::KeyMaterial_AES_GCM_GMAC result;
    CryptoPlugin->keyexchange()->KeyMaterialCDRDeserialize(result, &serialized);
    ASSERT_TRUE(
        (base.transformation_kind == result.transformation_kind) &
        (base.master_salt == result.master_salt) &
        (base.sender_key_id == result.sender_key_id) &
        (base.master_sender_key == result.master_sender_key) &
        (base.receiver_specific_key_id == result.receiver_specific_key_id) &
        (base.master_receiver_specific_key == result.master_receiver_specific_key)
        );

    CryptoPlugin->keyfactory()->unregister_participant(ParticipantA, exception);

    delete i_handle;
    delete perm_handle;

}

TEST_F(CryptographyPluginTest, exchange_ParticipantCryptoTokens)
{

    eprosima::fastrtps::rtps::security::PKIIdentityHandle* i_handle =
            new eprosima::fastrtps::rtps::security::PKIIdentityHandle();
    eprosima::fastrtps::rtps::security::AccessPermissionsHandle* perm_handle =
            new eprosima::fastrtps::rtps::security::AccessPermissionsHandle();
    eprosima::fastrtps::rtps::PropertySeq prop_handle;
    eprosima::fastrtps::rtps::security::ParticipantSecurityAttributes part_sec_attr;
    eprosima::fastrtps::rtps::security::SharedSecretHandle* shared_secret =
            new eprosima::fastrtps::rtps::security::SharedSecretHandle();

    eprosima::fastrtps::rtps::security::SecurityException exception;

    part_sec_attr.is_rtps_protected = true;
    part_sec_attr.plugin_participant_attributes = PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ENCRYPTED |
            PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ORIGIN_AUTHENTICATED;

    //Fill shared secret with dummy values
    std::vector<uint8_t> dummy_data, challenge_1, challenge_2;
    eprosima::fastrtps::rtps::security::SharedSecret::BinaryData binary_data;
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
    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* ParticipantA =
            CryptoPlugin->keyfactory()->register_local_participant(*i_handle, *perm_handle, prop_handle, part_sec_attr,
                    exception);
    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* ParticipantB =
            CryptoPlugin->keyfactory()->register_local_participant(*i_handle, *perm_handle, prop_handle, part_sec_attr,
                    exception);

    ASSERT_TRUE( (ParticipantA != nullptr) & (ParticipantB != nullptr) );

    //Register a remote for both Participants
    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* ParticipantA_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*ParticipantA, *i_handle, *perm_handle,
                    *shared_secret, exception);
    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* ParticipantB_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*ParticipantB, *i_handle, *perm_handle,
                    *shared_secret, exception);

    //Create CryptoTokens for both Participants
    eprosima::fastrtps::rtps::security::ParticipantCryptoTokenSeq ParticipantA_CryptoTokens, ParticipantB_CryptoTokens;

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
    eprosima::fastrtps::rtps::security::AESGCMGMAC_ParticipantCryptoHandle& Participant_A_remote =
            eprosima::fastrtps::rtps::security::AESGCMGMAC_ParticipantCryptoHandle::narrow(*ParticipantA_remote);
    eprosima::fastrtps::rtps::security::AESGCMGMAC_ParticipantCryptoHandle& Participant_B_remote =
            eprosima::fastrtps::rtps::security::AESGCMGMAC_ParticipantCryptoHandle::narrow(*ParticipantB_remote);

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

    delete shared_secret;
    delete perm_handle;
    delete i_handle;
}

TEST_F(CryptographyPluginTest, transform_RTPSMessage)
{

    eprosima::fastrtps::rtps::security::PKIIdentityHandle* i_handle =
            new eprosima::fastrtps::rtps::security::PKIIdentityHandle();
    eprosima::fastrtps::rtps::security::AccessPermissionsHandle* perm_handle =
            new eprosima::fastrtps::rtps::security::AccessPermissionsHandle();
    eprosima::fastrtps::rtps::PropertySeq prop_handle;
    eprosima::fastrtps::rtps::security::ParticipantSecurityAttributes part_sec_attr;
    eprosima::fastrtps::rtps::security::SharedSecretHandle* shared_secret =
            new eprosima::fastrtps::rtps::security::SharedSecretHandle();

    eprosima::fastrtps::rtps::security::SecurityException exception;

    part_sec_attr.is_rtps_protected = true;
    part_sec_attr.plugin_participant_attributes = PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ENCRYPTED |
            PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ORIGIN_AUTHENTICATED;

    //Fill shared secret with dummy values
    std::vector<uint8_t> dummy_data, challenge_1, challenge_2;
    eprosima::fastrtps::rtps::security::SharedSecret::BinaryData binary_data;
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
    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* ParticipantA =
            CryptoPlugin->keyfactory()->register_local_participant(*i_handle, *perm_handle, prop_handle, part_sec_attr,
                    exception);
    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* ParticipantB =
            CryptoPlugin->keyfactory()->register_local_participant(*i_handle, *perm_handle, prop_handle, part_sec_attr,
                    exception);

    ASSERT_TRUE( (ParticipantA != nullptr) & (ParticipantB != nullptr) );

    //Register a remote for both Participants
    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* ParticipantA_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*ParticipantA, *i_handle, *perm_handle,
                    *shared_secret, exception);
    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* ParticipantB_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*ParticipantB, *i_handle, *perm_handle,
                    *shared_secret, exception);

    //Create CryptoTokens for both Participants
    eprosima::fastrtps::rtps::security::ParticipantCryptoTokenSeq ParticipantA_CryptoTokens, ParticipantB_CryptoTokens;

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
    eprosima::fastrtps::rtps::CDRMessage_t plain_rtps_message(RTPSMESSAGE_DEFAULT_SIZE);
    eprosima::fastrtps::rtps::CDRMessage_t encoded_rtps_message(RTPSMESSAGE_DEFAULT_SIZE);
    eprosima::fastrtps::rtps::CDRMessage_t decoded_rtps_message(RTPSMESSAGE_DEFAULT_SIZE);

    char message[] = "RPTSMessage"; //Length 11
    memcpy(plain_rtps_message.buffer, message, 11);


    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* unintended_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*ParticipantA, *i_handle, *perm_handle,
                    *shared_secret, exception);
    std::vector<eprosima::fastrtps::rtps::security::ParticipantCryptoHandle*> receivers;

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
        decoded_rtps_message.pos = 0;
    }
    //Send message to unintended participant

    receivers.clear();
    receivers.push_back(unintended_remote);
    ASSERT_TRUE(CryptoPlugin->cryptotransform()->encode_rtps_message(encoded_rtps_message, plain_rtps_message,
            *ParticipantA, receivers, exception));
    ASSERT_FALSE(CryptoPlugin->cryptotransform()->decode_rtps_message(decoded_rtps_message, encoded_rtps_message,
            *ParticipantB, *ParticipantB_remote, exception));
    plain_rtps_message.pos = 0;
    encoded_rtps_message.pos = 0;
    decoded_rtps_message.pos = 0;


    CryptoPlugin->keyfactory()->unregister_participant(ParticipantA, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantB, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantA_remote, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantB_remote, exception);
    CryptoPlugin->keyfactory()->unregister_participant(unintended_remote, exception);


    //Now lets do the same with 256GCM
    //Fill prop_handle with info about the new mode we want
    eprosima::fastrtps::rtps::Property prop1;
    prop1.name("dds.sec.crypto.keysize");
    prop1.value("256");
    prop_handle.push_back(prop1);
    eprosima::fastrtps::rtps::Property prop2;
    prop2.name("dds.sec.crypto.maxblockspersession");
    prop2.value("16");
    prop_handle.push_back(prop2);
    //Create ParticipantA and ParticipantB
    ParticipantA = CryptoPlugin->keyfactory()->register_local_participant(*i_handle, *perm_handle, prop_handle,
                    part_sec_attr, exception);
    ParticipantB = CryptoPlugin->keyfactory()->register_local_participant(*i_handle, *perm_handle, prop_handle,
                    part_sec_attr, exception);

    //Register a remote for both Participants
    ParticipantA_remote = CryptoPlugin->keyfactory()->register_matched_remote_participant(*ParticipantA, *i_handle,
                    *perm_handle, *shared_secret,
                    exception);
    ParticipantB_remote = CryptoPlugin->keyfactory()->register_matched_remote_participant(*ParticipantB, *i_handle,
                    *perm_handle, *shared_secret,
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

    unintended_remote = CryptoPlugin->keyfactory()->register_matched_remote_participant(*ParticipantA, *i_handle,
                    *perm_handle, *shared_secret,
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

    delete shared_secret;
    delete i_handle;
    delete perm_handle;
}

TEST_F(CryptographyPluginTest, factory_CreateLocalWriterHandle)
{

    eprosima::fastrtps::rtps::security::PKIIdentityHandle* i_handle =
            new eprosima::fastrtps::rtps::security::PKIIdentityHandle();
    eprosima::fastrtps::rtps::security::AccessPermissionsHandle* perm_handle =
            new eprosima::fastrtps::rtps::security::AccessPermissionsHandle();
    eprosima::fastrtps::rtps::PropertySeq prop_handle;
    eprosima::fastrtps::rtps::security::ParticipantSecurityAttributes part_sec_attr;
    eprosima::fastrtps::rtps::security::EndpointSecurityAttributes sec_attrs;
    eprosima::fastrtps::rtps::security::SharedSecretHandle* shared_secret =
            new eprosima::fastrtps::rtps::security::SharedSecretHandle();

    eprosima::fastrtps::rtps::security::SecurityException exception;

    part_sec_attr.is_rtps_protected = true;
    part_sec_attr.plugin_participant_attributes = PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ENCRYPTED |
            PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ORIGIN_AUTHENTICATED;

    sec_attrs.is_submessage_protected = true;
    sec_attrs.is_payload_protected = false;
    sec_attrs.is_key_protected = false;
    sec_attrs.plugin_endpoint_attributes = PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED |
            PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED;

    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* participant =
            CryptoPlugin->keyfactory()->register_local_participant(*i_handle, *perm_handle, prop_handle, part_sec_attr,
                    exception);
    eprosima::fastrtps::rtps::security::DatawriterCryptoHandle* target =
            CryptoPlugin->keyfactory()->register_local_datawriter(*participant, prop_handle, sec_attrs, exception);
    ASSERT_TRUE(target != nullptr);

    eprosima::fastrtps::rtps::security::AESGCMGMAC_WriterCryptoHandle& local_writer =
            eprosima::fastrtps::rtps::security::AESGCMGMAC_WriterCryptoHandle::narrow(*target);
    ASSERT_TRUE(!local_writer.nil());

    ASSERT_TRUE(local_writer->Entity2RemoteKeyMaterial.empty());
    ASSERT_TRUE( (local_writer->EntityKeyMaterial.at(0).transformation_kind ==
            eprosima::fastrtps::rtps::security::c_transfrom_kind_aes256_gcm) );

    ASSERT_FALSE( std::all_of(local_writer->EntityKeyMaterial.at(0).master_salt.begin(),
            local_writer->EntityKeyMaterial.at(0).master_salt.end(), [](uint8_t i){
        return i == 0;
    }) );

    ASSERT_FALSE( std::all_of(local_writer->EntityKeyMaterial.at(0).master_sender_key.begin(),
            local_writer->EntityKeyMaterial.at(0).master_sender_key.end(), [](uint8_t i){
        return i == 0;
    }) );

    ASSERT_FALSE( std::any_of(local_writer->EntityKeyMaterial.at(0).receiver_specific_key_id.begin(),
            local_writer->EntityKeyMaterial.at(0).receiver_specific_key_id.end(), [](uint8_t i){
        return i != 0;
    }) );

    ASSERT_FALSE( std::any_of(local_writer->EntityKeyMaterial.at(0).master_receiver_specific_key.begin(),
            local_writer->EntityKeyMaterial.at(0).master_receiver_specific_key.end(), [](uint8_t i){
        return i != 0;
    }) );

    delete i_handle;
    delete perm_handle;
    delete shared_secret;
    //Release resources and check the handle is indeed empty

    CryptoPlugin->keyfactory()->unregister_datawriter(target, exception);
    CryptoPlugin->keyfactory()->unregister_participant(participant, exception);
}

TEST_F(CryptographyPluginTest, factory_CreateLocalReaderHandle)
{

    eprosima::fastrtps::rtps::security::PKIIdentityHandle* i_handle =
            new eprosima::fastrtps::rtps::security::PKIIdentityHandle();
    eprosima::fastrtps::rtps::security::AccessPermissionsHandle* perm_handle =
            new eprosima::fastrtps::rtps::security::AccessPermissionsHandle();
    eprosima::fastrtps::rtps::PropertySeq prop_handle;
    eprosima::fastrtps::rtps::security::ParticipantSecurityAttributes part_sec_attr;
    eprosima::fastrtps::rtps::security::EndpointSecurityAttributes sec_attrs;
    eprosima::fastrtps::rtps::security::SharedSecretHandle* shared_secret =
            new eprosima::fastrtps::rtps::security::SharedSecretHandle();

    eprosima::fastrtps::rtps::security::SecurityException exception;

    part_sec_attr.is_rtps_protected = true;
    part_sec_attr.plugin_participant_attributes = PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ENCRYPTED |
            PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ORIGIN_AUTHENTICATED;

    sec_attrs.is_submessage_protected = true;
    sec_attrs.is_payload_protected = false;
    sec_attrs.is_key_protected = false;
    sec_attrs.plugin_endpoint_attributes = PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED |
            PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED;

    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* participant =
            CryptoPlugin->keyfactory()->register_local_participant(*i_handle, *perm_handle, prop_handle, part_sec_attr,
                    exception);
    eprosima::fastrtps::rtps::security::DatareaderCryptoHandle* target =
            CryptoPlugin->keyfactory()->register_local_datareader(*participant, prop_handle, sec_attrs, exception);
    ASSERT_TRUE(target != nullptr);

    eprosima::fastrtps::rtps::security::AESGCMGMAC_ReaderCryptoHandle& local_reader =
            eprosima::fastrtps::rtps::security::AESGCMGMAC_ReaderCryptoHandle::narrow(*target);
    ASSERT_TRUE(!local_reader.nil());

    ASSERT_TRUE(local_reader->Entity2RemoteKeyMaterial.empty());
    ASSERT_TRUE( (local_reader->EntityKeyMaterial.at(0).transformation_kind ==
            eprosima::fastrtps::rtps::security::c_transfrom_kind_aes256_gcm) );

    ASSERT_FALSE( std::all_of(local_reader->EntityKeyMaterial.at(0).master_salt.begin(),
            local_reader->EntityKeyMaterial.at(0).master_salt.end(), [](uint8_t i){
        return i == 0;
    }) );

    ASSERT_FALSE( std::all_of(local_reader->EntityKeyMaterial.at(0).master_sender_key.begin(),
            local_reader->EntityKeyMaterial.at(0).master_sender_key.end(), [](uint8_t i){
        return i == 0;
    }) );

    ASSERT_FALSE( std::any_of(local_reader->EntityKeyMaterial.at(0).receiver_specific_key_id.begin(),
            local_reader->EntityKeyMaterial.at(0).receiver_specific_key_id.end(), [](uint8_t i){
        return i != 0;
    }) );

    ASSERT_FALSE( std::any_of(local_reader->EntityKeyMaterial.at(0).master_receiver_specific_key.begin(),
            local_reader->EntityKeyMaterial.at(0).master_receiver_specific_key.end(), [](uint8_t i){
        return i != 0;
    }) );

    delete i_handle;
    delete perm_handle;
    delete shared_secret;
    //Release resources and check the handle is indeed empty

    CryptoPlugin->keyfactory()->unregister_datareader(target, exception);
    CryptoPlugin->keyfactory()->unregister_participant(participant, exception);
}

TEST_F(CryptographyPluginTest, factory_RegisterRemoteReaderWriter)
{

    eprosima::fastrtps::rtps::security::PKIIdentityHandle* i_handle =
            new eprosima::fastrtps::rtps::security::PKIIdentityHandle();
    eprosima::fastrtps::rtps::security::AccessPermissionsHandle* perm_handle =
            new eprosima::fastrtps::rtps::security::AccessPermissionsHandle();
    eprosima::fastrtps::rtps::PropertySeq prop_handle;
    eprosima::fastrtps::rtps::security::ParticipantSecurityAttributes part_sec_attr;
    eprosima::fastrtps::rtps::security::EndpointSecurityAttributes sec_attrs;
    eprosima::fastrtps::rtps::security::SharedSecretHandle* shared_secret =
            new eprosima::fastrtps::rtps::security::SharedSecretHandle();

    eprosima::fastrtps::rtps::security::SecurityException exception;

    part_sec_attr.is_rtps_protected = true;
    part_sec_attr.plugin_participant_attributes = PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ENCRYPTED |
            PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ORIGIN_AUTHENTICATED;

    sec_attrs.is_submessage_protected = true;
    sec_attrs.is_payload_protected = false;
    sec_attrs.is_key_protected = false;
    sec_attrs.plugin_endpoint_attributes = PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED |
            PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED;

    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* participant_A =
            CryptoPlugin->keyfactory()->register_local_participant(*i_handle, *perm_handle, prop_handle, part_sec_attr,
                    exception);
    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* participant_B =
            CryptoPlugin->keyfactory()->register_local_participant(*i_handle, *perm_handle, prop_handle, part_sec_attr,
                    exception);

    eprosima::fastrtps::rtps::security::DatareaderCryptoHandle* reader =
            CryptoPlugin->keyfactory()->register_local_datareader(*participant_A, prop_handle, sec_attrs, exception);
    eprosima::fastrtps::rtps::security::DatareaderCryptoHandle* writer =
            CryptoPlugin->keyfactory()->register_local_datawriter(*participant_B, prop_handle, sec_attrs, exception);

    //Fill shared secret with dummy values
    std::vector<uint8_t> dummy_data, challenge_1, challenge_2;
    eprosima::fastrtps::rtps::security::SharedSecret::BinaryData binary_data;
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
    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* ParticipantA_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_A, *i_handle, *perm_handle,
                    *shared_secret, exception);
    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* ParticipantB_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_B, *i_handle, *perm_handle,
                    *shared_secret, exception);

    //Register DataReader with DataWriter
    eprosima::fastrtps::rtps::security::DatareaderCryptoHandle* remote_reader =
            CryptoPlugin->keyfactory()->register_matched_remote_datareader(*writer, *ParticipantB_remote,
                    *shared_secret, false, exception);
    ASSERT_TRUE(remote_reader != nullptr);


    //Register DataWriter with DataReader

    eprosima::fastrtps::rtps::security::DatawriterCryptoHandle* remote_writer =
            CryptoPlugin->keyfactory()->register_matched_remote_datawriter(*reader, *ParticipantA_remote,
                    *shared_secret, exception);
    ASSERT_TRUE(remote_writer != nullptr);

    delete i_handle;
    delete perm_handle;
    delete shared_secret;

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

    // Participant A owns Writer
    // Participant B owns Reader

    eprosima::fastrtps::rtps::security::PKIIdentityHandle* i_handle =
            new eprosima::fastrtps::rtps::security::PKIIdentityHandle();
    eprosima::fastrtps::rtps::security::AccessPermissionsHandle* perm_handle =
            new eprosima::fastrtps::rtps::security::AccessPermissionsHandle();
    eprosima::fastrtps::rtps::PropertySeq prop_handle;
    eprosima::fastrtps::rtps::security::ParticipantSecurityAttributes part_sec_attr;
    eprosima::fastrtps::rtps::security::EndpointSecurityAttributes sec_attrs;
    eprosima::fastrtps::rtps::security::SharedSecretHandle* shared_secret =
            new eprosima::fastrtps::rtps::security::SharedSecretHandle();

    eprosima::fastrtps::rtps::security::SecurityException exception;

    part_sec_attr.is_rtps_protected = true;
    part_sec_attr.plugin_participant_attributes = PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ENCRYPTED |
            PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ORIGIN_AUTHENTICATED;

    sec_attrs.is_submessage_protected = true;
    sec_attrs.is_payload_protected = false;
    sec_attrs.is_key_protected = false;
    sec_attrs.plugin_endpoint_attributes = PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED |
            PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED;

    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* participant_A =
            CryptoPlugin->keyfactory()->register_local_participant(*i_handle, *perm_handle, prop_handle, part_sec_attr,
                    exception);
    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* participant_B =
            CryptoPlugin->keyfactory()->register_local_participant(*i_handle, *perm_handle, prop_handle, part_sec_attr,
                    exception);

    eprosima::fastrtps::rtps::security::DatareaderCryptoHandle* reader =
            CryptoPlugin->keyfactory()->register_local_datareader(*participant_A, prop_handle, sec_attrs, exception);
    eprosima::fastrtps::rtps::security::DatareaderCryptoHandle* writer =
            CryptoPlugin->keyfactory()->register_local_datawriter(*participant_B, prop_handle, sec_attrs, exception);

    //Fill shared secret with dummy values
    std::vector<uint8_t> dummy_data, challenge_1, challenge_2;
    eprosima::fastrtps::rtps::security::SharedSecret::BinaryData binary_data;
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
    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* ParticipantA_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_A, *i_handle, *perm_handle,
                    *shared_secret, exception);
    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* ParticipantB_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_B, *i_handle, *perm_handle,
                    *shared_secret, exception);

    //Register DataReader with DataWriter
    eprosima::fastrtps::rtps::security::DatareaderCryptoHandle* remote_reader =
            CryptoPlugin->keyfactory()->register_matched_remote_datareader(*writer, *ParticipantB_remote,
                    *shared_secret, false, exception);

    //Register DataWriter with DataReader
    eprosima::fastrtps::rtps::security::DatawriterCryptoHandle* remote_writer =
            CryptoPlugin->keyfactory()->register_matched_remote_datawriter(*reader, *ParticipantA_remote,
                    *shared_secret, exception);

    //Create CryptoTokens for both Participants
    eprosima::fastrtps::rtps::security::ParticipantCryptoTokenSeq ParticipantA_CryptoTokens, ParticipantB_CryptoTokens;

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
    eprosima::fastrtps::rtps::security::DatawriterCryptoTokenSeq Writer_CryptoTokens, Reader_CryptoTokens;

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
    eprosima::fastrtps::rtps::security::AESGCMGMAC_WriterCryptoHandle& WriterH =
            eprosima::fastrtps::rtps::security::AESGCMGMAC_WriterCryptoHandle::narrow(*writer);
    eprosima::fastrtps::rtps::security::AESGCMGMAC_ReaderCryptoHandle& ReaderH =
            eprosima::fastrtps::rtps::security::AESGCMGMAC_ReaderCryptoHandle::narrow(*reader);

    ASSERT_TRUE(WriterH->Remote2EntityKeyMaterial.size() == 1);
    ASSERT_TRUE(ReaderH->Remote2EntityKeyMaterial.size() == 1);
    ASSERT_TRUE(WriterH->Entity2RemoteKeyMaterial.at(0).master_sender_key ==
            ReaderH->Remote2EntityKeyMaterial.at(0).master_sender_key);
    ASSERT_TRUE(ReaderH->Entity2RemoteKeyMaterial.at(0).master_sender_key ==
            WriterH->Remote2EntityKeyMaterial.at(0).master_sender_key);

    delete i_handle;
    delete perm_handle;
    delete shared_secret;

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

    // Participant A owns Writer
    // Participant B owns Reader
    eprosima::fastrtps::rtps::security::PKIIdentityHandle* i_handle =
            new eprosima::fastrtps::rtps::security::PKIIdentityHandle();
    eprosima::fastrtps::rtps::security::AccessPermissionsHandle* perm_handle =
            new eprosima::fastrtps::rtps::security::AccessPermissionsHandle();
    eprosima::fastrtps::rtps::PropertySeq prop_handle;
    eprosima::fastrtps::rtps::security::ParticipantSecurityAttributes part_sec_attr;
    eprosima::fastrtps::rtps::security::EndpointSecurityAttributes sec_attrs;
    eprosima::fastrtps::rtps::security::SharedSecretHandle* shared_secret =
            new eprosima::fastrtps::rtps::security::SharedSecretHandle();

    eprosima::fastrtps::rtps::security::SecurityException exception;

    part_sec_attr.is_rtps_protected = true;
    part_sec_attr.plugin_participant_attributes = PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ENCRYPTED |
            PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ORIGIN_AUTHENTICATED;

    sec_attrs.is_submessage_protected = true;
    sec_attrs.is_payload_protected = true;
    sec_attrs.is_key_protected = true;
    sec_attrs.plugin_endpoint_attributes = PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED |
            PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED |
            PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_PAYLOAD_ENCRYPTED;

    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* participant_A =
            CryptoPlugin->keyfactory()->register_local_participant(*i_handle, *perm_handle, prop_handle, part_sec_attr,
                    exception);
    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* participant_B =
            CryptoPlugin->keyfactory()->register_local_participant(*i_handle, *perm_handle, prop_handle, part_sec_attr,
                    exception);

    eprosima::fastrtps::rtps::security::DatareaderCryptoHandle* reader =
            CryptoPlugin->keyfactory()->register_local_datareader(*participant_A, prop_handle, sec_attrs, exception);
    eprosima::fastrtps::rtps::security::DatareaderCryptoHandle* writer =
            CryptoPlugin->keyfactory()->register_local_datawriter(*participant_B, prop_handle, sec_attrs, exception);

    //Fill shared secret with dummy values
    std::vector<uint8_t> dummy_data, challenge_1, challenge_2;
    eprosima::fastrtps::rtps::security::SharedSecret::BinaryData binary_data;
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
    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* ParticipantA_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_A, *i_handle, *perm_handle,
                    *shared_secret, exception);
    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* ParticipantB_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_B, *i_handle, *perm_handle,
                    *shared_secret, exception);

    //Register DataReader with DataWriter
    eprosima::fastrtps::rtps::security::DatareaderCryptoHandle* remote_reader =
            CryptoPlugin->keyfactory()->register_matched_remote_datareader(*writer, *ParticipantB_remote,
                    *shared_secret, false, exception);

    //Register DataWriter with DataReader
    eprosima::fastrtps::rtps::security::DatawriterCryptoHandle* remote_writer =
            CryptoPlugin->keyfactory()->register_matched_remote_datawriter(*reader, *ParticipantA_remote,
                    *shared_secret, exception);

    //Create CryptoTokens for both Participants
    eprosima::fastrtps::rtps::security::ParticipantCryptoTokenSeq ParticipantA_CryptoTokens, ParticipantB_CryptoTokens;

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
    eprosima::fastrtps::rtps::security::DatawriterCryptoTokenSeq Writer_CryptoTokens, Reader_CryptoTokens;

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
    eprosima::fastrtps::rtps::SerializedPayload_t plain_payload(18); // Message will have 18 length.
    eprosima::fastrtps::rtps::SerializedPayload_t encoded_payload(100);
    eprosima::fastrtps::rtps::SerializedPayload_t decoded_payload(18 + 32); // Message will have 18 length + cipher block size.

    char message[] = "My goose is cooked"; //Length 18
    memcpy(plain_payload.data, message, 18);
    plain_payload.length = 18;

    std::vector<uint8_t> inline_qos;

    //Send message to intended participant
    ASSERT_TRUE(CryptoPlugin->cryptotransform()->encode_serialized_payload(encoded_payload, inline_qos, plain_payload,
            *writer, exception));
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

    //Lets do it with the 256 version
    eprosima::fastrtps::rtps::Property prop1;
    prop1.name("dds.sec.crypto.keysize");
    prop1.value("256");
    prop_handle.push_back(prop1);
    eprosima::fastrtps::rtps::Property prop2;
    prop2.name("dds.sec.crypto.maxblockspersession");
    prop2.value("16");
    prop_handle.push_back(prop2);

    participant_A = CryptoPlugin->keyfactory()->register_local_participant(*i_handle, *perm_handle, prop_handle,
                    part_sec_attr, exception);
    participant_B = CryptoPlugin->keyfactory()->register_local_participant(*i_handle, *perm_handle, prop_handle,
                    part_sec_attr, exception);

    reader = CryptoPlugin->keyfactory()->register_local_datareader(*participant_A, prop_handle, sec_attrs, exception);
    writer = CryptoPlugin->keyfactory()->register_local_datawriter(*participant_B, prop_handle, sec_attrs, exception);

    //Register a remote for both Participants
    ParticipantA_remote = CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_A, *i_handle,
                    *perm_handle, *shared_secret,
                    exception);
    ParticipantB_remote = CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_B, *i_handle,
                    *perm_handle, *shared_secret,
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
    ASSERT_TRUE(CryptoPlugin->cryptotransform()->decode_serialized_payload(decoded_payload, encoded_payload, inline_qos,
            *reader, *remote_writer, exception));
    ASSERT_TRUE(memcmp(plain_payload.data, decoded_payload.data, 18) == 0);

    delete i_handle;
    delete perm_handle;
    delete shared_secret;

    CryptoPlugin->keyfactory()->unregister_datawriter(writer, exception);
    CryptoPlugin->keyfactory()->unregister_datawriter(remote_writer, exception);

    CryptoPlugin->keyfactory()->unregister_datareader(reader, exception);
    CryptoPlugin->keyfactory()->unregister_datareader(remote_reader, exception);

    CryptoPlugin->keyfactory()->unregister_participant(participant_A, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantA_remote, exception);
    CryptoPlugin->keyfactory()->unregister_participant(participant_B, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantB_remote, exception);

}

TEST_F(CryptographyPluginTest, transform_Writer_Submesage)
{

    // Participant A owns Writer
    // Participant B owns Reader

    eprosima::fastrtps::rtps::security::PKIIdentityHandle* i_handle =
            new eprosima::fastrtps::rtps::security::PKIIdentityHandle();
    eprosima::fastrtps::rtps::security::AccessPermissionsHandle* perm_handle =
            new eprosima::fastrtps::rtps::security::AccessPermissionsHandle();
    eprosima::fastrtps::rtps::PropertySeq prop_handle;
    eprosima::fastrtps::rtps::security::ParticipantSecurityAttributes part_sec_attr;
    eprosima::fastrtps::rtps::security::EndpointSecurityAttributes sec_attrs;
    eprosima::fastrtps::rtps::security::SharedSecretHandle* shared_secret =
            new eprosima::fastrtps::rtps::security::SharedSecretHandle();

    eprosima::fastrtps::rtps::security::SecurityException exception;

    part_sec_attr.is_rtps_protected = true;
    part_sec_attr.plugin_participant_attributes = PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ENCRYPTED |
            PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ORIGIN_AUTHENTICATED;

    sec_attrs.is_submessage_protected = true;
    sec_attrs.is_payload_protected = false;
    sec_attrs.is_key_protected = false;
    sec_attrs.plugin_endpoint_attributes = PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED |
            PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED;

    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* participant_A =
            CryptoPlugin->keyfactory()->register_local_participant(*i_handle, *perm_handle, prop_handle, part_sec_attr,
                    exception);
    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* participant_B =
            CryptoPlugin->keyfactory()->register_local_participant(*i_handle, *perm_handle, prop_handle, part_sec_attr,
                    exception);

    eprosima::fastrtps::rtps::security::DatareaderCryptoHandle* reader =
            CryptoPlugin->keyfactory()->register_local_datareader(*participant_A, prop_handle, sec_attrs, exception);
    eprosima::fastrtps::rtps::security::DatareaderCryptoHandle* writer =
            CryptoPlugin->keyfactory()->register_local_datawriter(*participant_B, prop_handle, sec_attrs, exception);

    //Fill shared secret with dummy values
    std::vector<uint8_t> dummy_data, challenge_1, challenge_2;
    eprosima::fastrtps::rtps::security::SharedSecret::BinaryData binary_data;
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
    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* ParticipantA_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_A, *i_handle, *perm_handle,
                    *shared_secret, exception);
    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* ParticipantB_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_B, *i_handle, *perm_handle,
                    *shared_secret, exception);

    //Register DataReader with DataWriter
    eprosima::fastrtps::rtps::security::DatareaderCryptoHandle* remote_reader =
            CryptoPlugin->keyfactory()->register_matched_remote_datareader(*writer, *ParticipantB_remote,
                    *shared_secret, false, exception);

    //Register DataWriter with DataReader
    eprosima::fastrtps::rtps::security::DatawriterCryptoHandle* remote_writer =
            CryptoPlugin->keyfactory()->register_matched_remote_datawriter(*reader, *ParticipantA_remote,
                    *shared_secret, exception);

    //Create CryptoTokens for both Participants
    eprosima::fastrtps::rtps::security::ParticipantCryptoTokenSeq ParticipantA_CryptoTokens, ParticipantB_CryptoTokens;

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
    eprosima::fastrtps::rtps::security::DatawriterCryptoTokenSeq Writer_CryptoTokens, Reader_CryptoTokens;

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
    std::vector<eprosima::fastrtps::rtps::security::DatareaderCryptoHandle*> receivers;
    receivers.push_back(remote_reader);

    //TODO(Ricardo) Fix
    //Send message to intended participant
    /*
       ASSERT_TRUE(CryptoPlugin->cryptotransform()->encode_datawriter_submessage(encoded_payload, plain_payload, *writer, receivers, exception));
       ASSERT_TRUE(CryptoPlugin->cryptotransform()->decode_datawriter_submessage(decoded_payload, encoded_payload, *reader, *remote_writer, exception));
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

    //Test the GCM256 version
    eprosima::fastrtps::rtps::Property prop1;
    prop1.name("dds.sec.crypto.keysize");
    prop1.value("256");
    prop_handle.push_back(prop1);
    eprosima::fastrtps::rtps::Property prop2;
    prop2.name("dds.sec.crypto.maxblockspersession");
    prop2.value("16");
    prop_handle.push_back(prop2);

    participant_A = CryptoPlugin->keyfactory()->register_local_participant(*i_handle, *perm_handle, prop_handle,
                    part_sec_attr, exception);
    participant_B = CryptoPlugin->keyfactory()->register_local_participant(*i_handle, *perm_handle, prop_handle,
                    part_sec_attr, exception);

    reader = CryptoPlugin->keyfactory()->register_local_datareader(*participant_A, prop_handle, sec_attrs, exception);
    writer = CryptoPlugin->keyfactory()->register_local_datawriter(*participant_B, prop_handle, sec_attrs, exception);

    //Register a remote for both Participants
    ParticipantA_remote = CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_A, *i_handle,
                    *perm_handle, *shared_secret,
                    exception);
    ParticipantB_remote = CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_B, *i_handle,
                    *perm_handle, *shared_secret,
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
       ASSERT_TRUE(CryptoPlugin->cryptotransform()->encode_datawriter_submessage(encoded_payload, plain_payload, *writer, receivers, exception));
       ASSERT_TRUE(CryptoPlugin->cryptotransform()->decode_datawriter_submessage(decoded_payload, encoded_payload, *reader, *remote_writer, exception));
       ASSERT_TRUE(plain_payload == decoded_payload);
     */

    delete i_handle;
    delete perm_handle;
    delete shared_secret;

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

    // Participant A owns Writer
    // Participant B owns Reader

    eprosima::fastrtps::rtps::security::PKIIdentityHandle* i_handle =
            new eprosima::fastrtps::rtps::security::PKIIdentityHandle();
    eprosima::fastrtps::rtps::security::AccessPermissionsHandle* perm_handle =
            new eprosima::fastrtps::rtps::security::AccessPermissionsHandle();
    eprosima::fastrtps::rtps::PropertySeq prop_handle;
    eprosima::fastrtps::rtps::security::ParticipantSecurityAttributes part_sec_attr;
    eprosima::fastrtps::rtps::security::EndpointSecurityAttributes sec_attrs;
    eprosima::fastrtps::rtps::security::SharedSecretHandle* shared_secret =
            new eprosima::fastrtps::rtps::security::SharedSecretHandle();

    eprosima::fastrtps::rtps::security::SecurityException exception;

    part_sec_attr.is_rtps_protected = true;
    part_sec_attr.plugin_participant_attributes = PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ENCRYPTED |
            PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ORIGIN_AUTHENTICATED;

    sec_attrs.is_submessage_protected = true;
    sec_attrs.is_payload_protected = false;
    sec_attrs.is_key_protected = false;
    sec_attrs.plugin_endpoint_attributes = PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED |
            PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED;

    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* participant_A =
            CryptoPlugin->keyfactory()->register_local_participant(*i_handle, *perm_handle, prop_handle, part_sec_attr,
                    exception);
    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* participant_B =
            CryptoPlugin->keyfactory()->register_local_participant(*i_handle, *perm_handle, prop_handle, part_sec_attr,
                    exception);

    eprosima::fastrtps::rtps::security::DatareaderCryptoHandle* reader =
            CryptoPlugin->keyfactory()->register_local_datareader(*participant_A, prop_handle, sec_attrs, exception);
    eprosima::fastrtps::rtps::security::DatareaderCryptoHandle* writer =
            CryptoPlugin->keyfactory()->register_local_datawriter(*participant_B, prop_handle, sec_attrs, exception);

    //Fill shared secret with dummy values
    std::vector<uint8_t> dummy_data, challenge_1, challenge_2;
    eprosima::fastrtps::rtps::security::SharedSecret::BinaryData binary_data;
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
    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* ParticipantA_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_A, *i_handle, *perm_handle,
                    *shared_secret, exception);
    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* ParticipantB_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_B, *i_handle, *perm_handle,
                    *shared_secret, exception);

    //Register DataReader with DataWriter
    eprosima::fastrtps::rtps::security::DatareaderCryptoHandle* remote_reader =
            CryptoPlugin->keyfactory()->register_matched_remote_datareader(*writer, *ParticipantB_remote,
                    *shared_secret, false, exception);

    //Register DataWriter with DataReader
    eprosima::fastrtps::rtps::security::DatawriterCryptoHandle* remote_writer =
            CryptoPlugin->keyfactory()->register_matched_remote_datawriter(*reader, *ParticipantA_remote,
                    *shared_secret, exception);

    //Create CryptoTokens for both Participants
    eprosima::fastrtps::rtps::security::ParticipantCryptoTokenSeq ParticipantA_CryptoTokens, ParticipantB_CryptoTokens;

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
    eprosima::fastrtps::rtps::security::DatawriterCryptoTokenSeq Writer_CryptoTokens, Reader_CryptoTokens;

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

    std::vector<eprosima::fastrtps::rtps::security::DatawriterCryptoHandle*> receivers;
    receivers.push_back(remote_writer);

    //TODO(Ricardo) Fix
    //Send message to intended participant
    /*
       ASSERT_TRUE(CryptoPlugin->cryptotransform()->encode_datareader_submessage(encoded_payload, plain_payload, *reader, receivers, exception));
       ASSERT_TRUE(CryptoPlugin->cryptotransform()->decode_datareader_submessage(decoded_payload, encoded_payload, *writer, *remote_reader, exception));
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

    //Test the GCM256 version
    eprosima::fastrtps::rtps::Property prop1;
    prop1.name("dds.sec.crypto.keysize");
    prop1.value("256");
    prop_handle.push_back(prop1);
    eprosima::fastrtps::rtps::Property prop2;
    prop2.name("dds.sec.crypto.maxblockspersession");
    prop2.value("16");
    prop_handle.push_back(prop2);

    participant_A = CryptoPlugin->keyfactory()->register_local_participant(*i_handle, *perm_handle, prop_handle,
                    part_sec_attr, exception);
    participant_B = CryptoPlugin->keyfactory()->register_local_participant(*i_handle, *perm_handle, prop_handle,
                    part_sec_attr, exception);

    reader = CryptoPlugin->keyfactory()->register_local_datareader(*participant_A, prop_handle, sec_attrs, exception);
    writer = CryptoPlugin->keyfactory()->register_local_datawriter(*participant_B, prop_handle, sec_attrs, exception);

    //Register a remote for both Participants
    ParticipantA_remote = CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_A, *i_handle,
                    *perm_handle, *shared_secret,
                    exception);
    ParticipantB_remote = CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_B, *i_handle,
                    *perm_handle, *shared_secret,
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
       ASSERT_TRUE(CryptoPlugin->cryptotransform()->encode_datareader_submessage(encoded_payload, plain_payload, *reader, receivers, exception));
       ASSERT_TRUE(CryptoPlugin->cryptotransform()->decode_datareader_submessage(decoded_payload, encoded_payload, *writer, *remote_reader, exception));
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

    delete i_handle;
    delete perm_handle;
    delete shared_secret;
}

TEST_F(CryptographyPluginTest, transform_preprocess_secure_submessage)
{
    // Participant A owns Writer
    // Participant B owns Reader

    eprosima::fastrtps::rtps::security::PKIIdentityHandle* i_handle =
            new eprosima::fastrtps::rtps::security::PKIIdentityHandle();
    eprosima::fastrtps::rtps::security::AccessPermissionsHandle* perm_handle =
            new eprosima::fastrtps::rtps::security::AccessPermissionsHandle();
    eprosima::fastrtps::rtps::PropertySeq prop_handle;
    eprosima::fastrtps::rtps::security::ParticipantSecurityAttributes part_sec_attr;
    eprosima::fastrtps::rtps::security::EndpointSecurityAttributes sec_attrs;
    eprosima::fastrtps::rtps::security::SharedSecretHandle* shared_secret =
            new eprosima::fastrtps::rtps::security::SharedSecretHandle();

    eprosima::fastrtps::rtps::security::SecurityException exception;

    part_sec_attr.is_rtps_protected = true;
    part_sec_attr.plugin_participant_attributes = PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ENCRYPTED |
            PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ORIGIN_AUTHENTICATED;

    sec_attrs.is_submessage_protected = true;
    sec_attrs.is_payload_protected = false;
    sec_attrs.is_key_protected = false;
    sec_attrs.plugin_endpoint_attributes = PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED |
            PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED;

    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* participant_A =
            CryptoPlugin->keyfactory()->register_local_participant(*i_handle, *perm_handle, prop_handle, part_sec_attr,
                    exception);
    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* participant_B =
            CryptoPlugin->keyfactory()->register_local_participant(*i_handle, *perm_handle, prop_handle, part_sec_attr,
                    exception);

    eprosima::fastrtps::rtps::security::DatareaderCryptoHandle* reader =
            CryptoPlugin->keyfactory()->register_local_datareader(*participant_B, prop_handle, sec_attrs, exception);
    eprosima::fastrtps::rtps::security::DatareaderCryptoHandle* writer =
            CryptoPlugin->keyfactory()->register_local_datawriter(*participant_A, prop_handle, sec_attrs, exception);

    //Fill shared secret with dummy values
    std::vector<uint8_t> dummy_data, challenge_1, challenge_2;
    eprosima::fastrtps::rtps::security::SharedSecret::BinaryData binary_data;
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
    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* ParticipantA_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_A, *i_handle, *perm_handle,
                    *shared_secret, exception);
    eprosima::fastrtps::rtps::security::ParticipantCryptoHandle* ParticipantB_remote =
            CryptoPlugin->keyfactory()->register_matched_remote_participant(*participant_B, *i_handle, *perm_handle,
                    *shared_secret, exception);

    //Register DataReader with DataWriter
    eprosima::fastrtps::rtps::security::DatareaderCryptoHandle* remote_reader =
            CryptoPlugin->keyfactory()->register_matched_remote_datareader(*writer, *ParticipantA_remote,
                    *shared_secret, false, exception);

    //Register DataWriter with DataReader
    eprosima::fastrtps::rtps::security::DatawriterCryptoHandle* remote_writer =
            CryptoPlugin->keyfactory()->register_matched_remote_datawriter(*reader, *ParticipantB_remote,
                    *shared_secret, exception);

    //Create CryptoTokens for both Participants
    eprosima::fastrtps::rtps::security::ParticipantCryptoTokenSeq ParticipantA_CryptoTokens, ParticipantB_CryptoTokens;

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
    eprosima::fastrtps::rtps::security::DatawriterCryptoTokenSeq Writer_CryptoTokens, Reader_CryptoTokens;

    CryptoPlugin->keyexchange()->create_local_datawriter_crypto_tokens(Writer_CryptoTokens, *writer, *remote_reader,
            exception);
    CryptoPlugin->keyexchange()->create_local_datareader_crypto_tokens(Reader_CryptoTokens, *reader, *remote_writer,
            exception);

    //Exchange Datareader and Datawriter Cryptotokens
    CryptoPlugin->keyexchange()->set_remote_datareader_crypto_tokens(*writer, *remote_reader, Reader_CryptoTokens,
            exception);
    CryptoPlugin->keyexchange()->set_remote_datawriter_crypto_tokens(*reader, *remote_writer, Writer_CryptoTokens,
            exception);

    //Verify each remote participant has data about the remote readers and writer
    eprosima::fastrtps::rtps::security::AESGCMGMAC_ParticipantCryptoHandle& P_B =
            eprosima::fastrtps::rtps::security::AESGCMGMAC_ParticipantCryptoHandle::narrow(*ParticipantB_remote);                                                                       //Owner of a Reader
    eprosima::fastrtps::rtps::security::AESGCMGMAC_ParticipantCryptoHandle& P_A =
            eprosima::fastrtps::rtps::security::AESGCMGMAC_ParticipantCryptoHandle::narrow(*ParticipantA_remote);                                                                       //Owner of a Writer

    ASSERT_TRUE( P_A->Readers.size() == 2);
    ASSERT_TRUE( P_A->Writers.size() == 1);
    ASSERT_TRUE( P_B->Writers.size() == 2);
    ASSERT_TRUE( P_B->Readers.size() == 1);

    //Perform sample message exchange
    eprosima::fastrtps::rtps::CDRMessage_t plain_payload(RTPSMESSAGE_DEFAULT_SIZE);
    eprosima::fastrtps::rtps::CDRMessage_t encoded_datareader_payload(RTPSMESSAGE_DEFAULT_SIZE);
    eprosima::fastrtps::rtps::CDRMessage_t encoded_datawriter_payload(RTPSMESSAGE_DEFAULT_SIZE);

    char message[] = "My goose is cooked"; //Length 18
    memcpy(plain_payload.buffer, message, 18);
    plain_payload.length = 18;

    std::vector<eprosima::fastrtps::rtps::security::DatawriterCryptoHandle*> receivers;
    receivers.push_back(remote_writer);

    CryptoPlugin->cryptotransform()->encode_datareader_submessage(encoded_datareader_payload, plain_payload, *reader,
            receivers, exception);

    receivers.clear();
    receivers.push_back(remote_reader);
    plain_payload.pos = 0;
    CryptoPlugin->cryptotransform()->encode_datawriter_submessage(encoded_datawriter_payload, plain_payload, *writer,
            receivers, exception);

    eprosima::fastrtps::rtps::security::SecureSubmessageCategory_t message_category;
    eprosima::fastrtps::rtps::security::DatareaderCryptoHandle** target_reader =
            new eprosima::fastrtps::rtps::security::DatareaderCryptoHandle *;
    eprosima::fastrtps::rtps::security::DatawriterCryptoHandle** target_writer =
            new eprosima::fastrtps::rtps::security::DatawriterCryptoHandle *;
    encoded_datareader_payload.pos = 0;
    ASSERT_TRUE(CryptoPlugin->cryptotransform()->preprocess_secure_submsg(target_writer, target_reader,
            message_category, encoded_datareader_payload, *participant_A, *ParticipantA_remote, exception));

    ASSERT_TRUE(message_category == eprosima::fastrtps::rtps::security::DATAREADER_SUBMESSAGE);
    ASSERT_TRUE(*target_reader == remote_reader);
    ASSERT_TRUE(*target_writer == writer);

    encoded_datawriter_payload.pos = 0;
    ASSERT_TRUE(CryptoPlugin->cryptotransform()->preprocess_secure_submsg(target_writer, target_reader,
            message_category, encoded_datawriter_payload, *participant_B, *ParticipantB_remote, exception));
    ASSERT_TRUE(message_category == eprosima::fastrtps::rtps::security::DATAWRITER_SUBMESSAGE);
    ASSERT_TRUE(*target_writer == remote_writer);
    ASSERT_TRUE(*target_reader == reader);

    delete target_reader;
    delete target_writer;

    CryptoPlugin->keyfactory()->unregister_datawriter(writer, exception);
    CryptoPlugin->keyfactory()->unregister_datawriter(remote_writer, exception);

    CryptoPlugin->keyfactory()->unregister_datareader(reader, exception);
    CryptoPlugin->keyfactory()->unregister_datareader(remote_reader, exception);

    CryptoPlugin->keyfactory()->unregister_participant(participant_A, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantA_remote, exception);
    CryptoPlugin->keyfactory()->unregister_participant(participant_B, exception);
    CryptoPlugin->keyfactory()->unregister_participant(ParticipantB_remote, exception);

    delete shared_secret;
    delete perm_handle;
    delete i_handle;
}

#endif