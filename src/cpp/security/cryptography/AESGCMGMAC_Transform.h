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
                DatawriterCryptoHandle &sending_datawriter_crypto,
                std::vector<DatareaderCryptoHandle*>& receiving_datareader_crypto_list,
                SecurityException &exception);

    bool encode_datareader_submessage(
                std::vector<uint8_t> &encoded_rtps_submessage,
                const std::vector<uint8_t> &plain_rtps_submessage,
                DatareaderCryptoHandle &sending_datareader_crypto,
                std::vector<DatawriterCryptoHandle*> &receiving_datawriter_crypto_list,
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
                const ParticipantCryptoHandle &sending_crypto,
                SecurityException &exception);

    bool preprocess_secure_submsg(
                DatawriterCryptoHandle **datawriter_crypto,
                DatareaderCryptoHandle **datareader_crypto,
                SecureSubmessageCategory_t &secure_submessage_category,
                const CDRMessage_t& encoded_rtps_submessage,
                ParticipantCryptoHandle &receiving_crypto,
                ParticipantCryptoHandle &sending_crypto,
                SecurityException &exception);

    bool decode_datawriter_submessage(
                CDRMessage_t& plain_rtps_submessage,
                CDRMessage_t& encoded_rtps_submessage,
                DatareaderCryptoHandle &receiving_datareader_crypto,
                DatawriterCryptoHandle &sending_datawriter_cryupto,
                SecurityException &exception);

    bool decode_datareader_submessage(
                CDRMessage_t& plain_rtps_submessage,
                CDRMessage_t& encoded_rtps_submessage,
                DatawriterCryptoHandle &receiving_datawriter_crypto,
                DatareaderCryptoHandle &sending_datareader_crypto,
                SecurityException &exception);

    bool decode_serialized_payload(
                std::vector<uint8_t> &plain_buffer,
                const std::vector<uint8_t> &encoded_buffer,
                const std::vector<uint8_t> &inline_qos,
                DatareaderCryptoHandle &receiving_datareader_crypto,
                DatawriterCryptoHandle &sending_datawriter_crypto,
                SecurityException &exception);

    //Aux function to compute session key from the master material
    std::array<uint8_t, 32> compute_sessionkey(const std::array<uint8_t, 32>& master_sender_key,
            const std::array<uint8_t, 32>& master_salt , const uint32_t session_id);

    //Serialization and deserialization of message components
    std::vector<uint8_t> serialize_SecureDataHeader(SecureDataHeader &input);
    std::vector<uint8_t> serialize_SecureDataBody(SecureDataBody &input);
    std::vector<uint8_t> serialize_SecureDataTag(SecureDataTag &input);
    SecureDataHeader deserialize_SecureDataHeader(std::vector<uint8_t> &input);
    SecureDataBody deserialize_SecureDataBody(std::vector<uint8_t> &input);
    SecureDataTag deserialize_SecureDataTag(std::vector<uint8_t> &input);

    //Wire assembly and disassembly of messages
    std::vector<uint8_t> assemble_serialized_payload(std::vector<uint8_t> &serialized_header,
            std::vector<uint8_t> &serialized_body,
            std::vector<uint8_t> &serialized_tag,
            unsigned char &flags);

    std::vector<uint8_t> assemble_endpoint_submessage(std::vector<uint8_t> &serialized_header,
             std::vector<uint8_t> &serialized_body,
             std::vector<uint8_t> &serialized_tag,
             unsigned char &flags);

     std::vector<uint8_t> assemble_rtps_message(std::vector<uint8_t> &rtps_header,
           std::vector<uint8_t> &serialized_header,
           std::vector<uint8_t> &serialized_body,
           std::vector<uint8_t> &serialized_tag,
           unsigned char &flags);

    bool disassemble_serialized_payload(const std::vector<uint8_t> &input,
            std::vector<uint8_t> &serialized_header,
            std::vector<uint8_t> &serialized_body,
            std::vector<uint8_t> &serialized_tag,
            unsigned char &flags);

    bool disassemble_endpoint_submessage(CDRMessage_t& input,
            std::vector<uint8_t> &serialized_header,
            std::vector<uint8_t> &serialized_body,
            std::vector<uint8_t> &serialized_tag,
            unsigned char &flags);

   bool disassemble_rtps_message(const std::vector<uint8_t> &input,
           std::vector<uint8_t> &serialized_header,
           std::vector<uint8_t> &serialized_body,
           std::vector<uint8_t> &serialized_tag,
           unsigned char &flags);

   uint32_t calculate_extra_size_for_rtps_message(uint32_t number_discovered_participants) const;

   uint32_t calculate_extra_size_for_rtps_submessage(uint32_t number_discovered_readers) const;

   uint32_t calculate_extra_size_for_encoded_payload(uint32_t number_discovered_readers) const;
};


} //namespace security
} //namespace rtps
} //namespace fastrtps
} //namespace eprosima

#endif // _SECURITY_AUTHENTICATION_AESGCMGMAC_TRANSFORM_H_
