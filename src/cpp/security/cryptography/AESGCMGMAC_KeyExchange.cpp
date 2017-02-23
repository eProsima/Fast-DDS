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
 * @file AESGCMGMAC_KeyExchange.cpp
 */

#include <sstream>
#include <fastrtps/rtps/common/Token.h>
#include <fastrtps/rtps/common/BinaryProperty.h>
#include "AESGCMGMAC_KeyExchange.h"
#include <fastrtps/log/Log.h>

#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

// Solve error with Win32 macro
#ifdef WIN32
#undef max
#endif

using namespace eprosima::fastrtps::rtps::security;

AESGCMGMAC_KeyExchange::AESGCMGMAC_KeyExchange(){}
AESGCMGMAC_KeyExchange::~AESGCMGMAC_KeyExchange(){}

bool AESGCMGMAC_KeyExchange::create_local_participant_crypto_tokens(
            ParticipantCryptoTokenSeq& local_participant_crypto_tokens,
            const ParticipantCryptoHandle& local_participant_crypto,
            ParticipantCryptoHandle& remote_participant_crypto,
            SecurityException& /*exception*/)
{

    const AESGCMGMAC_ParticipantCryptoHandle& local_participant = AESGCMGMAC_ParticipantCryptoHandle::narrow(local_participant_crypto);
    AESGCMGMAC_ParticipantCryptoHandle& remote_participant = AESGCMGMAC_ParticipantCryptoHandle::narrow(remote_participant_crypto);

    if( local_participant.nil() || remote_participant.nil() ){
        logWarning(SECURITY_CRYPTO, "Not a valid ParticipantCryptoHandle received");
        return false;
    }

    //Flush previously present CryptoTokens
    local_participant_crypto_tokens.clear();

    //Participant2ParticipantKeyMaterial will be come RemoteParticipant2ParticipantKeyMaterial on the other side
    {
        //Only the KeyMaterial used in conjunction with the remote_participant are tokenized. In this implementation only on Pariticipant2ParticipantKeyMaterial exists per matched Participant
        ParticipantCryptoToken temp;
        temp.class_id() = std::string("DDS:Crypto:AES_GCM_GMAC");
        BinaryProperty prop;
        prop.name() = std::string("dds.cryp.keymat");
        std::vector<uint8_t> plaintext= KeyMaterialCDRSerialize(remote_participant->Participant2ParticipantKeyMaterial.at(0));
        prop.value() = aes_128_gcm_encrypt(plaintext, remote_participant->Participant2ParticipantKxKeyMaterial.at(0).master_sender_key);
        prop.propagate(true);
        temp.binary_properties().push_back(std::move(prop));
        local_participant_crypto_tokens.push_back(std::move(temp));
    }

    return true;
}

