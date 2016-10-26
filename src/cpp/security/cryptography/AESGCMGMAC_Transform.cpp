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
 * @file AESGCMGMAC_Transform.cpp
 */

#include "AESGCMGMAC_Transform.h"

#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <cstring>
using namespace eprosima::fastrtps::rtps::security;

AESGCMGMAC_Transform::AESGCMGMAC_Transform(){}
AESGCMGMAC_Transform::~AESGCMGMAC_Transform(){}

bool AESGCMGMAC_Transform::encode_serialized_payload(
                std::vector<uint8_t> &encoded_buffer,
                std::vector<uint8_t> &extra_inline_qos,
                const std::vector<uint8_t> &plain_buffer,
                const DatawriterCryptoHandle &sending_datawriter_crypto,
                SecurityException &exception){

    exception = SecurityException("Not implemented");
    return false;
}
         
bool AESGCMGMAC_Transform::encode_datawriter_submessage(
                std::vector<uint8_t> &encoded_rtps_submessage,
                const std::vector<uint8_t> &plain_rtps_submessage,
                const DatawriterCryptoHandle &sending_datawriter_crypto,
                const std::vector<DatareaderCryptoHandle> receiving_datareader_crypto_list,
                SecurityException &exception){

    exception = SecurityException("Not implemented");
    return false;
}
    
bool AESGCMGMAC_Transform::encode_datareader_submessage(
                std::vector<uint8_t> &encoded_rtps_submessage,
                const std::vector<uint8_t> &plain_rtps_submessage,
                const DatareaderCryptoHandle &sending_datareader_crypto,
                const std::vector<DatawriterCryptoHandle> &receiving_datawriter_crypto_list,
                SecurityException &exception){

    exception = SecurityException("Not implemented");
    return false;
}
    
bool AESGCMGMAC_Transform::encode_rtps_message(
                std::vector<uint8_t> &encoded_rtps_message,
                const std::vector<uint8_t> &plain_rtps_message,
                ParticipantCryptoHandle &sending_crypto,
                const std::vector<ParticipantCryptoHandle> &receiving_crypto_list,
                SecurityException &exception){

    AESGCMGMAC_ParticipantCryptoHandle& local_participant = AESGCMGMAC_ParticipantCryptoHandle::narrow(sending_crypto);
    CipherData *m_cipherdata = nullptr;

    //Step 1 - Find the CriptoData associated to the Participant
    {

        std::map<CryptoTransformKeyId,CipherData>::iterator it= status.find(local_participant->ParticipantKeyMaterial.sender_key_id);
        if(it == status.end()){
            //First time the Participant sends data - Init struct
            CipherData new_data;
            new_data.master_key_id = local_participant->ParticipantKeyMaterial.sender_key_id;
            RAND_bytes( (unsigned char *)(&(new_data.session_id)), sizeof(uint16_t));
            
            new_data.max_blocks_per_session= 12000;//TODO (Santi) - This ough to be configurable
            new_data.session_block_counter= new_data.max_blocks_per_session; //Set to maximum so computation of new SessionKey is triggered
            status[local_participant->ParticipantKeyMaterial.sender_key_id] = new_data;
            it= status.find(local_participant->ParticipantKeyMaterial.sender_key_id);
            m_cipherdata = &(it->second);
        }else{
            m_cipherdata = &(it->second);
        }
    }
    //Step 2 - If the maximum number of blocks have been processed, generate a new SessionKey
    if(m_cipherdata->session_block_counter >= m_cipherdata->max_blocks_per_session){
        m_cipherdata->session_id += 1; 

        //Computation of SessionKey - Either because it has expired or because its the Participants first message
        unsigned char *source = (unsigned char*)malloc(32 + 10 + 32 + 2);
        memcpy(source, local_participant->ParticipantKeyMaterial.master_sender_key.data(), 32); 
        char seq[] = "SessionKey";
        memcpy(source+32, seq, 10);
        memcpy(source+32+10, local_participant->ParticipantKeyMaterial.master_salt.data(),32);
        memcpy(source+32+10+32, &(m_cipherdata->session_id),4);

        if(!EVP_Digest(source, 32+10+32+2, (unsigned char*)&(m_cipherdata->SessionKey), NULL, EVP_sha256(), NULL)){
            //logError(CRYPTOGRAPHY, "Could not compute sha256");
            delete(source);
            return false;
        }
        delete(source);
        //ReceiverSpecific keys shall be computed on site

        m_cipherdata->session_block_counter = 0;
    }

    //Step 2.5 - Build remaining NONCE elements
    uint64_t initialization_vector_suffix;  //iv suffix changes with every operation
    RAND_bytes( (unsigned char*)(&initialization_vector_suffix), sizeof(uint64_t) );
    std::array<uint8_t,12> initialization_vector; //96 bytes, session_id + suffix
    memcpy(initialization_vector.data(),&(m_cipherdata->session_id),4);
    memcpy(initialization_vector.data()+4,&initialization_vector_suffix,8);
    //Step 3 - Build SecureDataHeader
    SecureDataHeader header;
    
    header.transform_identifier.transformation_kind = local_participant->ParticipantKeyMaterial.transformation_kind;
    header.transform_identifier.transformation_key_id = local_participant->ParticipantKeyMaterial.sender_key_id;
    memcpy( &(header.session_id), &(m_cipherdata->session_id), 4);
    memcpy( &(header.initialization_vector_suffix), &initialization_vector_suffix, 8);


    //Step 4 -Cypher the plain rtps message -> SecureDataBody
    OpenSSL_add_all_ciphers();
    int rv = RAND_load_file("/dev/urandom", 32); //Init random number gen

    size_t enc_length = plain_rtps_message.size()*3;
    std::vector<uint8_t> output;
    output.resize(enc_length,'\0');

    unsigned char tag[AES_BLOCK_SIZE]; //Container for the Authentication Tag 
    
    int actual_size=0, final_size=0;
    EVP_CIPHER_CTX* e_ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit(e_ctx, EVP_aes_128_gcm(), (const unsigned char*)plain_rtps_message.data(), initialization_vector.data());
    EVP_EncryptUpdate(e_ctx, output.data(), &actual_size, (const unsigned char*)plain_rtps_message.data(), plain_rtps_message.size());
    EVP_EncryptFinal(e_ctx, output.data() + actual_size, &final_size);
    EVP_CIPHER_CTX_ctrl(e_ctx, EVP_CTRL_GCM_GET_TAG, 16, tag);
    output.resize(actual_size+final_size);
    EVP_CIPHER_CTX_free(e_ctx);

    //Step 4.5 - Copy the results into SecureDataBody
    SecureDataBody body;
    body.secure_data.resize(output.size());
    memcpy(body.secure_data.data(),output.data(),output.size());

    //Step 5 - Build Secure DataTag
    SecureDataTag dataTag;
    memcpy(dataTag.common_mac.data(),tag, 16);
    //for(...
        //tag.receiver_specific_macs; //Placeholder


    //Step 5 - Assemble the message


    exception = SecurityException("Not implemented");
    return false;
}

