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

#include <fastcdr/Cdr.h>
#include <fastdds/rtps/attributes/PropertyPolicy.hpp>
#include <rtps/security/cryptography/CryptoTransform.h>

#include <map>
#include <security/cryptography/AESGCMGMAC_Types.h>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

class AESGCMGMAC_Transform : public CryptoTransform
{
public:

    AESGCMGMAC_Transform();
    ~AESGCMGMAC_Transform();

    bool encode_serialized_payload(
            SerializedPayload_t& encoded_payload,
            std::vector<uint8_t>& extra_inline_qos,
            const SerializedPayload_t& payload,
            DatawriterCryptoHandle& sending_datawriter_crypto,
            SecurityException& exception) override;

    bool encode_datawriter_submessage(
            CDRMessage_t& encoded_rtps_submessage,
            const CDRMessage_t& plain_rtps_submessage,
            DatawriterCryptoHandle& sending_datawriter_crypto,
            std::vector<std::shared_ptr<DatareaderCryptoHandle>>& receiving_datareader_crypto_list,
            SecurityException& exception) override;

    bool encode_datareader_submessage(
            CDRMessage_t& encoded_rtps_submessage,
            const CDRMessage_t& plain_rtps_submessage,
            DatareaderCryptoHandle& sending_datareader_crypto,
            std::vector<std::shared_ptr<DatawriterCryptoHandle>>& receiving_datawriter_crypto_list,
            SecurityException& exception) override;

    bool encode_rtps_message(
            CDRMessage_t& encoded_rtps_message,
            const CDRMessage_t& plain_rtps_message,
            ParticipantCryptoHandle& sending_crypto,
            std::vector<std::shared_ptr<ParticipantCryptoHandle>>& receiving_crypto_list,
            SecurityException& exception) override;

    bool decode_rtps_message(
            CDRMessage_t& plain_buffer,
            const CDRMessage_t& encoded_buffer,
            const ParticipantCryptoHandle& receiving_crypto,
            const ParticipantCryptoHandle& sending_crypto,
            SecurityException& exception) override;

    bool preprocess_secure_submsg(
            DatawriterCryptoHandle** datawriter_crypto,
            DatareaderCryptoHandle** datareader_crypto,
            SecureSubmessageCategory_t& secure_submessage_category,
            const CDRMessage_t& encoded_rtps_submessage,
            ParticipantCryptoHandle& receiving_crypto,
            ParticipantCryptoHandle& sending_crypto,
            SecurityException& exception) override;

    bool decode_datawriter_submessage(
            CDRMessage_t& plain_rtps_submessage,
            CDRMessage_t& encoded_rtps_submessage,
            DatareaderCryptoHandle& receiving_datareader_crypto,
            DatawriterCryptoHandle& sending_datawriter_cryupto,
            SecurityException& exception) override;

    bool decode_datareader_submessage(
            CDRMessage_t& plain_rtps_submessage,
            CDRMessage_t& encoded_rtps_submessage,
            DatawriterCryptoHandle& receiving_datawriter_crypto,
            DatareaderCryptoHandle& sending_datareader_crypto,
            SecurityException& exception) override;

    bool decode_serialized_payload(
            SerializedPayload_t& plain_payload,
            const SerializedPayload_t& encoded_payload,
            const std::vector<uint8_t>& inline_qos,
            DatareaderCryptoHandle& receiving_datareader_crypto,
            DatawriterCryptoHandle& sending_datawriter_crypto,
            SecurityException& exception) override;

    //Aux functions to compute session key from the master material
    void compute_sessionkey(
            std::array<uint8_t, 32>& session_key,
            bool receiver_specific,
            const std::array<uint8_t, 32>& master_key,
            const std::array<uint8_t, 32>& master_salt,
            const uint32_t session_id,
            int key_len);

    void compute_sessionkey(
            std::array<uint8_t, 32>& session_key,
            const KeyMaterial_AES_GCM_GMAC& key,
            const uint32_t session_id);