bool AESGCMGMAC_KeyExchange::set_remote_participant_crypto_tokens(
            const ParticipantCryptoHandle &local_participant_crypto,
            ParticipantCryptoHandle &remote_participant_crypto,
            const ParticipantCryptoTokenSeq &remote_participant_tokens,
            SecurityException &exception){

    const AESGCMGMAC_ParticipantCryptoHandle& local_participant = AESGCMGMAC_ParticipantCryptoHandle::narrow(local_participant_crypto);
    AESGCMGMAC_ParticipantCryptoHandle& remote_participant = AESGCMGMAC_ParticipantCryptoHandle::narrow(remote_participant_crypto);

    if( local_participant.nil() || remote_participant.nil() ){
        logWarning(SECURITY_CRYPTO, "Not a valid ParticipantCryptoHandle received");
        return false;
    }
    //As only relevant KeyMaterials are tokenized, only one Token is exchanged
    if(remote_participant_tokens.size() != 1){
        logWarning(SECURITY_CRYPTO, "Invalid CryptoTokenSeq length");
        exception = SecurityException("Incorrect remote CryptoSequence length");
        return false;
    }
    if(remote_participant_tokens.at(0).class_id() != "DDS:Crypto:AES_GCM_GMAC"){
        logWarning(SECURITY_CRYPTO, "MalformedCryptoToken");
        exception = SecurityException("Incorrect token type received");
        return false;
    }
    if(remote_participant_tokens.at(0).binary_properties().size() !=1 || remote_participant_tokens.at(0).properties().size() != 0 ||
        remote_participant_tokens.at(0).binary_properties().at(0).name() != "dds.cryp.keymat")
    {
        logWarning(SECURITY_CRYPTO, "MalformedCryptoToken");
        exception = SecurityException("Malformed CryptoToken");
        return false;
    }
    //Valid CryptoToken, we can decrypt and push the resulting KeyMaterial in as a RemoteParticipant2ParticipantKeyMaterial
    std::vector<uint8_t> plaintext = aes_128_gcm_decrypt(remote_participant_tokens.at(0).binary_properties().at(0).value(),
        remote_participant->Participant2ParticipantKxKeyMaterial.at(0).master_sender_key);

    KeyMaterial_AES_GCM_GMAC keymat;
    keymat = KeyMaterialCDRDeserialize(&plaintext);
    remote_participant->RemoteParticipant2ParticipantKeyMaterial.push_back(keymat);

    return true;
}

bool AESGCMGMAC_KeyExchange::create_local_datawriter_crypto_tokens(
            DatawriterCryptoTokenSeq &local_datawriter_crypto_tokens,
            DatawriterCryptoHandle &local_datawriter_crypto,
            DatareaderCryptoHandle &remote_datareader_crypto,
            SecurityException& /*exception*/){

    AESGCMGMAC_WriterCryptoHandle& local_writer = AESGCMGMAC_WriterCryptoHandle::narrow(local_datawriter_crypto);
    AESGCMGMAC_ReaderCryptoHandle& remote_reader = AESGCMGMAC_ReaderCryptoHandle::narrow(remote_datareader_crypto);

    if( local_writer.nil() || remote_reader.nil() ){
        logWarning(SECURITY_CRYPTO,"Invalid CryptoHandle received");
        return false;
    }

    std::unique_lock<std::mutex> lock(remote_reader->mutex_);

    //Flush previously present CryptoTokens
    local_datawriter_crypto_tokens.clear();
    //Only the KeyMaterial used in conjunction with the remote_participant are tokenized. In this implementation only on Pariticipant2ParticipantKeyMaterial exists per matched Participant
    DatawriterCryptoToken temp;
    temp.class_id() = std::string("DDS:Crypto:AES_GCM_GMAC");
    BinaryProperty prop;
    prop.name() = std::string("dds.cryp.keymat");
    std::vector<uint8_t> plaintext= KeyMaterialCDRSerialize(remote_reader->Writer2ReaderKeyMaterial.at(0));
    prop.value() = aes_128_gcm_encrypt(plaintext, remote_reader->Participant2ParticipantKxKeyMaterial.master_sender_key);
    prop.propagate(true);
    temp.binary_properties().push_back(std::move(prop));
    local_datawriter_crypto_tokens.push_back(std::move(temp));

    return true;
}

