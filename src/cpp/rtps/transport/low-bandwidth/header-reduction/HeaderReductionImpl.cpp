// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "HeaderReductionImpl.h"
#include <fastrtps/rtps/messages/RTPS_messages.h>
#include <fastrtps/rtps/common/Guid.h>
#include <fastrtps/rtps/messages/CDRMessage.h>
#include <fastrtps/rtps/common/CDRMessage_t.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * \brief This auxiliary structure is used to move data that it's wanted between buffers.
 * The data can be moved in the same buffer too.
 */
struct HRBufferPointer
{

    HRBufferPointer()
        : buffer_new(nullptr)
        , buffer_old(nullptr)
        , length(0)
        , final_length(0)
    {
    }

    HRBufferPointer(
            fastrtps::rtps::octet* send_buf,
            uint32_t send_len)
        : buffer_new(send_buf)
        , buffer_old(send_buf)
        , length(send_len)
        , final_length(send_len)
    {
    }

    HRBufferPointer(
            fastrtps::rtps::octet* output_buf,
            fastrtps::rtps::octet* input_buf,
            const uint32_t input_length)
        : buffer_new(output_buf)
        , buffer_old(input_buf)
        , length(input_length)
        , final_length(input_length)
    {
    }

    void pointer_jump(
            const uint32_t number_of_bytes)
    {
        buffer_old = &buffer_old[number_of_bytes];
        length -= number_of_bytes;
        final_length -= number_of_bytes;
    }

    void pointer_move(
            const uint32_t number_of_bytes)
    {
        if (buffer_old != buffer_new)
        {
            memcpy(buffer_new, buffer_old, number_of_bytes);
        }

        buffer_old = &buffer_old[number_of_bytes];
        buffer_new = &buffer_new[number_of_bytes];
        length -= number_of_bytes;
    }

    void pointer_add(
            const fastrtps::rtps::octet* elements,
            const uint32_t number_of_bytes)
    {
        if (elements != nullptr)
        {
            memcpy(buffer_new, elements, number_of_bytes);
            buffer_new = &buffer_new[number_of_bytes];
            final_length += number_of_bytes;
        }
    }

