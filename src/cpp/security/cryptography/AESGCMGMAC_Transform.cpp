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

#include <fastrtps/log/Log.h>
#include <fastrtps/rtps/messages/CDRMessage.h>

#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <cstring>

 // Solve error with Win32 macro
#ifdef WIN32
#undef max
#endif

using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::rtps::security;

CONSTEXPR int initialization_vector_suffix_length = 8;

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
    if(local_writer.nil()){
        logWarning(SECURITY_CRYPTO,"Invalid CryptoHandle");
        return false;
    }

    // Precondition to use openssl
    if(payload.length > static_cast<uint32_t>(std::numeric_limits<int>::max()))
    {
        logError(SECURITY_CRYPTO, "Plain text too large");
        return false;
    }

    eprosima::fastcdr::FastBuffer output_buffer((char*)output_payload.data, output_payload.max_size);
    eprosima::fastcdr::Cdr serializer(output_buffer);

    std::unique_lock<std::mutex> lock(local_writer->mutex_);

    //If the maximum number of blocks have been processed, generate a new SessionKey
    if(local_writer->session_block_counter >= local_writer->max_blocks_per_session)
    {
        local_writer->session_id += 1;

        local_writer->SessionKey = compute_sessionkey(local_writer->EntityKeyMaterial.master_sender_key,
                local_writer->EntityKeyMaterial.master_salt,
                local_writer->session_id);

        //ReceiverSpecific keys shall be computed specifically when needed
        local_writer->session_block_counter = 0;
    }
    //In any case, increment session block counter
    local_writer->session_block_counter += 1;

    //Build NONCE elements (Build once, use once)
    std::array<uint8_t, initialization_vector_suffix_length> initialization_vector_suffix;  //iv suffix changes with every operation
    RAND_bytes(initialization_vector_suffix.data(), initialization_vector_suffix_length);
    std::array<uint8_t, 12> initialization_vector; //96 bytes, session_id + suffix
    memcpy(initialization_vector.data(),&(local_writer->session_id),4);
    memcpy(initialization_vector.data() + 4, initialization_vector_suffix.data(), 8);
    std::array<uint8_t, 4> session_id;
    memcpy(session_id.data(), &(local_writer->session_id), 4);

    //Header
    try
    {
        serialize_SecureDataHeader(serializer, local_writer->EntityKeyMaterial.transformation_kind,
                local_writer->EntityKeyMaterial.sender_key_id, session_id, initialization_vector_suffix);
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException&)
    {
        logError(SECURITY_CRYPTO, "Not enough memory to serialize SecureDataHeader");
        return false;
    }

    SecureDataTag tag;

    // Body
    try
    {
        if(!serialize_SecureDataBody(serializer, local_writer->transformation_kind, local_writer->SessionKey,
                    initialization_vector, output_buffer, payload.data, payload.length, tag))
        {
            return false;
        }
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException&)
    {
        logError(SECURITY_CRYPTO, "Not enough memory to serialize SecureDataBody");
        return false;
    }

    try
    {
        std::vector<DatareaderCryptoHandle*> receiving_datareader_crypto_list;
        if(!serialize_SecureDataTag(serializer, local_writer->transformation_kind, local_writer->session_id,
                    initialization_vector, receiving_datareader_crypto_list, false, tag))
        {
            return false;
        }
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException&)
    {
        logError(SECURITY_CRYPTO, "Not enough memory to serialize SecureDataTag");
        return false;
    }

    // Store information in CDRMessage_t
    output_payload.length = static_cast<uint32_t>(serializer.getSerializedDataLength());

    return true;
}

bool AESGCMGMAC_Transform::encode_datawriter_submessage(
        CDRMessage_t& encoded_rtps_submessage,
        const CDRMessage_t& plain_rtps_submessage,
        DatawriterCryptoHandle& sending_datawriter_crypto,
        std::vector<DatareaderCryptoHandle*>& receiving_datareader_crypto_list,
        SecurityException& /*exception*/)
{
    AESGCMGMAC_WriterCryptoHandle& local_writer = AESGCMGMAC_WriterCryptoHandle::narrow(sending_datawriter_crypto);

    if(local_writer.nil()){
        logWarning(SECURITY_CRYPTO,"Invalid cryptoHandle");
        return false;
    }

    if((plain_rtps_submessage.length  - plain_rtps_submessage.pos) > static_cast<uint32_t>(std::numeric_limits<int>::max()))
    {
        logError(SECURITY_CRYPTO, "Plain rtps submessage too large");
        return false;
    }

    eprosima::fastcdr::FastBuffer output_buffer((char*)&encoded_rtps_submessage.buffer[encoded_rtps_submessage.pos],
            encoded_rtps_submessage.max_size - encoded_rtps_submessage.pos);
    eprosima::fastcdr::Cdr serializer(output_buffer);

    std::unique_lock<std::mutex> lock(local_writer->mutex_);

    bool update_specific_keys = false;
    //If the maximum number of blocks have been processed, generate a new SessionKey
    if(local_writer->session_block_counter >= local_writer->max_blocks_per_session)
    {
        local_writer->session_id += 1;
        update_specific_keys = true;
        local_writer->SessionKey = compute_sessionkey(local_writer->EntityKeyMaterial.master_sender_key,
                local_writer->EntityKeyMaterial.master_salt,
                local_writer->session_id);

        //ReceiverSpecific keys shall be computed specifically when needed
        local_writer->session_block_counter = 0;
    }

    local_writer->session_block_counter += 1;

    //Build remaining NONCE elements
    std::array<uint8_t, initialization_vector_suffix_length> initialization_vector_suffix;  //iv suffix changes with every operation
    RAND_bytes(initialization_vector_suffix.data(), initialization_vector_suffix_length);
    std::array<uint8_t,12> initialization_vector; //96 bytes, session_id + suffix
    memcpy(initialization_vector.data(),&(local_writer->session_id),4);
    memcpy(initialization_vector.data() + 4, initialization_vector_suffix.data(), 8);
    std::array<uint8_t, 4> session_id;
    memcpy(session_id.data(), &(local_writer->session_id), 4);

#if __BIG_ENDIAN__
    octet flags = 0x0;
    serializer.changeEndianness(eprosima::fastcdr::Cdr::Endianness::BIG_ENDIANNESS);
#else
    octet flags = BIT(0);
    serializer.changeEndianness(eprosima::fastcdr::Cdr::Endianness::LITTLE_ENDIANNESS);
#endif

    //Header
    try
    {
        serializer << SEC_PREFIX << flags;
        eprosima::fastcdr::Cdr::state length_state = serializer.getState();
        uint16_t length = 0;
        serializer << length;

        const char* length_position = serializer.getCurrentPosition();

        serialize_SecureDataHeader(serializer, local_writer->EntityKeyMaterial.transformation_kind,
                local_writer->EntityKeyMaterial.sender_key_id, session_id, initialization_vector_suffix);

        eprosima::fastcdr::Cdr::state current_state = serializer.getState();
        //TODO(Ricardo) fastcdr functinality: length substracting two Cdr::state.
        length =  static_cast<uint16_t>(serializer.getCurrentPosition() - length_position);
        serializer.setState(length_state);
        serializer << length;
        serializer.setState(current_state);
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException& )
    {
        logError(SECURITY_CRYPTO, "Not enough memory to serialize SecureDataHeader");
        return false;
    }

    SecureDataTag tag;

    // Body
    try
    {
        if(!serialize_SecureDataBody(serializer, local_writer->transformation_kind, local_writer->SessionKey,
                    initialization_vector, output_buffer, &plain_rtps_submessage.buffer[plain_rtps_submessage.pos],
                    plain_rtps_submessage.length - plain_rtps_submessage.pos, tag))
        {
            return false;
        }
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException&)
    {
        logError(SECURITY_CRYPTO, "Not enough memory to serialize SecureDataBody");
        return false;
    }

    // Tag
    try
    {
        serializer << SEC_POSTFIX << flags;
        eprosima::fastcdr::Cdr::state length_state = serializer.getState();
        uint16_t length = 0;
        serializer << length;

        const char* length_position = serializer.getCurrentPosition();

        if(!serialize_SecureDataTag(serializer, local_writer->transformation_kind, local_writer->session_id,
                    initialization_vector, receiving_datareader_crypto_list, update_specific_keys, tag))
        {
            return false;
        }

        eprosima::fastcdr::Cdr::state current_state = serializer.getState();
        //TODO(Ricardo) fastcdr functinality: length substracting two Cdr::state.
        length =  static_cast<uint16_t>(serializer.getCurrentPosition() - length_position);
        serializer.setState(length_state);
        serializer << length;
        serializer.setState(current_state);
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException&)
    {
        logError(SECURITY_CRYPTO, "Not enough memory to serialize SecureDataTag");
        return false;
    }

    encoded_rtps_submessage.pos += static_cast<uint32_t>(serializer.getSerializedDataLength());
    encoded_rtps_submessage.length += static_cast<uint32_t>(serializer.getSerializedDataLength());

    return true;
}