bool AESGCMGMAC_KeyExchange::create_local_datareader_crypto_tokens(
            DatareaderCryptoTokenSeq &local_datareader_crypto_tokens,
            DatareaderCryptoHandle &local_datareader_crypto,
            DatawriterCryptoHandle &remote_datawriter_crypto,
            SecurityException& /*exception*/){

    AESGCMGMAC_ReaderCryptoHandle& local_reader = AESGCMGMAC_ReaderCryptoHandle::narrow(local_datareader_crypto);
    AESGCMGMAC_WriterCryptoHandle& remote_writer = AESGCMGMAC_WriterCryptoHandle::narrow(remote_datawriter_crypto);

    if( local_reader.nil() || remote_writer.nil() ){
        logWarning(SECURITY_CRYPTO,"Invalid CryptoHandle received");
        return false;
    }

    std::unique_lock<std::mutex> lock(remote_writer->mutex_);

    local_datareader_crypto_tokens.clear();
    //Participant2ParticipantKeyMaterial will be come RemoteParticipant2ParticipantKeyMaterial on the other side
    {
        //Only the KeyMaterial used in conjunction with the remote_participant are tokenized. In this implementation only on Pariticipant2ParticipantKeyMaterial exists per matched Participant
        DatareaderCryptoToken temp;
        temp.class_id() = std::string("DDS:Crypto:AES_GCM_GMAC");
        BinaryProperty prop;
        prop.name() = std::string("dds.cryp.keymat");
        std::vector<uint8_t> plaintext= KeyMaterialCDRSerialize(remote_writer->Reader2WriterKeyMaterial.at(0));
        prop.value() = aes_128_gcm_encrypt(plaintext, remote_writer->Participant2ParticipantKxKeyMaterial.master_sender_key);
        prop.propagate(true);
        temp.binary_properties().push_back(std::move(prop));
        local_datareader_crypto_tokens.push_back(std::move(temp));
    }
    return true;
}

bool AESGCMGMAC_KeyExchange::set_remote_datareader_crypto_tokens(
            DatawriterCryptoHandle &local_datawriter_crypto,
            DatareaderCryptoHandle &remote_datareader_crypto,
            const DatareaderCryptoTokenSeq &remote_datareader_tokens,
            SecurityException &exception){

    AESGCMGMAC_WriterCryptoHandle& local_writer = AESGCMGMAC_WriterCryptoHandle::narrow(local_datawriter_crypto);
    AESGCMGMAC_ReaderCryptoHandle& remote_reader = AESGCMGMAC_ReaderCryptoHandle::narrow(remote_datareader_crypto);

    if( local_writer.nil() || remote_reader.nil() ){
        logWarning(SECURITY_CRYPTO,"Invalid CryptoHandle received");
        return false;
    }
    //As only relevant KeyMaterials are tokenized, only one Token is exchanged
    if(remote_datareader_tokens.size() != 1){
        logWarning(SECURITY_CRYPTO,"Malformed CryptoTokenSequence");
        exception = SecurityException("Incorrect remote CryptoSequence length");
        return false;
    }
    if(remote_datareader_tokens.at(0).class_id() != "DDS:Crypto:AES_GCM_GMAC"){
        logWarning(SECURITY_CRYPTO,"Malformed CryptoToken");
        exception = SecurityException("Incorrect token type received");
        return false;
    }

    if(remote_datareader_tokens.at(0).binary_properties().size() !=1 || remote_datareader_tokens.at(0).properties().size() != 0 ||
            remote_datareader_tokens.at(0).binary_properties().at(0).name() != "dds.cryp.keymat")
    {
        logWarning(SECURITY_CRYPTO,"Malformed CryptoToken");
        exception = SecurityException("Malformed CryptoToken");
        return false;
    }

    std::unique_lock<std::mutex> remote_reader_lock(remote_reader->mutex_);

    //Valid CryptoToken, we can decrypt and push the resulting KeyMaterial in as a RemoteParticipant2ParticipantKeyMaterial
    std::vector<uint8_t> plaintext = aes_128_gcm_decrypt(remote_datareader_tokens.at(0).binary_properties().at(0).value(),
            remote_reader->Participant2ParticipantKxKeyMaterial.master_sender_key);

    KeyMaterial_AES_GCM_GMAC keymat;
    keymat = KeyMaterialCDRDeserialize(&plaintext);
    remote_reader->Reader2WriterKeyMaterial.push_back(keymat);

    remote_reader_lock.unlock();

    std::unique_lock<std::mutex> local_writer_lock(local_writer->mutex_);

    local_writer->Reader2WriterKeyMaterial.push_back(keymat);

    return true;
 }