    /**
     * \brief This function reduces some blocks of the same size.
     *
     * \param number_of_blocks Number of blocks that will be reduced.
     * \param size_of_block The size of block. Accepted values: 32, 16 or 8.
     * \param reduction_bytes Array that contains the information of how many bytes each block will be reduced.
     * This array has to have the same size that number of blocks.
     * \param endianess With value true, each block is in LITTLE_ENDIAN. With value false each value is in BIG_ENDIAN.
     */
    void pack_bytes(
            const uint32_t number_of_blocks,
            const uint32_t size_of_blocks,
            const uint32_t* reduction_bytes,
            bool endianess)
    {
        uint32_t necessaryBytes = 0, bytesCount = 0, count = 0, writeCount = 0, carry = 0, auxReduction = 0;

        fastrtps::rtps::CDRMessage_t auxReader(0);
        fastrtps::rtps::CDRMessage_t auxWriter(0);

        auxWriter.wraps = auxReader.wraps = true;
        auxWriter.buffer = auxReader.buffer = buffer_old;
        auxWriter.max_size = auxReader.max_size = auxReader.length = this->length;
        auxWriter.pos = auxReader.pos = 0;
        auxWriter.length = 0;
        auxWriter.msg_endian = fastrtps::rtps::BIGEND;
        auxReader.msg_endian = endianess ? fastrtps::rtps::LITTLEEND : fastrtps::rtps::BIGEND;

        // Calc how many bytes are needed.
        for (; count < number_of_blocks; count++)
        {
            bytesCount += reduction_bytes[count];
        }

        necessaryBytes = bytesCount / 8;
        if (bytesCount % 8 != 0)
        {
            necessaryBytes++;
        }

        count = 0;
        while (count < number_of_blocks)
        {
            uint32_t bites = size_of_blocks;
            uint32_t auxBytes = 0;

            while ((count < number_of_blocks) && (bites > 0))
            {
                if (reduction_bytes[count] > 0)
                {
                    if (carry == 0)
                    {
                        auxReduction = reduction_bytes[count];
                    }
                    else
                    {
                        auxReduction = carry;
                        carry = 0;
                    }

                    if (bites >= auxReduction)
                    {
                        if (size_of_blocks == 32)
                        {
                            uint32_t tmp;
                            fastrtps::rtps::CDRMessage::readUInt32(&auxReader, &tmp);
                            auxBytes += (tmp & (0xFFFFFFFF >> (size_of_blocks - reduction_bytes[count]))) <<
                                    (bites - auxReduction);
                        }
                        else if (size_of_blocks == 16)
                        {
                            uint16_t tmp;
                            fastrtps::rtps::CDRMessage::readUInt16(&auxReader, &tmp);
                            auxBytes += (tmp & (0xFFFF >> (size_of_blocks - reduction_bytes[count]))) <<
                                    (bites - auxReduction);
                        }
                        else if (size_of_blocks == 8)
                        {
                            auxBytes += (buffer_old[count] & (0xFF >> (size_of_blocks - reduction_bytes[count]))) <<
                                    (bites - auxReduction);
                        }
                        else
                        {
                            // printf("ERROR<%s>: Bad block size %u\n", METHOD_NAME, size_of_blocks);
                        }
                        bites -= auxReduction;
                        count++;
                    }
                    else
                    {
                        carry = auxReduction - bites;
                        if (size_of_blocks == 32)
                        {
                            uint32_t tmp;
                            fastrtps::rtps::CDRMessage::readUInt32(&auxReader, &tmp);
                            auxReader.pos -= sizeof(tmp);
                            auxBytes += (tmp & (0xFFFFFFFF >> (size_of_blocks - reduction_bytes[count]))) >> carry;
                        }
                        else if (size_of_blocks == 16)
                        {
                            uint16_t tmp;
                            fastrtps::rtps::CDRMessage::readUInt16(&auxReader, &tmp);
                            auxReader.pos -= sizeof(tmp);
                            auxBytes += (tmp & (0xFFFF >> (size_of_blocks - reduction_bytes[count]))) >> carry;
                        }
                        else if (size_of_blocks == 8)
                        {
                            auxBytes += (buffer_old[count] & (0xFF >> (size_of_blocks - reduction_bytes[count]))) >>
                                    carry;
                        }
                        else
                        {
                            //printf("ERROR<%s>: Bad block size %u\n", METHOD_NAME, size_of_blocks);
                        }
                        bites = 0;
                    }
                }
                else
                {
                    count++;
                }
            }

            // Salvamos lo comprimido en big-endian.
            if (size_of_blocks == 32)
            {
                fastrtps::rtps::CDRMessage::addUInt32(&auxWriter, auxBytes);
            }
            else if (size_of_blocks == 16)
            {
                fastrtps::rtps::CDRMessage::addUInt16(&auxWriter, (uint16_t) auxBytes);
            }
            else if (size_of_blocks == 8)
            {
                buffer_old[writeCount] = (fastrtps::rtps::octet)auxBytes;
            }
            else
            {
                //printf("ERROR<%s>: Bad block size %u\n", METHOD_NAME, size_of_blocks);
            }

            writeCount++;
        }

        pointer_move(necessaryBytes);
        pointer_jump((number_of_blocks * (size_of_blocks / 8)) - necessaryBytes);
    }

