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

#include <security/cryptography/AESGCMGMAC_Transform.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/common/CdrSerialization.hpp>
#include <rtps/messages/CDRMessage.hpp>

#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <cstring>

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
#define IS_OPENSSL_1_1 1
#else
#define IS_OPENSSL_1_1 0
#endif // if OPENSSL_VERSION_NUMBER >= 0x10100000L

// Solve error with Win32 macro
#ifdef WIN32
#undef max
#endif // ifdef WIN32

namespace eprosima {
namespace fastdds {
namespace rtps {

using namespace security;

constexpr int initialization_vector_suffix_length = 8;

static KeyMaterial_AES_GCM_GMAC* find_key(
        KeyMaterial_AES_GCM_GMAC_Seq& keys,
        const CryptoTransformIdentifier& id)
{
    for (auto& it : keys)
    {
        if (it.transformation_kind == id.transformation_kind)
        {
            if ((it.sender_key_id == id.transformation_key_id) ||
                    (it.receiver_specific_key_id == id.transformation_key_id))
            {
                return &it;
            }
        }
    }

    return nullptr;
}

AESGCMGMAC_Transform::AESGCMGMAC_Transform()
{
}

AESGCMGMAC_Transform::~AESGCMGMAC_Transform()
{
}

bool AESGCMGMAC_Transform::encode_serialized_payload(
        SerializedPayload_t& output_payload,
        std::vector<uint8_t>& /*extra_inline_qos*/,
        const SerializedPayload_t& payload,
        DatawriterCryptoHandle& sending_datawriter_crypto,
        SecurityException& /*exception*/)
{
    AESGCMGMAC_WriterCryptoHandle& local_writer = AESGCMGMAC_WriterCryptoHandle::narrow(sending_datawriter_crypto);
    if (local_writer.nil())
    {
        EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "Invalid CryptoHandle");
        return false;
    }

    // Precondition to use openssl
    if (payload.length > static_cast<uint32_t>(std::numeric_limits<int>::max()))
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Plain text too large");
        return false;
    }

    eprosima::fastcdr::FastBuffer output_buffer((char*)output_payload.data, output_payload.max_size);
    eprosima::fastcdr::Cdr serializer(output_buffer);

    std::unique_lock<std::mutex> lock(local_writer->mutex_);

    // Payload is always protected by the last key
    auto nKeys = local_writer->EntityKeyMaterial.size();
    auto& keyMat = local_writer->EntityKeyMaterial.at(nKeys - 1);
    auto session = &local_writer->Sessions[nKeys - 1];

    //If the maximum number of blocks have been processed, generate a new SessionKey
    if (session->session_block_counter >= local_writer->max_blocks_per_session)
    {
        session->session_id += 1;

        compute_sessionkey(session->SessionKey, keyMat, session->session_id);

        //ReceiverSpecific keys shall be computed specifically when needed
        session->session_block_counter = 0;
    }
    //In any case, increment session block counter
    session->session_block_counter += 1;

    //Build NONCE elements (Build once, use once)
    std::array<uint8_t, initialization_vector_suffix_length> initialization_vector_suffix;  //iv suffix changes with every operation
    RAND_bytes(initialization_vector_suffix.data(), initialization_vector_suffix_length);
    std::array<uint8_t, 12> initialization_vector; //96 bytes, session_id + suffix
    memcpy(initialization_vector.data(), &(session->session_id), 4);
    memcpy(initialization_vector.data() + 4, initialization_vector_suffix.data(), initialization_vector_suffix_length);
    std::array<uint8_t, 4> session_id;
    memcpy(session_id.data(), &(session->session_id), 4);

    //Header
    try
    {
        serialize_SecureDataHeader(serializer, keyMat.transformation_kind,
                keyMat.sender_key_id, session_id, initialization_vector_suffix);
    }
    catch (eprosima::fastcdr::exception::Exception&)
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Error in fastcdr trying to serialize SecureDataHeader");
        return false;
    }

    SecureDataTag tag;

    // Body
    try
    {
        if (!serialize_SecureDataBody(serializer, keyMat.transformation_kind, session->SessionKey,
                initialization_vector, output_buffer, payload.data, payload.length, tag, false))
        {
            return false;
        }
    }
    catch (eprosima::fastcdr::exception::Exception&)
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Error in fastcdr trying to serialize SecureDataBody");
        return false;
    }

    try
    {
        std::vector<std::shared_ptr<DatareaderCryptoHandle>> receiving_datareader_crypto_list;
        if (!serialize_SecureDataTag(serializer, keyMat.transformation_kind, session->session_id,
                initialization_vector, receiving_datareader_crypto_list, false, tag, nKeys - 1))
        {
            return false;
        }
    }
    catch (eprosima::fastcdr::exception::Exception&)
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Error in fastcdr trying to serialize SecureDataTag");
        return false;
    }

    // Store information in CDRMessage_t
    output_payload.length = static_cast<uint32_t>(serializer.get_serialized_data_length());

    return true;
}

bool AESGCMGMAC_Transform::encode_datawriter_submessage(
        CDRMessage_t& encoded_rtps_submessage,
        const CDRMessage_t& plain_rtps_submessage,
        DatawriterCryptoHandle& sending_datawriter_crypto,
        std::vector<std::shared_ptr<DatareaderCryptoHandle>>& receiving_datareader_crypto_list,
        SecurityException& /*exception*/)
{
    AESGCMGMAC_WriterCryptoHandle& local_writer = AESGCMGMAC_WriterCryptoHandle::narrow(sending_datawriter_crypto);

    if (local_writer.nil())
    {
        EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "Invalid cryptoHandle");
        return false;
    }

    if ((plain_rtps_submessage.length  - plain_rtps_submessage.pos) >
            static_cast<uint32_t>(std::numeric_limits<int>::max()))
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Plain rtps submessage too large");
        return false;
    }

    eprosima::fastcdr::FastBuffer output_buffer((char*)&encoded_rtps_submessage.buffer[encoded_rtps_submessage.pos],
            encoded_rtps_submessage.max_size - encoded_rtps_submessage.pos);
    eprosima::fastcdr::Cdr serializer(output_buffer);

    std::unique_lock<std::mutex> lock(local_writer->mutex_);

    // Submessage is always protected by the first key
    auto& keyMat = local_writer->EntityKeyMaterial.at(0);
    auto session = &local_writer->Sessions[0];

    bool update_specific_keys = false;
    //If the maximum number of blocks have been processed, generate a new SessionKey
    if (session->session_block_counter >= local_writer->max_blocks_per_session)
    {
        session->session_id += 1;
        update_specific_keys = true;
        compute_sessionkey(session->SessionKey, keyMat, session->session_id);

        //ReceiverSpecific keys shall be computed specifically when needed
        session->session_block_counter = 0;
    }

    session->session_block_counter += 1;

    //Build remaining NONCE elements
    std::array<uint8_t, initialization_vector_suffix_length> initialization_vector_suffix;  //iv suffix changes with every operation
    RAND_bytes(initialization_vector_suffix.data(), initialization_vector_suffix_length);
    std::array<uint8_t, 12> initialization_vector; //96 bytes, session_id + suffix
    memcpy(initialization_vector.data(), &(session->session_id), 4);
    memcpy(initialization_vector.data() + 4, initialization_vector_suffix.data(), 8);
    std::array<uint8_t, 4> session_id;
    memcpy(session_id.data(), &(session->session_id), 4);

#if FASTDDS_IS_BIG_ENDIAN_TARGET
    octet flags = 0x0;
    serializer.change_endianness(eprosima::fastcdr::Cdr::Endianness::BIG_ENDIANNESS);
#else
    octet flags = BIT(0);
    serializer.change_endianness(eprosima::fastcdr::Cdr::Endianness::LITTLE_ENDIANNESS);
#endif // if FASTDDS_IS_BIG_ENDIAN_TARGET

    //Header
    try
    {
        serializer << SEC_PREFIX << flags;
        eprosima::fastcdr::Cdr::state length_state = serializer.get_state();
        uint16_t length = 0;
        serializer << length;

        const char* length_position = serializer.get_current_position();

        serialize_SecureDataHeader(serializer, keyMat.transformation_kind,
                keyMat.sender_key_id, session_id, initialization_vector_suffix);

        eprosima::fastcdr::Cdr::state current_state = serializer.get_state();
        //TODO(Ricardo) fastcdr functinality: length substracting two Cdr::state.
        length =  static_cast<uint16_t>(serializer.get_current_position() - length_position);
        serializer.set_state(length_state);
        serializer << length;
        serializer.set_state(current_state);
    }
    catch (eprosima::fastcdr::exception::NotEnoughMemoryException& )
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Error in fastcdr trying to serialize SecureDataHeader");
        return false;
    }

    SecureDataTag tag;

    // Body
    try
    {
        if (!serialize_SecureDataBody(serializer, keyMat.transformation_kind, session->SessionKey,
                initialization_vector, output_buffer, &plain_rtps_submessage.buffer[plain_rtps_submessage.pos],
                plain_rtps_submessage.length - plain_rtps_submessage.pos, tag, true))
        {
            return false;
        }
    }
    catch (eprosima::fastcdr::exception::Exception&)
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Error in fastcdr trying to serialize SecureDataBody");
        return false;
    }

    // Tag
    try
    {
        serializer << SEC_POSTFIX << flags;
        eprosima::fastcdr::Cdr::state length_state = serializer.get_state();
        uint16_t length = 0;
        serializer << length;

        const char* length_position = serializer.get_current_position();

        if (!serialize_SecureDataTag(serializer, keyMat.transformation_kind, session->session_id,
                initialization_vector, receiving_datareader_crypto_list, update_specific_keys, tag, 0))
        {
            return false;
        }

        eprosima::fastcdr::Cdr::state current_state = serializer.get_state();
        //TODO(Ricardo) fastcdr functinality: length substracting two Cdr::state.
        length =  static_cast<uint16_t>(serializer.get_current_position() - length_position);
        serializer.set_state(length_state);
        serializer << length;
        serializer.set_state(current_state);
    }
    catch (eprosima::fastcdr::exception::Exception&)
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Error in fastcdr trying to serialize SecureDataTag");
        return false;
    }

    encoded_rtps_submessage.pos += static_cast<uint32_t>(serializer.get_serialized_data_length());
    encoded_rtps_submessage.length += static_cast<uint32_t>(serializer.get_serialized_data_length());

    return true;
}

