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

#include "../../../../src/cpp/security/cryptography/AESGCMGMAC.h"
#include "../../../../src/cpp/security/authentication/PKIIdentityHandle.h"
#include "../../../../src/cpp/security/access/mockAccessHandle.h"

#include <gtest/gtest.h>
#include <openssl/rand.h>
#include <cstdlib>
#include <cstring>

using namespace eprosima::fastrtps::rtps;
using namespace ::security;

class CryptographyPluginTest : public ::testing::Test{

    protected:
        virtual void SetUp(){
            PropertyPolicy m_propertypolicy;

            CryptoPlugin = new AESGCMGMAC(m_propertypolicy);
            
        }
        virtual void TearDown(){
            delete CryptoPlugin;

        }

    public:
        CryptographyPluginTest():CryptoPlugin(nullptr){};

        AESGCMGMAC* CryptoPlugin;

};

TEST_F(CryptographyPluginTest, factory_CreateLocalParticipantHandle)
{

    PKIIdentityHandle* i_handle = new PKIIdentityHandle();
    mockAccessHandle* perm_handle = new mockAccessHandle();
    PropertySeq prop_handle;

    SecurityException exception;

    ParticipantCryptoHandle *target = CryptoPlugin->keyfactory()->register_local_participant(*i_handle,*perm_handle,prop_handle,exception);
    ASSERT_TRUE(target != nullptr);

    AESGCMGMAC_ParticipantCryptoHandle& local_participant = AESGCMGMAC_ParticipantCryptoHandle::narrow(*target);
    ASSERT_TRUE(!local_participant.nil());
  
    ASSERT_TRUE(local_participant->Participant2ParticipantKeyMaterial.empty());
    ASSERT_TRUE(local_participant->Participant2ParticipantKxKeyMaterial.empty());
    ASSERT_TRUE( (local_participant->ParticipantKeyMaterial.transformation_kind == std::array<uint8_t,4>(CRYPTO_TRANSFORMATION_KIND_AES128_GCM)) );

    ASSERT_FALSE( std::all_of(local_participant->ParticipantKeyMaterial.master_salt.begin(),local_participant->ParticipantKeyMaterial.master_salt.end(), [](uint8_t i){return i==0;}) );
    
    ASSERT_FALSE( std::all_of(local_participant->ParticipantKeyMaterial.master_sender_key.begin(),local_participant->ParticipantKeyMaterial.master_sender_key.end(), [](uint8_t i){return i==0;}) );

    ASSERT_FALSE( std::any_of(local_participant->ParticipantKeyMaterial.receiver_specific_key_id.begin(),local_participant->ParticipantKeyMaterial.receiver_specific_key_id.end(), [](uint8_t i){return i!=0;}) );

    ASSERT_FALSE( std::any_of(local_participant->ParticipantKeyMaterial.master_receiver_specific_key.begin(),local_participant->ParticipantKeyMaterial.master_receiver_specific_key.end(), [](uint8_t i){return i!=0;}) );

    delete i_handle;
    delete perm_handle;

    //Release resources and check the handle is indeed empty

    CryptoPlugin->keyfactory()->unregister_participant(target,exception);
}

