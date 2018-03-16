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
#endif

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <fastrtps/log/Log.h>

#include <string.h>

#include "AESGCMGMAC_KeyFactory.h"

// Solve error with Win32 macro
#ifdef WIN32
#undef max
#endif

using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::rtps::security;

AESGCMGMAC_KeyFactory::AESGCMGMAC_KeyFactory(){}

ParticipantCryptoHandle* AESGCMGMAC_KeyFactory::register_local_participant(
                const IdentityHandle& /*participant_identity*/,
                const PermissionsHandle& /*participant_permissions*/,
                const PropertySeq &participant_properties,
                SecurityException& /*exception*/)
{

    //Create ParticipantCryptoHandle, fill Participant KeyMaterial and return it
    AESGCMGMAC_ParticipantCryptoHandle* PCrypto = nullptr;

    PCrypto = new AESGCMGMAC_ParticipantCryptoHandle();

    //Fill ParticipantKeyMaterial - This will be used to cipher full rpts messages
    std::array<uint8_t, 4> transformationtype{CRYPTO_TRANSFORMATION_KIND_AES128_GCM}; //Default to AES128_GCM if the user does not specify otherwise
    int maxblockspersession = 32; //Default to key update every 32 usages if the user does not specify otherwise
    if(!participant_properties.empty()){
          for(auto it=participant_properties.begin(); it!=participant_properties.end(); ++it){
              if( (it)->name() == "dds.sec.crypto.cryptotransformkind"){
                      if( it->value() == std::string("AES128_GCM") )
                          transformationtype = std::array<uint8_t, 4>{CRYPTO_TRANSFORMATION_KIND_AES128_GCM};
                      if( it->value() == std::string("AES128_GMAC") )
                          transformationtype = std::array<uint8_t, 4>{CRYPTO_TRANSFORMATION_KIND_AES128_GMAC};
                      if( it->value() == std::string("AES256_GCM") )
                          transformationtype = std::array<uint8_t, 4>{CRYPTO_TRANSFORMATION_KIND_AES256_GCM};
                      if( it->value() == std::string("AES256_GMAC") )
                          transformationtype = std::array<uint8_t, 4>{CRYPTO_TRANSFORMATION_KIND_AES256_GMAC};
              }// endif
              if( (it)->name() == "dds.sec.crypto.maxblockspersession"){
                  try{
                      maxblockspersession = std::stoi( (it)->value() );
                  }catch(std::invalid_argument){}
              }
          }//endfor
    }//endif

    (*PCrypto)->ParticipantKeyMaterial.transformation_kind =
        transformationtype;
    (*PCrypto)->transformation_kind = transformationtype;
    (*PCrypto)->ParticipantKeyMaterial.master_salt.fill(0);
    RAND_bytes( (*PCrypto)->ParticipantKeyMaterial.master_salt.data(), 16 );

    (*PCrypto)->ParticipantKeyMaterial.sender_key_id = make_unique_KeyId();

    (*PCrypto)->ParticipantKeyMaterial.master_sender_key.fill(0);
    RAND_bytes( (*PCrypto)->ParticipantKeyMaterial.master_sender_key.data(), 16 );

    //These values are set by the standard
    (*PCrypto)->ParticipantKeyMaterial.receiver_specific_key_id = {{0, 0, 0, 0}};  //No receiver specific, as this is the Master Participant Key
    (*PCrypto)->ParticipantKeyMaterial.master_receiver_specific_key = (*PCrypto)->ParticipantKeyMaterial.master_sender_key;

    //Set values related to key update policy
    (*PCrypto)->max_blocks_per_session = maxblockspersession;
    (*PCrypto)->session_block_counter = maxblockspersession+1; //Set to update upon first usage

    RAND_bytes( (unsigned char *)( &( (*PCrypto)->session_id ) ), sizeof(uint16_t));

    // Fill data to use with ourselves.
    KeyMaterial_AES_GCM_GMAC buffer;  //Buffer = Participant2ParticipantKeyMaterial

    //These values must match the ones in ParticipantKeymaterial
    buffer.transformation_kind = (*PCrypto)->ParticipantKeyMaterial.transformation_kind;
    buffer.master_salt = (*PCrypto)->ParticipantKeyMaterial.master_salt;
    buffer.master_sender_key = (*PCrypto)->ParticipantKeyMaterial.master_sender_key;
    buffer.sender_key_id = (*PCrypto)->ParticipantKeyMaterial.sender_key_id;
    buffer.receiver_specific_key_id = (*PCrypto)->ParticipantKeyMaterial.sender_key_id;
    buffer.master_receiver_specific_key = (*PCrypto)->ParticipantKeyMaterial.master_receiver_specific_key;

    (*PCrypto)->Participant2ParticipantKeyMaterial.push_back(buffer);
    (*PCrypto)->RemoteParticipant2ParticipantKeyMaterial.push_back(buffer);
    (*PCrypto)->Participant2ParticipantKxKeyMaterial.push_back(buffer);

    return PCrypto;
}

