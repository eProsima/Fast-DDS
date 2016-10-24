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

#include "AESGCMGMAC_KeyFactory.h"

using namespace eprosima::fastrtps::rtps::security;

AESGCMGMAC_KeyFactory::AESGCMGMAC_KeyFactory(){}

ParticipantCryptoHandle * AESGCMGMAC_KeyFactory::register_local_participant(
                const IdentityHandle &participant_identity, 
                const PermissionsHandle &participant_permissions, 
                const PropertySeq &participant_properties, 
                SecurityException &exception){
//Create ParticipantKeyMaterial and return a handle
    AESGCMGMAC_ParticipantCryptoHandle* PCrypto = nullptr;
    if( (!participant_identity.nil()) | (!participant_permissions.nil()) ){
        exception = SecurityException("Invalid input parameters");
        return nullptr;
    }

    PCrypto = new AESGCMGMAC_ParticipantCryptoHandle();
    (*PCrypto)->ParticipantKeyMaterial = new KeyMaterial_AES_GCM_GMAC();
    //Fill CryptoData
    (*PCrypto)->ParticipantKeyMaterial->transformation_kind = 
            std::array<uint8_t,4>(CRYPTO_TRANSFORMATION_KIND_AES128_GCM); //Configurability should be provided via participant_properties
    (*PCrypto)->ParticipantKeyMaterial->master_salt.fill(0);
    RAND_bytes( (*PCrypto)->ParticipantKeyMaterial->master_salt.data(), 128 );  
    (*PCrypto)->ParticipantKeyMaterial->sender_key_id = make_unique_KeyId();
    (*PCrypto)->ParticipantKeyMaterial->master_sender_key.fill(0);
    RAND_bytes( (*PCrypto)->ParticipantKeyMaterial->master_sender_key.data(), 128 );
    (*PCrypto)->ParticipantKeyMaterial->receiver_specific_key_id = {0,0,0,0};  //No receiver specific, as this is the Master Participant Key
    (*PCrypto)->ParticipantKeyMaterial->master_receiver_specific_key.fill(0); 
    return PCrypto;
}
        
ParticipantCryptoHandle * AESGCMGMAC_KeyFactory::register_matched_remote_participant(
                ParticipantCryptoHandle &local_participant_crypto_handle, 
                IdentityHandle &remote_participant_identity, 
                PermissionsHandle &remote_participant_permissions, 
                SharedSecretHandle &shared_secret, 
                SecurityException &exception){
//Create Participant2ParticipantKeyMaterial (Based on local ParticipantKeyMaterial) and ParticipantKxKeyMaterial (based on the SharedSecret)
//Put both elements in the local and remote ParticipantCryptoHandle
    AESGCMGMAC_ParticipantCryptoHandle& local_participant_handle = AESGCMGMAC_ParticipantCryptoHandle::narrow(local_participant_crypto_handle);
    AESGCMGMAC_ParticipantCryptoHandle* RPCrypto = nullptr;
    if( (!remote_participant_identity.nil()) | (!remote_participant_permissions.nil()) ){
        exception = SecurityException("Invalid input parameters");
        return nullptr;
    }
    
    RPCrypto = new AESGCMGMAC_ParticipantCryptoHandle();  //This will be the remote participant crypto handle
   
    KeyMaterial_AES_GCM_GMAC* buffer = new KeyMaterial_AES_GCM_GMAC();  //Buffer = Participant2ParticipantKeyMaterial
    /*Fill values for Participant2ParticipantKey data*/

    //These values must match the ones in ParticipantKeymaterial
    buffer->transformation_kind = local_participant_handle->ParticipantKeyMaterial->transformation_kind;
    buffer->master_salt = local_participant_handle->ParticipantKeyMaterial->master_salt;
    buffer->master_sender_key = local_participant_handle->ParticipantKeyMaterial->master_sender_key;
    //Generation of remainder values (Remote specific key)
    buffer->sender_key_id = make_unique_KeyId();
    buffer->receiver_specific_key_id = make_unique_KeyId();
    buffer->master_receiver_specific_key.fill(0);
    RAND_bytes( buffer->master_receiver_specific_key.data(), 128 );

    //Attach to Keyhandles
    (*RPCrypto)->Participant2ParticipantKeyMaterial.push_back(buffer);
    local_participant_handle->Participant2ParticipantKeyMaterial.push_back(buffer);

    /*Fill values for Participant2ParticipantKxKey data*/
    buffer = new KeyMaterial_AES_GCM_GMAC();
    //Fill values for Participant2ParticipantKxKey
    buffer->transformation_kind = std::array<uint8_t,4>(CRYPTO_TRANSFORMATION_KIND_AES128_GMAC);
    buffer->master_salt.fill(0);
    RAND_bytes( buffer->master_salt.data(), 128); // To substitute with HMAC_sha
    buffer->sender_key_id.fill(0); //Specified by standard
    buffer->master_sender_key.fill(0);
    RAND_bytes( buffer->master_sender_key.data(), 128); // To substitute with HMAC_sha
    buffer->receiver_specific_key_id.fill(0); //Specified by standard
    buffer->master_receiver_specific_key.fill(0); //Specified by standard

    //Attack to Keyhandles
    (*RPCrypto)->Participant2ParticipantKeyMaterial.push_back(buffer);
    local_participant_handle->Participant2ParticipantKeyMaterial.push_back(buffer);

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
                const ParticipantCryptoHandle &participant_crypto_handle,
                SecurityException &exception){

    exception = SecurityException("Not implemented");
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