    // Cuidado con la alineaci?n.
    void unpack_bytes(
            const uint32_t number_of_blocks,
            const uint32_t size_of_blocks,
            const uint32_t* reduction_bytes,
            bool endianess)
    {
        uint32_t necessaryBytes = 0, bytesCount = 0;
        int count = 0;

        fastrtps::rtps::CDRMessage_t auxWriter(0);
        auxWriter.wraps = true;
        auxWriter.msg_endian = endianess ? fastrtps::rtps::LITTLEEND : fastrtps::rtps::BIGEND;
        auxWriter.buffer = buffer_new;
        auxWriter.pos = auxWriter.length = 0;
        auxWriter.max_size = final_length;

        // Caculamos cuantos bytes necesitamos.
        for (; count < (int)number_of_blocks; count++)
        {
            bytesCount += reduction_bytes[count];
        }

        necessaryBytes = bytesCount / 8;
        if (bytesCount % 8 != 0)
        {
            necessaryBytes++;
        }

        count = number_of_blocks - 1;
        while (count >= 0)
        {
            uint32_t auxBytes = 0, block = 0, fblock = 0;
            uint32_t mask = 0xFFFFFFFF;

            if (reduction_bytes[count] > 0)
            {
                block = (bytesCount - reduction_bytes[count]) / 8;
                fblock = (bytesCount / 8) - ((bytesCount % 8) == 0 ? 1 : 0);

                uint32_t count2 = block;
                while (count2 <= fblock)
                {
                    uint32_t value = buffer_old[count2] & 0xFF;
                    int desp = reduction_bytes[count] - (((count2 + 1) * 8) - (bytesCount - reduction_bytes[count]));

                    // La informaci?n comprimida siempre estar? en big-endian.
                    if (desp >= 0)
                    {
                        auxBytes += value << desp;
                    }
                    else
                    {
                        auxBytes += value >> -desp;
                    }

                    count2++;
                }

                auxBytes &= mask >> (32 - reduction_bytes[count]);

            }
            else
            {
                auxBytes = 0x1;
            }

            if (size_of_blocks == 32)
            {
                auxWriter.pos = auxWriter.length = count * sizeof(uint32_t);
                fastrtps::rtps::CDRMessage::addUInt32(&auxWriter, auxBytes);
            }
            else if (size_of_blocks == 16)
            {
                auxWriter.pos = auxWriter.length = count * sizeof(uint16_t);
                fastrtps::rtps::CDRMessage::addUInt16(&auxWriter, (uint16_t) auxBytes);
            }
            else if (size_of_blocks == 8)
            {
                buffer_new[count] = (fastrtps::rtps::octet)auxBytes;
            }
            else
            {
                //printf("ERROR<%s>: Bad block size %u\n", METHOD_NAME, size_of_blocks);
            }

            bytesCount -= reduction_bytes[count];
            count--;
        }

        buffer_old = &buffer_old[necessaryBytes];
        length -= necessaryBytes;

        if (size_of_blocks == 32)
        {
            buffer_new += (number_of_blocks * sizeof(uint32_t));
        }
        else if (size_of_blocks == 16)
        {
            buffer_new += (number_of_blocks * sizeof(uint16_t));
        }
        else if (size_of_blocks == 8)
        {
            buffer_new += number_of_blocks;
        }
        else
        {
            //printf("ERROR<%s>: Bad block size %u\n", METHOD_NAME, size_of_blocks);
        }

        final_length += (number_of_blocks * (size_of_blocks / 8)) - necessaryBytes; // 32 bits each block..
    }