bool AESGCMGMAC_Transform::encode_datareader_submessage(
        CDRMessage_t& encoded_rtps_submessage,
        const CDRMessage_t& plain_rtps_submessage,
        DatareaderCryptoHandle& sending_datareader_crypto,
        std::vector<std::shared_ptr<DatawriterCryptoHandle>>& receiving_datawriter_crypto_list,
        SecurityException& /*exception*/)
{
    AESGCMGMAC_ReaderCryptoHandle& local_reader = AESGCMGMAC_ReaderCryptoHandle::narrow(sending_datareader_crypto);

    if (local_reader.nil())
    {
        EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "Invalid CryptoHandle");
        return false;
    }

    if ((plain_rtps_submessage.length  - plain_rtps_submessage.pos) >
            static_cast<uint32_t>(std::numeric_limits<int>::max()))
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Plain rtps submessage too large");
        return false;
    }

    eprosima::fastcdr::FastBuffer output_buffer((char*)&encoded_rtps_submessage.buffer[encoded_rtps_submessage.pos],
            encoded_rtps_submessage.max_size - encoded_rtps_submessage.pos);
    eprosima::fastcdr::Cdr serializer(output_buffer);

    std::unique_lock<std::mutex> lock(local_reader->mutex_);

    //Step 2 - If the maximum number of blocks have been processed, generate a new SessionKey
    auto session = &local_reader->Sessions[0];
    bool update_specific_keys = false;
    if (session->session_block_counter >= local_reader->max_blocks_per_session)
    {
        session->session_id += 1;
        update_specific_keys = true;
        compute_sessionkey(session->SessionKey, local_reader->EntityKeyMaterial.at(0),
                session->session_id);

        //ReceiverSpecific keys shall be computed specifically when needed
        session->session_block_counter = 0;
    }

    session->session_block_counter += 1;

    //Build remaining NONCE elements
    std::array<uint8_t, initialization_vector_suffix_length> initialization_vector_suffix;  //iv suffix changes with every operation
    RAND_bytes(initialization_vector_suffix.data(), initialization_vector_suffix_length);
    std::array<uint8_t, 12> initialization_vector; //96 bytes, session_id + suffix
    memcpy(initialization_vector.data(), &(session->session_id), 4);
    memcpy(initialization_vector.data() + 4, initialization_vector_suffix.data(), 8);
    std::array<uint8_t, 4> session_id;
    memcpy(session_id.data(), &(session->session_id), 4);

#if FASTDDS_IS_BIG_ENDIAN_TARGET
    octet flags = 0x0;
    serializer.change_endianness(eprosima::fastcdr::Cdr::Endianness::BIG_ENDIANNESS);
#else
    octet flags = BIT(0);
    serializer.change_endianness(eprosima::fastcdr::Cdr::Endianness::LITTLE_ENDIANNESS);
#endif // if FASTDDS_IS_BIG_ENDIAN_TARGET

    //Header
    try
    {
        serializer << SEC_PREFIX << flags;
        eprosima::fastcdr::Cdr::state length_state = serializer.get_state();
        uint16_t length = 0;
        serializer << length;

        const char* length_position = serializer.get_current_position();

        serialize_SecureDataHeader(serializer, local_reader->EntityKeyMaterial.at(0).transformation_kind,
                local_reader->EntityKeyMaterial.at(0).sender_key_id, session_id, initialization_vector_suffix);

        eprosima::fastcdr::Cdr::state current_state = serializer.get_state();
        //TODO(Ricardo) fastcdr functinality: length substracting two Cdr::state.
        length =  static_cast<uint16_t>(serializer.get_current_position() - length_position);
        serializer.set_state(length_state);
        serializer << length;
        serializer.set_state(current_state);
    }
    catch (eprosima::fastcdr::exception::Exception&)
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Error in fastcdr trying to serialize SecureDataHeader");
        return false;
    }

    SecureDataTag tag;

    // Body
    try
    {
        if (!serialize_SecureDataBody(serializer, local_reader->EntityKeyMaterial.at(0).transformation_kind,
                session->SessionKey,
                initialization_vector, output_buffer, &plain_rtps_submessage.buffer[plain_rtps_submessage.pos],
                plain_rtps_submessage.length - plain_rtps_submessage.pos, tag, true))
        {
            return false;
        }
    }
    catch (eprosima::fastcdr::exception::Exception&)
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Error in fastcdr trying to serialize SecureDataBody");
        return false;
    }

    // Tag
    try
    {
        serializer << SEC_POSTFIX << flags;
        eprosima::fastcdr::Cdr::state length_state = serializer.get_state();
        uint16_t length = 0;
        serializer << length;

        const char* length_position = serializer.get_current_position();

        if (!serialize_SecureDataTag(serializer, local_reader->EntityKeyMaterial.at(0).transformation_kind,
                session->session_id,
                initialization_vector, receiving_datawriter_crypto_list, update_specific_keys, tag, 0))
        {
            return false;
        }

        eprosima::fastcdr::Cdr::state current_state = serializer.get_state();
        //TODO(Ricardo) fastcdr functinality: length substracting two Cdr::state.
        length =  static_cast<uint16_t>(serializer.get_current_position() - length_position);
        serializer.set_state(length_state);
        serializer << length;
        serializer.set_state(current_state);
    }
    catch (eprosima::fastcdr::exception::Exception&)
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Error in fastcdr trying to serialize SecureDataTag");
        return false;
    }

    encoded_rtps_submessage.pos += static_cast<uint32_t>(serializer.get_serialized_data_length());
    encoded_rtps_submessage.length += static_cast<uint32_t>(serializer.get_serialized_data_length());

    return true;
}

bool AESGCMGMAC_Transform::encode_rtps_message(
        CDRMessage_t& encoded_rtps_message,
        const CDRMessage_t& plain_rtps_message,
        ParticipantCryptoHandle& sending_crypto,
        std::vector<std::shared_ptr<ParticipantCryptoHandle>>& receiving_crypto_list,
        SecurityException& /*exception*/)
{
    AESGCMGMAC_ParticipantCryptoHandle& local_participant = AESGCMGMAC_ParticipantCryptoHandle::narrow(sending_crypto);

    if (local_participant.nil())
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Invalid CryptoToken");
        return false;
    }

    if ((plain_rtps_message.length  - plain_rtps_message.pos) > static_cast<uint32_t>(std::numeric_limits<int>::max()))
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Plain rtps submessage too large");
        return false;
    }

    eprosima::fastcdr::FastBuffer output_buffer((char*)&encoded_rtps_message.buffer[encoded_rtps_message.pos],
            encoded_rtps_message.max_size - encoded_rtps_message.pos);
    eprosima::fastcdr::Cdr serializer(output_buffer);

    std::unique_lock<std::mutex> lock(local_participant->mutex_);

    // If the maximum number of blocks have been processed, generate a new SessionKey
    bool update_specific_keys = false;
    if (local_participant->Session.session_block_counter >= local_participant->max_blocks_per_session)
    {
        local_participant->Session.session_id += 1;
        update_specific_keys = true;
        compute_sessionkey(local_participant->Session.SessionKey, local_participant->ParticipantKeyMaterial,
                local_participant->Session.session_id);

        //ReceiverSpecific keys shall be computed specifically when needed
        local_participant->Session.session_block_counter = 0;
        //Insert outdate session_id values in all RemoteParticipant trackers to trigger a SessionkeyUpdate
    }

    local_participant->Session.session_block_counter += 1;

    //Build remaining NONCE elements
    std::array<uint8_t, initialization_vector_suffix_length> initialization_vector_suffix;  //iv suffix changes with every operation
    RAND_bytes(initialization_vector_suffix.data(), initialization_vector_suffix_length);
    std::array<uint8_t, 12> initialization_vector; //96 bytes, session_id + suffix
    memcpy(initialization_vector.data(), &(local_participant->Session.session_id), 4);
    memcpy(initialization_vector.data() + 4, initialization_vector_suffix.data(), 8);
    std::array<uint8_t, 4> session_id;
    memcpy(session_id.data(), &(local_participant->Session.session_id), 4);

#if FASTDDS_IS_BIG_ENDIAN_TARGET
    octet flags = 0x0;
    serializer.change_endianness(eprosima::fastcdr::Cdr::Endianness::BIG_ENDIANNESS);
#else
    octet flags = BIT(0);
    serializer.change_endianness(eprosima::fastcdr::Cdr::Endianness::LITTLE_ENDIANNESS);
#endif // if FASTDDS_IS_BIG_ENDIAN_TARGET

    //Header
    try
    {
        serializer << SRTPS_PREFIX << flags;
        eprosima::fastcdr::Cdr::state length_state = serializer.get_state();
        uint16_t length = 0;
        serializer << length;

        const char* length_position = serializer.get_current_position();

        serialize_SecureDataHeader(serializer, local_participant->ParticipantKeyMaterial.transformation_kind,
                local_participant->ParticipantKeyMaterial.sender_key_id, session_id, initialization_vector_suffix);

        eprosima::fastcdr::Cdr::state current_state = serializer.get_state();
        //TODO(Ricardo) fastcdr functinality: length substracting two Cdr::state.
        length =  static_cast<uint16_t>(serializer.get_current_position() - length_position);
        serializer.set_state(length_state);
        serializer << length;
        serializer.set_state(current_state);
    }
    catch (eprosima::fastcdr::exception::Exception&)
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Error in fastcdr trying to serialize SecureDataHeader");
        return false;
    }

    SecureDataTag tag;

    // Body
    try
    {
        if (!serialize_SecureDataBody(serializer, local_participant->ParticipantKeyMaterial.transformation_kind,
                local_participant->Session.SessionKey,
                initialization_vector, output_buffer, &plain_rtps_message.buffer[plain_rtps_message.pos],
                plain_rtps_message.length - plain_rtps_message.pos, tag, true))
        {
            return false;
        }
    }
    catch (eprosima::fastcdr::exception::Exception&)
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Error in fastcdr trying to serialize SecureDataBody");
        return false;
    }

    // Tag
    try
    {
        serializer << SRTPS_POSTFIX << flags;
        eprosima::fastcdr::Cdr::state length_state = serializer.get_state();
        uint16_t length = 0;
        serializer << length;

        const char* length_position = serializer.get_current_position();

        if (!serialize_SecureDataTag(serializer, local_participant, initialization_vector, receiving_crypto_list,
                update_specific_keys, tag))
        {
            return false;
        }

        eprosima::fastcdr::Cdr::state current_state = serializer.get_state();
        //TODO(Ricardo) fastcdr functinality: length substracting two Cdr::state.
        length =  static_cast<uint16_t>(serializer.get_current_position() - length_position);
        serializer.set_state(length_state);
        serializer << length;
        serializer.set_state(current_state);
    }
    catch (eprosima::fastcdr::exception::Exception&)
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Error in fastcdr trying to serialize SecureDataTag");
        return false;
    }

    encoded_rtps_message.pos += static_cast<uint32_t>(serializer.get_serialized_data_length());
    encoded_rtps_message.length += static_cast<uint32_t>(serializer.get_serialized_data_length());

    return true;
}