ParticipantCryptoHandle * AESGCMGMAC_KeyFactory::register_matched_remote_participant(
                const ParticipantCryptoHandle& local_participant_crypto_handle,
                const IdentityHandle& /*remote_participant_identity*/,
                const PermissionsHandle& /*remote_participant_permissions*/,
                const SharedSecretHandle &shared_secret,
                SecurityException &exception){

    //Extract information from the handshake. It will be needed in order to compute KeyMaterials
    const std::vector<uint8_t>* challenge_1 = SharedSecretHelper::find_data_value(**shared_secret,"Challenge1");
    const std::vector<uint8_t>* shared_secret_ss = SharedSecretHelper::find_data_value(**shared_secret,"SharedSecret");
    const std::vector<uint8_t>* challenge_2 = SharedSecretHelper::find_data_value(**shared_secret,"Challenge2");
    if( (challenge_1 == nullptr) | (shared_secret_ss == nullptr) | (challenge_2 == nullptr) ){
        logWarning(SECURITY_CRYPTO,"Malformed SharedSecretHandle");
        exception = SecurityException("Unable to read SharedSecret and Challenges");
        return nullptr;
    }

    //Create Participant2ParticipantKeyMaterial (Based on local ParticipantKeyMaterial) and ParticipantKxKeyMaterial (based on the SharedSecret)
    //Put both elements in the local and remote ParticipantCryptoHandle

    const AESGCMGMAC_ParticipantCryptoHandle& local_participant_handle = AESGCMGMAC_ParticipantCryptoHandle::narrow(local_participant_crypto_handle);
    AESGCMGMAC_ParticipantCryptoHandle* RPCrypto = new AESGCMGMAC_ParticipantCryptoHandle(); // Remote Participant CryptoHandle, to be returned at the end of the function

    (*RPCrypto)->transformation_kind = local_participant_handle->transformation_kind;

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
    }

    /*Fill values for Participant2ParticipantKxKeyMaterial - Used to encrypt CryptoTokens (exchange of key info) */
    { //scope for temp var buffer
        KeyMaterial_AES_GCM_GMAC buffer; //Buffer = Participant2ParticipantKxKeyMaterial

        buffer.transformation_kind = std::array<uint8_t,4>{CRYPTO_TRANSFORMATION_KIND_AES128_GMAC};
        buffer.master_salt.fill(0);

        std::vector<uint8_t> concatenation(challenge_1->size() + 16 +
                challenge_2->size()); //Assembly of the source concatenated sequence that is used to generate master_salt

        std::string KxKeyCookie("key exchange key");
        memcpy(concatenation.data(), challenge_1->data(), challenge_1->size());
        memcpy(concatenation.data() + challenge_1->size(), KxKeyCookie.data(), 16);
        memcpy(concatenation.data() + challenge_1->size() + 16, challenge_2->data(), challenge_2->size());

        //concatenation is used to generate the key to encode the shared_secret and produce the master_salt
        buffer.master_salt.fill(0);
        std::array<uint8_t,32> p_master_salt;
        if(!EVP_Digest(concatenation.data(), challenge_1->size() + 16 + challenge_2->size(), p_master_salt.data(), NULL, EVP_sha256(), NULL)){
            logWarning(SECURITY_CRYPTO,"Error generating the keys to perform token transaction");
            delete RPCrypto;
            return nullptr;
        }
        //The result of p_master_salt is now the key to perform an HMACsha256 of the shared secret
        EVP_PKEY *key = EVP_PKEY_new_mac_key(EVP_PKEY_HMAC, NULL, p_master_salt.data(), 32);
        EVP_MD_CTX* ctx = 
#if IS_OPENSSL_1_1
            EVP_MD_CTX_new();
#else
            (EVP_MD_CTX*)malloc(sizeof(EVP_MD_CTX));
#endif
        EVP_MD_CTX_init(ctx);
        EVP_DigestSignInit(ctx, NULL, EVP_sha256(), NULL, key);
        EVP_DigestSignUpdate(ctx, shared_secret_ss->data(), shared_secret_ss->size());
        size_t length = 0;
        EVP_DigestSignFinal(ctx, NULL, &length);
        if(length > 32){
            logWarning(SECURITY_CRYPTO,"Error generating the keys to perform token transaction");
            exception = SecurityException("Encountered an error while creating KxKeyMaterials");
            delete RPCrypto;
            EVP_PKEY_free(key);
#if IS_OPENSSL_1_1
            EVP_MD_CTX_free(ctx);
#else
            EVP_MD_CTX_cleanup(ctx);
            free(ctx);
#endif
            return nullptr;
        }
        EVP_DigestSignFinal(ctx, buffer.master_salt.data(), &length);
        EVP_PKEY_free(key);
#if IS_OPENSSL_1_1
        EVP_MD_CTX_free(ctx);
#else
        EVP_MD_CTX_cleanup(ctx);
        free(ctx);
#endif

        //Repeat process - concatenation is used to store the sequence to generate master_sender_key
        std::string KxSaltCookie("keyexchange salt");
        memcpy(concatenation.data(), challenge_2->data(), challenge_2->size());
        memcpy(concatenation.data() + challenge_2->size(), KxSaltCookie.data(), 16);
        memcpy(concatenation.data() + challenge_2->size() + 16, challenge_1->data(), challenge_1->size() );
        //Compute key to produce master_sender_key
        buffer.master_sender_key.fill(0);
        if(!EVP_Digest(concatenation.data(), challenge_1->size() + 16 + challenge_2->size(), p_master_salt.data(), NULL, EVP_sha256(), NULL)){
            logWarning(SECURITY_CRYPTO,"Error generating master key material");
            delete RPCrypto;
            return nullptr;
        }
        //The result of p_master_salt is now the key to perform an HMACsha256 of the shared secret that will go into master_sender_key
        key = EVP_PKEY_new_mac_key(EVP_PKEY_HMAC, NULL, p_master_salt.data(), 32);
        ctx =
#if IS_OPENSSL_1_1
            EVP_MD_CTX_new();
#else
            (EVP_MD_CTX*)malloc(sizeof(EVP_MD_CTX));
#endif
        EVP_MD_CTX_init(ctx);

        EVP_DigestSignInit(ctx, NULL, EVP_sha256(), NULL, key);
        EVP_DigestSignUpdate(ctx, shared_secret_ss->data(), shared_secret_ss->size());
        length = 0;
        EVP_DigestSignFinal(ctx, NULL, &length);
        if(length > 32){
            logWarning(SECURITY_CRYPTO,"Error generating master key material");
            delete RPCrypto;
            EVP_PKEY_free(key);
#if IS_OPENSSL_1_1
            EVP_MD_CTX_free(ctx);
#else
            EVP_MD_CTX_cleanup(ctx);
            free(ctx);
#endif
            return nullptr;
        }
        EVP_DigestSignFinal(ctx, buffer.master_sender_key.data(), &length);
        EVP_PKEY_free(key);
#if IS_OPENSSL_1_1
        EVP_MD_CTX_free(ctx);
#else
        EVP_MD_CTX_cleanup(ctx);
        free(ctx);
#endif

        buffer.sender_key_id.fill(0); //Specified by standard
        buffer.receiver_specific_key_id.fill(0); //Specified by standard
        buffer.master_receiver_specific_key.fill(0); //Specified by standard

        (*RPCrypto)->max_blocks_per_session = local_participant_handle->max_blocks_per_session;
        (*RPCrypto)->session_block_counter = local_participant_handle->session_block_counter;
        (*RPCrypto)->session_id = std::numeric_limits<uint32_t>::max();
        if((*RPCrypto)->session_id == local_participant_handle->session_id)
            (*RPCrypto)->session_id -= 1;

        //Attack to PartipantCryptoHandles - both local and remote
        (*RPCrypto)->Participant2ParticipantKxKeyMaterial.push_back(buffer);
    }

    return RPCrypto;
}