bool AESGCMGMAC_KeyExchange::set_remote_datawriter_crypto_tokens(
             DatareaderCryptoHandle &local_datareader_crypto,
             DatawriterCryptoHandle &remote_datawriter_crypto,
             const DatawriterCryptoTokenSeq &remote_datawriter_tokens,
             SecurityException &exception){

    AESGCMGMAC_ReaderCryptoHandle& local_reader = AESGCMGMAC_ReaderCryptoHandle::narrow(local_datareader_crypto);
    AESGCMGMAC_WriterCryptoHandle& remote_writer = AESGCMGMAC_WriterCryptoHandle::narrow(remote_datawriter_crypto);

    if( local_reader.nil() || remote_writer.nil() ){
        logWarning(SECURITY_CRYPTO,"Invalid CryptoHandle"); 
        return false;
    }
    //As only relevant KeyMaterials are tokenized, only one Token is exchanged
    if(remote_datawriter_tokens.size() != 1){
        logWarning(SECURITY_CRYPTO,"Malformed CryptoTokenSequence");
        exception = SecurityException("Incorrect remote CryptoSequence length");
        return false;
    }
    if(remote_datawriter_tokens.at(0).class_id() != "DDS:Crypto:AES_GCM_GMAC"){
        logWarning(SECURITY_CRYPTO,"Malformed CryptoToken");
        exception = SecurityException("Incorrect token type received");
        return false;
    }

    if(remote_datawriter_tokens.at(0).binary_properties().size() !=1 || remote_datawriter_tokens.at(0).properties().size() != 0 ||
            remote_datawriter_tokens.at(0).binary_properties().at(0).name() != "dds.cryp.keymat")
    {
        logWarning(SECURITY_CRYPTO,"Malformed CryptoToken");
        exception = SecurityException("Malformed CryptoToken");
        return false;
    }

    std::unique_lock<std::mutex> remote_writer_lock(remote_writer->mutex_);

    //Valid CryptoToken, we can decrypt and push the resulting KeyMaterial in as a RemoteParticipant2ParticipantKeyMaterial
    std::vector<uint8_t> plaintext = aes_128_gcm_decrypt(remote_datawriter_tokens.at(0).binary_properties().at(0).value(),
            remote_writer->Participant2ParticipantKxKeyMaterial.master_sender_key);

    KeyMaterial_AES_GCM_GMAC keymat;
    keymat = KeyMaterialCDRDeserialize(&plaintext);

    remote_writer->Writer2ReaderKeyMaterial.push_back(keymat);

    remote_writer_lock.unlock();

    std::unique_lock<std::mutex> local_writer_lock(local_reader->mutex_);

    local_reader->Writer2ReaderKeyMaterial.push_back(keymat);

    return true;
}

bool AESGCMGMAC_KeyExchange::return_crypto_tokens(
            const CryptoTokenSeq& /*crypto_tokens*/,
            SecurityException &exception)
{

    exception = SecurityException("Not implemented");
    return false;
}

std::vector<uint8_t> AESGCMGMAC_KeyExchange::KeyMaterialCDRSerialize(KeyMaterial_AES_GCM_GMAC &key){

std::vector<uint8_t> buffer;

    buffer.push_back(4);
    for(int i=0;i<4;i++){
        buffer.push_back(key.transformation_kind[i]);
    }
    buffer.push_back(2);
    for(int i=0;i<32;i++){
        buffer.push_back(key.master_salt[i]);
    }
    buffer.push_back(4);
    for(int i=0;i<4;i++){
        buffer.push_back(key.sender_key_id[i]);
    }
    buffer.push_back(32);
    for(int i=0;i<32;i++){
        buffer.push_back(key.master_sender_key[i]);
    }
    buffer.push_back(4);
    for(int i=0;i<4;i++){
        buffer.push_back(key.receiver_specific_key_id[i]);
    }
    buffer.push_back(32);
    for(int i=0;i<32;i++){
        buffer.push_back(key.master_receiver_specific_key[i]);
    }

    return buffer;
}