bool AESGCMGMAC_Transform::decode_rtps_message(
        CDRMessage_t& plain_buffer,
        const CDRMessage_t& encoded_buffer,
        const ParticipantCryptoHandle& /*receiving_crypto*/,
        const ParticipantCryptoHandle& sending_crypto,
        SecurityException& /*exception*/)
{
    const AESGCMGMAC_ParticipantCryptoHandle& sending_participant = AESGCMGMAC_ParticipantCryptoHandle::narrow(
        sending_crypto);

    if (sending_participant.nil())
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Invalid sending_crypto handle");
        return false;
    }

    // Output buffer has to have position and length with same value.
    if (plain_buffer.pos != plain_buffer.length)
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Output message is not set correctly");
        return false;
    }

    if ((encoded_buffer.length - encoded_buffer.pos) > static_cast<uint32_t>(std::numeric_limits<int>::max()))
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Encoded rtps message too large");
        return false;
    }

    if (sending_participant->RemoteParticipant2ParticipantKeyMaterial.size() == 0)
    {
        return false;
    }

    eprosima::fastcdr::FastBuffer input_buffer((char*)&encoded_buffer.buffer[encoded_buffer.pos],
            encoded_buffer.length - encoded_buffer.pos);
    eprosima::fastcdr::Cdr decoder(input_buffer);

    SecureDataHeader header;
    SecureDataTag tag;

    try
    {
        uint8_t id = 0, flags = 0;
        uint16_t length = 0;

        decoder >> id;

        if (id != SRTPS_PREFIX)
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Not valid SecureDataHeader submessage id");
            return false;
        }

        decoder >> flags;

        if (flags & BIT(0))
        {
            decoder.change_endianness(eprosima::fastcdr::Cdr::Endianness::LITTLE_ENDIANNESS);
        }
        else
        {
            decoder.change_endianness(eprosima::fastcdr::Cdr::Endianness::BIG_ENDIANNESS);
        }

        decoder >> length;
        const char* const current_position = decoder.get_current_position();

        header = deserialize_SecureDataHeader(decoder);

        if (length != (uint16_t)(decoder.get_current_position() - current_position))
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Invalid length for SecureDataHeader");
            return false;
        }
    }
    catch (eprosima::fastcdr::exception::Exception&)
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Error in fastcdr trying to deserialize SecureDataHeader");
        return false;
    }

    uint32_t session_id;
    memcpy(&session_id, header.session_id.data(), 4);

    //Sessionkey
    std::array<uint8_t, 32> session_key{};
    compute_sessionkey(session_key,
            sending_participant->RemoteParticipant2ParticipantKeyMaterial.at(0),
            session_id);
    //IV
    std::array<uint8_t, 12> initialization_vector{};
    memcpy(initialization_vector.data(), header.session_id.data(), 4);
    memcpy(initialization_vector.data() + 4,
            header.initialization_vector_suffix.data(), initialization_vector_suffix_length);

    // Body
    uint32_t body_length = 0, body_align = 0;
    eprosima::fastcdr::Cdr::state protected_body_state = decoder.get_state();
    bool is_encrypted = false;

    try
    {
        is_encrypted = predeserialize_SecureDataBody(decoder, body_length, body_align);
    }
    catch (eprosima::fastcdr::exception::Exception&)
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Error in fastcdr trying to deserialize SecureDataBody header");
        return false;
    }

    eprosima::fastcdr::Cdr::state body_state = decoder.get_state();
    decoder.jump(body_length + body_align);

    // Tag
    try
    {
        uint8_t id = 0, flags = 0;
        uint16_t length = 0;

        decoder >> id;

        // If encryption is disabled, jump over several submessages until we find the SecureDataTag
        while (!is_encrypted && (id != SRTPS_POSTFIX))
        {
            decoder >> flags;

            if (flags & BIT(0))
            {
                decoder.change_endianness(eprosima::fastcdr::Cdr::Endianness::LITTLE_ENDIANNESS);
            }
            else
            {
                decoder.change_endianness(eprosima::fastcdr::Cdr::Endianness::BIG_ENDIANNESS);
            }

            decoder >> length;

            body_length += body_align + 4;

            // Align submessage to 4.
            body_align = static_cast<uint32_t>(decoder.alignment((decoder.get_current_position() + length) -
                    decoder.get_buffer_pointer(), sizeof(int32_t)));

            body_length += length;
            decoder.jump(length + body_align);

            decoder >> id;
        }

        if (id != SRTPS_POSTFIX)
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Not valid SecureDataTag submessage id");
            return false;
        }

        decoder >> flags;

        if (flags & BIT(0))
        {
            decoder.change_endianness(eprosima::fastcdr::Cdr::Endianness::LITTLE_ENDIANNESS);
        }
        else
        {
            decoder.change_endianness(eprosima::fastcdr::Cdr::Endianness::BIG_ENDIANNESS);
        }

        decoder >> length;
        const char* const current_position = decoder.get_current_position();

        SecurityException exception;

        if (!deserialize_SecureDataTag(decoder, tag,
                sending_participant->RemoteParticipant2ParticipantKeyMaterial.at(0).transformation_kind,
                sending_participant->RemoteParticipant2ParticipantKeyMaterial.at(0).receiver_specific_key_id,
                sending_participant->RemoteParticipant2ParticipantKeyMaterial.at(0).master_receiver_specific_key,
                sending_participant->RemoteParticipant2ParticipantKeyMaterial.at(0).master_salt,
                initialization_vector, session_id, exception))
        {
            return false;
        }

        if (length != (uint16_t)(decoder.get_current_position() - current_position))
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Invalid length for SecureDataTag");
            return false;
        }
    }
    catch (eprosima::fastcdr::exception::Exception&)
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Error in fastcdr trying to deserialize SecureDataTag length");
        return false;
    }

    uint32_t length = plain_buffer.max_size - plain_buffer.pos;
    if (!deserialize_SecureDataBody(decoder, is_encrypted ? body_state : protected_body_state, tag,
            is_encrypted ? body_length : body_length + 4,
            sending_participant->RemoteParticipant2ParticipantKeyMaterial.at(0).transformation_kind,
            session_key, initialization_vector,
            &plain_buffer.buffer[plain_buffer.pos], length))
    {
        EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "Error decoding content");
        return false;
    }

    plain_buffer.length += length;
    return true;
}