bool AESGCMGMAC_Transform::encode_datareader_submessage(
        CDRMessage_t& encoded_rtps_submessage,
        const CDRMessage_t& plain_rtps_submessage,
        DatareaderCryptoHandle &sending_datareader_crypto,
        std::vector<DatawriterCryptoHandle*> &receiving_datawriter_crypto_list,
        SecurityException& /*exception*/)
{
    AESGCMGMAC_ReaderCryptoHandle& local_reader = AESGCMGMAC_ReaderCryptoHandle::narrow(sending_datareader_crypto);

    if(local_reader.nil()){
        logWarning(SECURITY_CRYPTO,"Invalid CryptoHandle");
        return false;
    }

    if((plain_rtps_submessage.length  - plain_rtps_submessage.pos) > static_cast<uint32_t>(std::numeric_limits<int>::max()))
    {
        logError(SECURITY_CRYPTO, "Plain rtps submessage too large");
        return false;
    }

    eprosima::fastcdr::FastBuffer output_buffer((char*)&encoded_rtps_submessage.buffer[encoded_rtps_submessage.pos],
            encoded_rtps_submessage.max_size - encoded_rtps_submessage.pos);
    eprosima::fastcdr::Cdr serializer(output_buffer);

    std::unique_lock<std::mutex> lock(local_reader->mutex_);

    //Step 2 - If the maximum number of blocks have been processed, generate a new SessionKey
    bool update_specific_keys = false;
    if(local_reader->session_block_counter >= local_reader->max_blocks_per_session){
        local_reader->session_id += 1;
        update_specific_keys = true;
        local_reader->SessionKey = compute_sessionkey(local_reader->EntityKeyMaterial.master_sender_key,
                local_reader->EntityKeyMaterial.master_salt,
                local_reader->session_id);

        //ReceiverSpecific keys shall be computed specifically when needed
        local_reader->session_block_counter = 0;
    }

    local_reader->session_block_counter += 1;

    //Build remaining NONCE elements
    std::array<uint8_t, initialization_vector_suffix_length> initialization_vector_suffix;  //iv suffix changes with every operation
    RAND_bytes(initialization_vector_suffix.data(), initialization_vector_suffix_length);
    std::array<uint8_t,12> initialization_vector; //96 bytes, session_id + suffix
    memcpy(initialization_vector.data(),&(local_reader->session_id),4);
    memcpy(initialization_vector.data() + 4, initialization_vector_suffix.data(), 8);
    std::array<uint8_t, 4> session_id;
    memcpy(session_id.data(), &(local_reader->session_id), 4);

#if __BIG_ENDIAN__
    octet flags = 0x0;
    serializer.changeEndianness(eprosima::fastcdr::Cdr::Endianness::BIG_ENDIANNESS);
#else
    octet flags = BIT(0);
    serializer.changeEndianness(eprosima::fastcdr::Cdr::Endianness::LITTLE_ENDIANNESS);
#endif

    //Header
    try
    {
        serializer << SEC_PREFIX << flags;
        eprosima::fastcdr::Cdr::state length_state = serializer.getState();
        uint16_t length = 0;
        serializer << length;

        const char* length_position = serializer.getCurrentPosition();

        serialize_SecureDataHeader(serializer, local_reader->EntityKeyMaterial.transformation_kind,
                local_reader->EntityKeyMaterial.sender_key_id, session_id, initialization_vector_suffix);

        eprosima::fastcdr::Cdr::state current_state = serializer.getState();
        //TODO(Ricardo) fastcdr functinality: length substracting two Cdr::state.
        length =  static_cast<uint16_t>(serializer.getCurrentPosition() - length_position);
        serializer.setState(length_state);
        serializer << length;
        serializer.setState(current_state);
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException&)
    {
        logError(SECURITY_CRYPTO, "Not enough memory to serialize SecureDataHeader");
        return false;
    }

    SecureDataTag tag;

    // Body
    try
    {
        if(!serialize_SecureDataBody(serializer, local_reader->transformation_kind, local_reader->SessionKey,
                    initialization_vector, output_buffer, &plain_rtps_submessage.buffer[plain_rtps_submessage.pos],
                    plain_rtps_submessage.length - plain_rtps_submessage.pos, tag))
        {
            return false;
        }
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException&)
    {
        logError(SECURITY_CRYPTO, "Not enough memory to serialize SecureDataBody");
        return false;
    }

    // Tag
    try
    {
        serializer << SEC_POSTFIX << flags;
        eprosima::fastcdr::Cdr::state length_state = serializer.getState();
        uint16_t length = 0;
        serializer << length;

        const char* length_position = serializer.getCurrentPosition();

        if(!serialize_SecureDataTag(serializer, local_reader->transformation_kind, local_reader->session_id,
                    initialization_vector, receiving_datawriter_crypto_list, update_specific_keys, tag))
        {
            return false;
        }

        eprosima::fastcdr::Cdr::state current_state = serializer.getState();
        //TODO(Ricardo) fastcdr functinality: length substracting two Cdr::state.
        length =  static_cast<uint16_t>(serializer.getCurrentPosition() - length_position);
        serializer.setState(length_state);
        serializer << length;
        serializer.setState(current_state);
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException&)
    {
        logError(SECURITY_CRYPTO, "Not enough memory to serialize SecureDataTag");
        return false;
    }

    encoded_rtps_submessage.pos += static_cast<uint32_t>(serializer.getSerializedDataLength());
    encoded_rtps_submessage.length += static_cast<uint32_t>(serializer.getSerializedDataLength());

    return true;
}