TEST_F(CryptographyPluginTest, factory_RegisterRemoteParticipant)
{
    
    PKIIdentityHandle* i_handle = new PKIIdentityHandle();
    mockAccessHandle* perm_handle = new mockAccessHandle();
    PropertySeq prop_handle;
    SharedSecretHandle* shared_secret = new SharedSecretHandle();

    SecurityException exception;

    ParticipantCryptoHandle *local = CryptoPlugin->keyfactory()->register_local_participant(*i_handle,*perm_handle,prop_handle,exception);

    ASSERT_TRUE(local != nullptr);

    //Dissect results to check correct creation
    AESGCMGMAC_ParticipantCryptoHandle& local_participant = AESGCMGMAC_ParticipantCryptoHandle::narrow(*local);

    //Fill shared secret with dummy values
    std::vector<uint8_t> dummy_data, challenge_1, challenge_2;
    SharedSecret::BinaryData binary_data;
    challenge_1.reserve(8);
    challenge_2.reserve(8);

    RAND_bytes(challenge_1.data(),8);
    binary_data.name("Challenge1");
    binary_data.value(challenge_1);
    (*shared_secret)->data_.push_back(binary_data);

    RAND_bytes(challenge_2.data(),8);
    binary_data.name("Challenge2");
    binary_data.value(challenge_2);
    (*shared_secret)->data_.push_back(binary_data);

    dummy_data.reserve(32);
    RAND_bytes(dummy_data.data(),32);
    binary_data.name("SharedSecret");
    binary_data.value(dummy_data);
    (*shared_secret)->data_.push_back(binary_data);

    ParticipantCryptoHandle *remote_A =CryptoPlugin->keyfactory()->register_matched_remote_participant(*local,*i_handle,*perm_handle,*shared_secret, exception);
    ParticipantCryptoHandle *remote_B =CryptoPlugin->keyfactory()->register_matched_remote_participant(*local,*i_handle,*perm_handle,*shared_secret, exception);
   
    ASSERT_TRUE( (remote_A != nullptr) );
    ASSERT_TRUE( (remote_B != nullptr) );

    AESGCMGMAC_ParticipantCryptoHandle& remote_participant_A = AESGCMGMAC_ParticipantCryptoHandle::narrow(*remote_A);
    AESGCMGMAC_ParticipantCryptoHandle& remote_participant_B = AESGCMGMAC_ParticipantCryptoHandle::narrow(*remote_B);

    //Check the presence of both remote P2PKeyMaterial and P2PKxKeyMaterial
    ASSERT_TRUE(local_participant->Participant2ParticipantKeyMaterial.size() == 2);
    ASSERT_TRUE(local_participant->Participant2ParticipantKxKeyMaterial.size() == 2);
    //Check that both remoteKeysMaterials have unique IDS (keys are the same since they use the same source material
    ASSERT_TRUE(local_participant->Participant2ParticipantKeyMaterial.at(0).sender_key_id != local_participant->Participant2ParticipantKeyMaterial.at(1).sender_key_id);
    //KxKeys should be the same since they derive from the same Shared Secret although Keys should not
    ASSERT_TRUE(local_participant->Participant2ParticipantKeyMaterial.at(0).master_receiver_specific_key != local_participant->Participant2ParticipantKeyMaterial.at(1).master_receiver_specific_key);
    ASSERT_TRUE(local_participant->Participant2ParticipantKxKeyMaterial.at(0).master_sender_key == local_participant->Participant2ParticipantKxKeyMaterial.at(1).master_sender_key);

    delete perm_handle;
    delete i_handle;
    delete shared_secret;
}

TEST_F(CryptographyPluginTest, exchange_CDRSerializenDeserialize){

    PKIIdentityHandle* i_handle = new PKIIdentityHandle();
    mockAccessHandle* perm_handle = new mockAccessHandle();
    PropertySeq prop_handle;
    SecurityException exception;

    ParticipantCryptoHandle *ParticipantA = CryptoPlugin->keyfactory()->register_local_participant(*i_handle,*perm_handle,prop_handle,exception);

    AESGCMGMAC_ParticipantCryptoHandle& Participant_A = AESGCMGMAC_ParticipantCryptoHandle::narrow(*ParticipantA);

    KeyMaterial_AES_GCM_GMAC base = Participant_A->ParticipantKeyMaterial;

    std::vector<uint8_t> serialized = CryptoPlugin->keyexchange()->KeyMaterialCDRSerialize(base);
    KeyMaterial_AES_GCM_GMAC result = CryptoPlugin->keyexchange()->KeyMaterialCDRDeserialize(&serialized);
    ASSERT_TRUE(
        (base.transformation_kind == result.transformation_kind) &
        (base.master_salt == result.master_salt) &
        (base.sender_key_id == result.sender_key_id) &
        (base.master_sender_key == result.master_sender_key) &
        (base.receiver_specific_key_id == result.receiver_specific_key_id) &
        (base.master_receiver_specific_key == result.master_receiver_specific_key)
    );
}