bool AESGCMGMAC_Transform::preprocess_secure_submsg(
        DatawriterCryptoHandle** datawriter_crypto,
        DatareaderCryptoHandle** datareader_crypto,
        SecureSubmessageCategory_t& secure_submessage_category,
        const CDRMessage_t& encoded_rtps_submessage,
        ParticipantCryptoHandle& receiving_crypto,
        ParticipantCryptoHandle& sending_crypto,
        SecurityException& exception)
{

    AESGCMGMAC_ParticipantCryptoHandle& remote_participant = AESGCMGMAC_ParticipantCryptoHandle::narrow(sending_crypto);
    if (remote_participant.nil())
    {
        EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "Invalid CryptoHandle");
        exception = SecurityException("Not a valid ParticipantCryptoHandle received");
        return false;
    }

    AESGCMGMAC_ParticipantCryptoHandle& local_participant =
            AESGCMGMAC_ParticipantCryptoHandle::narrow(receiving_crypto);
    if (local_participant.nil())
    {
        EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "Invalid CryptoHandle");
        exception = SecurityException("Not a valid ParticipantCryptoHandle received");
        return false;
    }

    eprosima::fastcdr::FastBuffer input_buffer((char*)&encoded_rtps_submessage.buffer[encoded_rtps_submessage.pos],
            encoded_rtps_submessage.length - encoded_rtps_submessage.pos);
    eprosima::fastcdr::Cdr decoder(input_buffer);

    SecureDataHeader header;

    try
    {
        uint8_t id = 0, flags = 0;
        uint16_t length = 0;

        decoder >> id;

        if (id != SEC_PREFIX)
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Not valid SecureDataHeader submessage id");
            return false;
        }

        decoder >> flags;

        if (flags & BIT(0))
        {
            decoder.change_endianness(eprosima::fastcdr::Cdr::Endianness::LITTLE_ENDIANNESS);
        }
        else
        {
            decoder.change_endianness(eprosima::fastcdr::Cdr::Endianness::BIG_ENDIANNESS);
        }

        decoder >> length;
        const char* const current_position = decoder.get_current_position();

        header = deserialize_SecureDataHeader(decoder);

        if (length != (uint16_t)(decoder.get_current_position() - current_position))
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Invalid length for SecureDataHeader");
            return false;
        }
    }
    catch (eprosima::fastcdr::exception::Exception&)
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Error in fastcdr trying to deserialize SecureDataHeader");
        return false;
    }

    bool is_key_id_zero = (header.transform_identifier.transformation_key_id == c_transformKeyIdZero);
    auto& key_id = header.transform_identifier.transformation_key_id;

    //TODO(Ricardo) Deserializing header two times, here preprocessing and decoding submessage.
    //KeyId is present in Header->transform_identifier->transformation_key_id and contains the sender_key_id

    for (auto& wt_sp : remote_participant->Writers)
    {
        AESGCMGMAC_WriterCryptoHandle& writer = AESGCMGMAC_WriterCryptoHandle::narrow(*wt_sp);
        auto& wKeyMats = writer->Entity2RemoteKeyMaterial;

        if (wKeyMats.size() == 0)
        {
            EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "No key material yet");
            continue;
        }

        if (wKeyMats.at(0).sender_key_id == key_id)
        {
            // Remote writer found
            secure_submessage_category = DATAWRITER_SUBMESSAGE;
            *datawriter_crypto = wt_sp.get();

            //We have the remote writer, now lets look for the local datareader
            bool found = lookup_reader(local_participant, datareader_crypto, key_id);

            if (found)
            {
                return true;
            }
            // Datareader not found locally. Look remotely (Discovery case)
            else if (is_key_id_zero)
            {
                found = lookup_reader(remote_participant, datareader_crypto, key_id);
                if (found)
                {
                    return true;
                }
            }
        } //Remote writer key found
    } //For each datawriter present in the remote participant

    for (auto& rd_sh : remote_participant->Readers)
    {
        AESGCMGMAC_ReaderCryptoHandle& reader = AESGCMGMAC_ReaderCryptoHandle::narrow(*rd_sh);

        auto& rKeyMats = reader->Entity2RemoteKeyMaterial;

        if (rKeyMats.size() == 0)
        {
            EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "No key material yet");
            continue;
        }

        if (rKeyMats.at(0).sender_key_id == key_id)
        {
            // Remote reader found
            secure_submessage_category = DATAREADER_SUBMESSAGE;
            *datareader_crypto = rd_sh.get();

            //We have the remote reader, now lets look for the local datawriter
            bool found = lookup_writer(local_participant, datawriter_crypto, key_id);

            if (found)
            {
                return true;
            }
            // Datawriter not found locally. Look remotely (Discovery case)
            else if (is_key_id_zero)
            {
                found = lookup_writer(remote_participant, datawriter_crypto, key_id);
                if (found)
                {
                    return true;
                }
            }
        } //Remote reader key found
    } //For each datareader present in the remote participant

    // EPROSIMA_LOG_WARNING(SECURITY_CRYPTO,"Unable to determine the nature of the message");
    return false;
}

bool AESGCMGMAC_Transform::decode_datawriter_submessage(
        CDRMessage_t& plain_rtps_submessage,
        CDRMessage_t& encoded_rtps_submessage,
        DatareaderCryptoHandle& /*receiving_datareader_crypto*/,
        DatawriterCryptoHandle& sending_datawriter_cryupto,
        SecurityException& /*exception*/)
{
    AESGCMGMAC_WriterCryptoHandle& sending_writer = AESGCMGMAC_WriterCryptoHandle::narrow(sending_datawriter_cryupto);

    if (sending_writer.nil())
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Invalid sending_writer handle");
        return false;
    }

    if (sending_writer->Entity2RemoteKeyMaterial.size() == 0)
    {
        EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "No key material yet");
        return false;
    }

    if (encoded_rtps_submessage.length - encoded_rtps_submessage.pos >
            static_cast<uint32_t>(std::numeric_limits<int>::max()))
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Encoded rtps submessage too large");
        return false;
    }

    eprosima::fastcdr::FastBuffer input_buffer((char*)&encoded_rtps_submessage.buffer[encoded_rtps_submessage.pos],
            encoded_rtps_submessage.length - encoded_rtps_submessage.pos);
    eprosima::fastcdr::Cdr decoder(input_buffer);

    //Fun reverse order process;
    SecureDataHeader header;
    SecureDataTag tag;

    try
    {
        uint8_t id = 0, flags = 0;
        uint16_t length = 0;

        decoder >> id;

        if (id != SEC_PREFIX)
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Not valid SecureDataHeader submessage id");
            return false;
        }

        decoder >> flags;

        if (flags & BIT(0))
        {
            decoder.change_endianness(eprosima::fastcdr::Cdr::Endianness::LITTLE_ENDIANNESS);
        }
        else
        {
            decoder.change_endianness(eprosima::fastcdr::Cdr::Endianness::BIG_ENDIANNESS);
        }

        decoder >> length;
        const char* const current_position = decoder.get_current_position();

        header = deserialize_SecureDataHeader(decoder);

        if (length != (uint16_t)(decoder.get_current_position() - current_position))
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Invalid length for SecureDataHeader");
            return false;
        }
    }
    catch (eprosima::fastcdr::exception::Exception&)
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Error in fastcdr trying to deserialize SecureDataHeader");
        return false;
    }

    auto keyMat = find_key(sending_writer->Entity2RemoteKeyMaterial, header.transform_identifier);
    if (keyMat == nullptr)
    {
        EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "Key material not found");
        return false;
    }

    uint32_t session_id;
    memcpy(&session_id, header.session_id.data(), 4);
    //Sessionkey
    std::array<uint8_t, 32> session_key{};
    compute_sessionkey(session_key, *keyMat, session_id);
    //IV
    std::array<uint8_t, 12> initialization_vector{};
    memcpy(initialization_vector.data(), header.session_id.data(), 4);
    memcpy(initialization_vector.data() + 4,
            header.initialization_vector_suffix.data(), initialization_vector_suffix_length);

    // Body
    uint32_t body_length = 0, body_align = 0;
    eprosima::fastcdr::Cdr::state protected_body_state = decoder.get_state();
    bool is_encrypted = false;

    try
    {
        is_encrypted = predeserialize_SecureDataBody(decoder, body_length, body_align);
    }
    catch (eprosima::fastcdr::exception::Exception&)
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Error in fastcdr trying to deserialize SecureDataBody header");
        return false;
    }

    eprosima::fastcdr::Cdr::state body_state = decoder.get_state();
    decoder.jump(body_length + body_align);

    // Tag
    try
    {
        uint8_t id = 0, flags = 0;
        uint16_t length = 0;

        decoder >> id;

        if (id != SEC_POSTFIX)
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Not valid SecureDataTag submessage id");
            return false;
        }

        decoder >> flags;

        if (flags & BIT(0))
        {
            decoder.change_endianness(eprosima::fastcdr::Cdr::Endianness::LITTLE_ENDIANNESS);
        }
        else
        {
            decoder.change_endianness(eprosima::fastcdr::Cdr::Endianness::BIG_ENDIANNESS);
        }

        decoder >> length;
        const char* const current_position = decoder.get_current_position();

        SecurityException exception;

        if (!deserialize_SecureDataTag(decoder, tag, keyMat->transformation_kind,
                keyMat->receiver_specific_key_id,
                keyMat->master_receiver_specific_key,
                keyMat->master_salt,
                initialization_vector, session_id, exception))
        {
            return false;
        }

        if (length != (uint16_t)(decoder.get_current_position() - current_position))
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Invalid length for SecureDataTag");
            return false;
        }
    }
    catch (eprosima::fastcdr::exception::Exception&)
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Error in fastcdr trying to deserialize SecureDataTag length");
        return false;
    }

    uint32_t length = plain_rtps_submessage.max_size - plain_rtps_submessage.pos;
    if (!deserialize_SecureDataBody(decoder, is_encrypted ? body_state : protected_body_state, tag,
            is_encrypted ? body_length : body_length + 4,
            keyMat->transformation_kind, session_key, initialization_vector,
            &plain_rtps_submessage.buffer[plain_rtps_submessage.pos], length))
    {
        EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "Error decoding content");
        return false;
    }

    plain_rtps_submessage.length += length;
    encoded_rtps_submessage.pos += static_cast<uint32_t>(decoder.get_serialized_data_length());

    return true;
}