    void pack_entity_id(
            const uint32_t* reduce_entitiesId)
    {
        uint32_t readerId = 0, reader_entityKind = 0, writerId = 0, writer_entityKind = 0;

        if (reduce_entitiesId != nullptr)
        {
            fastrtps::rtps::CDRMessage_t auxReader(0);
            fastrtps::rtps::CDRMessage_t auxWriter(0);

            auxWriter.wraps = auxReader.wraps = true;
            auxWriter.buffer = auxReader.buffer = buffer_old;
            auxWriter.max_size = auxReader.max_size = auxReader.length = length;
            auxWriter.pos = auxReader.pos = 0;
            auxWriter.length = 0;
            auxWriter.msg_endian = auxReader.msg_endian = fastrtps::rtps::BIGEND;

            fastrtps::rtps::CDRMessage::readUInt32(&auxReader, &readerId);
            reader_entityKind = ((readerId & 0x000000C0) >> 3) + (readerId & 0x00000007);
            fastrtps::rtps::CDRMessage::readUInt32(&auxReader, &writerId);
            writer_entityKind = ((writerId & 0x000000C0) >> 3) + (writerId & 0x00000007);

            if ((readerId & 0x000000C0) == 0x000000C0)
            {
                switch (readerId)
                {
                    case ENTITYID_P2P_BUILTIN_RTPSParticipant_MESSAGE_READER:
                        readerId = 0 << 5;
                        break;
                    case ENTITYID_SPDP_BUILTIN_RTPSParticipant_READER:
                        readerId = 1 << 5;
                        break;
                    case 0x000300C7:
                        readerId = 2 << 5;
                        break;
                    case 0x000400C7:
                        readerId = 3 << 5;
                        break;
                    case ENTITYID_SEDP_BUILTIN_TOPIC_READER:
                        readerId = 4 << 5;
                        break;
                    case ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER:
                        readerId = 5 << 5;
                        break;
                    case ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER:
                        readerId = 6 << 5;
                        break;
                    default:
                        //printf("ERROR<%s>: Reader RTPS object idenfier 0x%X is unknown\n", METHOD_NAME, readerId);
                        writerId = 7 << 5;
                        break;
                }

                readerId += reader_entityKind;
            }
            else
            {
                readerId = ((readerId & 0xFFFFFF00) >> 3) + reader_entityKind;
            }

            fastrtps::rtps::CDRMessage::addUInt32(&auxWriter, readerId);

            if ((writerId & 0x000000C0) == 0x000000C0)
            {
                switch (writerId)
                {
                    case ENTITYID_P2P_BUILTIN_RTPSParticipant_MESSAGE_WRITER:
                        writerId = 0 << 5;
                        break;
                    case ENTITYID_SPDP_BUILTIN_RTPSParticipant_WRITER:
                        writerId = 1 << 5;
                        break;
                    case 0x000300C2:
                        writerId = 2 << 5;
                        break;
                    case 0x000400C2:
                        writerId = 3 << 5;
                        break;
                    case ENTITYID_SEDP_BUILTIN_TOPIC_WRITER:
                        writerId = 4 << 5;
                        break;
                    case ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER:
                        writerId = 5 << 5;
                        break;
                    case ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER:
                        writerId = 6 << 5;
                        break;
                    default:
                        //printf("ERROR<%s>: Writer RTPS object idenfier 0x%X is unknown\n", METHOD_NAME, writerId);
                        writerId = 7 << 5;
                        break;
                }

                writerId += writer_entityKind;
            }
            else
            {
                writerId = ((writerId & 0xFFFFFF00) >> 3) + writer_entityKind;
            }

            fastrtps::rtps::CDRMessage::addUInt32(&auxWriter, writerId);

            pack_bytes(2, 32, reduce_entitiesId, false);
        }
        else
        {
            //printf("ERROR<%s>: Bad parameters\n", METHOD_NAME);
        }
    }

