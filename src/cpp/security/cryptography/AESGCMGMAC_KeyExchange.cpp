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

#include "AESGCMGMAC_KeyExchange.h"

#include <cstring>
#include <sstream>

#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/common/BinaryProperty.hpp>
#include <fastdds/rtps/common/Token.hpp>
#include <rtps/security/exceptions/SecurityException.h>

// Solve error with Win32 macro
#ifdef WIN32
#undef max
#endif // ifdef WIN32

using namespace eprosima::fastdds::rtps::security;

AESGCMGMAC_KeyExchange::AESGCMGMAC_KeyExchange()
{
}

AESGCMGMAC_KeyExchange::~AESGCMGMAC_KeyExchange()
{
}

bool AESGCMGMAC_KeyExchange::create_local_participant_crypto_tokens(
        ParticipantCryptoTokenSeq& local_participant_crypto_tokens,
        const ParticipantCryptoHandle& local_participant_crypto,
        ParticipantCryptoHandle& remote_participant_crypto,
        SecurityException& /*exception*/)
{

    const AESGCMGMAC_ParticipantCryptoHandle& local_participant = AESGCMGMAC_ParticipantCryptoHandle::narrow(
        local_participant_crypto);
    AESGCMGMAC_ParticipantCryptoHandle& remote_participant = AESGCMGMAC_ParticipantCryptoHandle::narrow(
        remote_participant_crypto);

    if (local_participant.nil() || remote_participant.nil())
    {
        EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "Not a valid ParticipantCryptoHandle received");
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
        std::vector<uint8_t> plaintext = KeyMaterialCDRSerialize(remote_participant->Participant2ParticipantKeyMaterial.at(
                            0));
        prop.value() = plaintext; //  aes_128_gcm_encrypt(plaintext, remote_participant->Participant2ParticipantKxKeyMaterial.at(0).master_sender_key);
        prop.propagate(true);

        if (prop.value().size() == 0)
        {
            return false;
        }

        temp.binary_properties().push_back(std::move(prop));
        local_participant_crypto_tokens.push_back(std::move(temp));
    }

    return true;
}

bool AESGCMGMAC_KeyExchange::set_remote_participant_crypto_tokens(
        const ParticipantCryptoHandle& local_participant_crypto,
        ParticipantCryptoHandle& remote_participant_crypto,
        const ParticipantCryptoTokenSeq& remote_participant_tokens,
        SecurityException& exception)
{

    const AESGCMGMAC_ParticipantCryptoHandle& local_participant = AESGCMGMAC_ParticipantCryptoHandle::narrow(
        local_participant_crypto);
    AESGCMGMAC_ParticipantCryptoHandle& remote_participant = AESGCMGMAC_ParticipantCryptoHandle::narrow(
        remote_participant_crypto);

    if (local_participant.nil() || remote_participant.nil())
    {
        EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "Not a valid ParticipantCryptoHandle received");
        return false;
    }
    //As only relevant KeyMaterials are tokenized, only one Token is exchanged
    if (remote_participant_tokens.size() != 1)
    {
        EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "Invalid CryptoTokenSeq length");
        exception = SecurityException("Incorrect remote CryptoSequence length");
        return false;
    }
    if (remote_participant_tokens.at(0).class_id() != "DDS:Crypto:AES_GCM_GMAC")
    {
        EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "MalformedCryptoToken");
        exception = SecurityException("Incorrect token type received");
        return false;
    }
    if (remote_participant_tokens.at(0).binary_properties().size() != 1 ||
            remote_participant_tokens.at(0).properties().size() != 0 ||
            remote_participant_tokens.at(0).binary_properties().at(0).name() != "dds.cryp.keymat")
    {
        EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "MalformedCryptoToken");
        exception = SecurityException("Malformed CryptoToken");
        return false;
    }
    //Valid CryptoToken, we can decrypt and push the resulting KeyMaterial in as a RemoteParticipant2ParticipantKeyMaterial
    std::vector<uint8_t> plaintext = remote_participant_tokens.at(0).binary_properties().at(0).value();
    // std::vector<uint8_t> plaintext = aes_128_gcm_decrypt(remote_participant_tokens.at(0).binary_properties().at(0).value(),
    //     remote_participant->Participant2ParticipantKxKeyMaterial.at(0).master_sender_key);

    if (plaintext.size() == 0)
    {
        return false;
    }

    KeyMaterial_AES_GCM_GMAC keymat;
    KeyMaterialCDRDeserialize(keymat, &plaintext);
    remote_participant->RemoteParticipant2ParticipantKeyMaterial.push_back(keymat);

    return true;
}