bool AESGCMGMAC_Transform::decode_datareader_submessage(
        CDRMessage_t& plain_rtps_submessage,
        CDRMessage_t& encoded_rtps_submessage,
        DatawriterCryptoHandle& /*receiving_datawriter_crypto*/,
        DatareaderCryptoHandle& sending_datareader_crypto,
        SecurityException& /*exception*/)
{
    AESGCMGMAC_ReaderCryptoHandle& sending_reader = AESGCMGMAC_ReaderCryptoHandle::narrow(sending_datareader_crypto);

    if (sending_reader.nil())
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Invalid sending_reader handle");
        return false;
    }

    if (sending_reader->Entity2RemoteKeyMaterial.size() == 0)
    {
        EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "No key material yet");
        return false;
    }

    if (encoded_rtps_submessage.length - encoded_rtps_submessage.pos >
            static_cast<uint32_t>(std::numeric_limits<int>::max()))
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Encoded rtps submessage too large");
        return false;
    }

    eprosima::fastcdr::FastBuffer input_buffer((char*)&encoded_rtps_submessage.buffer[encoded_rtps_submessage.pos],
            encoded_rtps_submessage.length - encoded_rtps_submessage.pos);
    eprosima::fastcdr::Cdr decoder(input_buffer);

    //Fun reverse order process;
    SecureDataHeader header;
    SecureDataTag tag;

    try
    {
        uint8_t id = 0, flags = 0;
        uint16_t length = 0;

        decoder >> id;

        if (id != SEC_PREFIX)
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Not valid SecureDataHeader submessage id");
            return false;
        }

        decoder >> flags;

        if (flags & BIT(0))
        {
            decoder.change_endianness(eprosima::fastcdr::Cdr::Endianness::LITTLE_ENDIANNESS);
        }
        else
        {
            decoder.change_endianness(eprosima::fastcdr::Cdr::Endianness::BIG_ENDIANNESS);
        }

        decoder >> length;
        const char* const current_position = decoder.get_current_position();

        header = deserialize_SecureDataHeader(decoder);

        if (length != (uint16_t)(decoder.get_current_position() - current_position))
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Invalid length for SecureDataHeader");
            return false;
        }
    }
    catch (eprosima::fastcdr::exception::Exception&)
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Error in fastcdr trying to deserialize SecureDataHeader");
        return false;
    }

    auto keyMat = find_key(sending_reader->Entity2RemoteKeyMaterial, header.transform_identifier);
    if (keyMat == nullptr)
    {
        EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "Could not find key material");
        return false;
    }

    uint32_t session_id;
    memcpy(&session_id, header.session_id.data(), 4);
    //Sessionkey
    std::array<uint8_t, 32> session_key{};
    compute_sessionkey(session_key, *keyMat, session_id);
    //IV
    std::array<uint8_t, 12> initialization_vector{};
    memcpy(initialization_vector.data(), header.session_id.data(), 4);
    memcpy(initialization_vector.data() + 4,
            header.initialization_vector_suffix.data(), initialization_vector_suffix_length);

    // Body
    uint32_t body_length = 0, body_align = 0;
    eprosima::fastcdr::Cdr::state protected_body_state = decoder.get_state();
    bool is_encrypted = false;

    try
    {
        is_encrypted = predeserialize_SecureDataBody(decoder, body_length, body_align);
    }
    catch (eprosima::fastcdr::exception::Exception&)
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Error in fastcdr trying to deserialize SecureDataBody header");
        return false;
    }

    eprosima::fastcdr::Cdr::state body_state = decoder.get_state();
    decoder.jump(body_length + body_align);

    // Tag
    try
    {
        uint8_t id = 0, flags = 0;
        uint16_t length = 0;

        decoder >> id;

        if (id != SEC_POSTFIX)
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Not valid SecureDataTag submessage id");
            return false;
        }

        decoder >> flags;

        if (flags & BIT(0))
        {
            decoder.change_endianness(eprosima::fastcdr::Cdr::Endianness::LITTLE_ENDIANNESS);
        }
        else
        {
            decoder.change_endianness(eprosima::fastcdr::Cdr::Endianness::BIG_ENDIANNESS);
        }

        decoder >> length;
        const char* const current_position = decoder.get_current_position();

        SecurityException exception;

        if (!deserialize_SecureDataTag(decoder, tag, keyMat->transformation_kind,
                keyMat->receiver_specific_key_id,
                keyMat->master_receiver_specific_key,
                keyMat->master_salt,
                initialization_vector, session_id, exception))
        {
            return false;
        }

        if (length != (uint16_t)(decoder.get_current_position() - current_position))
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Invalid length for SecureDataTag");
            return false;
        }
    }
    catch (eprosima::fastcdr::exception::Exception&)
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Error in fastcdr trying to deserialize SecureDataTag length");
        return false;
    }

    uint32_t length = plain_rtps_submessage.max_size - plain_rtps_submessage.pos;
    if (!deserialize_SecureDataBody(decoder, is_encrypted ? body_state : protected_body_state, tag,
            is_encrypted ? body_length : body_length + 4,
            keyMat->transformation_kind, session_key, initialization_vector,
            &plain_rtps_submessage.buffer[plain_rtps_submessage.pos], length))
    {
        EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "Error decoding content");
        return false;
    }

    plain_rtps_submessage.length += static_cast<uint32_t>(length);
    encoded_rtps_submessage.pos += static_cast<uint32_t>(decoder.get_serialized_data_length());

    return true;
}

bool AESGCMGMAC_Transform::decode_serialized_payload(
        SerializedPayload_t& plain_payload,
        const SerializedPayload_t& encoded_payload,
        const std::vector<uint8_t>& /*inline_qos*/,
        DatareaderCryptoHandle& /*receiving_datareader_crypto*/,
        DatawriterCryptoHandle& sending_datawriter_crypto,
        SecurityException& exception)
{

    AESGCMGMAC_WriterCryptoHandle& sending_writer = AESGCMGMAC_WriterCryptoHandle::narrow(sending_datawriter_crypto);

    if (sending_writer.nil())
    {
        exception = SecurityException("Not a valid sending_writer handle");
        return false;
    }

    if (sending_writer->Entity2RemoteKeyMaterial.size() == 0)
    {
        EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "No key material yet");
        return false;
    }

    if (encoded_payload.length > static_cast<uint32_t>(std::numeric_limits<int>::max()))
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Encoded payload too large");
        return false;
    }

    if (encoded_payload.length == 0)
    {
        plain_payload.length = 0;
        return true;
    }

    eprosima::fastcdr::FastBuffer input_buffer((char*)encoded_payload.data, encoded_payload.max_size);
    eprosima::fastcdr::Cdr decoder(input_buffer);

    SecureDataHeader header;
    SecureDataTag tag;

    //Header
    try
    {
        header = deserialize_SecureDataHeader(decoder);
    }
    catch (eprosima::fastcdr::exception::Exception&)
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Error in fastcdr trying to deserialize SecureDataHeader");
        return false;
    }

    auto keyMat = find_key(sending_writer->Entity2RemoteKeyMaterial, header.transform_identifier);
    if (keyMat == nullptr)
    {
        EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "Key material not found");
        return false;
    }

    uint32_t session_id;
    memcpy(&session_id, header.session_id.data(), 4);

    //Sessionkey
    std::array<uint8_t, 32> session_key{};
    compute_sessionkey(session_key, *keyMat, session_id);
    //IV
    std::array<uint8_t, 12> initialization_vector{};
    memcpy(initialization_vector.data(), header.session_id.data(), 4);
    memcpy(initialization_vector.data() + 4,
            header.initialization_vector_suffix.data(), initialization_vector_suffix_length);

    // Body
    uint32_t body_length = 0, body_align = 0;
    eprosima::fastcdr::Cdr::state protected_body_state = decoder.get_state();
    bool is_encrypted = false;

    try
    {
        is_encrypted =
                (header.transform_identifier.transformation_kind == c_transfrom_kind_aes128_gcm) ||
                (header.transform_identifier.transformation_kind == c_transfrom_kind_aes256_gcm);
        if (is_encrypted)
        {
            decoder.deserialize(body_length, eprosima::fastcdr::Cdr::Endianness::BIG_ENDIANNESS);
        }
        else
        {
            body_length = encoded_payload.length;
            body_length -= sizeof(header);
            // TODO: consider origin authentication case
            body_length -= sizeof(uint32_t) + 16;
        }
    }
    catch (eprosima::fastcdr::exception::Exception&)
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Error in fastcdr trying to deserialize SecureDataBody header");
        return false;
    }

    decoder.jump(body_length + body_align);

    // Tag
    try
    {
        deserialize_SecureDataTag(decoder, tag, {}, {}, {}, {}, {}, 0, exception);
    }
    catch (eprosima::fastcdr::exception::Exception&)
    {
        EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Error in fastcdr trying to deserialize SecureDataTag length");
        return false;
    }

    uint32_t length = plain_payload.max_size;
    if (!deserialize_SecureDataBody(decoder, protected_body_state, tag, body_length,
            keyMat->transformation_kind, session_key, initialization_vector,
            plain_payload.data, length))
    {
        EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "Error decoding content");
        return false;
    }

    plain_payload.length = length;
    plain_payload.encapsulation = encoded_payload.encapsulation;

    return true;
}

void AESGCMGMAC_Transform::compute_sessionkey(
        std::array<uint8_t, 32>& session_key,
        const KeyMaterial_AES_GCM_GMAC& key_mat,
        const uint32_t session_id)
{
    bool use_256_bits = (key_mat.transformation_kind == c_transfrom_kind_aes256_gcm ||
            key_mat.transformation_kind == c_transfrom_kind_aes256_gmac);
    int key_len = use_256_bits ? 32 : 16;

    compute_sessionkey(session_key, false, key_mat.master_sender_key, key_mat.master_salt, session_id, key_len);
}

void AESGCMGMAC_Transform::compute_sessionkey(
        std::array<uint8_t, 32>& session_key,
        bool receiver_specific,
        const std::array<uint8_t, 32>& master_key,
        const std::array<uint8_t, 32>& master_salt,
        const uint32_t session_id,
        int key_len)
{
    session_key.fill(0);

    int sourceLen = 0;
    unsigned char source[18 + 32 + 4];
    const char seq[] = "SessionKey";
    const char receiver_seq[] = "SessionReceiverKey";
    if (receiver_specific)
    {
        memcpy(source, receiver_seq, 18);
        sourceLen = 18;
    }
    else
    {
        memcpy(source, seq, 10);
        sourceLen = 10;
    }
    memcpy(source + sourceLen, master_salt.data(), key_len);
    sourceLen += key_len;
    memcpy(source + sourceLen, &session_id, 4);
    sourceLen += 4;

    EVP_PKEY* key = EVP_PKEY_new_mac_key(EVP_PKEY_HMAC, NULL, master_key.data(), key_len);
    EVP_MD_CTX* ctx =
#if IS_OPENSSL_1_1
            EVP_MD_CTX_new();
#else
            (EVP_MD_CTX*)malloc(sizeof(EVP_MD_CTX));
#endif // if IS_OPENSSL_1_1
    EVP_MD_CTX_init(ctx);
    EVP_DigestSignInit(ctx, NULL, EVP_sha256(), NULL, key);
    EVP_DigestSignUpdate(ctx, source, sourceLen);

    size_t finalLen;
    EVP_DigestSignFinal(ctx, NULL, &finalLen);
    EVP_DigestSignFinal(ctx, session_key.data(), &finalLen);

    EVP_PKEY_free(key);
#if IS_OPENSSL_1_1
    EVP_MD_CTX_free(ctx);
#else
    EVP_MD_CTX_cleanup(ctx);
    free(ctx);
#endif // if IS_OPENSSL_1_1
}

void AESGCMGMAC_Transform::serialize_SecureDataHeader(
        eprosima::fastcdr::Cdr& serializer,
        const CryptoTransformKind& transformation_kind,
        const CryptoTransformKeyId& transformation_key_id,
        const std::array<uint8_t, 4>& session_id,
        const std::array<uint8_t, 8>& initialization_vector_suffix)
{
    serializer << transformation_kind << transformation_key_id << session_id << initialization_vector_suffix;
}

