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
                const ParticipantCryptoHandle &sending_crypto,
                const std::vector<ParticipantCryptoHandle> &receiving_crypto_list,
                SecurityException &exception){

    unsigned char *key;
    unsigned char *iv;

    //EVP_CIPHER_CTX *ctx =  EVP_CIPHER_CTX_new();
    //EVP_EncryptInit(ctx, EVP_aes_128_gcm(), key, iv);
    /*
    // plain_rts_message -> unsigned char *
    int plaintext_len = plain_rtps_message.length();
    // key -> unsigned char *
    
    // iv -> unsigned char *
    
    EVP_CIPHER_CTX *ctx;
    
    if(!(ctx = EVP_CIPHER_CTX_new())){
        exception = SecurityException("Unagle to generate ctx");
        return false;
    }
    if(!EVP_ENCRYPTInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv)){
        exception = SecurityException("Unable to init encryption");
        return false;
    }
    if(!EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len){
        exception = SecurityException("Unable to encrypt");
        return false;
    }
    if(!EVP_EncryptFinal_ex(ctx, ciphertext, &len, plaintext, plaintext_len)){
        exception = SecurityException("Unable to finish encyption");
        return false;
    }
    */
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

