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

#include <openssl/opensslv.h>

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
#define IS_OPENSSL_1_1 1
#else
#define IS_OPENSSL_1_1 0
#endif // if OPENSSL_VERSION_NUMBER >= 0x10100000L

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <fastdds/dds/log/Log.hpp>

#include <string.h>

#include <security/cryptography/AESGCMGMAC_KeyFactory.h>

// Solve error with Win32 macro
#ifdef WIN32
#undef max
#endif // ifdef WIN32

static bool create_kx_key(
        std::array<uint8_t, 32>& out_data,
        const std::vector<uint8_t>* first_data,
        const char* cookie,
        const std::vector<uint8_t>* second_data,
        const std::vector<uint8_t>* shared_secret)
{
    uint8_t tmp_data[32 + 16 + 32];
    uint8_t sha256[32];

    out_data.fill(0);

    memcpy(tmp_data, first_data->data(), 32);
    memcpy(&tmp_data[32], cookie, 16);
    memcpy(&tmp_data[32 + 16], second_data->data(), 32);

    if (!EVP_Digest(tmp_data, 32 + 16 + 32, sha256, nullptr, EVP_sha256(), nullptr))
    {
        return false;
    }

    //The result of p_master_salt is now the key to perform an HMACsha256 of the shared secret
    EVP_PKEY* key = EVP_PKEY_new_mac_key(EVP_PKEY_HMAC, nullptr, sha256, 32);
    EVP_MD_CTX* ctx =
#if IS_OPENSSL_1_1
            EVP_MD_CTX_new();
#else
            (EVP_MD_CTX*)malloc(sizeof(EVP_MD_CTX));
#endif // if IS_OPENSSL_1_1
    EVP_MD_CTX_init(ctx);
    EVP_DigestSignInit(ctx, nullptr, EVP_sha256(), nullptr, key);
    EVP_DigestSignUpdate(ctx, shared_secret->data(), shared_secret->size());
    size_t length = 0;
    EVP_DigestSignFinal(ctx, nullptr, &length);
    if (length > 32)
    {
        EVP_PKEY_free(key);
#if IS_OPENSSL_1_1
        EVP_MD_CTX_free(ctx);
#else
        EVP_MD_CTX_cleanup(ctx);
        free(ctx);
#endif // if IS_OPENSSL_1_1
        return false;
    }
    EVP_DigestSignFinal(ctx, out_data.data(), &length);
    EVP_PKEY_free(key);
#if IS_OPENSSL_1_1
    EVP_MD_CTX_free(ctx);
#else
    EVP_MD_CTX_cleanup(ctx);
    free(ctx);
#endif // if IS_OPENSSL_1_1

    return true;
}

using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::rtps::security;

AESGCMGMAC_KeyFactory::AESGCMGMAC_KeyFactory()
{
}

ParticipantCryptoHandle* AESGCMGMAC_KeyFactory::register_local_participant(
        const IdentityHandle& /*participant_identity*/,
        const PermissionsHandle& /*participant_permissions*/,
        const PropertySeq& participant_properties,
        const ParticipantSecurityAttributes& participant_security_attributes,
        SecurityException& /*exception*/)
{
    //Create ParticipantCryptoHandle, fill Participant KeyMaterial and return it
    AESGCMGMAC_ParticipantCryptoHandle* PCrypto = nullptr;

    PCrypto = new AESGCMGMAC_ParticipantCryptoHandle();
    auto plugin_attrs = participant_security_attributes.plugin_participant_attributes;
    (*PCrypto)->ParticipantPluginAttributes = plugin_attrs;

    //Fill ParticipantKeyMaterial - This will be used to cipher full rpts messages
    //Default to AES128 if the user does not specify otherwise (GCM / GMAC depending of RTPS protection kind)
    bool is_rtps_encrypted = (plugin_attrs & PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ENCRYPTED) != 0;
    bool is_origin_auth =
            (plugin_attrs & PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ORIGIN_AUTHENTICATED) != 0;
    bool use_256_bits = true;
    uint64_t maxblockspersession = 32; //Default to key update every 32 usages if the user does not specify otherwise
    if (!participant_properties.empty())
    {
        for (auto it = participant_properties.begin(); it != participant_properties.end(); ++it)
        {
            if ((it)->name().compare("dds.sec.crypto.keysize") == 0)
            {
                if (it->value().compare("128") == 0)
                {
                    use_256_bits = false;
                }
            }
            if ((it)->name().compare("dds.sec.crypto.maxblockspersession") == 0)
            {
                try
                {
                    int tmp = std::stoi((it)->value());
                    if (tmp > 0)
                    {
                        maxblockspersession = tmp;
                    }
                }
                catch (std::invalid_argument&)
                {
                }
            }
        }//endfor
    }//endif

    create_key((*PCrypto)->ParticipantKeyMaterial, is_rtps_encrypted, use_256_bits);

    //Set values related to key update policy
    (*PCrypto)->max_blocks_per_session = maxblockspersession;
    (*PCrypto)->session_block_counter = maxblockspersession + 1; //Set to update upon first usage

    RAND_bytes((unsigned char*)( &((*PCrypto)->session_id )), sizeof(uint32_t));

    // Fill data to use with ourselves.
    KeyMaterial_AES_GCM_GMAC buffer;  //Buffer = Participant2ParticipantKeyMaterial

    //These values must match the ones in ParticipantKeymaterial
    buffer.transformation_kind = (*PCrypto)->ParticipantKeyMaterial.transformation_kind;
    buffer.master_salt = (*PCrypto)->ParticipantKeyMaterial.master_salt;
    buffer.master_sender_key = (*PCrypto)->ParticipantKeyMaterial.master_sender_key;
    buffer.sender_key_id = (*PCrypto)->ParticipantKeyMaterial.sender_key_id;
    if (is_origin_auth)
    {
        buffer.receiver_specific_key_id = (*PCrypto)->ParticipantKeyMaterial.sender_key_id;
        buffer.master_receiver_specific_key = (*PCrypto)->ParticipantKeyMaterial.master_sender_key;
    }
    else
    {
        buffer.receiver_specific_key_id = c_transformKeyIdZero;
        buffer.master_receiver_specific_key.fill(0);
    }

    (*PCrypto)->Participant2ParticipantKeyMaterial.push_back(buffer);
    (*PCrypto)->RemoteParticipant2ParticipantKeyMaterial.push_back(buffer);
    (*PCrypto)->Participant2ParticipantKxKeyMaterial.push_back(buffer);

    return PCrypto;
}