bool AESGCMGMAC_Transform::encode_rtps_message(
        CDRMessage_t& encoded_rtps_message,
        const CDRMessage_t& plain_rtps_message,
        ParticipantCryptoHandle &sending_crypto,
        std::vector<ParticipantCryptoHandle*> &receiving_crypto_list,
        SecurityException& /*exception*/)
{
    AESGCMGMAC_ParticipantCryptoHandle& local_participant = AESGCMGMAC_ParticipantCryptoHandle::narrow(sending_crypto);

    if(local_participant.nil())
    {
        logError(SECURITY_CRYPTO,"Invalid CryptoToken");
        return false;
    }

    if((plain_rtps_message.length  - plain_rtps_message.pos) > static_cast<uint32_t>(std::numeric_limits<int>::max()))
    {
        logError(SECURITY_CRYPTO, "Plain rtps submessage too large");
        return false;
    }

    eprosima::fastcdr::FastBuffer output_buffer((char*)&encoded_rtps_message.buffer[encoded_rtps_message.pos],
            encoded_rtps_message.max_size - encoded_rtps_message.pos);
    eprosima::fastcdr::Cdr serializer(output_buffer);

    std::unique_lock<std::mutex> lock(local_participant->mutex_);

    // If the maximum number of blocks have been processed, generate a new SessionKey
    bool update_specific_keys = false;
    if(local_participant->session_block_counter >= local_participant->max_blocks_per_session)
    {
        local_participant->session_id += 1;
        update_specific_keys = true;
        local_participant->SessionKey = compute_sessionkey(local_participant->ParticipantKeyMaterial.master_sender_key,
                local_participant->ParticipantKeyMaterial.master_salt,
                local_participant->session_id);

        //ReceiverSpecific keys shall be computed specifically when needed
        local_participant->session_block_counter = 0;
        //Insert outdate session_id values in all RemoteParticipant trackers to trigger a SessionkeyUpdate
    }

    local_participant->session_block_counter += 1;

    //Build remaining NONCE elements
    std::array<uint8_t, initialization_vector_suffix_length> initialization_vector_suffix;  //iv suffix changes with every operation
    RAND_bytes(initialization_vector_suffix.data(), initialization_vector_suffix_length);
    std::array<uint8_t,12> initialization_vector; //96 bytes, session_id + suffix
    memcpy(initialization_vector.data(),&(local_participant->session_id),4);
    memcpy(initialization_vector.data() + 4, initialization_vector_suffix.data(), 8);
    std::array<uint8_t, 4> session_id;
    memcpy(session_id.data(), &(local_participant->session_id), 4);

#if __BIG_ENDIAN__
    octet flags = 0x0;
    serializer.changeEndianness(eprosima::fastcdr::Cdr::Endianness::BIG_ENDIANNESS);
#else
    octet flags = BIT(0);
    serializer.changeEndianness(eprosima::fastcdr::Cdr::Endianness::LITTLE_ENDIANNESS);
#endif

    //Header
    try
    {
        serializer << SRTPS_PREFIX << flags;
        eprosima::fastcdr::Cdr::state length_state = serializer.getState();
        uint16_t length = 0;
        serializer << length;

        const char* length_position = serializer.getCurrentPosition();

        serialize_SecureDataHeader(serializer, local_participant->ParticipantKeyMaterial.transformation_kind,
                local_participant->ParticipantKeyMaterial.sender_key_id, session_id, initialization_vector_suffix);

        eprosima::fastcdr::Cdr::state current_state = serializer.getState();
        //TODO(Ricardo) fastcdr functinality: length substracting two Cdr::state.
        length =  static_cast<uint16_t>(serializer.getCurrentPosition() - length_position);
        serializer.setState(length_state);
        serializer << length;
        serializer.setState(current_state);
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException&)
    {
        logError(SECURITY_CRYPTO, "Not enough memory to serialize SecureDataHeader");
        return false;
    }

    SecureDataTag tag;

    // Body
    try
    {
        if(!serialize_SecureDataBody(serializer, local_participant->transformation_kind, local_participant->SessionKey,
                    initialization_vector, output_buffer, &plain_rtps_message.buffer[plain_rtps_message.pos],
                    plain_rtps_message.length - plain_rtps_message.pos, tag))
        {
            return false;
        }
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException&)
    {
        logError(SECURITY_CRYPTO, "Not enough memory to serialize SecureDataBody");
        return false;
    }

    // Tag
    try
    {
        serializer << SRTPS_POSTFIX << flags;
        eprosima::fastcdr::Cdr::state length_state = serializer.getState();
        uint16_t length = 0;
        serializer << length;

        const char* length_position = serializer.getCurrentPosition();

        if(!serialize_SecureDataTag(serializer, local_participant, initialization_vector, receiving_crypto_list,
                    update_specific_keys, tag))
        {
            return false;
        }

        eprosima::fastcdr::Cdr::state current_state = serializer.getState();
        //TODO(Ricardo) fastcdr functinality: length substracting two Cdr::state.
        length =  static_cast<uint16_t>(serializer.getCurrentPosition() - length_position);
        serializer.setState(length_state);
        serializer << length;
        serializer.setState(current_state);
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException&)
    {
        logError(SECURITY_CRYPTO, "Not enough memory to serialize SecureDataTag");
        return false;
    }

    encoded_rtps_message.pos += static_cast<uint32_t>(serializer.getSerializedDataLength());
    encoded_rtps_message.length += static_cast<uint32_t>(serializer.getSerializedDataLength());

    return true;
}

bool AESGCMGMAC_Transform::decode_rtps_message(
        CDRMessage_t& plain_buffer,
        const CDRMessage_t& encoded_buffer,
        const ParticipantCryptoHandle& /*receiving_crypto*/,
        const ParticipantCryptoHandle &sending_crypto,
        SecurityException& /*exception*/)
{
    const AESGCMGMAC_ParticipantCryptoHandle& sending_participant = AESGCMGMAC_ParticipantCryptoHandle::narrow(sending_crypto);

    if(sending_participant.nil())
    {
        logError(SECURITY_CRYPTO, "Invalid sending_crypto handle");
        return false;
    }

    // Output buffer has to have position and length with same value.
    if(plain_buffer.pos != plain_buffer.length)
    {
        logError(SECURITY_CRYPTO, "Output message is not set correctly");
        return false;
    }

    if((encoded_buffer.length - encoded_buffer.pos) > static_cast<uint32_t>(std::numeric_limits<int>::max()))
    {
        logError(SECURITY_CRYPTO, "Encoded rtps message too large");
        return false;
    }

    if(sending_participant->RemoteParticipant2ParticipantKeyMaterial.size() == 0)
        return false;

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

        if(id != SRTPS_PREFIX)
        {
            logError(SECURITY_CRYPTO, "Not valid SecureDataHeader submessage id");
            return false;
        }

        decoder >> flags;

        if(flags & BIT(0))
        {
            decoder.changeEndianness(eprosima::fastcdr::Cdr::Endianness::LITTLE_ENDIANNESS);
        }
        else
        {
            decoder.changeEndianness(eprosima::fastcdr::Cdr::Endianness::BIG_ENDIANNESS);
        }

        decoder >> length;
        const char* const current_position = decoder.getCurrentPosition();

        header = deserialize_SecureDataHeader(decoder);

        if(length != (uint16_t)(decoder.getCurrentPosition() - current_position))
        {
            logError(SECURITY_CRYPTO, "Invalid length for SecureDataHeader");
            return false;
        }
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException&)
    {
        logError(SECURITY_CRYPTO, "Not enough memory to deserialize SecureDataHeader");
        return false;
    }

    uint32_t session_id;
    memcpy(&session_id, header.session_id.data(), 4);

    //Sessionkey
    std::array<uint8_t,32> session_key = compute_sessionkey(
            sending_participant->RemoteParticipant2ParticipantKeyMaterial.at(0).master_sender_key,
            sending_participant->RemoteParticipant2ParticipantKeyMaterial.at(0).master_salt,
            session_id);
    //IV
    std::array<uint8_t,12> initialization_vector;
    memcpy(initialization_vector.data(), header.session_id.data(), 4);
    memcpy(initialization_vector.data() + 4, header.initialization_vector_suffix.data(), 8);

    // Body
    uint32_t body_length = 0, body_align = 0;

    try
    {   if(!predeserialize_SecureDataBody(decoder, body_length, body_align))
        {
            logError(SECURITY_CRYPTO, "Error deserializing SecureDataBody header");
            return false;
        }
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException&)
    {
        logError(SECURITY_CRYPTO, "Not enough memory to deserialize SecureDataBody header");
        return false;
    }

    eprosima::fastcdr::Cdr::state body_state = decoder.getState();
    decoder.jump(body_length + body_align);

    // Tag
    try
    {
        uint8_t id = 0, flags = 0;
        uint16_t length = 0;

        decoder >> id;

        if(id != SRTPS_POSTFIX)
        {
            logError(SECURITY_CRYPTO, "Not valid SecureDataTag submessage id");
            return false;
        }

        decoder >> flags;

        if(flags & BIT(0))
        {
            decoder.changeEndianness(eprosima::fastcdr::Cdr::Endianness::LITTLE_ENDIANNESS);
        }
        else
        {
            decoder.changeEndianness(eprosima::fastcdr::Cdr::Endianness::BIG_ENDIANNESS);
        }

        decoder >> length;
        const char* const current_position = decoder.getCurrentPosition();

        SecurityException exception;

        if(!deserialize_SecureDataTag(decoder, tag, sending_participant->transformation_kind,
                sending_participant->RemoteParticipant2ParticipantKeyMaterial.at(0).receiver_specific_key_id,
                sending_participant->RemoteParticipant2ParticipantKeyMaterial.at(0).master_receiver_specific_key,
                sending_participant->RemoteParticipant2ParticipantKeyMaterial.at(0).master_salt,
                initialization_vector, session_id, exception))
        {
            return false;
        }

        if(length != (uint16_t)(decoder.getCurrentPosition() - current_position))
        {
            logError(SECURITY_CRYPTO, "Invalid length for SecureDataTag");
            return false;
        }
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException&)
    {
        logError(SECURITY_CRYPTO, "Not enough memory to deserialize SecureDataTag length");
        return false;
    }

    uint32_t length = plain_buffer.max_size - plain_buffer.pos;
    if(!deserialize_SecureDataBody(decoder, body_state, tag, body_length,
            sending_participant->transformation_kind, session_key, initialization_vector,
            &plain_buffer.buffer[plain_buffer.pos], length))
    {
        logError(SECURITY_CRYPTO, "Error decoding content");
        return false;
    }

    plain_buffer.length += length;
    return true;
}