bool AESGCMGMAC_Transform::serialize_SecureDataBody(
        eprosima::fastcdr::Cdr& serializer,
        const std::array<uint8_t, 4>& transformation_kind,
        const std::array<uint8_t, 32>& session_key,
        const std::array<uint8_t, 12>& initialization_vector,
        eprosima::fastcdr::FastBuffer& output_buffer,
        octet* plain_buffer,
        uint32_t plain_buffer_len,
        SecureDataTag& tag,
        bool submessage)
{
    bool do_encryption = (transformation_kind == c_transfrom_kind_aes128_gcm ||
            transformation_kind == c_transfrom_kind_aes256_gcm);
    bool use_256_bits = (transformation_kind == c_transfrom_kind_aes256_gcm ||
            transformation_kind == c_transfrom_kind_aes256_gmac);

    // AES_BLOCK_SIZE = 16
    int cipher_block_size = 0, actual_size = 0, final_size = 0;
    EVP_CIPHER_CTX* e_ctx = EVP_CIPHER_CTX_new();

    if (!use_256_bits)
    {
        if (!EVP_EncryptInit(e_ctx, EVP_aes_128_gcm(), (const unsigned char*)(session_key.data()),
                initialization_vector.data()))
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO,
                    "Unable to encode the payload. EVP_EncryptInit function returns an error");
            EVP_CIPHER_CTX_free(e_ctx);
            return false;
        }

        cipher_block_size = EVP_CIPHER_block_size(EVP_aes_128_gcm());
    }
    else
    {
        if (!EVP_EncryptInit(e_ctx, EVP_aes_256_gcm(), (const unsigned char*)(session_key.data()),
                initialization_vector.data()))
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO,
                    "Unable to encode the payload. EVP_EncryptInit function returns an error");
            EVP_CIPHER_CTX_free(e_ctx);
            return false;
        }

        cipher_block_size = EVP_CIPHER_block_size(EVP_aes_256_gcm());
    }

    if (!do_encryption)
    {
        // Auth only. SEC_BODY should not be created. Plain buffer should be copied instead.
        if ((output_buffer.getBufferSize() - (serializer.get_current_position() - serializer.get_buffer_pointer())) <
                plain_buffer_len)
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Error in fastcdr trying to copy payload");
            EVP_CIPHER_CTX_free(e_ctx);
            return false;
        }
        memcpy(serializer.get_current_position(), plain_buffer, plain_buffer_len);
        serializer.jump(plain_buffer_len);

        if (!EVP_EncryptUpdate(e_ctx, nullptr, &actual_size, plain_buffer, static_cast<int>(plain_buffer_len)))
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO,
                    "Unable to encode the payload. EVP_EncryptUpdate function returns an error");
            EVP_CIPHER_CTX_free(e_ctx);
            return false;
        }

        if (!EVP_EncryptFinal(e_ctx, nullptr, &final_size))
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO,
                    "Unable to encode the payload. EVP_EncryptFinal function returns an error");
            EVP_CIPHER_CTX_free(e_ctx);
            return false;
        }
    }
    else
    {
#if FASTDDS_IS_BIG_ENDIAN_TARGET
        octet flags = 0x0;
        serializer.change_endianness(eprosima::fastcdr::Cdr::Endianness::BIG_ENDIANNESS);
#else
        octet flags = BIT(0);
        serializer.change_endianness(eprosima::fastcdr::Cdr::Endianness::LITTLE_ENDIANNESS);
#endif // if FASTDDS_IS_BIG_ENDIAN_TARGET

        if (submessage)
        {
            serializer << SecureBodySubmessage << flags;
        }

        // Store current state to serialize sequence length at the end of the function
        eprosima::fastcdr::Cdr::state sequence_length_state = serializer.get_state();

        if (submessage)
        {
            // Serialize dummy length
            uint16_t length = 0;
            serializer << length;
        }

        // Serialize dummy content length
        uint32_t cnt_length = 0;
        serializer << cnt_length;

        //Cypher the plain rtps message -> SecureDataBody

        unsigned char* output_buffer_raw = (unsigned char*)serializer.get_current_position();

        // Check output_buffer contains enough memory to cypher.
        // - EVP_EncryptUpdate needs at maximum: plain_buffer_len + cipher_block_size - 1.
        // - EVP_EncryptFinal needs ad maximum cipher_block_size.
        if ((output_buffer.getBufferSize() - (serializer.get_current_position() - serializer.get_buffer_pointer())) <
                (plain_buffer_len + (2 * cipher_block_size) - 1))
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Error in fastcdr trying to cipher payload");
            EVP_CIPHER_CTX_free(e_ctx);
            return false;
        }

        if (!EVP_EncryptUpdate(e_ctx, output_buffer_raw, &actual_size, plain_buffer,
                static_cast<int>(plain_buffer_len)))
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO,
                    "Unable to encode the payload. EVP_EncryptUpdate function returns an error");
            EVP_CIPHER_CTX_free(e_ctx);
            return false;
        }

        if (!EVP_EncryptFinal(e_ctx, &output_buffer_raw[actual_size], &final_size))
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO,
                    "Unable to encode the payload. EVP_EncryptFinal function returns an error");
            EVP_CIPHER_CTX_free(e_ctx);
            return false;
        }

        serializer.jump(actual_size + final_size);

        eprosima::fastcdr::Cdr::state current_state = serializer.get_state();

        // Serialize body sequence length;
        cnt_length = static_cast<uint32_t>(actual_size + final_size);
        serializer.set_state(sequence_length_state);
        if (submessage)
        {
            uint16_t length = static_cast<uint16_t>(actual_size + final_size + sizeof(uint32_t));
            length = (length + 3) & ~3;
            serializer << length;
        }

        serializer.serialize(cnt_length, eprosima::fastcdr::Cdr::Endianness::BIG_ENDIANNESS);

        serializer.set_state(current_state);

    }

    // Get commmon_mac
    EVP_CIPHER_CTX_ctrl(e_ctx, EVP_CTRL_GCM_GET_TAG, AES_BLOCK_SIZE, tag.common_mac.data());
    EVP_CIPHER_CTX_free(e_ctx);

    if (submessage)
    {
        // Align submessage to 4.
        size_t alignment =
                serializer.alignment(serializer.get_current_position() - serializer.get_buffer_pointer(),
                        sizeof(int32_t));
        for (size_t count = 0; count != alignment; ++count)
        {
            uint8_t c = 0;
            serializer << c;
        }
    }

    return true;
}

bool AESGCMGMAC_Transform::serialize_SecureDataTag(
        eprosima::fastcdr::Cdr& serializer,
        const std::array<uint8_t, 4>& transformation_kind,
        const uint32_t session_id,
        const std::array<uint8_t, 12>& initialization_vector,
        std::vector<std::shared_ptr<ParticipantCryptoHandle>>& receiving_crypto_list,
        bool update_specific_keys,
        SecureDataTag& tag,
        size_t sessionIndex)
{
    bool use_256_bits = (transformation_kind == c_transfrom_kind_aes256_gcm ||
            transformation_kind == c_transfrom_kind_aes256_gmac);
    int key_len = use_256_bits ? 32 : 16;

    serializer << tag.common_mac;

    // Align to 4.
    size_t alignment =
            serializer.alignment(serializer.get_current_position() - serializer.get_buffer_pointer(), sizeof(int32_t));
    for (size_t count = 0; count != alignment; ++count)
    {
        uint8_t c = 0;
        serializer << c;
    }

    eprosima::fastcdr::Cdr::state length_state = serializer.get_state();
    uint32_t length = 0;
    serializer << length;

    //Check the list of receivers, search for keys and compute session keys as needed
    for (auto rec = receiving_crypto_list.begin(); rec != receiving_crypto_list.end(); ++rec)
    {
        AESGCMGMAC_EntityCryptoHandle& remote_entity = AESGCMGMAC_ReaderCryptoHandle::narrow(**rec);

        if (remote_entity.nil())
        {
            EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "Invalid CryptoHandle");
            continue;
        }

        if (remote_entity->Remote2EntityKeyMaterial.size() == 0)
        {
            EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "No key material yet");
            continue;
        }

        auto& keyMat = remote_entity->Remote2EntityKeyMaterial.at(0);
        if (keyMat.receiver_specific_key_id == c_transformKeyIdZero)
        {
            // This means origin authentication is disabled. As it is configured on the writer, we know all other
            // receiving entities will have its specific key to null value, and we can skip the whole loop
            break;
        }

        //Update the key if needed
        if (update_specific_keys || remote_entity->Sessions[sessionIndex].session_id != session_id)
        {
            //Update triggered!
            remote_entity->Sessions[sessionIndex].session_id = session_id;
            compute_sessionkey(remote_entity->Sessions[sessionIndex].SessionKey, true,
                    keyMat.master_receiver_specific_key, keyMat.master_salt, session_id, key_len);
        }

        //Obtain MAC using ReceiverSpecificKey and the same Initialization Vector as before
        int actual_size = 0, final_size = 0;
        EVP_CIPHER_CTX* e_ctx = EVP_CIPHER_CTX_new();
        if (transformation_kind == c_transfrom_kind_aes128_gcm ||
                transformation_kind == c_transfrom_kind_aes128_gmac)
        {
            if (!EVP_EncryptInit(e_ctx, EVP_aes_128_gcm(),
                    (const unsigned char*)(remote_entity->Sessions[sessionIndex].SessionKey.data()),
                    initialization_vector.data()))
            {
                EPROSIMA_LOG_ERROR(SECURITY_CRYPTO,
                        "Unable to encode the payload. EVP_EncryptInit function returns an error");
                EVP_CIPHER_CTX_free(e_ctx);
                continue;
            }
        }
        else if (transformation_kind == c_transfrom_kind_aes256_gcm ||
                transformation_kind == c_transfrom_kind_aes256_gmac)
        {
            if (!EVP_EncryptInit(e_ctx, EVP_aes_256_gcm(),
                    (const unsigned char*)(remote_entity->Sessions[sessionIndex].SessionKey.data()),
                    initialization_vector.data()))
            {
                EPROSIMA_LOG_ERROR(SECURITY_CRYPTO,
                        "Unable to encode the payload. EVP_EncryptInit function returns an error");
                EVP_CIPHER_CTX_free(e_ctx);
                continue;
            }
        }
        if (!EVP_EncryptUpdate(e_ctx, NULL, &actual_size, tag.common_mac.data(), 16))
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO,
                    "Unable to create authentication for the datawriter submessage. EVP_EncryptUpdate function returns an error");
            EVP_CIPHER_CTX_free(e_ctx);
            continue;
        }
        if (!EVP_EncryptFinal(e_ctx, NULL, &final_size))
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO,
                    "Unable to create authentication for the datawriter submessage. EVP_EncryptFinal function returns an error");
            EVP_CIPHER_CTX_free(e_ctx);
            continue;
        }
        serializer << remote_entity->Remote2EntityKeyMaterial.at(0).receiver_specific_key_id;
        EVP_CIPHER_CTX_ctrl(e_ctx, EVP_CTRL_GCM_GET_TAG, AES_BLOCK_SIZE, serializer.get_current_position());
        serializer.jump(16);
        EVP_CIPHER_CTX_free(e_ctx);

        ++length;
    }

    eprosima::fastcdr::Cdr::state current_state = serializer.get_state();
    serializer.set_state(length_state);
    serializer.serialize(length, eprosima::fastcdr::Cdr::Endianness::BIG_ENDIANNESS);
    serializer.set_state(current_state);
    return true;
}