ParticipantCryptoHandle* AESGCMGMAC_KeyFactory::register_matched_remote_participant(
        const ParticipantCryptoHandle& local_participant_crypto_handle,
        const IdentityHandle& /*remote_participant_identity*/,
        const PermissionsHandle& /*remote_participant_permissions*/,
        const SharedSecretHandle& shared_secret,
        SecurityException& exception)
{
    //Extract information from the handshake. It will be needed in order to compute KeyMaterials
    const std::vector<uint8_t>* challenge_1 = SharedSecretHelper::find_data_value(**shared_secret, "Challenge1");
    const std::vector<uint8_t>* shared_secret_ss = SharedSecretHelper::find_data_value(**shared_secret, "SharedSecret");
    const std::vector<uint8_t>* challenge_2 = SharedSecretHelper::find_data_value(**shared_secret, "Challenge2");
    if ((challenge_1 == nullptr) || (shared_secret_ss == nullptr) || (challenge_2 == nullptr))
    {
        logWarning(SECURITY_CRYPTO, "Malformed SharedSecretHandle");
        exception = SecurityException("Unable to read SharedSecret and Challenges");
        return nullptr;
    }

    // Create Participant2ParticipantKeyMaterial (Based on local ParticipantKeyMaterial) and
    // ParticipantKxKeyMaterial (based on the SharedSecret)
    // Put both elements in the local and remote ParticipantCryptoHandle

    const AESGCMGMAC_ParticipantCryptoHandle& local_participant_handle =
            AESGCMGMAC_ParticipantCryptoHandle::narrow(local_participant_crypto_handle);

    auto plugin_attrs = local_participant_handle->ParticipantPluginAttributes;
    bool is_origin_auth =
            (plugin_attrs & PLUGIN_PARTICIPANT_SECURITY_ATTRIBUTES_FLAG_IS_RTPS_ORIGIN_AUTHENTICATED) != 0;

    // Remote Participant CryptoHandle, to be returned at the end of the function
    AESGCMGMAC_ParticipantCryptoHandle* RPCrypto = new AESGCMGMAC_ParticipantCryptoHandle();

    (*RPCrypto)->ParticipantPluginAttributes = plugin_attrs;

    /*Fill values for Participant2ParticipantKeyMaterial - Used to encrypt outgoing data */
    {
        //scope for temp var buffer
        KeyMaterial_AES_GCM_GMAC buffer;  //Buffer = Participant2ParticipantKeyMaterial

        //These values must match the ones in ParticipantKeymaterial
        buffer.transformation_kind = local_participant_handle->ParticipantKeyMaterial.transformation_kind;
        buffer.master_salt = local_participant_handle->ParticipantKeyMaterial.master_salt;
        buffer.master_sender_key = local_participant_handle->ParticipantKeyMaterial.master_sender_key;
        buffer.sender_key_id = local_participant_handle->ParticipantKeyMaterial.sender_key_id;
        //Generation of remainder values (Remote specific key)
        buffer.master_receiver_specific_key.fill(0);
        buffer.receiver_specific_key_id = c_transformKeyIdZero;
        if (is_origin_auth)
        {
            buffer.receiver_specific_key_id = make_unique_KeyId();
            RAND_bytes(buffer.master_receiver_specific_key.data(), 32);
        }
        //Attach to both local and remote CryptoHandles
        (*RPCrypto)->Participant2ParticipantKeyMaterial.push_back(buffer);
    }

    /*Fill values for Participant2ParticipantKxKeyMaterial - Used to encrypt CryptoTokens (exchange of key info) */
    {
        //scope for temp var buffer
        KeyMaterial_AES_GCM_GMAC buffer; //Buffer = Participant2ParticipantKxKeyMaterial

        buffer.transformation_kind = c_transfrom_kind_aes256_gcm;
        buffer.sender_key_id.fill(0);
        buffer.receiver_specific_key_id.fill(0);
        buffer.master_receiver_specific_key.fill(0);

        if (!create_kx_key(buffer.master_salt, challenge_1, "keyexchange salt", challenge_2, shared_secret_ss))
        {
            logWarning(SECURITY_CRYPTO, "Error generating the keys to perform token transaction");
            exception = SecurityException("Encountered an error while creating KxKeyMaterials");
            delete RPCrypto;
            return nullptr;
        }

        if (!create_kx_key(buffer.master_sender_key, challenge_2, "key exchange key", challenge_1, shared_secret_ss))
        {
            logWarning(SECURITY_CRYPTO, "Error generating the keys to perform token transaction");
            exception = SecurityException("Encountered an error while creating KxKeyMaterials");
            delete RPCrypto;
            return nullptr;
        }


        (*RPCrypto)->max_blocks_per_session = local_participant_handle->max_blocks_per_session;
        (*RPCrypto)->session_block_counter = local_participant_handle->max_blocks_per_session + 1;
        (*RPCrypto)->session_id = std::numeric_limits<uint32_t>::max();
        if ((*RPCrypto)->session_id == local_participant_handle->session_id)
        {
            (*RPCrypto)->session_id -= 1;
        }

        //Attack to PartipantCryptoHandles - both local and remote
        (*RPCrypto)->Participant2ParticipantKxKeyMaterial.push_back(buffer);

        // Create builtin key exchange writer handle
        AESGCMGMAC_WriterCryptoHandle* wHandle = new AESGCMGMAC_WriterCryptoHandle();
        (*wHandle)->EndpointPluginAttributes = PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED;
        (*wHandle)->Participant_master_key_id = c_transformKeyIdZero;
        (*wHandle)->Parent_participant = RPCrypto;
        (*wHandle)->EntityKeyMaterial.push_back(buffer);
        (*wHandle)->Entity2RemoteKeyMaterial.push_back(buffer);
        (*wHandle)->Remote2EntityKeyMaterial.push_back(buffer);
        (*wHandle)->Sessions[0].session_id = (*RPCrypto)->session_id;
        (*wHandle)->max_blocks_per_session = (*RPCrypto)->max_blocks_per_session;
        (*wHandle)->Sessions[0].session_block_counter = (*RPCrypto)->session_block_counter;
        (*RPCrypto)->Writers.push_back(wHandle);

        // Create builtin key exchange reader handle
        AESGCMGMAC_ReaderCryptoHandle* rHandle = new AESGCMGMAC_ReaderCryptoHandle();
        (*rHandle)->EndpointPluginAttributes = PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED;
        (*rHandle)->Participant_master_key_id = c_transformKeyIdZero;
        (*rHandle)->Parent_participant = RPCrypto;
        (*rHandle)->EntityKeyMaterial.push_back(buffer);
        (*rHandle)->Entity2RemoteKeyMaterial.push_back(buffer);
        (*rHandle)->Remote2EntityKeyMaterial.push_back(buffer);
        (*rHandle)->Sessions[0].session_id = (*RPCrypto)->session_id;
        (*rHandle)->max_blocks_per_session = (*RPCrypto)->max_blocks_per_session;
        (*rHandle)->Sessions[0].session_block_counter = (*RPCrypto)->session_block_counter;
        (*RPCrypto)->Readers.push_back(rHandle);
    }

    return RPCrypto;
}