bool AESGCMGMAC_Transform::preprocess_secure_submsg(
        DatawriterCryptoHandle **datawriter_crypto,
        DatareaderCryptoHandle **datareader_crypto,
        SecureSubmessageCategory_t &secure_submessage_category,
        const CDRMessage_t& encoded_rtps_submessage,
        ParticipantCryptoHandle &receiving_crypto,
        ParticipantCryptoHandle &sending_crypto,
        SecurityException &exception){

    AESGCMGMAC_ParticipantCryptoHandle& remote_participant = AESGCMGMAC_ParticipantCryptoHandle::narrow(sending_crypto);
    if(remote_participant.nil()){
        logWarning(SECURITY_CRYPTO,"Invalid CryptoHandle");
        exception = SecurityException("Not a valid ParticipantCryptoHandle received");
        return false;
    }

    AESGCMGMAC_ParticipantCryptoHandle& local_participant = AESGCMGMAC_ParticipantCryptoHandle::narrow(receiving_crypto);
    if(local_participant.nil()){
        logWarning(SECURITY_CRYPTO,"Invalid CryptoHandle");
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

        if(id != SEC_PREFIX)
        {
            logError(SECURITY_CRYPTO, "Not valid SecureDataHeader submessage id");
            return false;
        }

        decoder >> flags;

        if(flags & BIT(0))
        {
            decoder.changeEndianness(eprosima::fastcdr::Cdr::Endianness::LITTLE_ENDIANNESS);
        }
        else
        {
            decoder.changeEndianness(eprosima::fastcdr::Cdr::Endianness::BIG_ENDIANNESS);
        }

        decoder >> length;
        const char* const current_position = decoder.getCurrentPosition();

        header = deserialize_SecureDataHeader(decoder);

        if(length != (uint16_t)(decoder.getCurrentPosition() - current_position))
        {
            logError(SECURITY_CRYPTO, "Invalid length for SecureDataHeader");
            return false;
        }
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException&)
    {
        logError(SECURITY_CRYPTO, "Not enough memory to deserialize SecureDataHeader");
        return false;
    }

    //TODO(Ricardo) Deserializing header two times, here preprocessing and decoding submessage.
    //KeyId is present in Header->transform_identifier->transformation_key_id and contains the sender_key_id

    for(std::vector<DatawriterCryptoHandle *>::iterator it = remote_participant->Writers.begin();
            it != remote_participant->Writers.end(); ++it)
    {
        AESGCMGMAC_WriterCryptoHandle& writer = AESGCMGMAC_WriterCryptoHandle::narrow(**it);

        if(writer->Entity2RemoteKeyMaterial.size() == 0)
        {
            logWarning(SECURITY_CRYPTO, "No key material yet");
            continue;
        }

        if(writer->Entity2RemoteKeyMaterial.at(0).sender_key_id == header.transform_identifier.transformation_key_id)
        {
            secure_submessage_category = DATAWRITER_SUBMESSAGE;
            *datawriter_crypto = *it;
            //We have the remote writer, now lets look for the local datareader
            for(std::vector<DatareaderCryptoHandle *>::iterator itt = local_participant->Readers.begin(); itt != local_participant->Readers.end(); ++itt)
            {
                AESGCMGMAC_ReaderCryptoHandle& reader = AESGCMGMAC_ReaderCryptoHandle::narrow(**itt);

                if(reader->Entity2RemoteKeyMaterial.size() == 0)
                {
                    logWarning(SECURITY_CRYPTO, "No key material yet");
                    continue;
                }

                for(size_t i=0; i < reader->Entity2RemoteKeyMaterial.size(); ++i)
                {
                    if(reader->Entity2RemoteKeyMaterial.at(i).receiver_specific_key_id ==
                            writer->Remote2EntityKeyMaterial.at(0).receiver_specific_key_id)
                    {
                        *datareader_crypto = *itt;
                        return true;
                    }
                }   //For each Reader2WriterKeyMaterial in the local datareader
            } //For each datareader present in the local participant
        }
    }

    for(std::vector<DatareaderCryptoHandle *>::iterator it = remote_participant->Readers.begin();
            it != remote_participant->Readers.end(); ++it)
    {
        AESGCMGMAC_ReaderCryptoHandle& reader = AESGCMGMAC_ReaderCryptoHandle::narrow(**it);

        if(reader->Entity2RemoteKeyMaterial.size() == 0)
        {
            logWarning(SECURITY_CRYPTO, "No key material yet");
            continue;
        }

        if(reader->Entity2RemoteKeyMaterial.at(0).sender_key_id == header.transform_identifier.transformation_key_id)
        {
            secure_submessage_category = DATAREADER_SUBMESSAGE;
            *datareader_crypto = *it;

            //We have the remote reader, now lets look for the local datawriter
            for(std::vector<DatawriterCryptoHandle *>::iterator itt = local_participant->Writers.begin(); itt != local_participant->Writers.end(); ++itt)
            {
                AESGCMGMAC_WriterCryptoHandle& writer = AESGCMGMAC_WriterCryptoHandle::narrow(**itt);
                for(size_t i = 0; i < writer->Entity2RemoteKeyMaterial.size(); ++i)
                {
                    if(writer->Entity2RemoteKeyMaterial.at(i).receiver_specific_key_id ==
                            reader->Remote2EntityKeyMaterial.at(0).receiver_specific_key_id)
                    {
                        *datawriter_crypto = *itt;
                        return true;
                    }
                }   //For each Writer2ReaderKeyMaterial in the local datawriter
            } //For each datawriter present in the local participant
        }
    }
    logWarning(SECURITY_CRYPTO,"Unable to determine the nature of the message");

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

    if(sending_writer.nil())
    {
        logError(SECURITY_CRYPTO, "Invalid sending_writer handle");
        return false;
    }

    if(sending_writer->Entity2RemoteKeyMaterial.size() == 0)
    {
        logWarning(SECURITY_CRYPTO, "No key material yet");
        return false;
    }

    if(encoded_rtps_submessage.length - encoded_rtps_submessage.pos > static_cast<uint32_t>(std::numeric_limits<int>::max()))
    {
        logError(SECURITY_CRYPTO, "Encoded rtps submessage too large");
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

        if(id != SEC_PREFIX)
        {
            logError(SECURITY_CRYPTO, "Not valid SecureDataHeader submessage id");
            return false;
        }

        decoder >> flags;

        if(flags & BIT(0))
        {
            decoder.changeEndianness(eprosima::fastcdr::Cdr::Endianness::LITTLE_ENDIANNESS);
        }
        else
        {
            decoder.changeEndianness(eprosima::fastcdr::Cdr::Endianness::BIG_ENDIANNESS);
        }

        decoder >> length;
        const char* const current_position = decoder.getCurrentPosition();

        header = deserialize_SecureDataHeader(decoder);

        if(length != (uint16_t)(decoder.getCurrentPosition() - current_position))
        {
            logError(SECURITY_CRYPTO, "Invalid length for SecureDataHeader");
            return false;
        }
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException&)
    {
        logError(SECURITY_CRYPTO, "Not enough memory to deserialize SecureDataHeader");
        return false;
    }

    uint32_t session_id;
    memcpy(&session_id,header.session_id.data(),4);
    //Sessionkey
    std::array<uint8_t,32> session_key = compute_sessionkey(
            sending_writer->Entity2RemoteKeyMaterial.at(0).master_sender_key,
            sending_writer->Entity2RemoteKeyMaterial.at(0).master_salt,
            session_id);
    //IV
    std::array<uint8_t,12> initialization_vector;
    memcpy(initialization_vector.data(), header.session_id.data(), 4);
    memcpy(initialization_vector.data() + 4, header.initialization_vector_suffix.data(), 8);

    // Body
    uint32_t body_length = 0, body_align = 0;

    try
    {   if(!predeserialize_SecureDataBody(decoder, body_length, body_align))
        {
            logError(SECURITY_CRYPTO, "Error deserializing SecureDataBody header");
            return false;
        }
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException&)
    {
        logError(SECURITY_CRYPTO, "Not enough memory to deserialize SecureDataBody header");
        return false;
    }

    eprosima::fastcdr::Cdr::state body_state = decoder.getState();
    decoder.jump(body_length + body_align);

    // Tag
    try
    {
        uint8_t id = 0, flags = 0;
        uint16_t length = 0;

        decoder >> id;

        if(id != SEC_POSTFIX)
        {
            logError(SECURITY_CRYPTO, "Not valid SecureDataTag submessage id");
            return false;
        }

        decoder >> flags;

        if(flags & BIT(0))
        {
            decoder.changeEndianness(eprosima::fastcdr::Cdr::Endianness::LITTLE_ENDIANNESS);
        }
        else
        {
            decoder.changeEndianness(eprosima::fastcdr::Cdr::Endianness::BIG_ENDIANNESS);
        }

        decoder >> length;
        const char* const current_position = decoder.getCurrentPosition();

        SecurityException exception;

        if(!deserialize_SecureDataTag(decoder, tag, sending_writer->transformation_kind,
                sending_writer->Entity2RemoteKeyMaterial.at(0).receiver_specific_key_id,
                sending_writer->Entity2RemoteKeyMaterial.at(0).master_receiver_specific_key,
                sending_writer->Entity2RemoteKeyMaterial.at(0).master_salt,
                initialization_vector, session_id, exception))
        {
            return false;
        }

        if(length != (uint16_t)(decoder.getCurrentPosition() - current_position))
        {
            logError(SECURITY_CRYPTO, "Invalid length for SecureDataTag");
            return false;
        }
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException&)
    {
        logError(SECURITY_CRYPTO, "Not enough memory to deserialize SecureDataTag length");
        return false;
    }

    uint32_t length = plain_rtps_submessage.max_size - plain_rtps_submessage.pos;
    if(!deserialize_SecureDataBody(decoder, body_state, tag, body_length,
            sending_writer->transformation_kind, session_key, initialization_vector,
            &plain_rtps_submessage.buffer[plain_rtps_submessage.pos], length))
    {
        logError(SECURITY_CRYPTO, "Error decoding content");
        return false;
    }

    plain_rtps_submessage.length += length;
    encoded_rtps_submessage.pos += static_cast<uint32_t>(decoder.getSerializedDataLength());

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

    if(sending_reader.nil())
    {
        logError(SECURITY_CRYPTO, "Invalid sending_reader handle");
        return false;
    }

    if(sending_reader->Entity2RemoteKeyMaterial.size() == 0)
    {
        logWarning(SECURITY_CRYPTO, "No key material yet");
        return false;
    }

    if(encoded_rtps_submessage.length - encoded_rtps_submessage.pos > static_cast<uint32_t>(std::numeric_limits<int>::max()))
    {
        logError(SECURITY_CRYPTO, "Encoded rtps submessage too large");
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

        if(id != SEC_PREFIX)
        {
            logError(SECURITY_CRYPTO, "Not valid SecureDataHeader submessage id");
            return false;
        }

        decoder >> flags;

        if(flags & BIT(0))
        {
            decoder.changeEndianness(eprosima::fastcdr::Cdr::Endianness::LITTLE_ENDIANNESS);
        }
        else
        {
            decoder.changeEndianness(eprosima::fastcdr::Cdr::Endianness::BIG_ENDIANNESS);
        }

        decoder >> length;
        const char* const current_position = decoder.getCurrentPosition();

        header = deserialize_SecureDataHeader(decoder);

        if(length != (uint16_t)(decoder.getCurrentPosition() - current_position))
        {
            logError(SECURITY_CRYPTO, "Invalid length for SecureDataHeader");
            return false;
        }
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException&)
    {
        logError(SECURITY_CRYPTO, "Not enough memory to deserialize SecureDataHeader");
        return false;
    }

    uint32_t session_id;
    memcpy(&session_id,header.session_id.data(),4);
    //Sessionkey
    std::array<uint8_t,32> session_key = compute_sessionkey(
            sending_reader->Entity2RemoteKeyMaterial.at(0).master_sender_key,
            sending_reader->Entity2RemoteKeyMaterial.at(0).master_salt,
            session_id);
    //IV
    std::array<uint8_t,12> initialization_vector;
    memcpy(initialization_vector.data(), header.session_id.data(), 4);
    memcpy(initialization_vector.data() + 4, header.initialization_vector_suffix.data(), 8);

    // Body
    uint32_t body_length = 0, body_align = 0;

    try
    {   if(!predeserialize_SecureDataBody(decoder, body_length, body_align))
        {
            logError(SECURITY_CRYPTO, "Error deserializing SecureDataBody header");
            return false;
        }
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException&)
    {
        logError(SECURITY_CRYPTO, "Not enough memory to deserialize SecureDataBody header");
        return false;
    }

    eprosima::fastcdr::Cdr::state body_state = decoder.getState();
    decoder.jump(body_length + body_align);

    // Tag
    try
    {
        uint8_t id = 0, flags = 0;
        uint16_t length = 0;

        decoder >> id;

        if(id != SEC_POSTFIX)
        {
            logError(SECURITY_CRYPTO, "Not valid SecureDataTag submessage id");
            return false;
        }

        decoder >> flags;

        if(flags & BIT(0))
        {
            decoder.changeEndianness(eprosima::fastcdr::Cdr::Endianness::LITTLE_ENDIANNESS);
        }
        else
        {
            decoder.changeEndianness(eprosima::fastcdr::Cdr::Endianness::BIG_ENDIANNESS);
        }

        decoder >> length;
        const char* const current_position = decoder.getCurrentPosition();

        SecurityException exception;

        if(!deserialize_SecureDataTag(decoder, tag, sending_reader->transformation_kind,
                sending_reader->Entity2RemoteKeyMaterial.at(0).receiver_specific_key_id,
                sending_reader->Entity2RemoteKeyMaterial.at(0).master_receiver_specific_key,
                sending_reader->Entity2RemoteKeyMaterial.at(0).master_salt,
                initialization_vector, session_id, exception))
        {
            return false;
        }

        if(length != (uint16_t)(decoder.getCurrentPosition() - current_position))
        {
            logError(SECURITY_CRYPTO, "Invalid length for SecureDataTag");
            return false;
        }
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException&)
    {
        logError(SECURITY_CRYPTO, "Not enough memory to deserialize SecureDataTag length");
        return false;
    }

    uint32_t length = plain_rtps_submessage.max_size - plain_rtps_submessage.pos;
    if(!deserialize_SecureDataBody(decoder, body_state, tag, body_length,
            sending_reader->transformation_kind, session_key, initialization_vector,
            &plain_rtps_submessage.buffer[plain_rtps_submessage.pos], length))
    {
        logError(SECURITY_CRYPTO, "Error decoding content");
        return false;
    }

    plain_rtps_submessage.length += static_cast<uint32_t>(length);
    encoded_rtps_submessage.pos += static_cast<uint32_t>(decoder.getSerializedDataLength());

    return true;
}