DatawriterCryptoHandle * AESGCMGMAC_KeyFactory::register_local_datawriter(
                ParticipantCryptoHandle &participant_crypto,
                const PropertySeq &datawriter_prop,
                SecurityException& /*exception*/){

    AESGCMGMAC_ParticipantCryptoHandle& participant_handle = AESGCMGMAC_ParticipantCryptoHandle::narrow(participant_crypto);
    if(participant_handle.nil()){
        logWarning(SECURITY_CRYPTO,"Invalid ParticipantCryptoHandle");
        return nullptr;
    }

    //Create ParticipantCryptoHandle, fill Participant KeyMaterial and return it
    AESGCMGMAC_WriterCryptoHandle* WCrypto = new AESGCMGMAC_WriterCryptoHandle();

    std::array<uint8_t, 4> transformationtype{CRYPTO_TRANSFORMATION_KIND_AES128_GCM}; //Default to AES128_GCM
    int maxblockspersession = 32; //Default to key update every 32 usages
    if(!datawriter_prop.empty()){
        for(auto it=datawriter_prop.begin(); it!=datawriter_prop.end(); ++it)
        {
            if( (it)->name() == "dds.sec.crypto.cryptotransformkind")
            {
                if(it->value() == std::string("AES128_GCM"))
                    transformationtype = std::array<uint8_t, 4>{CRYPTO_TRANSFORMATION_KIND_AES128_GCM};
                if(it->value() == std::string("AES128_GMAC"))
                    transformationtype = std::array<uint8_t, 4>{CRYPTO_TRANSFORMATION_KIND_AES128_GMAC};
                if(it->value() == std::string("AES256_GCM"))
                    transformationtype = std::array<uint8_t, 4>{CRYPTO_TRANSFORMATION_KIND_AES256_GCM};
                if(it->value() == std::string("AES256_GMAC"))
                    transformationtype = std::array<uint8_t, 4>{CRYPTO_TRANSFORMATION_KIND_AES256_GMAC};
            }// endif
            if( (it)->name() == "dds.sec.crypto.maxblockspersession"){
                try{
                    maxblockspersession = std::stoi( (it)->value() );
                }catch(std::invalid_argument){}
            }
        }//endfor
    }//endif
    (*WCrypto)->transformation_kind = transformationtype;
    //Fill WriterKeyMaterial - This will be used to cipher full rpts messages

    (*WCrypto)->EntityKeyMaterial.transformation_kind = transformationtype;
    (*WCrypto)->EntityKeyMaterial.master_salt.fill(0);
    RAND_bytes( (*WCrypto)->EntityKeyMaterial.master_salt.data(), 16 );

    (*WCrypto)->EntityKeyMaterial.sender_key_id = make_unique_KeyId();

    (*WCrypto)->EntityKeyMaterial.master_sender_key.fill(0);
    RAND_bytes( (*WCrypto)->EntityKeyMaterial.master_sender_key.data(), 16 );

    (*WCrypto)->EntityKeyMaterial.receiver_specific_key_id = {{0, 0, 0, 0}};  //No receiver specific, as this is the Master Participant Key
    (*WCrypto)->EntityKeyMaterial.master_receiver_specific_key.fill(0);

    (*WCrypto)->max_blocks_per_session = maxblockspersession;
    (*WCrypto)->session_block_counter = maxblockspersession+1; //Set to update upon first usage
    RAND_bytes( (unsigned char *)( &( (*WCrypto)->session_id ) ), sizeof(uint16_t));

    std::unique_lock<std::mutex>(participant_handle->mutex_);

    (*WCrypto)->Participant_master_key_id = participant_handle->ParticipantKeyMaterial.sender_key_id;

    (*WCrypto)->Parent_participant = &participant_crypto;

    participant_handle->Writers.push_back(WCrypto);

    return WCrypto;
}