DatawriterCryptoHandle* AESGCMGMAC_KeyFactory::register_local_datawriter(
        ParticipantCryptoHandle& participant_crypto,
        const PropertySeq& datawriter_prop,
        const EndpointSecurityAttributes& datawriter_security_properties,
        SecurityException& /*exception*/)
{
    AESGCMGMAC_ParticipantCryptoHandle& participant_handle =
            AESGCMGMAC_ParticipantCryptoHandle::narrow(participant_crypto);

    if (participant_handle.nil())
    {
        logWarning(SECURITY_CRYPTO, "Invalid ParticipantCryptoHandle");
        return nullptr;
    }

    auto plugin_attrs = datawriter_security_properties.plugin_endpoint_attributes;
    bool is_sub_encrypted = (plugin_attrs & PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED) != 0;
    bool is_payload_encrypted = (plugin_attrs & PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_PAYLOAD_ENCRYPTED) != 0;
    bool use_256_bits = true;
    bool use_kx_keys = false;
    uint64_t maxblockspersession = 32; //Default to key update every 32 usages
    if (!datawriter_prop.empty())
    {
        for (auto it = datawriter_prop.begin(); it != datawriter_prop.end(); ++it)
        {
            if (it->name().compare("dds.sec.crypto.keysize") == 0)
            {
                if (it->value().compare("128") == 0)
                {
                    use_256_bits = false;
                }
            }
            else if (it->name().compare("dds.sec.crypto.maxblockspersession") == 0)
            {
                try
                {
                    int tmp = std::stoi((it)->value());
                    if (tmp > 0)
                    {
                        maxblockspersession = tmp;
                    }
                }
                catch (std::invalid_argument&)
                {
                }
            }
            else if (it->name().compare("dds.sec.builtin_endpoint_name") == 0)
            {
                if (it->value().compare("BuiltinParticipantVolatileMessageSecureWriter") == 0)
                {
                    use_kx_keys = true;
                }
            }
        }//endfor
    }//endif

    if (use_kx_keys)
    {
        return participant_handle->Writers.at(0);
    }

    //Create ParticipantCryptoHandle, fill Participant KeyMaterial and return it
    AESGCMGMAC_WriterCryptoHandle* WCrypto = new AESGCMGMAC_WriterCryptoHandle();
    (*WCrypto)->EndpointPluginAttributes = plugin_attrs;

    auto session = &(*WCrypto)->Sessions[0];

    if (datawriter_security_properties.is_submessage_protected)
    {
        KeyMaterial_AES_GCM_GMAC buffer;
        create_key(buffer, is_sub_encrypted, use_256_bits);
        (*WCrypto)->EntityKeyMaterial.push_back(buffer);
        session->session_block_counter = maxblockspersession + 1; //Set to update upon first usage
        RAND_bytes((unsigned char*)(&(session->session_id)), sizeof(uint32_t));
        session++;
    }

    if (datawriter_security_properties.is_payload_protected)
    {
        // TODO: let user decide on key reuse
        if (!datawriter_security_properties.is_submessage_protected ||
                (is_payload_encrypted != is_sub_encrypted))
        {
            KeyMaterial_AES_GCM_GMAC buffer;
            create_key(buffer, is_payload_encrypted, use_256_bits);
            (*WCrypto)->EntityKeyMaterial.push_back(buffer);
            session->session_block_counter = maxblockspersession + 1; //Set to update upon first usage
            RAND_bytes((unsigned char*)(&(session->session_id)), sizeof(uint32_t));
        }
    }

    (*WCrypto)->max_blocks_per_session = maxblockspersession;

    // Issue #697 by DavidLoftus, who catched an unnamed lock, causing the mutex being freed inmediatly.
    std::unique_lock<std::mutex> david_loftus_lock(participant_handle->mutex_);

    (*WCrypto)->Participant_master_key_id = participant_handle->ParticipantKeyMaterial.sender_key_id;

    (*WCrypto)->Parent_participant = &participant_crypto;

    participant_handle->Writers.push_back(WCrypto);

    return WCrypto;
}