TEST_F(CryptographyPluginTest, exchange_ParticipantCryptoTokens)
{
    
    PKIIdentityHandle* i_handle = new PKIIdentityHandle();
    mockAccessHandle* perm_handle = new mockAccessHandle();
    PropertySeq prop_handle;
    SharedSecretHandle* shared_secret = new SharedSecretHandle();

    SecurityException exception;
    
    //Fill shared secret with dummy values
    std::vector<uint8_t> dummy_data, challenge_1, challenge_2;
    SharedSecret::BinaryData binary_data;
    challenge_1.reserve(8);
    challenge_2.reserve(8);

    RAND_bytes(challenge_1.data(),8);
    binary_data.name("Challenge1");
    binary_data.value(challenge_1);
    (*shared_secret)->data_.push_back(binary_data);

    RAND_bytes(challenge_2.data(),8);
    binary_data.name("Challenge2");
    binary_data.value(challenge_2);
    (*shared_secret)->data_.push_back(binary_data);

    dummy_data.reserve(32);
    RAND_bytes(dummy_data.data(),32);
    binary_data.name("SharedSecret");
    binary_data.value(dummy_data);
    (*shared_secret)->data_.push_back(binary_data);

    //Create ParticipantA and ParticipantB
    ParticipantCryptoHandle *ParticipantA = CryptoPlugin->keyfactory()->register_local_participant(*i_handle,*perm_handle,prop_handle,exception);
    ParticipantCryptoHandle *ParticipantB = CryptoPlugin->keyfactory()->register_local_participant(*i_handle,*perm_handle,prop_handle,exception);

    ASSERT_TRUE( (ParticipantA != nullptr) & (ParticipantB != nullptr) ); 

    //Register a remote for both Participants
    ParticipantCryptoHandle *ParticipantA_remote =CryptoPlugin->keyfactory()->register_matched_remote_participant(*ParticipantA,*i_handle,*perm_handle,*shared_secret, exception);
    ParticipantCryptoHandle *ParticipantB_remote =CryptoPlugin->keyfactory()->register_matched_remote_participant(*ParticipantB,*i_handle,*perm_handle,*shared_secret, exception);

    //Create CryptoTokens for both Participants
    ParticipantCryptoTokenSeq ParticipantA_CryptoTokens, ParticipantB_CryptoTokens;

    ASSERT_TRUE(
            CryptoPlugin->keyexchange()->create_local_participant_crypto_tokens(ParticipantA_CryptoTokens, *ParticipantA, *ParticipantA_remote, exception)
    );
    ASSERT_TRUE(
            CryptoPlugin->keyexchange()->create_local_participant_crypto_tokens(ParticipantB_CryptoTokens, *ParticipantB, *ParticipantB_remote, exception)
    );

    //Set ParticipantA token into ParticipantB and viceversa
    ASSERT_TRUE(
            CryptoPlugin->keyexchange()->set_remote_participant_crypto_tokens(*ParticipantA,*ParticipantA_remote,ParticipantB_CryptoTokens,exception)
    );
     ASSERT_TRUE(
            CryptoPlugin->keyexchange()->set_remote_participant_crypto_tokens(*ParticipantB,*ParticipantB_remote,ParticipantA_CryptoTokens,exception)
    );
    
    //Check that ParticipantB's KeyMaterial is congruent with ParticipantA and viceversa
    AESGCMGMAC_ParticipantCryptoHandle& Participant_A = AESGCMGMAC_ParticipantCryptoHandle::narrow(*ParticipantA);
    AESGCMGMAC_ParticipantCryptoHandle& Participant_B = AESGCMGMAC_ParticipantCryptoHandle::narrow(*ParticipantB);

    ASSERT_TRUE(Participant_A->RemoteParticipant2ParticipantKeyMaterial.size() == 1);
    ASSERT_TRUE(Participant_B->RemoteParticipant2ParticipantKeyMaterial.size() == 1);
    ASSERT_TRUE(Participant_A->Participant2ParticipantKeyMaterial.at(0).master_sender_key == Participant_B->RemoteParticipant2ParticipantKeyMaterial.at(0).master_sender_key);
    ASSERT_TRUE(Participant_B->Participant2ParticipantKeyMaterial.at(0).master_sender_key == Participant_A->RemoteParticipant2ParticipantKeyMaterial.at(0).master_sender_key);

}