bool AESGCMGMAC_Transform::serialize_SecureDataTag(
        eprosima::fastcdr::Cdr& serializer,
        const AESGCMGMAC_ParticipantCryptoHandle& local_participant,
        const std::array<uint8_t, 12>& initialization_vector,
        std::vector<std::shared_ptr<ParticipantCryptoHandle>>& receiving_crypto_list,
        bool update_specific_keys,
        SecureDataTag& tag)
{
    serializer << tag.common_mac;

    eprosima::fastcdr::Cdr::state length_state = serializer.get_state();
    uint32_t length = 0;
    serializer << length;

    //Check the list of receivers, search for keys and compute session keys as needed
    for (auto rec = receiving_crypto_list.begin(); rec != receiving_crypto_list.end(); ++rec)
    {

        AESGCMGMAC_ParticipantCryptoHandle& remote_participant = AESGCMGMAC_ParticipantCryptoHandle::narrow(**rec);

        if (remote_participant.nil())
        {
            EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "Invalid CryptoHandle");
            continue;
        }

        if (remote_participant->Participant2ParticipantKeyMaterial.size() == 0)
        {
            EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "No key material yet");
            continue;
        }

        auto& keyMat = remote_participant->Participant2ParticipantKeyMaterial.at(0);
        if (keyMat.receiver_specific_key_id == c_transformKeyIdZero)
        {
            // This means origin authentication is disabled. As it is configured on the writer, we know all other
            // receiving entities will have its specific key to null value, and we can skip the whole loop
            break;
        }

        bool use_256_bits = (keyMat.transformation_kind == c_transfrom_kind_aes256_gcm ||
                keyMat.transformation_kind == c_transfrom_kind_aes256_gmac);
        int key_len = use_256_bits ? 32 : 16;

        //Update the key if needed
        if ((update_specific_keys || remote_participant->Session.session_id != local_participant->Session.session_id) &&
                (*remote_participant != *local_participant))
        {
            //Update triggered!
            remote_participant->Session.session_id = local_participant->Session.session_id;
            compute_sessionkey(remote_participant->Session.SessionKey, true,
                    keyMat.master_receiver_specific_key, keyMat.master_salt, remote_participant->Session.session_id,
                    key_len);
        }

        //Obtain MAC using ReceiverSpecificKey and the same Initialization Vector as before
        int actual_size = 0, final_size = 0;
        EVP_CIPHER_CTX* e_ctx = EVP_CIPHER_CTX_new();
        auto& trans_kind = remote_participant->Participant2ParticipantKeyMaterial.at(0).transformation_kind;
        if (trans_kind == c_transfrom_kind_aes128_gcm ||
                trans_kind == c_transfrom_kind_aes128_gmac)
        {
            if (!EVP_EncryptInit(e_ctx, EVP_aes_128_gcm(),
                    (const unsigned char*)(remote_participant->Session.SessionKey.data()),
                    initialization_vector.data()))
            {
                EPROSIMA_LOG_ERROR(SECURITY_CRYPTO,
                        "Unable to encode the payload. EVP_EncryptInit function returns an error");
                EVP_CIPHER_CTX_free(e_ctx);
                continue;
            }
        }
        else if (trans_kind == c_transfrom_kind_aes256_gcm ||
                trans_kind == c_transfrom_kind_aes256_gmac)
        {
            if (!EVP_EncryptInit(e_ctx, EVP_aes_256_gcm(),
                    (const unsigned char*)(remote_participant->Session.SessionKey.data()),
                    initialization_vector.data()))
            {
                EPROSIMA_LOG_ERROR(SECURITY_CRYPTO,
                        "Unable to encode the payload. EVP_EncryptInit function returns an error");
                EVP_CIPHER_CTX_free(e_ctx);
                continue;
            }
        }
        if (!EVP_EncryptUpdate(e_ctx, NULL, &actual_size, tag.common_mac.data(), 16))
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO,
                    "Unable to create authentication for the datawriter submessage. EVP_EncryptUpdate function returns an error");
            EVP_CIPHER_CTX_free(e_ctx);
            continue;
        }
        if (!EVP_EncryptFinal(e_ctx, NULL, &final_size))
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO,
                    "Unable to create authentication for the datawriter submessage. EVP_EncryptFinal function returns an error");
            EVP_CIPHER_CTX_free(e_ctx);
            continue;
        }
        serializer << remote_participant->Participant2ParticipantKeyMaterial.at(0).receiver_specific_key_id;
        EVP_CIPHER_CTX_ctrl(e_ctx, EVP_CTRL_GCM_GET_TAG, AES_BLOCK_SIZE, serializer.get_current_position());
        serializer.jump(16);
        EVP_CIPHER_CTX_free(e_ctx);

        ++length;
    }

    eprosima::fastcdr::Cdr::state current_state = serializer.get_state();
    serializer.set_state(length_state);
    serializer.serialize(length, eprosima::fastcdr::Cdr::Endianness::BIG_ENDIANNESS);
    serializer.set_state(current_state);
    return true;
}

SecureDataHeader AESGCMGMAC_Transform::deserialize_SecureDataHeader(
        eprosima::fastcdr::Cdr& decoder)
{
    SecureDataHeader header;

    decoder >> header.transform_identifier.transformation_kind >> header.transform_identifier.transformation_key_id >>
    header.session_id >> header.initialization_vector_suffix;

    return header;
}

bool AESGCMGMAC_Transform::deserialize_SecureDataBody(
        eprosima::fastcdr::Cdr& decoder,
        eprosima::fastcdr::Cdr::state& body_state,
        SecureDataTag& tag,
        const uint32_t body_length,
        const std::array<uint8_t, 4>& transformation_kind,
        const std::array<uint8_t, 32>& session_key,
        const std::array<uint8_t, 12>& initialization_vector,
        octet* plain_buffer,
        uint32_t& plain_buffer_len)
{
    eprosima::fastcdr::Cdr::state current_state = decoder.get_state();
    decoder.set_state(body_state);

    bool do_encryption = (transformation_kind == c_transfrom_kind_aes128_gcm ||
            transformation_kind == c_transfrom_kind_aes256_gcm);
    bool use_256_bits = (transformation_kind == c_transfrom_kind_aes256_gcm ||
            transformation_kind == c_transfrom_kind_aes256_gmac);

    EVP_CIPHER_CTX* d_ctx = EVP_CIPHER_CTX_new();
    int cipher_block_size = 0, actual_size = 0, final_size = 0;

    if (!use_256_bits)
    {
        if (!EVP_DecryptInit(d_ctx, EVP_aes_128_gcm(), (const unsigned char*)session_key.data(),
                initialization_vector.data()))
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO,
                    "Unable to decode the payload. EVP_DecryptInit function returns an error");
            EVP_CIPHER_CTX_free(d_ctx);
            return false;
        }

        cipher_block_size = EVP_CIPHER_block_size(EVP_aes_128_gcm());
    }
    else
    {
        if (!EVP_DecryptInit(d_ctx, EVP_aes_256_gcm(), (const unsigned char*)session_key.data(),
                initialization_vector.data()))
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO,
                    "Unable to decode the payload. EVP_DecryptInit function returns an error");
            EVP_CIPHER_CTX_free(d_ctx);
            return false;
        }

        cipher_block_size = EVP_CIPHER_block_size(EVP_aes_256_gcm());
    }

    uint32_t protected_len = body_length;
    if (do_encryption)
    {
        decoder.deserialize(protected_len, eprosima::fastcdr::Cdr::Endianness::BIG_ENDIANNESS);

        // Check plain_payload contains enough memory to cypher.
        // - EVP_DecryptUpdate needs at maximum: body_length + cipher_block_size.
        if (plain_buffer_len < (protected_len + cipher_block_size))
        {
            EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "Error in fastcdr trying to decode payload");
            EVP_CIPHER_CTX_free(d_ctx);
            return false;
        }
    }

    octet* output_buffer = do_encryption ? plain_buffer : nullptr;
    unsigned char* input_buffer = (unsigned char*)decoder.get_current_position();
    if (!EVP_DecryptUpdate(d_ctx, output_buffer, &actual_size, input_buffer, protected_len))
    {
        EPROSIMA_LOG_WARNING(SECURITY_CRYPTO,
                "Unable to decode the payload. EVP_DecryptUpdate function returns an error");
        EVP_CIPHER_CTX_free(d_ctx);
        return false;
    }

    EVP_CIPHER_CTX_ctrl(d_ctx, EVP_CTRL_GCM_SET_TAG, AES_BLOCK_SIZE, tag.common_mac.data());

    if (!EVP_DecryptFinal(d_ctx, output_buffer ? &output_buffer[actual_size] : NULL, &final_size))
    {
        EPROSIMA_LOG_WARNING(SECURITY_CRYPTO,
                "Unable to decode the payload. EVP_DecryptFinal function returns an error");
        EVP_CIPHER_CTX_free(d_ctx);
        return false;
    }
    EVP_CIPHER_CTX_free(d_ctx);

    uint32_t cnt_len = do_encryption ? static_cast<uint32_t>(actual_size + final_size) : body_length;
    if (plain_buffer_len < cnt_len)
    {
        EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "Error in fastcdr trying to decode payload");
        return false;
    }

    plain_buffer_len = cnt_len;
    if (output_buffer == nullptr)
    {
        memcpy(plain_buffer, input_buffer, plain_buffer_len);
    }

    decoder.set_state(current_state);

    // Align submessage to 4.
    size_t alignment =
            decoder.alignment(decoder.get_current_position() - decoder.get_buffer_pointer(), sizeof(int32_t));
    for (size_t count = 0; count != alignment; ++count)
    {
        uint8_t c = 0;
        decoder >> c;
    }

    return true;
}