bool AESGCMGMAC_Transform::decode_serialized_payload(
        SerializedPayload_t& plain_payload,
        const SerializedPayload_t& encoded_payload,
        const std::vector<uint8_t>& /*inline_qos*/,
        DatareaderCryptoHandle& /*receiving_datareader_crypto*/,
        DatawriterCryptoHandle& sending_datawriter_crypto,
        SecurityException &exception){

    AESGCMGMAC_WriterCryptoHandle& sending_writer = AESGCMGMAC_WriterCryptoHandle::narrow(sending_datawriter_crypto);

    if(sending_writer.nil()){
        exception = SecurityException("Not a valid sending_writer handle");
        return false;
    }

    if(sending_writer->Entity2RemoteKeyMaterial.size() == 0)
    {
        logWarning(SECURITY_CRYPTO, "No key material yet");
        return false;
    }

    if(encoded_payload.length > static_cast<uint32_t>(std::numeric_limits<int>::max()))
    {
        logError(SECURITY_CRYPTO, "Encoded payload too large");
        return false;
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
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException&)
    {
        logError(SECURITY_CRYPTO, "Not enough memory to deserialize SecureDataHeader");
        return false;
    }

    uint32_t session_id;
    memcpy(&session_id, header.session_id.data(), 4);

    //Sessionkey
    std::array<uint8_t,32> session_key = compute_sessionkey(
            sending_writer->Entity2RemoteKeyMaterial.at(0).master_sender_key,
            sending_writer->Entity2RemoteKeyMaterial.at(0).master_salt,
            session_id);
    //IV
    std::array<uint8_t,12> initialization_vector;
    memcpy(initialization_vector.data(), header.session_id.data(), 4);
    memcpy(initialization_vector.data() + 4, header.initialization_vector_suffix.data(), 8);

    // Body
    uint32_t body_length = 0, body_align = 0;

    try
    {   if(!predeserialize_SecureDataBody(decoder, body_length, body_align))
        {
            logError(SECURITY_CRYPTO, "Error deserializing SecureDataBody header");
            return false;
        }
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException&)
    {
        logError(SECURITY_CRYPTO, "Not enough memory to deserialize SecureDataBody header");
        return false;
    }

    eprosima::fastcdr::Cdr::state body_state = decoder.getState();
    decoder.jump(body_length + body_align);

    // Tag
    try
    {
        deserialize_SecureDataTag(decoder, tag, {}, {}, {}, {}, {}, 0, exception);
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException&)
    {
        logError(SECURITY_CRYPTO, "Not enough memory to deserialize SecureDataTag length");
        return false;
    }

    uint32_t length = plain_payload.max_size;
    if(!deserialize_SecureDataBody(decoder, body_state, tag, body_length,
            sending_writer->transformation_kind, session_key, initialization_vector,
            plain_payload.data, length))
    {
        logError(SECURITY_CRYPTO, "Error decoding content");
        return false;
    }

    plain_payload.length = length;
    plain_payload.encapsulation = encoded_payload.encapsulation;

    return true;
}