DatareaderCryptoHandle* AESGCMGMAC_KeyFactory::register_matched_remote_datareader(
        DatawriterCryptoHandle& local_datawriter_crypto_handle,
        ParticipantCryptoHandle& remote_participant_crypto,
        const SharedSecretHandle& /*shared_secret*/,
        const bool relay_only,
        SecurityException& /*exception*/)
{
    //Create Participant2ParticipantKeyMaterial (Based on local ParticipantKeyMaterial) and
    //ParticipantKxKeyMaterial (based on the SharedSecret)
    //Put both elements in the local and remote ParticipantCryptoHandle

    AESGCMGMAC_WriterCryptoHandle& local_writer_handle =
            AESGCMGMAC_WriterCryptoHandle::narrow(local_datawriter_crypto_handle);

    if (local_writer_handle.nil())
    {
        logWarning(SECURITY_CRYPTO, "Malformed DataWriterCryptoHandle");
        return nullptr;
    }

    AESGCMGMAC_ParticipantCryptoHandle& remote_participant =
            AESGCMGMAC_ParticipantCryptoHandle::narrow(remote_participant_crypto);

    std::unique_lock<std::mutex> writer_lock(local_writer_handle->mutex_);
    auto plugin_attrs = local_writer_handle->EndpointPluginAttributes;
    bool is_origin_auth =
            (plugin_attrs & PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED) != 0;

    // Remote Reader CryptoHandle, to be returned at the end of the function
    AESGCMGMAC_ReaderCryptoHandle* RRCrypto = new AESGCMGMAC_ReaderCryptoHandle();

    (*RRCrypto)->EndpointPluginAttributes = plugin_attrs;
    (*RRCrypto)->Participant_master_key_id = local_writer_handle->Participant_master_key_id;

    if (local_writer_handle->EntityKeyMaterial.size() == 0)
    {
        // This means the local writer is a key exchange writer
        (*RRCrypto)->Remote2EntityKeyMaterial.push_back(remote_participant->Participant2ParticipantKxKeyMaterial.at(0));
        (*RRCrypto)->Entity2RemoteKeyMaterial.push_back(remote_participant->Participant2ParticipantKxKeyMaterial.at(0));
    }
    else
    {
        /*Fill values for Writer2ReaderKeyMaterial - Used to encrypt outgoing data */
        KeyMaterial_AES_GCM_GMAC buffer;  //Buffer = Writer2ReaderKeyMaterial

        //These values must match the ones in ParticipantKeymaterial
        buffer.transformation_kind = local_writer_handle->EntityKeyMaterial.at(0).transformation_kind;
        buffer.master_salt = local_writer_handle->EntityKeyMaterial.at(0).master_salt;
        buffer.master_sender_key = local_writer_handle->EntityKeyMaterial.at(0).master_sender_key;

        buffer.sender_key_id = local_writer_handle->EntityKeyMaterial.at(0).sender_key_id;
        //Generation of remainder values (Remote specific key)
        buffer.master_receiver_specific_key.fill(0);
        buffer.receiver_specific_key_id = c_transformKeyIdZero;
        if (is_origin_auth)
        {
            buffer.receiver_specific_key_id = make_unique_KeyId();
            RAND_bytes(buffer.master_receiver_specific_key.data(), 16);
        }

        //Attach to both local and remote CryptoHandles
        (*RRCrypto)->Remote2EntityKeyMaterial.push_back(buffer);
        if (is_origin_auth)
        {
            local_writer_handle->Entity2RemoteKeyMaterial.push_back(buffer);
        }
    }

    auto session = &(*RRCrypto)->Sessions[0];
    session->session_block_counter = local_writer_handle->Sessions[0].session_block_counter;
    session->session_id = std::numeric_limits<uint32_t>::max();
    if (session->session_id == local_writer_handle->Sessions[0].session_id)
    {
        session->session_id -= 1;
    }

    if (!relay_only && local_writer_handle->EntityKeyMaterial.size() > 1)
    {
        KeyMaterial_AES_GCM_GMAC buffer;  //Buffer = Writer2ReaderKeyMaterial

        buffer.transformation_kind = local_writer_handle->EntityKeyMaterial.at(1).transformation_kind;
        buffer.master_salt = local_writer_handle->EntityKeyMaterial.at(1).master_salt;
        buffer.master_sender_key = local_writer_handle->EntityKeyMaterial.at(1).master_sender_key;
        buffer.sender_key_id = local_writer_handle->EntityKeyMaterial.at(1).sender_key_id;
        buffer.master_receiver_specific_key.fill(0);
        buffer.receiver_specific_key_id = c_transformKeyIdZero;

        //Attach only to remote CryptoHandles
        (*RRCrypto)->Remote2EntityKeyMaterial.push_back(buffer);

        session++;
        session->session_block_counter = local_writer_handle->Sessions[0].session_block_counter;
        session->session_id = std::numeric_limits<uint32_t>::max();
        if (session->session_id == local_writer_handle->Sessions[0].session_id)
        {
            session->session_id -= 1;
        }
    }

    (*RRCrypto)->max_blocks_per_session = local_writer_handle->max_blocks_per_session;

    writer_lock.unlock();

    std::unique_lock<std::mutex> remote_participant_lock(remote_participant->mutex_);

    // (*RRCrypto)->Participant2ParticipantKxKeyMaterial
    //     = remote_participant->Participant2ParticipantKxKeyMaterial.at(0);

    (*RRCrypto)->Parent_participant = &remote_participant_crypto;
    //Save this CryptoHandle as part of the remote participant

    (*remote_participant)->Readers.push_back(RRCrypto);

    return RRCrypto;
}