bool AESGCMGMAC_Transform::predeserialize_SecureDataBody(
        eprosima::fastcdr::Cdr& decoder,
        uint32_t& body_length,
        uint32_t& body_align)
{
    octet secure_submsg_id = 0, flags = 0;
    uint16_t body_length_short;

    decoder >> secure_submsg_id;

    decoder >> flags;

    if (flags & BIT(0))
    {
        decoder.change_endianness(eprosima::fastcdr::Cdr::Endianness::LITTLE_ENDIANNESS);
    }
    else
    {
        decoder.change_endianness(eprosima::fastcdr::Cdr::Endianness::BIG_ENDIANNESS);
    }

    decoder >> body_length_short;
    body_length = body_length_short;

    // Align submessage to 4.
    body_align = static_cast<uint32_t>(decoder.alignment((decoder.get_current_position() + body_length) -
            decoder.get_buffer_pointer(), sizeof(int32_t)));

    return (secure_submsg_id == SecureBodySubmessage);
}

bool AESGCMGMAC_Transform::deserialize_SecureDataTag(
        eprosima::fastcdr::Cdr& decoder,
        SecureDataTag& tag,
        const CryptoTransformKind& transformation_kind,
        const CryptoTransformKeyId& receiver_specific_key_id,
        const std::array<uint8_t, 32>& receiver_specific_key,
        const std::array<uint8_t, 32>& master_salt,
        const std::array<uint8_t, 12>& initialization_vector,
        const uint32_t session_id,
        SecurityException& exception)
{
    decoder >> tag.common_mac;

    uint32_t sequence_length = 0;
    decoder.deserialize(sequence_length, eprosima::fastcdr::Cdr::Endianness::BIG_ENDIANNESS);

    if (sequence_length > 0)
    {
        bool mac_found = false;

        // TODO(Ricardo) Review SessionReceiverSpecificKey (248pag)
        uint32_t count = 0;
        for (; !mac_found && count < sequence_length; ++count)
        {
            decoder >> tag.receiver_mac_key_id >> tag.receiver_mac;

            if (receiver_specific_key_id == tag.receiver_mac_key_id)
            {
                mac_found = true;
            }
        }

        decoder.jump((sequence_length - count) * (tag.receiver_mac_key_id.size() + tag.receiver_mac.size()));

        if (!mac_found)
        {
            EPROSIMA_LOG_WARNING(SECURITY_CRYPTO,
                    "Unable to authenticate the message: message does not target this Participant");
            exception = SecurityException(
                "Message does not contain a suitable specific MAC for the receiving Participant");
            return false;
        }

        //Auth message - The point is that we cannot verify the authorship of the message with our receiver_specific_key the message could be crafted
        EVP_CIPHER_CTX* d_ctx = EVP_CIPHER_CTX_new();
        const EVP_CIPHER* d_cipher = nullptr;

        int actual_size = 0, final_size = 0;

        //Get ReceiverSpecificSessionKey
        std::array<uint8_t, 32> specific_session_key{};

        //Verify specific MAC
        if (transformation_kind == c_transfrom_kind_aes128_gcm ||
                transformation_kind == c_transfrom_kind_aes128_gmac)
        {
            compute_sessionkey(specific_session_key, true, receiver_specific_key, master_salt, session_id, 16);
            d_cipher = EVP_aes_128_gcm();
        }
        else if (transformation_kind == c_transfrom_kind_aes256_gcm ||
                transformation_kind == c_transfrom_kind_aes256_gmac)
        {
            compute_sessionkey(specific_session_key, true, receiver_specific_key, master_salt, session_id, 32);
            d_cipher = EVP_aes_256_gcm();
        }
        else
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO, "Invalid transformation kind)");
            EVP_CIPHER_CTX_free(d_ctx);
            return false;
        }

        if (!EVP_DecryptInit(d_ctx, d_cipher, (const unsigned char*)specific_session_key.data(),
                initialization_vector.data()))
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO,
                    "Unable to authenticate the message. EVP_DecryptInit function returns an error");
            EVP_CIPHER_CTX_free(d_ctx);
            return false;
        }

        if (!EVP_DecryptUpdate(d_ctx, NULL, &actual_size, tag.common_mac.data(), 16))
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO,
                    "Unable to authenticate the message. EVP_DecryptUpdate function returns an error");
            EVP_CIPHER_CTX_free(d_ctx);
            return false;
        }

        if (!EVP_CIPHER_CTX_ctrl(d_ctx, EVP_CTRL_GCM_SET_TAG, AES_BLOCK_SIZE, tag.receiver_mac.data()))
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO,
                    "Unable to authenticate the message. EVP_CIPHER_CTX_ctrl function returns an error");
            EVP_CIPHER_CTX_free(d_ctx);
            return false;
        }

        if (!EVP_DecryptFinal_ex(d_ctx, NULL, &final_size))
        {
            EPROSIMA_LOG_ERROR(SECURITY_CRYPTO,
                    "Unable to authenticate the message. EVP_DecryptFinal_ex function returns an error");
            EVP_CIPHER_CTX_free(d_ctx);
            return false;
        }

        EVP_CIPHER_CTX_free(d_ctx);
    }

    return true;
}

constexpr uint32_t srtps_prefix_length = 4;
// 4 bytes to serialize length of the body.
constexpr uint32_t srtps_postfix_length = 4;
constexpr uint32_t sec_prefix_length = 4;
// 4 bytes to serialize length of the body.
constexpr uint32_t sec_postfix_length = 4;
constexpr uint32_t aesgcmgmac_header_length = 20;
constexpr uint32_t aesgcmgmac_body_length_attr = 4 + 3 /*possible alignment*/;
constexpr uint32_t aesgcmgmac_common_tag = 16;

uint32_t AESGCMGMAC_Transform::calculate_extra_size_for_rtps_message(
        uint32_t number_discovered_participants) const
{
    uint32_t calculate = srtps_prefix_length  +
            aesgcmgmac_header_length +
            aesgcmgmac_body_length_attr +
            AES_BLOCK_SIZE + // Padding
            srtps_postfix_length +
            aesgcmgmac_common_tag;

    // Minimum like there is 10 participants.
    calculate += number_discovered_participants > 10 ? number_discovered_participants * 20 : 200;

    return calculate;
}

uint32_t AESGCMGMAC_Transform::calculate_extra_size_for_rtps_submessage(
        uint32_t number_discovered_readers) const
{
    uint32_t calculate = sec_prefix_length  +
            aesgcmgmac_header_length +
            aesgcmgmac_body_length_attr +
            AES_BLOCK_SIZE + // Padding
            sec_postfix_length +
            aesgcmgmac_common_tag;

    // Minimum like there is 10 participants.
    calculate += number_discovered_readers > 10 ? number_discovered_readers * 20 : 200;

    return calculate;
}

uint32_t AESGCMGMAC_Transform::calculate_extra_size_for_encoded_payload(
        uint32_t number_discovered_readers) const
{
    uint32_t calculate = aesgcmgmac_header_length +
            aesgcmgmac_body_length_attr +
            AES_BLOCK_SIZE + // Padding
            aesgcmgmac_common_tag;

    // Minimum like there is 10 participants.
    calculate += number_discovered_readers > 10 ? number_discovered_readers * 20 : 200;

    return calculate;
}

bool AESGCMGMAC_Transform::lookup_reader(
        AESGCMGMAC_ParticipantCryptoHandle& participant,
        DatareaderCryptoHandle** datareader_crypto,
        CryptoTransformKeyId key_id)
{
    for (auto& readerHandle : participant->Readers)
    {
        AESGCMGMAC_ReaderCryptoHandle& reader = AESGCMGMAC_ReaderCryptoHandle::narrow(*readerHandle);

        if (reader->Remote2EntityKeyMaterial.empty())
        {
            EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "No key material yet");
            continue;
        }

        for (const KeyMaterial_AES_GCM_GMAC& elem : reader->Remote2EntityKeyMaterial)
        {
            if (elem.sender_key_id == key_id)
            {
                *datareader_crypto = readerHandle.get();
                return true;
            }
        }   //For each Reader2WriterKeyMaterial in the datareader
    } //For each datareader present in the participant

    return false;
}

bool AESGCMGMAC_Transform::lookup_writer(
        AESGCMGMAC_ParticipantCryptoHandle& participant,
        DatawriterCryptoHandle** datawriter_crypto,
        CryptoTransformKeyId key_id)
{
    for (auto& writerHandle : participant->Writers)
    {
        AESGCMGMAC_WriterCryptoHandle& writer = AESGCMGMAC_WriterCryptoHandle::narrow(*writerHandle);

        if (writer->Remote2EntityKeyMaterial.empty())
        {
            EPROSIMA_LOG_WARNING(SECURITY_CRYPTO, "No key material yet");
            continue;
        }

        for (const KeyMaterial_AES_GCM_GMAC& elem : writer->Remote2EntityKeyMaterial)
        {
            if (elem.sender_key_id == key_id)
            {
                *datawriter_crypto = writerHandle.get();
                return true;
            }
        }   //For each Writer2ReaderKeyMaterial in the datawriter
    } //For each datawriter present in the participant

    return false;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