DatareaderCryptoHandle * AESGCMGMAC_KeyFactory::register_matched_remote_datareader(
                DatawriterCryptoHandle &local_datawriter_crypto_handle,
                ParticipantCryptoHandle &remote_participant_crypto,
                const SharedSecretHandle& /*shared_secret*/,
                const bool /*relay_only*/,
                SecurityException& /*exception*/)
{
    //Create Participant2ParticipantKeyMaterial (Based on local ParticipantKeyMaterial) and ParticipantKxKeyMaterial (based on the SharedSecret)
    //Put both elements in the local and remote ParticipantCryptoHandle

    AESGCMGMAC_WriterCryptoHandle& local_writer_handle = AESGCMGMAC_WriterCryptoHandle::narrow(local_datawriter_crypto_handle);
    if(local_writer_handle.nil()){
        logWarning(SECURITY_CRYPTO,"Malformed DataWriterCryptoHandle");
        return nullptr;
    }

    std::unique_lock<std::mutex> writer_lock(local_writer_handle->mutex_);

    AESGCMGMAC_ReaderCryptoHandle* RRCrypto = new AESGCMGMAC_ReaderCryptoHandle(); // Remote Reader CryptoHandle, to be returned at the end of the function

    (*RRCrypto)->transformation_kind = local_writer_handle->transformation_kind;

    (*RRCrypto)->Participant_master_key_id = local_writer_handle->Participant_master_key_id;
    /*Fill values for Writer2ReaderKeyMaterial - Used to encrypt outgoing data */
    { //scope for temp var buffer
        KeyMaterial_AES_GCM_GMAC buffer;  //Buffer = Writer2ReaderKeyMaterial

        //These values must match the ones in ParticipantKeymaterial
        buffer.transformation_kind = local_writer_handle->EntityKeyMaterial.transformation_kind;
        buffer.master_salt = local_writer_handle->EntityKeyMaterial.master_salt;
        buffer.master_sender_key = local_writer_handle->EntityKeyMaterial.master_sender_key;

        buffer.sender_key_id = local_writer_handle->EntityKeyMaterial.sender_key_id;
        //buffer.sender_key_id = make_unique_KeyId();  //Unique identifier within the Participant (used to identity submessage types)
        //Generation of remainder values (Remote specific key)
        buffer.receiver_specific_key_id = make_unique_KeyId();
        buffer.master_receiver_specific_key.fill(0);
        RAND_bytes( buffer.master_receiver_specific_key.data(), 16 );

        //Attach to both local and remote CryptoHandles
        (*RRCrypto)->Remote2EntityKeyMaterial.push_back(buffer);
        local_writer_handle->Entity2RemoteKeyMaterial.push_back(buffer);
    }

    (*RRCrypto)->max_blocks_per_session = local_writer_handle->max_blocks_per_session;
    (*RRCrypto)->session_block_counter = local_writer_handle->session_block_counter;
    (*RRCrypto)->session_id = std::numeric_limits<uint32_t>::max();
    if((*RRCrypto)->session_id == local_writer_handle->session_id)
        (*RRCrypto)->session_id -= 1;

    writer_lock.unlock();

    AESGCMGMAC_ParticipantCryptoHandle& remote_participant = AESGCMGMAC_ParticipantCryptoHandle::narrow(remote_participant_crypto);

    std::unique_lock<std::mutex> remote_participant_lock(remote_participant->mutex_);

    (*RRCrypto)->Participant2ParticipantKxKeyMaterial = remote_participant->Participant2ParticipantKxKeyMaterial.at(0);

    (*RRCrypto)->Parent_participant = &remote_participant_crypto;
    //Save this CryptoHandle as part of the remote participant

    (*remote_participant)->Readers.push_back(RRCrypto);

    return RRCrypto;
}