KeyMaterial_AES_GCM_GMAC AESGCMGMAC_KeyExchange::KeyMaterialCDRDeserialize(std::vector<uint8_t> *CDR)
{
    KeyMaterial_AES_GCM_GMAC buffer;
    for(int i=1; i<5; i++){
        buffer.transformation_kind[i-1] = CDR->at(i);
    }
    for(int i=6; i<38; i++){
        buffer.master_salt[i-6] = CDR->at(i);
    }
    for(int i=39; i<43; i++){
        buffer.sender_key_id[i-39] = CDR->at(i);
    }
    for(int i=44; i<76; i++){
        buffer.master_sender_key[i-44] = CDR->at(i);
    }
    for(int i=77; i<81; i++){
        buffer.receiver_specific_key_id[i-77] = CDR->at(i);
    }
    for(int i=82; i< 114; i++){
        buffer.master_receiver_specific_key[i-82] = CDR->at(i);
    }
    return buffer;
}

std::vector<uint8_t> AESGCMGMAC_KeyExchange::aes_128_gcm_encrypt(std::vector<uint8_t> plaintext, std::array<uint8_t,32> key)
{
    std::vector<uint8_t> output;

    if(plaintext.size() <= std::numeric_limits<int>::max())
    {
        size_t enc_length = plaintext.size() * 3; // TODO(Ricardo) Review size.
        output.resize(enc_length, '\0');

        unsigned char tag[AES_BLOCK_SIZE];
        unsigned char iv[AES_BLOCK_SIZE];
        RAND_bytes(iv, sizeof(iv));
        std::copy(iv, iv + 16, output.begin() + 16);

        int actual_size = 0, final_size = 0;
        EVP_CIPHER_CTX* e_ctx = EVP_CIPHER_CTX_new();
        EVP_EncryptInit(e_ctx, EVP_aes_128_gcm(), (const unsigned char*)key.data(), iv);
        EVP_EncryptUpdate(e_ctx, &output[32], &actual_size, (const unsigned char*)plaintext.data(), static_cast<int>(plaintext.size()));
        EVP_EncryptFinal(e_ctx, &output[32 + actual_size], &final_size);
        EVP_CIPHER_CTX_ctrl(e_ctx, EVP_CTRL_GCM_GET_TAG, 16, tag);
        std::copy(iv, iv + 16, output.begin());
        std::copy(iv, iv + 16, output.begin() + 16);
        output.resize(32 + actual_size + final_size);
        EVP_CIPHER_CTX_free(e_ctx);
    }

    return output;
}

std::vector<uint8_t> AESGCMGMAC_KeyExchange::aes_128_gcm_decrypt(std::vector<uint8_t> crypto, std::array<uint8_t,32> key)
{
    std::vector<uint8_t> plaintext;

    if(crypto.size() - 32 <= std::numeric_limits<int>::max())
    {
        unsigned char tag[AES_BLOCK_SIZE];
        unsigned char iv[AES_BLOCK_SIZE];
        std::copy(crypto.begin(), crypto.begin() + 16, tag);
        std::copy(crypto.begin() + 16, crypto.begin() + 32, iv);
        plaintext.resize(crypto.size(), '\0');

        int actual_size = 0, final_size = 0;
        EVP_CIPHER_CTX* d_ctx = EVP_CIPHER_CTX_new();
        EVP_DecryptInit(d_ctx, EVP_aes_128_gcm(), (const unsigned char*)key.data(), iv);
        EVP_DecryptUpdate(d_ctx, &plaintext[0], &actual_size, (const unsigned char*)crypto.data() + 32, static_cast<int>(crypto.size() - 32));
        EVP_CIPHER_CTX_ctrl(d_ctx, EVP_CTRL_GCM_SET_TAG, 16, tag);
        EVP_DecryptFinal(d_ctx, &plaintext[actual_size], &final_size);
        plaintext.resize(actual_size + final_size, '\0');
        EVP_CIPHER_CTX_free(d_ctx);
    }

    return plaintext;
}