bool AESGCMGMAC_KeyExchange::create_local_datawriter_crypto_tokens(
        DatawriterCryptoTokenSeq& local_datawriter_crypto_tokens,
        DatawriterCryptoHandle& local_datawriter_crypto,
        DatareaderCryptoHandle& remote_datareader_crypto,
        SecurityException& /*exception*/)
{

    AESGCMGMAC_WriterCryptoHandle& local_writer = AESGCMGMAC_WriterCryptoHandle::narrow(local_datawriter_crypto);
    AESGCMGMAC_ReaderCryptoHandle& remote_reader = AESGCMGMAC_ReaderCryptoHandle::narrow(remote_datareader_crypto);

    if (local_writer.nil() || remote_reader.nil())
    {
        EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "Invalid CryptoHandle received");
        return false;
    }

    std::unique_lock<std::mutex> lock(remote_reader->mutex_);

    //Flush previously present CryptoTokens
    local_datawriter_crypto_tokens.clear();
    for (auto& it : remote_reader->Remote2EntityKeyMaterial)
    {
        //Only the KeyMaterial used in conjunction with the remote_participant are tokenized. In this implementation only on Pariticipant2ParticipantKeyMaterial exists per matched Participant
        DatawriterCryptoToken temp;
        temp.class_id() = std::string("DDS:Crypto:AES_GCM_GMAC");
        BinaryProperty prop;
        prop.name() = std::string("dds.cryp.keymat");
        std::vector<uint8_t> plaintext = KeyMaterialCDRSerialize(it);
        prop.value() = plaintext; // aes_128_gcm_encrypt(plaintext, remote_reader->Participant2ParticipantKxKeyMaterial.master_sender_key);
        prop.propagate(true);

        if (prop.value().size() == 0)
        {
            return false;
        }

        temp.binary_properties().push_back(std::move(prop));
        local_datawriter_crypto_tokens.push_back(std::move(temp));
    }

    return true;
}

bool AESGCMGMAC_KeyExchange::create_local_datareader_crypto_tokens(
        DatareaderCryptoTokenSeq& local_datareader_crypto_tokens,
        DatareaderCryptoHandle& local_datareader_crypto,
        DatawriterCryptoHandle& remote_datawriter_crypto,
        SecurityException& /*exception*/)
{

    AESGCMGMAC_ReaderCryptoHandle& local_reader = AESGCMGMAC_ReaderCryptoHandle::narrow(local_datareader_crypto);
    AESGCMGMAC_WriterCryptoHandle& remote_writer = AESGCMGMAC_WriterCryptoHandle::narrow(remote_datawriter_crypto);

    if (local_reader.nil() || remote_writer.nil())
    {
        EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "Invalid CryptoHandle received");
        return false;
    }

    std::unique_lock<std::mutex> lock(remote_writer->mutex_);

    local_datareader_crypto_tokens.clear();
    //Participant2ParticipantKeyMaterial will be come RemoteParticipant2ParticipantKeyMaterial on the other side
    for (auto& it : remote_writer->Remote2EntityKeyMaterial)
    {
        //Only the KeyMaterial used in conjunction with the remote_participant are tokenized. In this implementation only on Pariticipant2ParticipantKeyMaterial exists per matched Participant
        DatareaderCryptoToken temp;
        temp.class_id() = std::string("DDS:Crypto:AES_GCM_GMAC");
        BinaryProperty prop;
        prop.name() = std::string("dds.cryp.keymat");
        std::vector<uint8_t> plaintext = KeyMaterialCDRSerialize(it);
        prop.value() = plaintext; // aes_128_gcm_encrypt(plaintext, remote_writer->Participant2ParticipantKxKeyMaterial.master_sender_key);
        prop.propagate(true);

        if (prop.value().size() == 0)
        {
            return false;
        }

        temp.binary_properties().push_back(std::move(prop));
        local_datareader_crypto_tokens.push_back(std::move(temp));
    }
    return true;
}