DatareaderCryptoHandle* AESGCMGMAC_KeyFactory::register_local_datareader(
        ParticipantCryptoHandle& participant_crypto,
        const PropertySeq& datareader_properties,
        const EndpointSecurityAttributes& datareder_security_attributes,
        SecurityException& /*exception*/)
{
    AESGCMGMAC_ParticipantCryptoHandle& participant_handle =
            AESGCMGMAC_ParticipantCryptoHandle::narrow(participant_crypto);

    if (participant_handle.nil())
    {
        logWarning(SECURITY_CRYPTO, "Invalid ParticipantCryptoHandle");
        return nullptr;
    }

    //Create ParticipantCryptoHandle, fill Participant KeyMaterial and return it
    AESGCMGMAC_ReaderCryptoHandle* RCrypto = nullptr;

    auto plugin_attrs = datareder_security_attributes.plugin_endpoint_attributes;
    bool is_sub_encrypted = (plugin_attrs & PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED) != 0;
    bool use_256_bits = true;
    bool use_kx_keys = false;
    uint64_t maxblockspersession = 32; //Default to key update every 32 usages
    if (!datareader_properties.empty())
    {
        for (auto it = datareader_properties.begin(); it != datareader_properties.end(); ++it)
        {
            if (it->name().compare("dds.sec.crypto.keysize") == 0)
            {
                if (it->value().compare("128") == 0)
                {
                    use_256_bits = false;
                }
            }
            else if (it->name().compare("dds.sec.crypto.maxblockspersession") == 0)
            {
                try
                {
                    int tmp = std::stoi((it)->value());
                    if (tmp > 0)
                    {
                        maxblockspersession = tmp;
                    }
                }
                catch (std::invalid_argument&)
                {
                }
            }
            else if (it->name().compare("dds.sec.builtin_endpoint_name") == 0)
            {
                if (it->value().compare("BuiltinParticipantVolatileMessageSecureReader") == 0)
                {
                    use_kx_keys = true;
                }
            }
        }//endfor
    }//endif

    if (use_kx_keys)
    {
        return participant_handle->Readers.at(0);
    }

    RCrypto = new AESGCMGMAC_ReaderCryptoHandle();
    (*RCrypto)->EndpointPluginAttributes = plugin_attrs;

    {
        //Fill ParticipantKeyMaterial - This will be used to cipher full rpts messages
        KeyMaterial_AES_GCM_GMAC buffer;
        create_key(buffer, is_sub_encrypted, use_256_bits);
        (*RCrypto)->EntityKeyMaterial.push_back(buffer);
    }

    (*RCrypto)->max_blocks_per_session = maxblockspersession;
    (*RCrypto)->Sessions[0].session_block_counter = maxblockspersession + 1;
    RAND_bytes((unsigned char*)( &((*RCrypto)->Sessions[0].session_id )), sizeof(uint32_t));

    std::unique_lock<std::mutex> lock(participant_handle->mutex_);

    (*RCrypto)->Participant_master_key_id = participant_handle->ParticipantKeyMaterial.sender_key_id;

    (*RCrypto)->Parent_participant = &participant_crypto;

    participant_handle->Readers.push_back(RCrypto);

    return RCrypto;
}

