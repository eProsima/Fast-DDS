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

/*!
 * @file AESGCMGMAC_KeyFactory.cpp
 */

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include <string.h>

#include "AESGCMGMAC_KeyFactory.h"

using namespace eprosima::fastrtps::rtps::security;

AESGCMGMAC_KeyFactory::AESGCMGMAC_KeyFactory(){}

ParticipantCryptoHandle* AESGCMGMAC_KeyFactory::register_local_participant(
                const IdentityHandle &participant_identity, 
                const PermissionsHandle &participant_permissions, 
                const PropertySeq &participant_properties, 
                SecurityException &exception)
{

    //Create ParticipantCryptoHandle, fill Participant KeyMaterial and return it
    AESGCMGMAC_ParticipantCryptoHandle* PCrypto = nullptr;

    PCrypto = new AESGCMGMAC_ParticipantCryptoHandle();
    
    //Fill ParticipantKeyMaterial - This will be used to cipher full rpts messages

    (*PCrypto)->ParticipantKeyMaterial.transformation_kind = 
            std::array<uint8_t,4>(CRYPTO_TRANSFORMATION_KIND_AES128_GCM); //TODO (Santi) Define and implement a mechanism to change this via participant_properties
    
    (*PCrypto)->ParticipantKeyMaterial.master_salt.fill(0);
    RAND_bytes( (*PCrypto)->ParticipantKeyMaterial.master_salt.data(), 16 );  
    
    (*PCrypto)->ParticipantKeyMaterial.sender_key_id = make_unique_KeyId();
    
    (*PCrypto)->ParticipantKeyMaterial.master_sender_key.fill(0);
    RAND_bytes( (*PCrypto)->ParticipantKeyMaterial.master_sender_key.data(), 16 );
    
    (*PCrypto)->ParticipantKeyMaterial.receiver_specific_key_id = {0,0,0,0};  //No receiver specific, as this is the Master Participant Key
    (*PCrypto)->ParticipantKeyMaterial.master_receiver_specific_key.fill(0); 
    
    return PCrypto;
}
        