TEST_F(CryptographyPluginTest, transform_MessageExchange)
{

    PKIIdentityHandle* i_handle = new PKIIdentityHandle();
    mockAccessHandle* perm_handle = new mockAccessHandle();
    PropertySeq prop_handle;
    SharedSecretHandle* shared_secret = new SharedSecretHandle();

    SecurityException exception;
    
    //Fill shared secret with dummy values
    std::vector<uint8_t> dummy_data, challenge_1, challenge_2;
    SharedSecret::BinaryData binary_data;
    challenge_1.reserve(8);
    challenge_2.reserve(8);

    RAND_bytes(challenge_1.data(),8);
    binary_data.name("Challenge1");
    binary_data.value(challenge_1);
    (*shared_secret)->data_.push_back(binary_data);

    RAND_bytes(challenge_2.data(),8);
    binary_data.name("Challenge2");
    binary_data.value(challenge_2);
    (*shared_secret)->data_.push_back(binary_data);

    dummy_data.reserve(32);
    RAND_bytes(dummy_data.data(),32);
    binary_data.name("SharedSecret");
    binary_data.value(dummy_data);
    (*shared_secret)->data_.push_back(binary_data);

    //Create ParticipantA and ParticipantB
    ParticipantCryptoHandle *ParticipantA = CryptoPlugin->keyfactory()->register_local_participant(*i_handle,*perm_handle,prop_handle,exception);
    ParticipantCryptoHandle *ParticipantB = CryptoPlugin->keyfactory()->register_local_participant(*i_handle,*perm_handle,prop_handle,exception);

    ASSERT_TRUE( (ParticipantA != nullptr) & (ParticipantB != nullptr) ); 

    //Register a remote for both Participants
    ParticipantCryptoHandle *ParticipantA_remote =CryptoPlugin->keyfactory()->register_matched_remote_participant(*ParticipantA,*i_handle,*perm_handle,*shared_secret, exception);
    ParticipantCryptoHandle *ParticipantB_remote =CryptoPlugin->keyfactory()->register_matched_remote_participant(*ParticipantB,*i_handle,*perm_handle,*shared_secret, exception);

    //Create CryptoTokens for both Participants
    ParticipantCryptoTokenSeq ParticipantA_CryptoTokens, ParticipantB_CryptoTokens;

    CryptoPlugin->keyexchange()->create_local_participant_crypto_tokens(ParticipantA_CryptoTokens, *ParticipantA, *ParticipantA_remote, exception);
    CryptoPlugin->keyexchange()->create_local_participant_crypto_tokens(ParticipantB_CryptoTokens, *ParticipantB, *ParticipantB_remote, exception);

    //Set ParticipantA token into ParticipantB and viceversa
    CryptoPlugin->keyexchange()->set_remote_participant_crypto_tokens(*ParticipantA,*ParticipantA_remote,ParticipantB_CryptoTokens,exception);
    CryptoPlugin->keyexchange()->set_remote_participant_crypto_tokens(*ParticipantB,*ParticipantB_remote,ParticipantA_CryptoTokens,exception);
    
    //Perform sample message exchange


}

#endif
