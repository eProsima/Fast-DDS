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
 * @file AESGCMGMAC_KeyExchange.h
 */

#ifndef _SECURITY_AUTHENTICATION_AESGCMGMAC_KEYEXCHANGE_H_
#define _SECURITY_AUTHENTICATION_AESGCMGMAC_KEYEXCHANGE_H_


#include <fastdds/rtps/attributes/PropertyPolicy.hpp>
#include <rtps/security/cryptography/CryptoKeyExchange.h>
#include <security/cryptography/AESGCMGMAC_Types.h>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

class AESGCMGMAC_KeyExchange : public CryptoKeyExchange
{
public:

    AESGCMGMAC_KeyExchange();
    ~AESGCMGMAC_KeyExchange();

    bool create_local_participant_crypto_tokens(
            ParticipantCryptoTokenSeq& local_participant_crypto_tokens,
            const ParticipantCryptoHandle& local_participant_crypto,
            ParticipantCryptoHandle& remote_participant_crypto,
            SecurityException& exception) override;

    bool set_remote_participant_crypto_tokens(
            const ParticipantCryptoHandle& local_participant_crypto,
            ParticipantCryptoHandle& remote_participant_crypto,
            const ParticipantCryptoTokenSeq& remote_participant_tokens,
            SecurityException& exception) override;

    bool create_local_datawriter_crypto_tokens(
            DatawriterCryptoTokenSeq& local_datawriter_crypto_tokens,
            DatawriterCryptoHandle& local_datawriter_crypto,
            DatareaderCryptoHandle& remote_datareader_crypto,
            SecurityException& exception) override;

    bool create_local_datareader_crypto_tokens(
            DatareaderCryptoTokenSeq& local_datareader_crypto_tokens,
            DatareaderCryptoHandle& local_datareader_crypto,
            DatawriterCryptoHandle& remote_datawriter_crypto,
            SecurityException& exception) override;

    bool set_remote_datareader_crypto_tokens(
            DatawriterCryptoHandle& local_datawriter_crypto,
            DatareaderCryptoHandle& remote_datareader_crypto,
            const DatareaderCryptoTokenSeq& remote_datareader_tokens,
            SecurityException& exception) override;

    bool set_remote_datawriter_crypto_tokens(
            DatareaderCryptoHandle& local_datareader_crypto,
            DatawriterCryptoHandle& remote_datawriter_crypto,
            const DatawriterCryptoTokenSeq& remote_datawriter_tokens,
            SecurityException& exception) override;

    bool return_crypto_tokens(
            const CryptoTokenSeq& crypto_tokens,
            SecurityException& exception) override;

    //CDR Serialization and Deserialization of KeyMaterials
    std::vector<uint8_t> KeyMaterialCDRSerialize(
            KeyMaterial_AES_GCM_GMAC& key);
    void KeyMaterialCDRDeserialize(
            KeyMaterial_AES_GCM_GMAC& buffer,
            std::vector<uint8_t>* CDR);

    //Aux functions to cipher and decipher CryptoTokens
    // std::vector<uint8_t> aes_128_gcm_encrypt(const std::vector<uint8_t>& plaintext, const std::array<uint8_t,32>& key);
    // std::vector<uint8_t> aes_128_gcm_decrypt(const std::vector<uint8_t>& crypto, const std::array<uint8_t,32>& key);

};

} //namespace security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima

#endif // _SECURITY_AUTHENTICATION_AESGCMGMAC_KEYEXCHANGE_H_