    //Serialization and deserialization of message components
    void serialize_SecureDataHeader(
            eprosima::fastcdr::Cdr& serializer,
            const CryptoTransformKind& transformation_kind,
            const CryptoTransformKeyId& transformation_key_id,
            const std::array<uint8_t, 4>& session_id,
            const std::array<uint8_t, 8>& initialization_vector_suffix);

    bool serialize_SecureDataBody(
            eprosima::fastcdr::Cdr& serializer,
            const std::array<uint8_t, 4>& transformation_kind,
            const std::array<uint8_t, 32>& session_key,
            const std::array<uint8_t, 12>& initialization_vector,
            eprosima::fastcdr::FastBuffer& output_buffer,
            octet* plain_buffer,
            uint32_t plain_buffer_len,
            SecureDataTag& tag,
            bool submessage);

    bool serialize_SecureDataTag(
            eprosima::fastcdr::Cdr& serializer,
            const std::array<uint8_t, 4>& transformation_kind,
            const uint32_t session_id,
            const std::array<uint8_t, 12>& initialization_vector,
            std::vector<std::shared_ptr<EntityCryptoHandle>>& receiving_crypto_list,
            bool update_specific_keys,
            SecureDataTag& tag,
            size_t sessionIndex);

    bool serialize_SecureDataTag(
            eprosima::fastcdr::Cdr& serializer,
            const AESGCMGMAC_ParticipantCryptoHandle& local_participant,
            const std::array<uint8_t, 12>& initialization_vector,
            std::vector<std::shared_ptr<EntityCryptoHandle>>& receiving_crypto_list,
            bool update_specific_keys,
            SecureDataTag& tag);

    SecureDataHeader deserialize_SecureDataHeader(
            eprosima::fastcdr::Cdr& decoder);

    /**
     * Get information on the data between a Header and a Tag submessage.
     * @param decoder Cdr decoding stream pointing to the first byte after the Header submessage
     * @param body_length Outputs length of protected data
     * @param body_align Outputs number of alignment bytes after protected data
     * @return true when protected data is encrypted (i.e. it is a SEC_BODY submessage)
     */
    bool predeserialize_SecureDataBody(
            eprosima::fastcdr::Cdr& decoder,
            uint32_t& body_length,
            uint32_t& body_align);

    bool deserialize_SecureDataBody(
            eprosima::fastcdr::Cdr& decoder,
            eprosima::fastcdr::Cdr::state& body_state,
            SecureDataTag& tag,
            uint32_t body_length,
            const std::array<uint8_t, 4>& transformation_kind,
            const std::array<uint8_t, 32>& session_key,
            const std::array<uint8_t, 12>& initialization_vector,
            octet* plain_buffer,
            uint32_t& plain_buffer_len);

    bool deserialize_SecureDataTag(
            eprosima::fastcdr::Cdr& decoder,
            SecureDataTag& tag,
            const CryptoTransformKind& transformation_kind,
            const CryptoTransformKeyId& receiver_specific_key_id,
            const std::array<uint8_t, 32>& receiver_specific_key,
            const std::array<uint8_t, 32>& master_salt,
            const std::array<uint8_t, 12>& initialization_vector,
            uint32_t session_id,
            SecurityException& exception);

    uint32_t calculate_extra_size_for_rtps_message(
            uint32_t number_discovered_participants) const override;

    uint32_t calculate_extra_size_for_rtps_submessage(
            uint32_t number_discovered_readers) const override;

    uint32_t calculate_extra_size_for_encoded_payload(
            uint32_t number_discovered_readers) const override;

private:

    //Aux function to lookup endpoints
    bool lookup_reader(
            AESGCMGMAC_ParticipantCryptoHandle& participant,
            DatareaderCryptoHandle** datareader_crypto,
            CryptoTransformKeyId key_id);

    bool lookup_writer(
            AESGCMGMAC_ParticipantCryptoHandle& participant,
            DatawriterCryptoHandle** datawriter_crypto,
            CryptoTransformKeyId key_id);

};


} //namespace security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima

#endif // _SECURITY_AUTHENTICATION_AESGCMGMAC_TRANSFORM_H_