bool AESGCMGMAC_KeyExchange::set_remote_datareader_crypto_tokens(
        DatawriterCryptoHandle& local_datawriter_crypto,
        DatareaderCryptoHandle& remote_datareader_crypto,
        const DatareaderCryptoTokenSeq& remote_datareader_tokens,
        SecurityException& exception)
{

    AESGCMGMAC_WriterCryptoHandle& local_writer = AESGCMGMAC_WriterCryptoHandle::narrow(local_datawriter_crypto);
    AESGCMGMAC_ReaderCryptoHandle& remote_reader = AESGCMGMAC_ReaderCryptoHandle::narrow(remote_datareader_crypto);

    if (local_writer.nil() || remote_reader.nil())
    {
        EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "Invalid CryptoHandle received");
        return false;
    }
    //As only relevant KeyMaterials are tokenized, only one or two Token are exchanged
    auto nTokens = remote_datareader_tokens.size();
    if (nTokens != 1 && nTokens != 2)
    {
        EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "Malformed CryptoTokenSequence");
        exception = SecurityException("Incorrect remote CryptoSequence length");
        return false;
    }
    for (size_t i = 0; i < nTokens; i++)
    {
        if (remote_datareader_tokens.at(i).class_id() != "DDS:Crypto:AES_GCM_GMAC")
        {
            EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "Malformed CryptoToken");
            exception = SecurityException("Incorrect token type received");
            return false;
        }

        if (remote_datareader_tokens.at(i).binary_properties().size() != 1 ||
                remote_datareader_tokens.at(i).properties().size() != 0 ||
                remote_datareader_tokens.at(i).binary_properties().at(0).name() != "dds.cryp.keymat")
        {
            EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "Malformed CryptoToken");
            exception = SecurityException("Malformed CryptoToken");
            return false;
        }

        std::unique_lock<std::mutex> remote_reader_lock(remote_reader->mutex_);

        //Valid CryptoToken, we can decrypt and push the resulting KeyMaterial in as a RemoteParticipant2ParticipantKeyMaterial
        std::vector<uint8_t> plaintext = remote_datareader_tokens.at(i).binary_properties().at(0).value();
        // std::vector<uint8_t> plaintext = aes_128_gcm_decrypt(remote_datareader_tokens.at(i).binary_properties().at(0).value(),
        //     remote_reader->Participant2ParticipantKxKeyMaterial.master_sender_key);

        if (plaintext.size() == 0)
        {
            return false;
        }

        KeyMaterial_AES_GCM_GMAC keymat;
        KeyMaterialCDRDeserialize(keymat, &plaintext);
        remote_reader->Entity2RemoteKeyMaterial.push_back(keymat);

        remote_reader_lock.unlock();

        std::unique_lock<std::mutex> local_writer_lock(local_writer->mutex_);

        local_writer->Remote2EntityKeyMaterial.push_back(keymat);
    }

    return true;
}