    void unpack_entity_id(
            const uint32_t* reduce_entitiesId)
    {
        uint32_t readerId = 0, reader_entityKind = 0, writerId = 0, writer_entityKind = 0;

        if (reduce_entitiesId != nullptr)
        {
            fastrtps::rtps::CDRMessage_t auxReader(0);
            fastrtps::rtps::CDRMessage_t auxWriter(0);

            auxWriter.wraps = auxReader.wraps = true;
            auxWriter.max_size = auxReader.max_size = auxReader.length = length;
            auxWriter.pos = auxReader.pos = 0;
            auxWriter.length = 0;
            auxWriter.msg_endian = auxReader.msg_endian = fastrtps::rtps::BIGEND;

            unpack_bytes(2, 32, reduce_entitiesId, false);

            auxWriter.buffer = auxReader.buffer = (fastrtps::rtps::octet*) &((uint32_t*)buffer_new)[-2];

            fastrtps::rtps::CDRMessage::readUInt32(&auxReader, &readerId);
            reader_entityKind = ((readerId & 0x00000018) << 3) + (readerId & 0x00000007);
            fastrtps::rtps::CDRMessage::readUInt32(&auxReader, &writerId);
            writer_entityKind = ((writerId & 0x00000018) << 3) + (writerId & 0x00000007);

            if ((reader_entityKind & 0x000000C0) == 0x000000C0)
            {
                readerId = readerId >> 5;

                switch (readerId)
                {
                    case 0:
                        readerId = ENTITYID_P2P_BUILTIN_RTPSParticipant_MESSAGE_READER & 0xFFFFFF00;
                        break;
                    case 1:
                        readerId = ENTITYID_SPDP_BUILTIN_RTPSParticipant_READER & 0xFFFFFF00;
                        break;
                    case 2:
                        readerId = 0x000300C7 & 0xFFFFFF00;
                        break;
                    case 3:
                        readerId = 0x000400C7 & 0xFFFFFF00;
                        break;
                    case 4:
                        readerId = ENTITYID_SEDP_BUILTIN_TOPIC_READER & 0xFFFFFF00;
                        break;
                    case 5:
                        readerId = ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER & 0xFFFFFF00;
                        break;
                    case 6:
                        readerId = ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER & 0xFFFFFF00;
                        break;

                    case 7:
                    default:
                        //printf("ERROR<%s>: Reader RTPS object idenfier 0x%X is unknown\n", METHOD_NAME, readerId);
                        readerId = 0;
                        break;
                }

                readerId = readerId + reader_entityKind;
            }
            else
            {
                readerId = ((readerId & 0xFFFFFFE0) << 3) + reader_entityKind;
            }

            fastrtps::rtps::CDRMessage::addUInt32(&auxWriter, readerId);

            if ((writer_entityKind & 0x000000C0) == 0x000000C0)
            {
                writerId = writerId >> 5;

                switch (writerId)
                {
                    case 0:
                        writerId = ENTITYID_P2P_BUILTIN_RTPSParticipant_MESSAGE_WRITER & 0xFFFFFF00;
                        break;
                    case 1:
                        writerId = ENTITYID_SPDP_BUILTIN_RTPSParticipant_WRITER & 0xFFFFFF00;
                        break;
                    case 2:
                        writerId = 0x000300C2 & 0xFFFFFF00;
                        break;
                    case 3:
                        writerId = 0x000400C2 & 0xFFFFFF00;
                        break;
                    case 4:
                        writerId = ENTITYID_SEDP_BUILTIN_TOPIC_WRITER & 0xFFFFFF00;
                        break;
                    case 5:
                        writerId = ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER & 0xFFFFFF00;
                        break;
                    case 6:
                        writerId = ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER & 0xFFFFFF00;
                        break;

                    case 7:
                    default:
                        //printf("ERROR<%s>: Writer RTPS object idenfier 0x%X is unknown\n", METHOD_NAME, writerId);
                        writerId = 0;
                        break;
                }

                writerId = writerId + writer_entityKind;
            }
            else
            {
                writerId = ((writerId & 0xFFFFFFE0) << 3) + writer_entityKind;
            }

            fastrtps::rtps::CDRMessage::addUInt32(&auxWriter, writerId);
        }
        else
        {
            //printf("ERROR<%s>: Bad parameters\n", METHOD_NAME);
        }
    }

    void pointer_finalize()
    {
        if (buffer_old != buffer_new)
        {
            memcpy(buffer_new, buffer_old, length);
        }

        buffer_old = &buffer_old[length];
        buffer_new = &buffer_new[length];
        length = 0;
    }

    /**
     * \brief Pointer to the position of the buffer where the data will be moved from other position of the buffer (msgOld)
     * or will be added from external buf..
     */
    fastrtps::rtps::octet* buffer_new;
    /**
     * \brief Pointer to the position of the buffer where the data will be moved to other position of the buffer (msgNew)
     * or will be jumped without move the data..
     */
    fastrtps::rtps::octet* buffer_old;
    /**
     * \brief Length of the data that it's still not used and it's pointed by msgOld. When the work finished this value should be 0.
     */
    uint32_t length;
    /**
     * \brief When the work finished, this variable should store the length of the new data pointed by msgNew.
     */
    uint32_t final_length;
};