ParticipantCryptoHandle * AESGCMGMAC_KeyFactory::register_matched_remote_participant(
                ParticipantCryptoHandle &local_participant_crypto_handle, 
                IdentityHandle &remote_participant_identity, 
                PermissionsHandle &remote_participant_permissions, 
                SharedSecretHandle &shared_secret, 
                SecurityException &exception){

    //Extract information from the handshake. It will be needed in order to compute KeyMaterials
    const std::vector<uint8_t>* challenge_1 = SharedSecretHelper::find_data_value(**shared_secret,"Challenge1");
    const std::vector<uint8_t>* shared_secret_ss = SharedSecretHelper::find_data_value(**shared_secret,"SharedSecret");
    const std::vector<uint8_t>* challenge_2 = SharedSecretHelper::find_data_value(**shared_secret,"Challenge2");
    if( (challenge_1 == nullptr) | (shared_secret_ss == nullptr) | (challenge_2 == nullptr) ){
        exception = SecurityException("Unable to read SharedSecret and Challenges");
        return nullptr;
    }

    //Create Participant2ParticipantKeyMaterial (Based on local ParticipantKeyMaterial) and ParticipantKxKeyMaterial (based on the SharedSecret)
    //Put both elements in the local and remote ParticipantCryptoHandle
    
    AESGCMGMAC_ParticipantCryptoHandle& local_participant_handle = AESGCMGMAC_ParticipantCryptoHandle::narrow(local_participant_crypto_handle);
    AESGCMGMAC_ParticipantCryptoHandle* RPCrypto = new AESGCMGMAC_ParticipantCryptoHandle(); // Remote Participant CryptoHandle, to be returned at the end of the function 

    /*Fill values for Participant2ParticipantKeyMaterial - Used to encrypt outgoing data */
    { //scope for temp var buffer
        KeyMaterial_AES_GCM_GMAC buffer;  //Buffer = Participant2ParticipantKeyMaterial
        
        //These values must match the ones in ParticipantKeymaterial
        buffer.transformation_kind = local_participant_handle->ParticipantKeyMaterial.transformation_kind;
        buffer.master_salt = local_participant_handle->ParticipantKeyMaterial.master_salt;
        buffer.master_sender_key = local_participant_handle->ParticipantKeyMaterial.master_sender_key;
        //Generation of remainder values (Remote specific key)
        buffer.sender_key_id = make_unique_KeyId();
        buffer.receiver_specific_key_id = make_unique_KeyId();
        buffer.master_receiver_specific_key.fill(0);
        RAND_bytes( buffer.master_receiver_specific_key.data(), 16 );

        //Attach to both local and remote CryptoHandles
        (*RPCrypto)->Participant2ParticipantKeyMaterial.push_back(buffer);
        local_participant_handle->Participant2ParticipantKeyMaterial.push_back(buffer);
    }

    /*Fill values for Participant2ParticipantKxKeyMaterial - Used to encrypt CryptoTokens (exchange of key info) */
    { //scope for temp var buffer
        KeyMaterial_AES_GCM_GMAC buffer; //Buffer = Participant2ParticipantKxKeyMaterial

        buffer.transformation_kind = std::array<uint8_t,4>(CRYPTO_TRANSFORMATION_KIND_AES128_GMAC);
        buffer.master_salt.fill(0);


        std::array<uint8_t,32> concatenation; //Assembly of the source concatenated sequence that is used to generate master_salt
        
        std::string KxKeyCookie("key exchange key");
        concatenation.fill(0);
        memcpy(concatenation.data(),challenge_1->data(),challenge_1->size());
        memcpy(concatenation.data() + challenge_1->size(), KxKeyCookie.data(), 16);
        memcpy(concatenation.data() + challenge_1->size() + 16, challenge_2->data(), challenge_2->size());

        //concatenation is used to generate the key to encode the shared_secret and produce the master_salt
        buffer.master_salt.fill(0);
        std::array<uint8_t,32> p_master_salt;
        if(!EVP_Digest(concatenation.data(), challenge_1->size() + 16 + challenge_2->size(), p_master_salt.data(), NULL, EVP_sha256(), NULL)){
            //TODO(Santi) Provide insight
            delete RPCrypto;
            return nullptr;
        }
        //The result of p_master_salt is now the key to perform an HMACsha256 of the shared secret
        EVP_PKEY *key = EVP_PKEY_new_mac_key(EVP_PKEY_HMAC, NULL, p_master_salt.data(), 32);
        EVP_MD_CTX ctx;
        EVP_MD_CTX_init(&ctx);
        EVP_DigestSignInit(&ctx, NULL, EVP_sha256(), NULL, key);
        EVP_DigestSignUpdate(&ctx, shared_secret_ss->data(), shared_secret_ss->size());
        size_t length = 0;
        EVP_DigestSignFinal(&ctx, NULL, &length);
        if(length > 32){
            exception = SecurityException("Encountered an error while creating KxKeyMaterials");
            delete RPCrypto;
            return nullptr;
        }
        EVP_DigestSignFinal(&ctx, buffer.master_salt.data(), &length);
        EVP_PKEY_free(key);
        EVP_MD_CTX_cleanup(&ctx);

        //Repeat process - concatenation is used to store the sequence to generate master_sender_key
        std::string KxSaltCookie("keyexchange salt");
        concatenation.fill(0);
        memcpy(concatenation.data(), challenge_2->data(), challenge_2->size());
        memcpy(concatenation.data() + challenge_2->size(), KxSaltCookie.data(), 16);
        memcpy(concatenation.data() + challenge_2->size() + 16, challenge_1->data(), challenge_1->size() );
        //Compute key to produce master_sender_key
        buffer.master_sender_key.fill(0);
        if(!EVP_Digest(concatenation.data(), challenge_1->size() + 16 + challenge_2->size(), p_master_salt.data(), NULL, EVP_sha256(), NULL)){
            delete RPCrypto;
            return nullptr;
        }
        //The result of p_master_salt is now the key to perform an HMACsha256 of the shared secret that will go into master_sender_key
        key = EVP_PKEY_new_mac_key(EVP_PKEY_HMAC, NULL, p_master_salt.data(), 32);
        EVP_MD_CTX_init(&ctx);

        EVP_DigestSignInit(&ctx, NULL, EVP_sha256(), NULL, key);
        EVP_DigestSignUpdate(&ctx, shared_secret_ss->data(), shared_secret_ss->size());
        length = 0;
        EVP_DigestSignFinal(&ctx, NULL, &length);
        if(length > 32){
            //TODO (Santi) Provide insight 
            delete RPCrypto;
            return nullptr;
        }
        EVP_DigestSignFinal(&ctx, buffer.master_sender_key.data(), &length);
        EVP_PKEY_free(key);

        buffer.sender_key_id.fill(0); //Specified by standard
        buffer.receiver_specific_key_id.fill(0); //Specified by standard
        buffer.master_receiver_specific_key.fill(0); //Specified by standard

        //Attack to ParitipantCryptoHandles - both local and remote
        (*RPCrypto)->Participant2ParticipantKxKeyMaterial.push_back(buffer);
        local_participant_handle->Participant2ParticipantKxKeyMaterial.push_back(buffer);
    }

    return RPCrypto;
}