DatawriterCryptoHandle* AESGCMGMAC_KeyFactory::register_matched_remote_datawriter(
        DatareaderCryptoHandle& local_datareader_crypto_handle,
        ParticipantCryptoHandle& remote_participant_crypt,
        const SharedSecretHandle& /*shared_secret*/,
        SecurityException& /*exception*/)
{
    //Create Participant2ParticipantKeyMaterial (Based on local ParticipantKeyMaterial) and
    //ParticipantKxKeyMaterial (based on the SharedSecret)
    //Put both elements in the local and remote ParticipantCryptoHandle

    AESGCMGMAC_ReaderCryptoHandle& local_reader_handle =
            AESGCMGMAC_ReaderCryptoHandle::narrow(local_datareader_crypto_handle);

    if (local_reader_handle.nil())
    {
        logWarning(SECURITY_CRYPTO, "Invalid DataReaderCryptoHandle");
        return nullptr;
    }

    AESGCMGMAC_ParticipantCryptoHandle& remote_participant =
            AESGCMGMAC_ParticipantCryptoHandle::narrow(remote_participant_crypt);

    std::unique_lock<std::mutex> reader_lock(local_reader_handle->mutex_);
    auto plugin_attrs = local_reader_handle->EndpointPluginAttributes;
    bool is_origin_auth =
            (plugin_attrs & PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED) != 0;

    // Remote Writer CryptoHandle, to be returned at the end of the function
    AESGCMGMAC_WriterCryptoHandle* RWCrypto = new AESGCMGMAC_WriterCryptoHandle();

    (*RWCrypto)->Participant_master_key_id = local_reader_handle->Participant_master_key_id;
    (*RWCrypto)->EndpointPluginAttributes = local_reader_handle->EndpointPluginAttributes;

    if (local_reader_handle->EntityKeyMaterial.size() == 0)
    {
        // This means the local reader is a key exchange writer
        (*RWCrypto)->Remote2EntityKeyMaterial.push_back(remote_participant->Participant2ParticipantKxKeyMaterial.at(0));
        (*RWCrypto)->Entity2RemoteKeyMaterial.push_back(remote_participant->Participant2ParticipantKxKeyMaterial.at(0));
    }
    else
    {
        /*Fill values for Writer2ReaderKeyMaterial - Used to encrypt outgoing data */
        KeyMaterial_AES_GCM_GMAC buffer;  //Buffer = Writer2ReaderKeyMaterial

        //These values must match the ones in ParticipantKeymaterial
        buffer.transformation_kind = local_reader_handle->EntityKeyMaterial.at(0).transformation_kind;
        buffer.master_salt = local_reader_handle->EntityKeyMaterial.at(0).master_salt;
        buffer.master_sender_key = local_reader_handle->EntityKeyMaterial.at(0).master_sender_key;
        buffer.sender_key_id = local_reader_handle->EntityKeyMaterial.at(0).sender_key_id;
        //Generation of remainder values (Remote specific key)
        buffer.master_receiver_specific_key.fill(0);
        buffer.receiver_specific_key_id = c_transformKeyIdZero;
        if (is_origin_auth)
        {
            buffer.receiver_specific_key_id = make_unique_KeyId();
            RAND_bytes(buffer.master_receiver_specific_key.data(), 16);
        }

        //Attach to both local and remote CryptoHandles
        (*RWCrypto)->Remote2EntityKeyMaterial.push_back(buffer);
        if (is_origin_auth)
        {
            local_reader_handle->Entity2RemoteKeyMaterial.push_back(buffer);
        }
    }

    (*RWCrypto)->max_blocks_per_session = local_reader_handle->max_blocks_per_session;
    auto session = &(*RWCrypto)->Sessions[0];
    session->session_block_counter = local_reader_handle->Sessions[0].session_block_counter;
    session->session_id = std::numeric_limits<uint32_t>::max();
    if (session->session_id == local_reader_handle->Sessions[0].session_id)
    {
        session->session_id -= 1;
    }

    reader_lock.unlock();

    std::unique_lock<std::mutex> remote_participant_lock(remote_participant->mutex_);

    // (*RWCrypto)->Participant2ParticipantKxKeyMaterial =
    //     remote_participant->Participant2ParticipantKxKeyMaterial.at(0);

    (*RWCrypto)->Parent_participant = &remote_participant_crypt;

    //Save this CryptoHandle as part of the remote participant
    (*remote_participant)->Writers.push_back(RWCrypto);

    return RWCrypto;
}

