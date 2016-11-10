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
 * @file AESGCMGMAC_Transform.h
 */

#ifndef _SECURITY_AUTHENTICATION_AESGCMGMAC_TRANSFORM_H_
#define _SECURITY_AUTHENTICATION_AESGCMGMAC_TRANSFORM_H_

#include <fastrtps/rtps/security/cryptography/CryptoTransform.h>
#include <fastrtps/rtps/attributes/PropertyPolicy.h>

#include <map>
#include "AESGCMGMAC_Types.h"

namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace security {

struct CipherData{

    CryptoTransformKeyId master_key_id;
    uint32_t session_id;
    std::array<uint8_t,32> SessionKey;
    uint64_t session_block_counter;
    uint64_t max_blocks_per_session;
};

class AESGCMGMAC_Transform : public CryptoTransform
{
    public:

    AESGCMGMAC_Transform();
    ~AESGCMGMAC_Transform();

    bool encode_serialized_payload(
                std::vector<uint8_t> &encoded_buffer,
                std::vector<uint8_t> &extra_inline_qos,
                const std::vector<uint8_t> &plain_buffer,
                DatawriterCryptoHandle &sending_datawriter_crypto,
                SecurityException &exception);
         
    bool encode_datawriter_submessage(
                std::vector<uint8_t> &encoded_rtps_submessage,
                const std::vector<uint8_t> &plain_rtps_submessage,
                const DatawriterCryptoHandle &sending_datawriter_crypto,
                const std::vector<DatareaderCryptoHandle> receiving_datareader_crypto_list,
                SecurityException &exception);
    
    bool encode_datareader_submessage(
                std::vector<uint8_t> &encoded_rtps_submessage,
                const std::vector<uint8_t> &plain_rtps_submessage,
                const DatareaderCryptoHandle &sending_datareader_crypto,
                const std::vector<DatawriterCryptoHandle> &receiving_datawriter_crypto_list,
                SecurityException &exception);
    
    bool encode_rtps_message(
                std::vector<uint8_t> &encoded_rtps_message,
                const std::vector<uint8_t> &plain_rtps_message,
                ParticipantCryptoHandle &sending_crypto,
                const std::vector<ParticipantCryptoHandle*> &receiving_crypto_list,
                SecurityException &exception);

    bool decode_rtps_message(
                std::vector<uint8_t> &plain_buffer,
                const std::vector<uint8_t> &encoded_buffer,
                const ParticipantCryptoHandle &receiving_crypto,
                ParticipantCryptoHandle &sending_crypto,
                SecurityException &exception);
        
    bool preprocess_secure_submsg(
                DatawriterCryptoHandle &datawriter_crypto,
                DatareaderCryptoHandle &datareader_crypto,
                SecureSubmessageCategory_t &secure_submessage_category,
                const std::vector<uint8_t> encoded_rtps_submessage,
                const ParticipantCryptoHandle &receiving_crypto,
                const ParticipantCryptoHandle &sending_crypto,
                SecurityException &exception);

    bool decode_datawriter_submessage(
                std::vector<uint8_t> &plain_rtps_submessage,
                const std::vector<uint8_t> &encoded_rtps_submessage,
                const DatareaderCryptoHandle &receiving_datareader_crypto,
                const DatawriterCryptoHandle &sending_datawriter_cryupto,
                SecurityException &exception);

    bool decode_datareader_submessage(
                std::vector<uint8_t> &plain_rtps_submessage,
                const std::vector<uint8_t> &encoded_rtps_submessage,
                const DatawriterCryptoHandle &receiving_datawriter_crypto,
                const DatareaderCryptoHandle &sending_datareader_crypto,
                SecurityException &exception);

    bool decode_serialized_payload(
                std::vector<uint8_t> &plain_buffer,
                const std::vector<uint8_t> &encoded_buffer,
                const std::vector<uint8_t> &inline_qos,
                DatareaderCryptoHandle &receiving_datareader_crypto,
                DatawriterCryptoHandle &sending_datawriter_crypto,
                SecurityException &exception);

    bool remove_KeyId(CryptoTransformKeyId m_key);

    private:

    std::map<CryptoTransformKeyId, CipherData> status;

    std::array<uint8_t, 32> compute_sessionkey(std::array<uint8_t, 32> master_sender_key,std::array<uint8_t, 32> master_salt , uint32_t &session_id);

};

} //namespace security
} //namespace rtps
} //namespace fastrtps
} //namespace eprosima

#endif // _SECURITY_AUTHENTICATION_AESGCMGMAC_TRANSFORM_H_