bool AESGCMGMAC_KeyExchange::set_remote_datawriter_crypto_tokens(
        DatareaderCryptoHandle& local_datareader_crypto,
        DatawriterCryptoHandle& remote_datawriter_crypto,
        const DatawriterCryptoTokenSeq& remote_datawriter_tokens,
        SecurityException& exception)
{

    AESGCMGMAC_ReaderCryptoHandle& local_reader = AESGCMGMAC_ReaderCryptoHandle::narrow(local_datareader_crypto);
    AESGCMGMAC_WriterCryptoHandle& remote_writer = AESGCMGMAC_WriterCryptoHandle::narrow(remote_datawriter_crypto);

    if (local_reader.nil() || remote_writer.nil())
    {
        EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "Invalid CryptoHandle");
        return false;
    }
    //As only relevant KeyMaterials are tokenized, only one or two Token are exchanged
    auto nTokens = remote_datawriter_tokens.size();
    if (nTokens != 1 && nTokens != 2)
    {
        EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "Malformed CryptoTokenSequence");
        exception = SecurityException("Incorrect remote CryptoSequence length");
        return false;
    }

    for (size_t i = 0; i < nTokens; i++)
    {
        if (remote_datawriter_tokens.at(i).class_id() != "DDS:Crypto:AES_GCM_GMAC")
        {
            EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "Malformed CryptoToken");
            exception = SecurityException("Incorrect token type received");
            return false;
        }

        if (remote_datawriter_tokens.at(i).binary_properties().size() != 1 ||
                remote_datawriter_tokens.at(i).properties().size() != 0 ||
                remote_datawriter_tokens.at(i).binary_properties().at(0).name() != "dds.cryp.keymat")
        {
            EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "Malformed CryptoToken");
            exception = SecurityException("Malformed CryptoToken");
            return false;
        }

        std::unique_lock<std::mutex> remote_writer_lock(remote_writer->mutex_);

        //Valid CryptoToken, we can decrypt and push the resulting KeyMaterial in as a RemoteParticipant2ParticipantKeyMaterial
        std::vector<uint8_t> plaintext = remote_datawriter_tokens.at(i).binary_properties().at(0).value();
        // std::vector<uint8_t> plaintext = aes_128_gcm_decrypt(remote_datawriter_tokens.at(i).binary_properties().at(0).value(),
        //     remote_writer->Participant2ParticipantKxKeyMaterial.master_sender_key);

        if (plaintext.size() == 0)
        {
            return false;
        }

        KeyMaterial_AES_GCM_GMAC keymat;
        KeyMaterialCDRDeserialize(keymat, &plaintext);

        remote_writer->Entity2RemoteKeyMaterial.push_back(keymat);

        remote_writer_lock.unlock();

        std::unique_lock<std::mutex> local_writer_lock(local_reader->mutex_);

        //TODO(Ricardo) Why?
        local_reader->Remote2EntityKeyMaterial.push_back(keymat);
    }

    return true;
}

bool AESGCMGMAC_KeyExchange::return_crypto_tokens(
        const CryptoTokenSeq& /*crypto_tokens*/,
        SecurityException& exception)
{

    exception = SecurityException("Not implemented");
    return false;
}