bool AESGCMGMAC_KeyFactory::unregister_participant(
        ParticipantCryptoHandle* participant_crypto_handle,
        SecurityException& exception)
{
    if (participant_crypto_handle == nullptr)
    {
        return false;
    }

    //De-register the IDs
    AESGCMGMAC_ParticipantCryptoHandle& local_participant =
            AESGCMGMAC_ParticipantCryptoHandle::narrow(*participant_crypto_handle);

    if (local_participant.nil())
    {
        return false;
    }

    release_key_id(local_participant->ParticipantKeyMaterial.sender_key_id);

    //Unregister all writers and readers
    std::vector<DatawriterCryptoHandle*>::iterator wit = local_participant->Writers.begin();
    while (wit != local_participant->Writers.end())
    {
        DatawriterCryptoHandle* writer = (DatawriterCryptoHandle*)(*wit);
        unregister_datawriter(writer, exception);
        wit = local_participant->Writers.begin();
    }

    std::vector<DatareaderCryptoHandle*>::iterator rit = local_participant->Readers.begin();
    while (rit != local_participant->Readers.end())
    {
        DatareaderCryptoHandle* reader = (DatareaderCryptoHandle*)(*rit);
        unregister_datareader(reader, exception);
        rit = local_participant->Readers.begin();
    }

    delete &local_participant;

    return true;

}

