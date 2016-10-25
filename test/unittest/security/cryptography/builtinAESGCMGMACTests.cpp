


#include <iostream>
#include <algorithm>
#include "CryptographyPluginTests.hpp"

IdentityHandle* CryptographyPluginTest::create_placeholder_identityhandle(){
    IdentityHandle* buffer;

    return buffer;
}

PermissionsHandle* CryptographyPluginTest::create_placeholder_permissionshandle(){
    PermissionsHandle* buffer;

    return buffer;
}

PropertySeq* CryptographyPluginTest::create_placeholder_propertyseq(){
    PropertySeq* buffer;

    return buffer;
}

bool CryptographyPluginTest::check_localparticipanthandle(PropertySeq &properties, ParticipantCryptoHandle *local){

    AESGCMGMAC_ParticipantCryptoHandle& local_participant = AESGCMGMAC_ParticipantCryptoHandle::narrow(*local);
    bool result = true;
    
    if(local_participant->ParticipantKeyMaterial == nullptr){
        std::cout << "LocalParticipantHandle does not have ParticipantKeyMaterial" << std::endl;
        result = false;
    }
    if(local_participant->Participant2ParticipantKeyMaterial.size() != 0){
        std::cout << "LocalParticipantHandle does have ParticipantKeyMaterial when it should not" << std::endl;
        result = false;
    }
    if(local_participant->Participant2ParticipantKxKeyMaterial.size() != 0){
        std::cout << "LocalParticipantHandle does have ParticipantKxKeymaterial when it should not" << std::endl;
        result = false;
    }
    
    if(local_participant->ParticipantKeyMaterial->transformation_kind != std::array<uint8_t,4>(CRYPTO_TRANSFORMATION_KIND_AES128_GCM)){
        //TODO (Santiago) - Configuration of the transformation kind should be done via PropertySeq. This oughta be implemented in the near future and the test will then must be updated
        std::cout << "Incorrect transformation_kind present in ParticipantKeyMaterial" << std::endl;
        result = false;
    }

    if( std::all_of(local_participant->ParticipantKeyMaterial->master_salt.begin(),local_participant->ParticipantKeyMaterial->master_salt.end(), [](uint8_t i){return i==0;}) ){
        std::cout << "ParticipantKeyMaterial master_salt is uninitialized" << std::endl;
        result = false;
            }


    /*if(local_participant->ParticipantKeyMaterial->master_sender_key ==

    if(local_participant->ParticipantKeyMaterial->receiver_specific_key_id

    */
    if( std::all_of(local_participant->ParticipantKeyMaterial->master_receiver_specific_key.begin(),local_participant->ParticipantKeyMaterial->master_receiver_specific_key.end(), [](uint8_t i){return i==0;}) ) {
        std::cout << "ParticipantKeyMaterial as receiver_key set when it shouldnt" << std::endl;
        result = false;
    }


    return result;
}

TEST_F(CryptographyPluginTest, mocktest){
    uint8_t mock = 7;

    ASSERT_TRUE(mock == 7);

}


int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