DatareaderCryptoHandle * AESGCMGMAC_KeyFactory::register_local_datareader(
                ParticipantCryptoHandle &participant_crypto,
                const PropertySeq &datareader_properties,
                SecurityException& /*exception*/)
{
    AESGCMGMAC_ParticipantCryptoHandle& participant_handle = AESGCMGMAC_ParticipantCryptoHandle::narrow(participant_crypto);
    if(participant_handle.nil()){
        logWarning(SECURITY_CRYPTO,"Invalid ParticipantCryptoHandle");
        return nullptr;
    }

    //Create ParticipantCryptoHandle, fill Participant KeyMaterial and return it
    AESGCMGMAC_ReaderCryptoHandle* RCrypto = nullptr;

    RCrypto = new AESGCMGMAC_ReaderCryptoHandle();

    std::array<uint8_t, 4> transformationtype{CRYPTO_TRANSFORMATION_KIND_AES128_GCM}; //Default to AES128_GCM
    int maxblockspersession = 32; //Default to key update every 32 usages
    if(!datareader_properties.empty()){
          for(auto it=datareader_properties.begin(); it!=datareader_properties.end(); ++it){
              if( (it)->name() == "dds.sec.crypto.cryptotransformkind"){
                      if( it->value() == std::string("AES128_GCM") )
                            transformationtype = std::array<uint8_t, 4>{CRYPTO_TRANSFORMATION_KIND_AES128_GCM};
                      if( it->value() == std::string("AES128_GMAC") )
                            transformationtype = std::array<uint8_t, 4>{CRYPTO_TRANSFORMATION_KIND_AES128_GMAC};
                      if( it->value() == std::string("AES256_GCM") )
                            transformationtype = std::array<uint8_t, 4>{CRYPTO_TRANSFORMATION_KIND_AES256_GCM};
                      if( it->value() == std::string("AES256_GMAC") )
                            transformationtype = std::array<uint8_t, 4>{CRYPTO_TRANSFORMATION_KIND_AES256_GMAC};
              }// endif
              if( (it)->name() == "dds.sec.crypto.maxblockspersession"){
                  try{
                      maxblockspersession = std::stoi( (it)->value() );
                  }catch(std::invalid_argument){}
              }
          }//endfor
    }//endif

    (*RCrypto)->transformation_kind = transformationtype;
    //Fill ParticipantKeyMaterial - This will be used to cipher full rpts messages

    (*RCrypto)->EntityKeyMaterial.transformation_kind = transformationtype;

    (*RCrypto)->EntityKeyMaterial.master_salt.fill(0);
    RAND_bytes( (*RCrypto)->EntityKeyMaterial.master_salt.data(), 16 );

    (*RCrypto)->EntityKeyMaterial.sender_key_id = make_unique_KeyId();

    (*RCrypto)->EntityKeyMaterial.master_sender_key.fill(0);
    RAND_bytes( (*RCrypto)->EntityKeyMaterial.master_sender_key.data(), 16 );

    (*RCrypto)->EntityKeyMaterial.receiver_specific_key_id = {{0, 0, 0, 0}};  //No receiver specific, as this is the Master Participant Key
    (*RCrypto)->EntityKeyMaterial.master_receiver_specific_key.fill(0);

    (*RCrypto)->max_blocks_per_session = maxblockspersession;
    (*RCrypto)->session_block_counter = maxblockspersession+1;
    RAND_bytes( (unsigned char *)( &( (*RCrypto)->session_id ) ), sizeof(uint16_t));

    std::unique_lock<std::mutex> lock(participant_handle->mutex_);

    (*RCrypto)->Participant_master_key_id = participant_handle->ParticipantKeyMaterial.sender_key_id;

    (*RCrypto)->Parent_participant = &participant_crypto;

    participant_handle->Readers.push_back(RCrypto);

    return RCrypto;
}