bool HeaderReduction_Reduce(
        fastrtps::rtps::octet* dest_buffer,
        const fastrtps::rtps::octet* src_buffer,
        uint32_t& buffer_length,
        const HeaderReductionOptions& options)
{
    if (dest_buffer != nullptr && src_buffer != nullptr && buffer_length >= RTPSMESSAGE_HEADER_SIZE &&
            memcmp(src_buffer, "RTPS", 4) == 0)
    {
        memcpy(dest_buffer, src_buffer, buffer_length);
        fastrtps::rtps::octet* buffer = dest_buffer;

        HRBufferPointer buf(buffer, buffer_length);

        if (options.rtps_header.eliminate_protocol == true)
        {
            buf.pointer_jump(HRCONFIG_RTPS_HEADER_PROTOCOL_SIZE - 1);
            // Set 'L' to know that was reduced by this plugin.
            buf.buffer_old[0] = (fastrtps::rtps::octet) 'L';
            buf.pointer_move(1);
        }
        else
        {
            // Set 'LTPS' to know that was reduced by this plugin.
            buf.buffer_old[0] = 'L';
            buf.pointer_move(HRCONFIG_RTPS_HEADER_PROTOCOL_SIZE);
        }

        if (options.rtps_header.eliminate_version == true)
        {
            buf.pointer_jump(HRCONFIG_RTPS_HEADER_VERSION_SIZE);
        }
        else
        {
            buf.pointer_move(HRCONFIG_RTPS_HEADER_VERSION_SIZE);
        }

        if (options.rtps_header.eliminate_vendorId == true)
        {
            buf.pointer_jump(HRCONFIG_RTPS_HEADER_VENDORID_SIZE);
        }
        else
        {
            buf.pointer_move(HRCONFIG_RTPS_HEADER_VENDORID_SIZE);
        }

        if (options.rtps_header.reduce_guidPrefix[0] != 32 ||
                options.rtps_header.reduce_guidPrefix[1] != 32 ||
                options.rtps_header.reduce_guidPrefix[2] != 32)
        {
            buf.pack_bytes(3, 32, options.rtps_header.reduce_guidPrefix, false);
        }
        else
        {
            buf.pointer_move(HRCONFIG_RTPS_HEADER_GUIDPREFIX_SIZE);
        }

        while (buf.length > 0)
        {
            fastrtps::rtps::octet submessageId = buf.buffer_old[0];
            uint16_t submessageSize = 0;
            bool endianess = (0x1 & buf.buffer_old[1]) == 0x1;

            if (options.submessage_header.combine_submessageId_with_flags == true)
            {
                uint32_t reductions[] = { 5, 3 };
                buf.pack_bytes(2, 8, reductions, false);
            }
            else
            {
                buf.pointer_move(HRCONFIG_SUBMESSAGE_HEADER_ID_SIZE + HRCONFIG_SUBMESSAGE_HEADER_FLAGS_SIZE);
            }

            submessageSize = endianess ? buf.buffer_old[1] : buf.buffer_old[0];
            submessageSize <<= 8;
            submessageSize |= endianess ? buf.buffer_old[0] : buf.buffer_old[1];

            buf.pointer_move(2 /*octetsToNextHeader*/);

            if (submessageId == fastrtps::rtps::DATA || submessageId == fastrtps::rtps::DATA_FRAG)
            {
                if (options.submessage_body.eliminate_extraFlags == true)
                {
                    buf.pointer_jump(HRCONFIG_SUBMESSAGE_BODY_EXTRAFLAGS_SIZE);
                    buf.pointer_move(HRCONFIG_SUBMESSAGE_BODY_OCTETSTOINLINEQOS_SIZE);
                }
                else
                {
                    buf.pointer_move(
                        HRCONFIG_SUBMESSAGE_BODY_EXTRAFLAGS_SIZE +
                        HRCONFIG_SUBMESSAGE_BODY_OCTETSTOINLINEQOS_SIZE);
                }

                submessageSize -= 4; // extraFlags & octetsOnInlineQos
            }

            if (submessageId == fastrtps::rtps::ACKNACK || submessageId == fastrtps::rtps::HEARTBEAT ||
                    submessageId == fastrtps::rtps::GAP
                    || submessageId == fastrtps::rtps::NACK_FRAG || submessageId == fastrtps::rtps::HEARTBEAT_FRAG
                    || submessageId == fastrtps::rtps::DATA || submessageId == fastrtps::rtps::DATA_FRAG)
            {
                if (options.submessage_body.reduce_entitiesId[0] != 32 ||
                        options.submessage_body.reduce_entitiesId[1] != 32)
                {
                    buf.pack_entity_id(options.submessage_body.reduce_entitiesId);
                }
                else
                {
                    buf.pointer_move(HRCONFIG_SUBMESSAGE_BODY_ENTITIESID_SIZE);
                }

                submessageSize -= 8; // readerEntityId & writerEntityId

                if (options.submessage_body.reduce_sequenceNumber != 64)
                {
                    if (options.submessage_body.reduce_sequenceNumber > 32)
                    {
                        uint32_t reduction = options.submessage_body.reduce_sequenceNumber - 32;
                        buf.pack_bytes(1, 32, &reduction, endianess);
                    }
                    else
                    {
                        buf.pointer_jump(4);
                    }

                    if (options.submessage_body.reduce_sequenceNumber < 32)
                    {
                        buf.pack_bytes(1, 32, &options.submessage_body.reduce_sequenceNumber, endianess);
                    }
                    else
                    {
                        buf.pointer_move(4);
                    }
                }
                else
                {
                    buf.pointer_move(HRCONFIG_SUBMESSAGE_BODY_SEQUENCENUMBER_SIZE);
                }

                submessageSize -= 8; // SequenceNumber
            }

            buf.pointer_move(submessageSize);
        }

        buf.pointer_finalize();
        buffer_length = buf.final_length;
        return true;
    }

    // TODO: log error
    // printf("ERROR<%s>: Bad parameters\n", METHOD_NAME);
    return false;
}