std::array<uint8_t, 32> AESGCMGMAC_Transform::compute_sessionkey(const std::array<uint8_t,32>& master_sender_key,
        const std::array<uint8_t,32>& master_salt , const uint32_t session_id)
{

    std::array<uint8_t,32> session_key;
    unsigned char *source = (unsigned char*)malloc(32 + 10 + 32 + 4);
    memcpy(source, master_sender_key.data(), 32);
    char seq[] = "SessionKey";
    memcpy(source+32, seq, 10);
    memcpy(source+32+10, master_salt.data(),32);
    memcpy(source+32+10+32, &(session_id),4);

    EVP_Digest(source, 32+10+32+4, session_key.data(), NULL, EVP_sha256(), NULL);

    free(source);
    return session_key;
}

void AESGCMGMAC_Transform::serialize_SecureDataHeader(eprosima::fastcdr::Cdr& serializer,
        const CryptoTransformKind& transformation_kind, const CryptoTransformKeyId& transformation_key_id,
        const std::array<uint8_t, 4>& session_id, const std::array<uint8_t, 8>& initialization_vector_suffix)
{
    serializer << transformation_kind << transformation_key_id << session_id << initialization_vector_suffix;
}

bool AESGCMGMAC_Transform::serialize_SecureDataBody(eprosima::fastcdr::Cdr& serializer,
        const std::array<uint8_t, 4>& transformation_kind, const std::array<uint8_t,32>& session_key,
        const std::array<uint8_t, 12>& initialization_vector,
        eprosima::fastcdr::FastBuffer& output_buffer, octet* plain_buffer, uint32_t plain_buffer_len,
        SecureDataTag& tag)
{
#if __BIG_ENDIAN__
    octet flags = 0x0;
    serializer.changeEndianness(eprosima::fastcdr::Cdr::Endianness::BIG_ENDIANNESS);
#else
    octet flags = BIT(0);
    serializer.changeEndianness(eprosima::fastcdr::Cdr::Endianness::LITTLE_ENDIANNESS);
#endif

    serializer << SecureBodySubmessage << flags;

    // Store current state to serialize sequence length at the end of the function
    eprosima::fastcdr::Cdr::state sequence_length_state = serializer.getState();

    // Serialize dummy length
    uint16_t length = 0;
    serializer << length;

    //Cypher the plain rtps message -> SecureDataBody

    // AES_BLOCK_SIZE = 16
    int cipher_block_size = 0, actual_size = 0, final_size = 0;
    char* output_buffer_raw = nullptr;
    EVP_CIPHER_CTX* e_ctx = EVP_CIPHER_CTX_new();
    if(transformation_kind == std::array<uint8_t, 4>{CRYPTO_TRANSFORMATION_KIND_AES128_GCM} ||
            transformation_kind == std::array<uint8_t, 4>{CRYPTO_TRANSFORMATION_KIND_AES128_GMAC})
    {
        if(!EVP_EncryptInit(e_ctx, EVP_aes_128_gcm(), (const unsigned char*)(session_key.data()),
                    initialization_vector.data()))
        {
            logError(SECURITY_CRYPTO, "Unable to encode the payload. EVP_EncryptInit function returns an error");
            return false;
        }

        cipher_block_size = EVP_CIPHER_block_size(EVP_aes_128_gcm());

        if(transformation_kind == std::array<uint8_t, 4>{CRYPTO_TRANSFORMATION_KIND_AES128_GCM})
        {
            output_buffer_raw = serializer.getCurrentPosition();
        }
    }
    else if(transformation_kind == std::array<uint8_t, 4>{CRYPTO_TRANSFORMATION_KIND_AES256_GCM} ||
            transformation_kind == std::array<uint8_t, 4>{CRYPTO_TRANSFORMATION_KIND_AES256_GMAC})
    {
        if(!EVP_EncryptInit(e_ctx, EVP_aes_256_gcm(), (const unsigned char*)(session_key.data()),
                    initialization_vector.data()))
        {
            logError(SECURITY_CRYPTO, "Unable to encode the payload. EVP_EncryptInit function returns an error");
            return false;
        }

        cipher_block_size = EVP_CIPHER_block_size(EVP_aes_256_gcm());

        if(transformation_kind == std::array<uint8_t, 4>{CRYPTO_TRANSFORMATION_KIND_AES256_GCM})
        {
            output_buffer_raw = serializer.getCurrentPosition();
        }
    }

    if(output_buffer_raw != nullptr)
    {
        // Check output_buffer contains enough memory to cypher.
        // - EVP_EncryptUpdate needs at maximum: plain_buffer_len + cipher_block_size - 1.
        // - EVP_EncryptFinal needs ad maximun cipher_block_size.
        if((output_buffer.getBufferSize() - (serializer.getCurrentPosition() - serializer.getBufferPointer())) <
                (plain_buffer_len + (2* cipher_block_size) - 1))
        {
            logError(SECURITY_CRYPTO, "Not enough memory to cipher payload");
            return false;
        }
    }

    if(!EVP_EncryptUpdate(e_ctx, (unsigned char*)output_buffer_raw, &actual_size, plain_buffer,
                static_cast<int>(plain_buffer_len)))
    {
        logError(SECURITY_CRYPTO, "Unable to encode the payload. EVP_EncryptUpdate function returns an error");
        return false;
    }

    if(!EVP_EncryptFinal(e_ctx, (unsigned char*)output_buffer_raw, &final_size))
    {
        logError(SECURITY_CRYPTO, "Unable to encode the payload. EVP_EncryptFinal function returns an error");
        return false;
    }

    if(output_buffer_raw != nullptr)
    {
        serializer.jump(actual_size + final_size);
    }
    else
    {
        memcpy(serializer.getCurrentPosition(), plain_buffer, plain_buffer_len);
        serializer.jump(plain_buffer_len);
    }

    EVP_CIPHER_CTX_ctrl(e_ctx, EVP_CTRL_GCM_GET_TAG, AES_BLOCK_SIZE, tag.common_mac.data());
    EVP_CIPHER_CTX_free(e_ctx);

    eprosima::fastcdr::Cdr::state current_state = serializer.getState();

    // Serialize body sequence length;
    serializer.setState(sequence_length_state);
    serializer << static_cast<uint16_t>(actual_size + final_size);

    serializer.setState(current_state);

    // Align submessage to 4.
    size_t alignment = serializer.alignment(serializer.getCurrentPosition() - serializer.getBufferPointer(), sizeof(int32_t));
    for(size_t count = 0; count != alignment; ++count)
    {
        uint8_t c = 0;
        serializer << c;
    }

    return true;
}