DatawriterCryptoHandle * AESGCMGMAC_KeyFactory::register_matched_remote_datawriter(
                DatareaderCryptoHandle &local_datareader_crypto_handle,
                ParticipantCryptoHandle &remote_participant_crypt,
                const SharedSecretHandle& /*shared_secret*/,
                SecurityException& /*exception*/)
{

    //Create Participant2ParticipantKeyMaterial (Based on local ParticipantKeyMaterial) and ParticipantKxKeyMaterial (based on the SharedSecret)
    //Put both elements in the local and remote ParticipantCryptoHandle

    AESGCMGMAC_ReaderCryptoHandle& local_reader_handle = AESGCMGMAC_ReaderCryptoHandle::narrow(local_datareader_crypto_handle);
    if(local_reader_handle.nil()){
        logWarning(SECURITY_CRYPTO,"Invalid DataReaderCryptoHandle");
        return nullptr;
    }

    std::unique_lock<std::mutex> reader_lock(local_reader_handle->mutex_);

    AESGCMGMAC_WriterCryptoHandle* RWCrypto = new AESGCMGMAC_WriterCryptoHandle(); // Remote Writer CryptoHandle, to be returned at the end of the function

    (*RWCrypto)->Participant_master_key_id = local_reader_handle->Participant_master_key_id;
    (*RWCrypto)->transformation_kind = local_reader_handle->transformation_kind;
    /*Fill values for Writer2ReaderKeyMaterial - Used to encrypt outgoing data */
    { //scope for temp var buffer
        KeyMaterial_AES_GCM_GMAC buffer;  //Buffer = Writer2ReaderKeyMaterial

        //These values must match the ones in ParticipantKeymaterial
        buffer.transformation_kind = local_reader_handle->EntityKeyMaterial.transformation_kind;
        buffer.master_salt = local_reader_handle->EntityKeyMaterial.master_salt;
        buffer.master_sender_key = local_reader_handle->EntityKeyMaterial.master_sender_key;
        //Generation of remainder values (Remote specific key)

        buffer.sender_key_id = local_reader_handle->EntityKeyMaterial.sender_key_id;
        //buffer.sender_key_id = make_unique_KeyId();
        buffer.receiver_specific_key_id = make_unique_KeyId();
        buffer.master_receiver_specific_key.fill(0);
        RAND_bytes( buffer.master_receiver_specific_key.data(), 16 );

        //Attach to both local and remote CryptoHandles
        (*RWCrypto)->Remote2EntityKeyMaterial.push_back(buffer);
        local_reader_handle->Entity2RemoteKeyMaterial.push_back(buffer);
    }

    (*RWCrypto)->max_blocks_per_session = local_reader_handle->max_blocks_per_session;
    (*RWCrypto)->session_block_counter = local_reader_handle->session_block_counter;
    (*RWCrypto)->session_id = std::numeric_limits<uint32_t>::max();
    if((*RWCrypto)->session_id == local_reader_handle->session_id)
        (*RWCrypto)->session_id -= 1;

    reader_lock.unlock();

    AESGCMGMAC_ParticipantCryptoHandle& remote_participant = AESGCMGMAC_ParticipantCryptoHandle::narrow(remote_participant_crypt);

    std::unique_lock<std::mutex> remote_participant_lock(remote_participant->mutex_);

    (*RWCrypto)->Participant2ParticipantKxKeyMaterial = remote_participant->Participant2ParticipantKxKeyMaterial.at(0);

    (*RWCrypto)->Parent_participant = &remote_participant_crypt;

    //Save this CryptoHandle as part of the remote participant
    (*remote_participant)->Writers.push_back(RWCrypto);

    return RWCrypto;
}