bool HeaderReduction_Recover(
        fastrtps::rtps::octet* newBuffer,
        const fastrtps::rtps::octet* oldBuffer,
        uint32_t& messageLength,
        const HeaderReductionOptions& options)
{
    if (newBuffer != nullptr && oldBuffer != nullptr)
    {
        HRBufferPointer buf(newBuffer, (fastrtps::rtps::octet*) oldBuffer, messageLength);

        // Check 'L' to know that was reduced by this plugin.
        if (buf.buffer_old[0] != 'L')
        {
            return false;
        }

        if (options.rtps_header.eliminate_protocol == true)
        {
            const fastrtps::rtps::octet protocol[] = "RTPS";
            buf.pointer_add(protocol, HRCONFIG_RTPS_HEADER_PROTOCOL_SIZE);
            // Jump 'L';
            buf.buffer_old++;
            buf.length--;
            buf.final_length--;
        }
        else
        {
            buf.buffer_old[0] = 'R';
            buf.pointer_move(HRCONFIG_RTPS_HEADER_PROTOCOL_SIZE);
        }

        if (options.rtps_header.eliminate_version == true)
        {
            const fastrtps::rtps::octet version[2] =
            { fastrtps::rtps::c_ProtocolVersion.m_major,
              fastrtps::rtps::c_ProtocolVersion.m_minor };
            buf.pointer_add(version, HRCONFIG_RTPS_HEADER_VERSION_SIZE);
        }
        else
        {
            buf.pointer_move(HRCONFIG_RTPS_HEADER_VERSION_SIZE);
        }

        if (options.rtps_header.eliminate_vendorId == true)
        {
            const fastrtps::rtps::octet vendor[2] =
            { fastrtps::rtps::c_VendorId_eProsima[0], fastrtps::rtps::c_VendorId_eProsima[1] };
            buf.pointer_add(vendor, HRCONFIG_RTPS_HEADER_VENDORID_SIZE);
        }
        else
        {
            buf.pointer_move(HRCONFIG_RTPS_HEADER_VENDORID_SIZE);
        }

        if (options.rtps_header.reduce_guidPrefix[0] != 32 ||
                options.rtps_header.reduce_guidPrefix[1] != 32 ||
                options.rtps_header.reduce_guidPrefix[2] != 32)
        {
            buf.unpack_bytes(3, 32, options.rtps_header.reduce_guidPrefix, false);
        }
        else
        {
            buf.pointer_move(HRCONFIG_RTPS_HEADER_GUIDPREFIX_SIZE);
        }

        while (buf.length > 0)
        {
            fastrtps::rtps::octet submessageId = 0;
            uint16_t submessageSize = 0;
            bool endianess;

            if (options.submessage_header.combine_submessageId_with_flags == true)
            {
                const uint32_t reductions[2] = { 5, 3 };
                buf.unpack_bytes(2, 8, reductions, false);
            }
            else
            {
                buf.pointer_move(HRCONFIG_SUBMESSAGE_HEADER_ID_SIZE + HRCONFIG_SUBMESSAGE_HEADER_FLAGS_SIZE);
            }

            submessageId = buf.buffer_new[-2];
            endianess = buf.buffer_new[-1] & 0x1;
            submessageSize = endianess ? buf.buffer_old[1] : buf.buffer_old[0];
            submessageSize <<= 8;
            submessageSize |= endianess ? buf.buffer_old[0] : buf.buffer_old[1];

            buf.pointer_move(2 /*octetsToNextHeader*/);

            if (submessageId == fastrtps::rtps::DATA || submessageId == fastrtps::rtps::DATA_FRAG)
            {
                if (options.submessage_body.eliminate_extraFlags == true)
                {
                    fastrtps::rtps::octet extraFlags[2] = { '\0', '\0' };
                    buf.pointer_add(extraFlags, HRCONFIG_SUBMESSAGE_BODY_EXTRAFLAGS_SIZE);
                    buf.pointer_move(HRCONFIG_SUBMESSAGE_BODY_OCTETSTOINLINEQOS_SIZE);
                }
                else
                {
                    buf.pointer_move(
                        HRCONFIG_SUBMESSAGE_BODY_EXTRAFLAGS_SIZE +
                        HRCONFIG_SUBMESSAGE_BODY_OCTETSTOINLINEQOS_SIZE);
                }

                submessageSize -= 4; // extraFlags & octetsOnInlineQos
            }

            if (submessageId == fastrtps::rtps::ACKNACK || submessageId == fastrtps::rtps::HEARTBEAT ||
                    submessageId == fastrtps::rtps::GAP
                    || submessageId == fastrtps::rtps::NACK_FRAG || submessageId == fastrtps::rtps::HEARTBEAT_FRAG
                    || submessageId == fastrtps::rtps::DATA || submessageId == fastrtps::rtps::DATA_FRAG)
            {
                if (options.submessage_body.reduce_entitiesId[0] != 32 ||
                        options.submessage_body.reduce_entitiesId[1] != 32)
                {
                    buf.unpack_entity_id(options.submessage_body.reduce_entitiesId);
                }
                else
                {
                    buf.pointer_move(HRCONFIG_SUBMESSAGE_BODY_ENTITIESID_SIZE);
                }

                submessageSize -= 8;

                if (options.submessage_body.reduce_sequenceNumber != 64)
                {
                    if (options.submessage_body.reduce_sequenceNumber > 32)
                    {
                        uint32_t reduction = options.submessage_body.reduce_sequenceNumber - 32;
                        buf.unpack_bytes(1, 32, &reduction, endianess);
                    }
                    else
                    {
                        fastrtps::rtps::octet seq_empty[4] = { '\0', '\0', '\0', '\0' };
                        buf.pointer_add(seq_empty, 4);
                    }

                    if (options.submessage_body.reduce_sequenceNumber < 32)
                    {
                        buf.unpack_bytes(1, 32, &options.submessage_body.reduce_sequenceNumber, endianess);
                    }
                    else
                    {
                        buf.pointer_move(4);
                    }
                }
                else
                {
                    buf.pointer_move(HRCONFIG_SUBMESSAGE_BODY_SEQUENCENUMBER_SIZE);
                }

                submessageSize -= 8; // SequenceNumber
            }

            buf.pointer_move(submessageSize);
        }

        buf.pointer_finalize();
        messageLength = buf.final_length;
        return true;
    }

    //printf("ERROR<%s>: Bad parameters\n", METHOD_NAME);
    return false;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
