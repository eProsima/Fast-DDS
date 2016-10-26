


#ifndef _UNITTEST_SECURITY_CRYPTOGRAPHY_CRYPTOGRAPHYPLUGINTESTS_HPP_
#define _UNITTEST_SECURITY_CRYPTOGRAPHY_CRYPTOGRAPHYPLUGINTESTS_HPP_

#include "../../../../src/cpp/security/cryptography/AESGCMGMAC.h"
#include "../../../../src/cpp/security/authentication/PKIIdentityHandle.h"
#include "../../../../src/cpp/security/access/mockAccessHandle.h"

#include <gtest/gtest.h>

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

    ParticipantCryptoHandle *target = CryptoPlugin->m_cryptokeyfactory->register_local_participant(*i_handle,*perm_handle,prop_handle,exception);

    ASSERT_TRUE(target != nullptr);

    AESGCMGMAC_ParticipantCryptoHandle& local_participant = AESGCMGMAC_ParticipantCryptoHandle::narrow(*target);
    bool result = true;
   
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

    CryptoPlugin->m_cryptokeyfactory->unregister_participant(target,exception);
}

TEST_F(CryptographyPluginTest, factory_RegisterRemoteParticipant)
{
    
    PKIIdentityHandle* i_handle = new PKIIdentityHandle();
    mockAccessHandle* perm_handle = new mockAccessHandle();
    PropertySeq prop_handle;

    SecurityException exception;

    ParticipantCryptoHandle *local = CryptoPlugin->m_cryptokeyfactory->register_local_participant(*i_handle,*perm_handle,prop_handle,exception);

    ASSERT_TRUE(local != nullptr);


    //Dissect results to check correct creation
    AESGCMGMAC_ParticipantCryptoHandle& local_participant = AESGCMGMAC_ParticipantCryptoHandle::narrow(*local);


    //Register remote into local
    ParticipantCryptoHandle *remote_A;
    ParticipantCryptoHandle *remote_B;
    /*
    ParticipantCryptoHandle *remote_A =CryptoPlugin->m_cryptokeyfactory->register_matched_remote_participant(*local,*i_handle,*perm_handle,prop_handle,exception);
    ParticipantCryptoHandle *remote_B =CryptoPlugin->m_cryptokeyfactory->register_matched_remote_participant(*local,*i_handle,*perm_handle,prop_handle,exception);
    */
    ASSERT_TRUE( (remote_A != nullptr) & (remote_B != nullptr) );

    AESGCMGMAC_ParticipantCryptoHandle& remote_participant_A = AESGCMGMAC_ParticipantCryptoHandle::narrow(*remote_A);
    AESGCMGMAC_ParticipantCryptoHandle& remote_participant_B = AESGCMGMAC_ParticipantCryptoHandle::narrow(*remote_B);

    delete perm_handle;
    delete i_handle;
}

#endif