bool AESGCMGMAC_KeyFactory::unregister_datawriter(
        DatawriterCryptoHandle* datawriter_crypto_handle,
        SecurityException& exception)
{
    if (datawriter_crypto_handle == nullptr)
    {
        return false;
    }

    AESGCMGMAC_WriterCryptoHandle& datawriter = AESGCMGMAC_WriterCryptoHandle::narrow(*datawriter_crypto_handle);

    if (datawriter.nil())
    {
        exception = SecurityException("Not a valid DataWriterCryptoHandle has been passed as an argument");
        return false;
    }

    if ((datawriter->Parent_participant) == nullptr)
    {
        AESGCMGMAC_WriterCryptoHandle* me = (AESGCMGMAC_WriterCryptoHandle*)datawriter_crypto_handle;
        delete me;
        return true;
    }

    AESGCMGMAC_ParticipantCryptoHandle& parent_participant =
            AESGCMGMAC_ParticipantCryptoHandle::narrow(*(datawriter->Parent_participant));

    if (parent_participant.nil())
    {
        exception = SecurityException("Malformed AESGCMGMAC_WriterCryptohandle");
        return false;
    }

    //Remove reference in parent participant
    for (auto it = parent_participant->Writers.begin(); it != parent_participant->Writers.end(); it++)
    {
        if (*it == datawriter_crypto_handle)
        {
            parent_participant->Writers.erase(it);
            AESGCMGMAC_WriterCryptoHandle* me = (AESGCMGMAC_WriterCryptoHandle*)datawriter_crypto_handle;
            delete me;
            return true;
        }
    }

    return false;
}

bool AESGCMGMAC_KeyFactory::unregister_datareader(
        DatareaderCryptoHandle* datareader_crypto_handle,
        SecurityException& exception)
{
    if (datareader_crypto_handle == nullptr)
    {
        return false;
    }

    AESGCMGMAC_ReaderCryptoHandle& datareader = AESGCMGMAC_ReaderCryptoHandle::narrow(*datareader_crypto_handle);

    if (datareader.nil())
    {
        exception = SecurityException("Not a valid DataReaderCryptoHandle has been passed as an argument");
        return false;
    }

    if ((datareader->Parent_participant) == nullptr)
    {
        AESGCMGMAC_ReaderCryptoHandle* me = (AESGCMGMAC_ReaderCryptoHandle*)datareader_crypto_handle;
        delete me;
        return true;
    }

    AESGCMGMAC_ParticipantCryptoHandle& parent_participant =
            AESGCMGMAC_ParticipantCryptoHandle::narrow( *(datareader->Parent_participant));

    if (parent_participant.nil())
    {
        exception = SecurityException("Malformed AESGCMGMAC_WriterCryptohandle");
        return false;
    }

    //Remove reference in parent participant
    for (auto it = parent_participant->Readers.begin(); it != parent_participant->Readers.end(); it++)
    {
        if (*it == datareader_crypto_handle)
        {
            parent_participant->Readers.erase(it);
            AESGCMGMAC_ReaderCryptoHandle* parent = (AESGCMGMAC_ReaderCryptoHandle*)datareader_crypto_handle;
            delete parent;
            return true;
        }
    }

    return false;
}

void AESGCMGMAC_KeyFactory::create_key(
        KeyMaterial_AES_GCM_GMAC& key,
        bool encrypt_then_sign,
        bool use_256_bits)
{
    std::array<uint8_t, 4> transformationtype = encrypt_then_sign
            ? (use_256_bits
                ? c_transfrom_kind_aes256_gcm
                : c_transfrom_kind_aes128_gcm)
            : (use_256_bits
                ? c_transfrom_kind_aes256_gmac
                : c_transfrom_kind_aes128_gmac);

    int nBytes = use_256_bits ? 32 : 16;

    key.transformation_kind = transformationtype;

    key.master_salt.fill(0);
    RAND_bytes(key.master_salt.data(), nBytes);

    key.sender_key_id = make_unique_KeyId();
    key.master_sender_key.fill(0);
    RAND_bytes(key.master_sender_key.data(), nBytes);

    key.receiver_specific_key_id = c_transformKeyIdZero;
    key.master_receiver_specific_key.fill(0);
}

CryptoTransformKeyId AESGCMGMAC_KeyFactory::make_unique_KeyId()
{
    CryptoTransformKeyId buffer{{0, 0, 0, 0}};
    bool unique = false;

    while (!unique)
    {
        RAND_bytes(buffer.data(), 4);
        unique =
                std::find(m_CryptoTransformKeyIds.begin(), m_CryptoTransformKeyIds.end(),
                        buffer) == m_CryptoTransformKeyIds.end();
    }

    m_CryptoTransformKeyIds.push_back(buffer);

    return buffer;
}

void AESGCMGMAC_KeyFactory::release_key_id(
        CryptoTransformKeyId key)
{
    std::vector<CryptoTransformKeyId>::iterator it;
    it = std::find(m_CryptoTransformKeyIds.begin(), m_CryptoTransformKeyIds.end(), key);
    if (it != m_CryptoTransformKeyIds.end())
    {
        m_CryptoTransformKeyIds.erase(it);
    }
}