std::vector<uint8_t> AESGCMGMAC_KeyExchange::KeyMaterialCDRSerialize(
        KeyMaterial_AES_GCM_GMAC& key)
{
    std::vector<uint8_t> buffer;

    // transform_kind : octet[4]
    for (int i = 0; i < 4; i++)
    {
        buffer.push_back(key.transformation_kind[i]);
    }

    // Only last byte different from 0
    auto kind = key.transformation_kind[3];
    if (kind == 0)
    {
        // KIND = NONE => empty key material

        // empty master_salt - 0 0 0 0
        // empty sender_key_id - 0 0 0 0
        // empty master_sender_key - 0 0 0 0
        // empty receiver_specific_key_id - 0 0 0 0
        // empty master_receiver_specific_key - 0 0 0 0
        // Total 40 bytes to 0
        buffer.insert(buffer.end(), 40, 0x00);
    }
    else
    {
        // 128 bits for kinds 1 and 2. 256 bits for kinds 3 and 4.
        uint8_t key_len = kind <= 2 ? 16 : 32;

        // master_salt : sequence<octet,32>
        buffer.insert(buffer.end(), 3, 0);
        buffer.push_back(key_len);
        for (uint8_t i = 0; i < key_len; i++)
        {
            buffer.push_back(key.master_salt[i]);
        }

        // sender_key_id : octet[4]
        for (int i = 0; i < 4; i++)
        {
            buffer.push_back(key.sender_key_id[i]);
        }

        // master_sender_key : sequence<octet,32>
        buffer.insert(buffer.end(), 3, 0);
        buffer.push_back(key_len);
        for (uint8_t i = 0; i < key_len; i++)
        {
            buffer.push_back(key.master_sender_key[i]);
        }

        // receiver_specific_key_id : octet[4]
        uint8_t has_specific_key = 0;
        for (int i = 0; i < 4; i++)
        {
            has_specific_key |= key.receiver_specific_key_id[i];
            buffer.push_back(key.receiver_specific_key_id[i]);
        }

        if (has_specific_key == 0)
        {
            // empty master_receiver_specific_key - 0 0 0 0
            buffer.insert(buffer.end(), 4, 0);
        }
        else
        {
            // master_receiver_specific_key : sequence<octet,32>
            buffer.insert(buffer.end(), 3, 0);
            buffer.push_back(key_len);
            for (uint8_t i = 0; i < key_len; i++)
            {
                buffer.push_back(key.master_receiver_specific_key[i]);
            }
        }
    }

    return buffer;
}

void AESGCMGMAC_KeyExchange::KeyMaterialCDRDeserialize(
        KeyMaterial_AES_GCM_GMAC& buffer,
        std::vector<uint8_t>* CDR)
{
    buffer.transformation_kind.fill(0);
    buffer.master_salt.fill(0);
    buffer.master_sender_key.fill(0);
    buffer.master_receiver_specific_key.fill(0);

    // transformation kind is always 0 0 0 n
    // TODO: Check 0 values
    const uint8_t* data = CDR->data();
    uint8_t kind = data[3];
    buffer.transformation_kind[3] = kind;
    if (kind == 0)
    {
        // empty key material
        buffer.sender_key_id.fill(0);
        buffer.receiver_specific_key_id.fill(0);
    }
    else
    {
        // 128 bits for kinds 1 and 2. 256 bits for kinds 3 and 4.
        // TODO: Check desired length
        // uint8_t desired_key_len = kind <= 2 ? 16 : 32;

        uint8_t key_len;
        uint8_t pos;

        // master_salt : sequence<octet,32>
        //    seq_len would always be 0 0 0 n
        //    TODO: check 0 values
        pos = 4 + 3;  // 4 - transformation_kind. 3 - 0's
        key_len = data[pos++];
        // TODO: check key_len
        memcpy(buffer.master_salt.data(), &data[pos], key_len);
        pos += key_len;

        // sender_key_id : octet[4]
        memcpy(buffer.sender_key_id.data(), &data[pos], 4);
        pos += 4;

        // master_sender_key : sequence<octet,32>
        //    seq_len would always be 0 0 0 n
        //    TODO: check 0 values
        pos += 3;
        key_len = data[pos++];
        // TODO: check key_len
        memcpy(buffer.master_sender_key.data(), &data[pos], key_len);
        pos += key_len;

        // receiver_specific_key_id : octet[4]
        uint8_t has_specific_key = 0;
        for (uint8_t i = 0; i < 4; i++)
        {
            buffer.receiver_specific_key_id[i] = data[pos++];
            has_specific_key |= buffer.receiver_specific_key_id[i];
        }

        if (has_specific_key != 0)
        {
            // master_receiver_specific_key : sequence<octet,32>
            //    seq_len would always be 0 0 0 n
            //    TODO: check 0 values
            pos += 3;
            key_len = data[pos++];
            // TODO: check key_len
            memcpy(buffer.master_receiver_specific_key.data(), &data[pos], key_len);
        }
    }
}