bool AESGCMGMAC_Transform::decode_rtps_message(
                std::vector<uint8_t> &plain_buffer,
                const std::vector<uint8_t> &encoded_buffer,
                const ParticipantCryptoHandle &receiving_crypto,
                const ParticipantCryptoHandle &sending_crypto,
                SecurityException &exception){

    exception = SecurityException("Not implemented");
    return false;
}
        
bool AESGCMGMAC_Transform::preprocess_secure_submsg(
                DatawriterCryptoHandle &datawriter_crypto,
                DatareaderCryptoHandle &datareader_crypto,
                SecureSubmessageCategory_t &secure_submessage_category,
                const std::vector<uint8_t> encoded_rtps_submessage,
                const ParticipantCryptoHandle &receiving_crypto,
                const ParticipantCryptoHandle &sending_crypto,
                SecurityException &exception){

    exception = SecurityException("Not implemented");
    return false;
}

bool AESGCMGMAC_Transform::decode_datawriter_submessage(
                std::vector<uint8_t> &plain_rtps_submessage,
                const std::vector<uint8_t> &encoded_rtps_submessage,
                const DatareaderCryptoHandle &receiving_datareader_crypto,
                const DatawriterCryptoHandle &sending_datawriter_cryupto,
                SecurityException &exception){

    exception = SecurityException("Not implemented");
    return false;
}

bool AESGCMGMAC_Transform::decode_datareader_submessage(
                std::vector<uint8_t> &plain_rtps_submessage,
                const std::vector<uint8_t> &encoded_rtps_submessage,
                const DatawriterCryptoHandle &receiving_datawriter_crypto,
                const DatareaderCryptoHandle &sending_datareader_crypto,
                SecurityException &exception){

    exception = SecurityException("Not implemented");
    return false;
}


bool AESGCMGMAC_Transform::decode_serialized_payload(
                std::vector<uint8_t> &plain_buffer,
                const std::vector<uint8_t> &encoded_buffer,
                const std::vector<uint8_t> &inline_qos,
                const DatareaderCryptoHandle &receiving_datareader_crypto,
                const DatawriterCryptoHandle &sending_datawriter_crypto,
                SecurityException &exception){

    exception = SecurityException("Not implemented");
    return false;
}