bool AESGCMGMAC_Transform::serialize_SecureDataTag(eprosima::fastcdr::Cdr& serializer,
        const std::array<uint8_t, 4>& transformation_kind, const uint32_t session_id,
        const std::array<uint8_t, 12>& initialization_vector,
        std::vector<EntityCryptoHandle*>& receiving_crypto_list, bool update_specific_keys,
        SecureDataTag& tag)
{
    serializer << tag.common_mac;

    eprosima::fastcdr::Cdr::state length_state = serializer.getState();
    uint32_t length = 0;
    serializer << length;

    //Check the list of receivers, search for keys and compute session keys as needed
    for(auto rec = receiving_crypto_list.begin(); rec != receiving_crypto_list.end(); ++rec)
    {

        AESGCMGMAC_EntityCryptoHandle& remote_entity = AESGCMGMAC_ReaderCryptoHandle::narrow(**rec);

        if(remote_entity.nil())
        {
            logWarning(SECURITY_CRYPTO, "Invalid CryptoHandle");
            continue;
        }

        if(remote_entity->Remote2EntityKeyMaterial.size() == 0)
        {
            logWarning(SECURITY_CRYPTO, "No key material yet");
            continue;
        }

        //Update the key if needed
        if(update_specific_keys || remote_entity->session_id != session_id)
        {
            //Update triggered!
            remote_entity->session_id = session_id;
            remote_entity->SessionKey = compute_sessionkey(remote_entity->Remote2EntityKeyMaterial.at(0).master_receiver_specific_key,
                    remote_entity->Remote2EntityKeyMaterial.at(0).master_salt,
                    remote_entity->session_id);
        }

        //Obtain MAC using ReceiverSpecificKey and the same Initialization Vector as before
        int actual_size = 0, final_size = 0;
        EVP_CIPHER_CTX* e_ctx = EVP_CIPHER_CTX_new();
        if(transformation_kind == std::array<uint8_t, 4>{CRYPTO_TRANSFORMATION_KIND_AES128_GCM} ||
                transformation_kind == std::array<uint8_t, 4>{CRYPTO_TRANSFORMATION_KIND_AES128_GMAC})
        {
            if(!EVP_EncryptInit(e_ctx, EVP_aes_128_gcm(), (const unsigned char*)(remote_entity->SessionKey.data()),
                        initialization_vector.data()))
            {
                logError(SECURITY_CRYPTO, "Unable to encode the payload. EVP_EncryptInit function returns an error");
                continue;
            }
        }
        else if(transformation_kind == std::array<uint8_t, 4>{CRYPTO_TRANSFORMATION_KIND_AES256_GCM} ||
                transformation_kind == std::array<uint8_t, 4>{CRYPTO_TRANSFORMATION_KIND_AES256_GMAC})
        {
            if(!EVP_EncryptInit(e_ctx, EVP_aes_256_gcm(), (const unsigned char*)(remote_entity->SessionKey.data()),
                        initialization_vector.data()))
            {
                logError(SECURITY_CRYPTO, "Unable to encode the payload. EVP_EncryptInit function returns an error");
                continue;
            }
        }
        if(!EVP_EncryptUpdate(e_ctx, NULL, &actual_size, tag.common_mac.data(), 16))
        {
            logError(SECURITY_CRYPTO, "Unable to create authentication for the datawriter submessage. EVP_EncryptUpdate function returns an error");
            continue;
        }
        if(!EVP_EncryptFinal(e_ctx, NULL, &final_size))
        {
            logError(SECURITY_CRYPTO, "Unable to create authentication for the datawriter submessage. EVP_EncryptFinal function returns an error");
            continue;
        }
        serializer << remote_entity->Remote2EntityKeyMaterial.at(0).receiver_specific_key_id;
        EVP_CIPHER_CTX_ctrl(e_ctx, EVP_CTRL_GCM_GET_TAG, 16, serializer.getCurrentPosition());
        serializer.jump(16);
        EVP_CIPHER_CTX_free(e_ctx);

        ++length;
    }

    eprosima::fastcdr::Cdr::state current_state = serializer.getState();
    serializer.setState(length_state);
    serializer << length;
    serializer.setState(current_state);
    return true;
}

bool AESGCMGMAC_Transform::serialize_SecureDataTag(eprosima::fastcdr::Cdr& serializer,
        const AESGCMGMAC_ParticipantCryptoHandle& local_participant,
        const std::array<uint8_t, 12>& initialization_vector,
        std::vector<ParticipantCryptoHandle*>& receiving_crypto_list, bool update_specific_keys,
        SecureDataTag& tag)
{
    serializer << tag.common_mac;

    eprosima::fastcdr::Cdr::state length_state = serializer.getState();
    uint32_t length = 0;
    serializer << length;

    //Check the list of receivers, search for keys and compute session keys as needed
    for(auto rec = receiving_crypto_list.begin(); rec != receiving_crypto_list.end(); ++rec)
    {

        AESGCMGMAC_ParticipantCryptoHandle& remote_participant = AESGCMGMAC_ParticipantCryptoHandle::narrow(**rec);

        if(remote_participant.nil())
        {
            logWarning(SECURITY_CRYPTO, "Invalid CryptoHandle");
            continue;
        }

        if(remote_participant->Participant2ParticipantKeyMaterial.size() == 0)
        {
            logWarning(SECURITY_CRYPTO, "No key material yet");
            continue;
        }

        //Update the key if needed
        if((update_specific_keys || remote_participant->session_id != local_participant->session_id) &&
                (*remote_participant != *local_participant))
        {
            //Update triggered!
            remote_participant->session_id = local_participant->session_id;
            remote_participant->SessionKey = compute_sessionkey(
                    remote_participant->Participant2ParticipantKeyMaterial.at(0).master_receiver_specific_key,
                    remote_participant->Participant2ParticipantKeyMaterial.at(0).master_salt,
                    remote_participant->session_id);
        }

        //Obtain MAC using ReceiverSpecificKey and the same Initialization Vector as before
        int actual_size = 0, final_size = 0;
        EVP_CIPHER_CTX* e_ctx = EVP_CIPHER_CTX_new();
        if(local_participant->transformation_kind == std::array<uint8_t, 4>{CRYPTO_TRANSFORMATION_KIND_AES128_GCM} ||
                local_participant->transformation_kind == std::array<uint8_t, 4>{CRYPTO_TRANSFORMATION_KIND_AES128_GMAC})
        {
            if(!EVP_EncryptInit(e_ctx, EVP_aes_128_gcm(), (const unsigned char*)(remote_participant->SessionKey.data()),
                        initialization_vector.data()))
            {
                logError(SECURITY_CRYPTO, "Unable to encode the payload. EVP_EncryptInit function returns an error");
                //TODO(Ricardo) Free context;
                continue;
            }
        }
        else if(local_participant->transformation_kind == std::array<uint8_t, 4>{CRYPTO_TRANSFORMATION_KIND_AES256_GCM} ||
                local_participant->transformation_kind == std::array<uint8_t, 4>{CRYPTO_TRANSFORMATION_KIND_AES256_GMAC})
        {
            if(!EVP_EncryptInit(e_ctx, EVP_aes_256_gcm(), (const unsigned char*)(remote_participant->SessionKey.data()),
                        initialization_vector.data()))
            {
                logError(SECURITY_CRYPTO, "Unable to encode the payload. EVP_EncryptInit function returns an error");
                continue;
            }
        }
        if(!EVP_EncryptUpdate(e_ctx, NULL, &actual_size, tag.common_mac.data(), 16))
        {
            logError(SECURITY_CRYPTO, "Unable to create authentication for the datawriter submessage. EVP_EncryptUpdate function returns an error");
            continue;
        }
        if(!EVP_EncryptFinal(e_ctx, NULL, &final_size))
        {
            logError(SECURITY_CRYPTO, "Unable to create authentication for the datawriter submessage. EVP_EncryptFinal function returns an error");
            continue;
        }
        serializer << remote_participant->Participant2ParticipantKeyMaterial.at(0).receiver_specific_key_id;
        EVP_CIPHER_CTX_ctrl(e_ctx, EVP_CTRL_GCM_GET_TAG, 16, serializer.getCurrentPosition());
        serializer.jump(16);
        EVP_CIPHER_CTX_free(e_ctx);

        ++length;
    }

    eprosima::fastcdr::Cdr::state current_state = serializer.getState();
    serializer.setState(length_state);
    serializer << length;
    serializer.setState(current_state);
    return true;
}

SecureDataHeader AESGCMGMAC_Transform::deserialize_SecureDataHeader(eprosima::fastcdr::Cdr& decoder)
{
    SecureDataHeader header;

    decoder >> header.transform_identifier.transformation_kind >> header.transform_identifier.transformation_key_id >>
        header.session_id >> header.initialization_vector_suffix;

    return header;
}