DatawriterCryptoHandle * AESGCMGMAC_KeyFactory::register_local_datawriter(
                const ParticipantCryptoHandle &participant_crypto,
                const PropertySeq &datawriter_prop,
                SecurityException &exception){

    exception = SecurityException("Not implemented");
    return nullptr;
}

DatareaderCryptoHandle * AESGCMGMAC_KeyFactory::register_matched_remote_datareader(
                const DatawriterCryptoHandle &local_datawriter_crypto_handle,
                const ParticipantCryptoHandle &lremote_participant_crypto,
                const SharedSecretHandle &shared_secret,
                const bool relay_only,
                SecurityException &exception){

    exception = SecurityException("Not implemented");
    return nullptr;
}

DatareaderCryptoHandle * AESGCMGMAC_KeyFactory::register_local_datareader(
                const ParticipantCryptoHandle &participant_crypto,
                const PropertySeq &datareader_properties,
                SecurityException &exception){

    exception = SecurityException("Not implemented");
    return nullptr;
}

DatawriterCryptoHandle * AESGCMGMAC_KeyFactory::register_matched_remote_datawriter(
                const DatareaderCryptoHandle &local_datareader_crypto_handle,
                const ParticipantCryptoHandle &remote_participant_crypt,
                const SharedSecretHandle &shared_secret,
                SecurityException &exception){

    exception = SecurityException("Not implemented");
    return nullptr;
}

bool AESGCMGMAC_KeyFactory::unregister_participant(
                ParticipantCryptoHandle* participant_crypto_handle,
                SecurityException &exception){
   
    bool return_code = false;
    
    //De-register the IDs
    AESGCMGMAC_ParticipantCryptoHandle& local_participant = AESGCMGMAC_ParticipantCryptoHandle::narrow(*participant_crypto_handle);
    for(std::vector<CryptoTransformKeyId>::iterator it = m_CryptoTransformKeyIds.begin(); it != m_CryptoTransformKeyIds.end();it++){
        if( (*it) == local_participant->ParticipantKeyMaterial.sender_key_id ){
            m_CryptoTransformKeyIds.erase(it);
                return_code = true;
        }
    }

    if(return_code){
        return true;
    }else{
        exception = SecurityException("Tried to unregister a participant not present in the plugin");
    }
    return false;
}
        
bool AESGCMGMAC_KeyFactory::unregister_datawriter(
                const DatawriterCryptoHandle &datawriter_crypto_handle,
                SecurityException &exception){

    exception = SecurityException("Not implemented");
    return false;
}

        
bool AESGCMGMAC_KeyFactory::unregister_datareader(
                const DatareaderCryptoHandle &datareader_crypto_handle,
                SecurityException &exception){

    exception = SecurityException("Not implemented");
    return false;
}

CryptoTransformKeyId AESGCMGMAC_KeyFactory::make_unique_KeyId(){
    CryptoTransformKeyId buffer;
    bool unique = false;

    while(!unique){
        RAND_bytes(buffer.data(),4);
        unique = true;
        //Iterate existing KeyIds to see if one is matching
        for(std::vector<CryptoTransformKeyId>::iterator it=m_CryptoTransformKeyIds.begin(); it!=m_CryptoTransformKeyIds.end();it++){
            if(*it == buffer)   unique = false;
        }
    }
    return buffer;
}