bool AESGCMGMAC_KeyFactory::unregister_participant(
                ParticipantCryptoHandle* participant_crypto_handle,
                SecurityException &exception){

    if(participant_crypto_handle == nullptr)
        return false;

    //De-register the IDs
    AESGCMGMAC_ParticipantCryptoHandle& local_participant = AESGCMGMAC_ParticipantCryptoHandle::narrow(*participant_crypto_handle);

    if(local_participant.nil())
        return false;

    for(std::vector<CryptoTransformKeyId>::iterator it = m_CryptoTransformKeyIds.begin(); it != m_CryptoTransformKeyIds.end();it++)
    {
        if( (*it) == local_participant->ParticipantKeyMaterial.sender_key_id ){
            m_CryptoTransformKeyIds.erase(it);
        }
    }

    //Unregister all writers and readers
    std::vector<DatawriterCryptoHandle*>::iterator wit = local_participant->Writers.begin();
    while(wit != local_participant->Writers.end())
    {
        DatawriterCryptoHandle* writer = (DatawriterCryptoHandle*)(*wit);
        unregister_datawriter(writer, exception);
        wit = local_participant->Writers.begin();
    }

    std::vector<DatareaderCryptoHandle*>::iterator rit = local_participant->Readers.begin();
    while(rit != local_participant->Readers.end())
    {
        DatareaderCryptoHandle* reader = (DatareaderCryptoHandle*)(*rit);
        unregister_datareader(reader, exception);
        rit = local_participant->Readers.begin();
    }

    delete &local_participant;

    return true;

}