/*
   std::vector<uint8_t> AESGCMGMAC_KeyExchange::aes_128_gcm_encrypt(const std::vector<uint8_t>& plaintext,
        const std::array<uint8_t,32>& key)
   {
    std::vector<uint8_t> output;

    if(plaintext.size() <= static_cast<size_t>(std::numeric_limits<int>::max()))
    {
        size_t enc_length = plaintext.size() * 3; // TODO(Ricardo) Review size.
        output.resize(enc_length, '\0');

        unsigned char tag[AES_BLOCK_SIZE];
        unsigned char iv[AES_BLOCK_SIZE];
        RAND_bytes(iv, sizeof(iv));
        std::copy(iv, iv + 16, output.begin() + 16);

        int actual_size = 0, final_size = 0;
        EVP_CIPHER_CTX* e_ctx = EVP_CIPHER_CTX_new();
        if(!EVP_EncryptInit(e_ctx, EVP_aes_128_gcm(), (const unsigned char*)key.data(), iv))
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Unable to encrypt data. EVP_EncryptInit function returns an error");
            output.clear();
            return output;
        }
        if(!EVP_EncryptUpdate(e_ctx, &output[32], &actual_size, (const unsigned char*)plaintext.data(), static_cast<int>(plaintext.size())))
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Unable to encrypt data. EVP_EncryptUpdate function returns an error");
            output.clear();
            return output;
        }
        if(!EVP_EncryptFinal(e_ctx, &output[32 + actual_size], &final_size))
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Unable to encrypt data. EVP_EncryptFinal function returns an error");
            output.clear();
            return output;
        }
        EVP_CIPHER_CTX_ctrl(e_ctx, EVP_CTRL_GCM_GET_TAG, 16, tag);
        std::copy(tag, tag + 16, output.begin());
        std::copy(iv, iv + 16, output.begin() + 16);
        output.resize(32 + actual_size + final_size);
        EVP_CIPHER_CTX_free(e_ctx);
    }

    return output;
   }

   std::vector<uint8_t> AESGCMGMAC_KeyExchange::aes_128_gcm_decrypt(const std::vector<uint8_t>& crypto,
        const std::array<uint8_t,32>& key)
   {
    std::vector<uint8_t> plaintext;

    if(crypto.size() - 32 <= static_cast<size_t>(std::numeric_limits<int>::max()))
    {
        unsigned char tag[AES_BLOCK_SIZE];
        unsigned char iv[AES_BLOCK_SIZE];
        std::copy(crypto.begin(), crypto.begin() + 16, tag);
        std::copy(crypto.begin() + 16, crypto.begin() + 32, iv);
        plaintext.resize(crypto.size(), '\0');

        int actual_size = 0, final_size = 0;
        EVP_CIPHER_CTX* d_ctx = EVP_CIPHER_CTX_new();
        if(!EVP_DecryptInit(d_ctx, EVP_aes_128_gcm(), (const unsigned char*)key.data(), iv))
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Unable to decrypt data. EVP_DecryptInit function returns an error");
            plaintext.clear();
            return plaintext;
        }
        if(!EVP_DecryptUpdate(d_ctx, &plaintext[0], &actual_size, (const unsigned char*)crypto.data() + 32, static_cast<int>(crypto.size() - 32)))
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Unable to decrypt data. EVP_DecryptUpdate function returns an error");
            plaintext.clear();
            return plaintext;
        }
        EVP_CIPHER_CTX_ctrl(d_ctx, EVP_CTRL_GCM_SET_TAG, 16, tag);
        if(!EVP_DecryptFinal(d_ctx, &plaintext[actual_size], &final_size))
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Unable to decrypt data. EVP_DecryptFinal function returns an error");
            plaintext.clear();
            return plaintext;
        }
        plaintext.resize(actual_size + final_size, '\0');
        EVP_CIPHER_CTX_free(d_ctx);
    }

    return plaintext;
   }
 */