bool AESGCMGMAC_Transform::deserialize_SecureDataBody(eprosima::fastcdr::Cdr& decoder,
        eprosima::fastcdr::Cdr::state& body_state, SecureDataTag& tag, const uint32_t body_length,
        const std::array<uint8_t, 4> transformation_kind,
        const std::array<uint8_t,32>& session_key, const std::array<uint8_t, 12>& initialization_vector,
        octet* plain_buffer, uint32_t& plain_buffer_len)
{
    eprosima::fastcdr::Cdr::state current_state = decoder.getState();
    decoder.setState(body_state);

    EVP_CIPHER_CTX *d_ctx = EVP_CIPHER_CTX_new();
    int cipher_block_size = 0, actual_size = 0, final_size = 0;
    octet* output_buffer = nullptr;

    if(transformation_kind == std::array<uint8_t,4>{CRYPTO_TRANSFORMATION_KIND_AES128_GCM} ||
            transformation_kind == std::array<uint8_t,4>{CRYPTO_TRANSFORMATION_KIND_AES128_GMAC})
    {
        if(!EVP_DecryptInit(d_ctx, EVP_aes_128_gcm(), (const unsigned char *)session_key.data(), initialization_vector.data()))
        {
            logError(SECURITY_CRYPTO, "Unable to decode the payload. EVP_DecryptInit function returns an error");
            return false;
        }

        cipher_block_size = EVP_CIPHER_block_size(EVP_aes_128_gcm());

        if(transformation_kind == std::array<uint8_t,4>{CRYPTO_TRANSFORMATION_KIND_AES128_GCM})
        {
            output_buffer = plain_buffer;
        }
    }
    if(transformation_kind == std::array<uint8_t,4>{CRYPTO_TRANSFORMATION_KIND_AES256_GCM} ||
            transformation_kind == std::array<uint8_t,4>{CRYPTO_TRANSFORMATION_KIND_AES256_GMAC})
    {
        if(!EVP_DecryptInit(d_ctx, EVP_aes_256_gcm(), (const unsigned char *)session_key.data(), initialization_vector.data()))
        {
            logError(SECURITY_CRYPTO, "Unable to decode the payload. EVP_DecryptInit function returns an error");
            return false;
        }

        cipher_block_size = EVP_CIPHER_block_size(EVP_aes_256_gcm());

        if(transformation_kind == std::array<uint8_t,4>{CRYPTO_TRANSFORMATION_KIND_AES256_GCM})
        {
            output_buffer = plain_buffer;
        }
    }

    // Check plain_payload contains enough memory to cypher.
    // - EVP_DecryptUpdate needs at maximum: body_length + cipher_block_size.
    if(output_buffer != nullptr && (plain_buffer_len < (body_length + cipher_block_size)))
    {
        logError(SECURITY_CRYPTO, "Not enough memory to decode payload");
        return false;
    }

    if(!EVP_DecryptUpdate(d_ctx, output_buffer, &actual_size,
                (unsigned char*)decoder.getCurrentPosition(), body_length))
    {
        logError(SECURITY_CRYPTO, "Unable to decode the payload. EVP_DecryptUpdate function returns an error");
        return false;
    }

    EVP_CIPHER_CTX_ctrl(d_ctx, EVP_CTRL_GCM_SET_TAG, AES_BLOCK_SIZE, tag.common_mac.data());

    if(!EVP_DecryptFinal(d_ctx, output_buffer, &final_size))
    {
        logError(SECURITY_CRYPTO, "Unable to decode the payload. EVP_DecryptFinal function returns an error");
        return false;
    }
    EVP_CIPHER_CTX_free(d_ctx);

    plain_buffer_len = actual_size + final_size;

    decoder.setState(current_state);

    // Align submessage to 4.
    size_t alignment = decoder.alignment(decoder.getCurrentPosition() - decoder.getBufferPointer(), sizeof(int32_t));
    for(size_t count = 0; count != alignment; ++count)
    {
        uint8_t c = 0;
        decoder >> c;
    }

    return true;
}

bool AESGCMGMAC_Transform::predeserialize_SecureDataBody(eprosima::fastcdr::Cdr& decoder, uint32_t& body_length,
        uint32_t& body_align)
{
    octet secure_submsg_id = 0, flags = 0;
    uint16_t body_length_short;

    decoder >> secure_submsg_id;

    if(secure_submsg_id != SecureBodySubmessage)
    {
        logError(SECURITY_CRYPTO, "Expected SecureDataBody submsg id");
        return false;
    }

    decoder >> flags;

    if(flags & BIT(0))
    {
        decoder.changeEndianness(eprosima::fastcdr::Cdr::Endianness::LITTLE_ENDIANNESS);
    }
    else
    {
        decoder.changeEndianness(eprosima::fastcdr::Cdr::Endianness::BIG_ENDIANNESS);
    }

    decoder >> body_length_short;
    body_length = body_length_short;

    // Align submessage to 4.
    body_align = static_cast<uint32_t>(decoder.alignment((decoder.getCurrentPosition() + body_length) -
                decoder.getBufferPointer(), sizeof(int32_t)));

    return true;
}

bool AESGCMGMAC_Transform::deserialize_SecureDataTag(eprosima::fastcdr::Cdr& decoder, SecureDataTag& tag,
        const CryptoTransformKind& transformation_kind,
        const CryptoTransformKeyId& receiver_specific_key_id, const std::array<uint8_t, 32>& receiver_specific_key,
        const std::array<uint8_t,32>& master_salt, const std::array<uint8_t,12>& initialization_vector,
        const uint32_t session_id, SecurityException& exception)
{
    decoder >> tag.common_mac;

    uint32_t sequence_length = 0;
    decoder >> sequence_length;

    if(sequence_length > 0)
    {
        bool mac_found = false;

        // TODO(Ricardo) Review SessionReceiverSpecificKey (248pag)
        uint32_t count = 0;
        for(; !mac_found && count < sequence_length; ++count)
        {
            decoder >> tag.receiver_mac_key_id >> tag.receiver_mac;

            if(receiver_specific_key_id == tag.receiver_mac_key_id)
            {
                mac_found = true;
            }
        }

        decoder.jump((sequence_length - count) * (tag.receiver_mac_key_id.size() + tag.receiver_mac.size()));

        if(!mac_found)
        {
            logWarning(SECURITY_CRYPTO,"Unable to authenticate the message: message does not target this Participant");
            exception = SecurityException("Message does not contain a suitable specific MAC for the receiving Participant");
            return false;
        }

        //Auth message - The point is that we cannot verify the authorship of the message with our receiver_specific_key the message could be crafted
        EVP_CIPHER_CTX* d_ctx = EVP_CIPHER_CTX_new();
        const EVP_CIPHER* d_cipher = nullptr;

        int actual_size = 0, final_size = 0;

        //Get ReceiverSpecificSessionKey
        std::array<uint8_t,32> specific_session_key = compute_sessionkey(receiver_specific_key,
                master_salt, session_id);

        //Verify specific MAC
        if(transformation_kind == std::array<uint8_t, 4>{CRYPTO_TRANSFORMATION_KIND_AES128_GCM} ||
                transformation_kind == std::array<uint8_t,4>{CRYPTO_TRANSFORMATION_KIND_AES128_GMAC})
        {
            d_cipher = EVP_aes_128_gcm();
        }
        else if(transformation_kind == std::array<uint8_t,4>{CRYPTO_TRANSFORMATION_KIND_AES256_GCM} ||
                transformation_kind == std::array<uint8_t,4>{CRYPTO_TRANSFORMATION_KIND_AES256_GMAC})
        {
            d_cipher = EVP_aes_256_gcm();
        }
        else
        {
            logError(SECURITY_CRYPTO, "Invalid transformation kind)");
            return false;
        }

        if(!EVP_DecryptInit(d_ctx, d_cipher, (const unsigned char *)specific_session_key.data(),
                    initialization_vector.data()))
        {
            logError(SECURITY_CRYPTO, "Unable to authenticate the message. EVP_DecryptInit function returns an error");
            return false;
        }

        if(!EVP_CIPHER_CTX_ctrl(d_ctx, EVP_CTRL_GCM_SET_TAG, 16, tag.receiver_mac.data()))
        {
            logError(SECURITY_CRYPTO, "Unable to authenticate the message. EVP_CIPHER_CTX_ctrl function returns an error");
            return false;
        }

        if(!EVP_DecryptUpdate(d_ctx, NULL, &actual_size, tag.common_mac.data(), 16))
        {
            logError(SECURITY_CRYPTO, "Unable to authenticate the message. EVP_DecryptUpdate function returns an error");
            return false;
        }

        if(!EVP_DecryptFinal_ex(d_ctx, NULL, &final_size))
        {
            logError(SECURITY_CRYPTO, "Unable to authenticate the message. EVP_DecryptFinal_ex function returns an error");
            return false;
        }

        // TODO(Ricardo) No freed in errors.
        EVP_CIPHER_CTX_free(d_ctx);
    }

    return true;
}

CONSTEXPR uint32_t srtps_prefix_length = 4;
// 4 bytes to serialize length of the body.
CONSTEXPR uint32_t srtps_postfix_length = 4;
CONSTEXPR uint32_t sec_prefix_length = 4;
// 4 bytes to serialize length of the body.
CONSTEXPR uint32_t sec_postfix_length = 4;
CONSTEXPR uint32_t aesgcmgmac_header_length = 20;
CONSTEXPR uint32_t aesgcmgmac_body_length_attr = 4 + 3 /*possible alignment*/;
CONSTEXPR uint32_t aesgcmgmac_common_tag = 16;

uint32_t AESGCMGMAC_Transform::calculate_extra_size_for_rtps_message(uint32_t number_discovered_participants) const
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

uint32_t AESGCMGMAC_Transform::calculate_extra_size_for_rtps_submessage(uint32_t number_discovered_readers) const
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

uint32_t AESGCMGMAC_Transform::calculate_extra_size_for_encoded_payload(uint32_t number_discovered_readers) const
{
    uint32_t calculate = aesgcmgmac_header_length +
        aesgcmgmac_body_length_attr +
        AES_BLOCK_SIZE + // Padding
        aesgcmgmac_common_tag;

    // Minimum like there is 10 participants.
    calculate += number_discovered_readers > 10 ? number_discovered_readers * 20 : 200;

    return calculate;
}