bool AESGCMGMAC_KeyFactory::unregister_datawriter(
                DatawriterCryptoHandle *datawriter_crypto_handle,
                SecurityException &exception){

    if(datawriter_crypto_handle == nullptr)
        return false;

    AESGCMGMAC_WriterCryptoHandle& datawriter = AESGCMGMAC_WriterCryptoHandle::narrow(*datawriter_crypto_handle);

    if(datawriter.nil()){
        exception = SecurityException("Not a valid DataWriterCryptoHandle has been passed as an argument");
        return false;
    }
    if( (datawriter->Parent_participant) == nullptr){
        AESGCMGMAC_WriterCryptoHandle *me = (AESGCMGMAC_WriterCryptoHandle *)datawriter_crypto_handle;
        delete me;
        return true;
    }
    AESGCMGMAC_ParticipantCryptoHandle& parent_participant = AESGCMGMAC_ParticipantCryptoHandle::narrow( *(datawriter->Parent_participant) );
    if(parent_participant.nil()){
        exception = SecurityException("Malformed AESGCMGMAC_WriterCryptohandle");
        return false;
    }

    //Remove reference in parent participant
    for(auto it = parent_participant->Writers.begin(); it != parent_participant->Writers.end(); it++){
        if( *it == datawriter_crypto_handle){
            parent_participant->Writers.erase(it);
            AESGCMGMAC_WriterCryptoHandle *me = (AESGCMGMAC_WriterCryptoHandle *)datawriter_crypto_handle;
            delete me;
            return true;
        }
    }

    return false;
}


bool AESGCMGMAC_KeyFactory::unregister_datareader(
                DatareaderCryptoHandle *datareader_crypto_handle,
                SecurityException &exception){

    if(datareader_crypto_handle == nullptr)
        return false;

    AESGCMGMAC_ReaderCryptoHandle& datareader = AESGCMGMAC_ReaderCryptoHandle::narrow(*datareader_crypto_handle);

    if(datareader.nil()){
        exception = SecurityException("Not a valid DataReaderCryptoHandle has been passed as an argument");
        return false;
    }
    if( (datareader->Parent_participant) == nullptr){
        AESGCMGMAC_ReaderCryptoHandle *me = (AESGCMGMAC_ReaderCryptoHandle *)datareader_crypto_handle;
        delete me;
        return true;
    }
    AESGCMGMAC_ParticipantCryptoHandle& parent_participant = AESGCMGMAC_ParticipantCryptoHandle::narrow( *(datareader->Parent_participant) );
    if(parent_participant.nil()){
        exception = SecurityException("Malformed AESGCMGMAC_WriterCryptohandle");
        return false;
    }

    //Remove reference in parent participant
    for(auto it = parent_participant->Readers.begin(); it != parent_participant->Readers.end(); it++){
        if( *it == datareader_crypto_handle){
            parent_participant->Readers.erase(it);
            AESGCMGMAC_ReaderCryptoHandle *parent = (AESGCMGMAC_ReaderCryptoHandle *)datareader_crypto_handle;
            delete parent;
            return true;
        }
    }

    return false;
}

CryptoTransformKeyId AESGCMGMAC_KeyFactory::make_unique_KeyId(){
    CryptoTransformKeyId buffer{{0, 0, 0, 0}};
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
