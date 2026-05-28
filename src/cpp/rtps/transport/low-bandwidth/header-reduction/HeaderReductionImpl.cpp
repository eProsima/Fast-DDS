/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#include "HeaderReductionImpl.hpp"

#include <fastcdr/Cdr.h>
#include <fastcdr/FastBuffer.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/messages/RTPS_messages.hpp>

#define RTPSMESSAGE_HEADER_SIZE 20  //header size in bytes

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
            fastdds::rtps::octet* send_buf,
            uint32_t send_len)
        : buffer_new(send_buf)
        , buffer_old(send_buf)
        , length(send_len)
        , final_length(send_len)
    {
    }

    HRBufferPointer(
            fastdds::rtps::octet* output_buf,
            fastdds::rtps::octet* input_buf,
            const uint32_t input_length)
        : buffer_new(output_buf)
        , buffer_old(input_buf)
        , length(input_length)
        , final_length(input_length)
    {
    }

    /*!
     * \brief Jumps a number of bytes on the source buffer.
     *
     * \param[in] number_of_bytes Number of bytes to jump
     */
    void pointer_jump(
            const uint32_t number_of_bytes)
    {
        buffer_old = &buffer_old[number_of_bytes];
        length -= number_of_bytes;
        final_length -= number_of_bytes;
    }

    /*!
     * \brief Moves the content of a number of bytes from the source buffer to the destination buffer.
     *
     * \param[in] number_of_bytes Number of bytes to be moved.
     */
    void pointer_move(
            const uint32_t number_of_bytes)
    {
        if (buffer_old != buffer_new)
        {
            memmove(buffer_new, buffer_old, number_of_bytes);
        }

        buffer_old = &buffer_old[number_of_bytes];
        buffer_new = &buffer_new[number_of_bytes];
        length -= number_of_bytes;
    }

    /*!
     * \brief Adds the content of a number of bytes in the destination buffer.
     *
     * \param[in] elements Bytes to be added.
     * \param[in] Number of bytes
     */
    void pointer_add(
            const fastdds::rtps::octet* elements,
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

        fastcdr::FastBuffer read_buffer(reinterpret_cast<char*>(buffer_old), this->length);
        fastcdr::Cdr read_cdr(read_buffer,
                endianess ? fastcdr::Cdr::Endianness::LITTLE_ENDIANNESS : fastcdr::Cdr::Endianness::BIG_ENDIANNESS,
                fastcdr::CdrVersion::DDS_CDR);
        fastcdr::FastBuffer write_buffer(reinterpret_cast<char*>(buffer_old), this->length);
        fastcdr::Cdr write_cdr(write_buffer, fastcdr::Cdr::Endianness::BIG_ENDIANNESS, fastcdr::CdrVersion::DDS_CDR);

        // Calculate how many bytes are needed.
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
                            read_cdr >> tmp;
                            auxBytes += (tmp & (0xFFFFFFFF >> (size_of_blocks - reduction_bytes[count])))
                                    << (bites - auxReduction);
                        }
                        else if (size_of_blocks == 16)
                        {
                            uint16_t tmp;
                            read_cdr >> tmp;
                            auxBytes += (tmp & (0xFFFF >> (size_of_blocks - reduction_bytes[count])))
                                    << (bites - auxReduction);
                        }
                        else if (size_of_blocks == 8)
                        {
                            auxBytes += (buffer_old[count] & (0xFF >> (size_of_blocks - reduction_bytes[count])))
                                    << (bites - auxReduction);
                        }
                        else
                        {
                            EPROSIMA_LOG_ERROR(RTPS_HEADERREDUCTION_TRANSPORT, "Bad block size " << size_of_blocks);
                        }
                        bites -= auxReduction;
                        count++;
                    }
                    else
                    {
                        carry = auxReduction - bites;
                        if (size_of_blocks == 32)
                        {
                            fastcdr::Cdr::state prev_state(read_cdr);
                            uint32_t tmp;
                            read_cdr >> tmp;
                            const auto last_position {read_cdr.get_current_position()};
                            read_cdr.set_state(prev_state);
                            read_cdr.jump(last_position - read_cdr.get_current_position() - sizeof(tmp));
                            auxBytes += (tmp & (0xFFFFFFFF >> (size_of_blocks - reduction_bytes[count]))) >> carry;
                        }
                        else if (size_of_blocks == 16)
                        {
                            fastcdr::Cdr::state prev_state(read_cdr);
                            uint16_t tmp;
                            read_cdr >> tmp;
                            const auto last_position {read_cdr.get_current_position()};
                            read_cdr.set_state(prev_state);
                            read_cdr.jump(last_position - read_cdr.get_current_position() - sizeof(tmp));
                            auxBytes += (tmp & (0xFFFF >> (size_of_blocks - reduction_bytes[count]))) >> carry;
                        }
                        else if (size_of_blocks == 8)
                        {
                            auxBytes += (buffer_old[count] & (0xFF >> (size_of_blocks - reduction_bytes[count])))
                                    >> carry;
                        }
                        else
                        {
                            EPROSIMA_LOG_ERROR(RTPS_HEADERREDUCTION_TRANSPORT, "Bad block size " << size_of_blocks);
                        }
                        bites = 0;
                    }
                }
                else
                {
                    count++;
                }
            }

            // Store the reduced bytes in big-endian
            if (size_of_blocks == 32)
            {
                write_cdr << auxBytes;
            }
            else if (size_of_blocks == 16)
            {
                write_cdr << static_cast<uint16_t>(auxBytes);
            }
            else if (size_of_blocks == 8)
            {
                buffer_old[writeCount] = (fastdds::rtps::octet)auxBytes;
            }
            else
            {
                EPROSIMA_LOG_ERROR(RTPS_HEADERREDUCTION_TRANSPORT, "Bad block size " << size_of_blocks);
            }

            writeCount++;
        }

        pointer_move(necessaryBytes);
        pointer_jump((number_of_blocks * (size_of_blocks / 8)) - necessaryBytes);
    }

    /**
     * \brief This function expands blocks of the same size that were reduced previously by pack_bytes function.
     *
     * \param number_of_blocks Number of blocks that will be expanded.
     * \param size_of_block The size of block. Accepted values: 32, 16 or 8.
     * \param reduction_bytes Array that contains the information of how many bytes each block were reduced.
     * This array has to have the same size that number of blocks.
     * \param endianess With value true, each block is in LITTLE_ENDIAN. With value false each value is in BIG_ENDIAN.
     */
    void unpack_bytes(
            const uint32_t number_of_blocks,
            const uint32_t size_of_blocks,
            const uint32_t* reduction_bytes,
            bool endianess)
    {
        uint32_t necessaryBytes = 0, bytesCount = 0;
        int count = 0;

        fastcdr::FastBuffer write_buffer(reinterpret_cast<char*>(buffer_new), final_length);
        fastcdr::Cdr write_cdr(write_buffer,
                endianess ? fastcdr::Cdr::Endianness::LITTLE_ENDIANNESS : fastcdr::Cdr::Endianness::BIG_ENDIANNESS,
                fastcdr::CdrVersion::DDS_CDR);

        // Calculate how many bytes are needed.
        for (; count < (int)number_of_blocks; count++)
        {
            bytesCount += reduction_bytes[count];
        }

        necessaryBytes = bytesCount / 8;
        if (bytesCount % 8 != 0)
        {
            necessaryBytes++;
        }

        fastcdr::Cdr::state origin_state(write_cdr);
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

                    // The reduced information are always in big-endian
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
                write_cdr.set_state(origin_state);
                write_cdr.jump(count * sizeof(uint32_t));
                write_cdr << auxBytes;
            }
            else if (size_of_blocks == 16)
            {
                write_cdr.set_state(origin_state);
                write_cdr.jump(count * sizeof(uint16_t));
                write_cdr << static_cast<uint16_t>(auxBytes);
            }
            else if (size_of_blocks == 8)
            {
                buffer_new[count] = (fastdds::rtps::octet)auxBytes;
            }
            else
            {
                EPROSIMA_LOG_ERROR(RTPS_HEADERREDUCTION_TRANSPORT, "Bad block size " << size_of_blocks);
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
            EPROSIMA_LOG_ERROR(RTPS_HEADERREDUCTION_TRANSPORT, "Bad block size " << size_of_blocks);
        }

        final_length += (number_of_blocks * (size_of_blocks / 8)) - necessaryBytes; // 32 bits each block..
    }

    /*!
     * \brief Reduces the space used by RTPS Submessage's EntitiesID.
     *
     * \param[in] Pointer where RTPS Submessage's EntitiesID begins.
     */
    void pack_entity_id(
            const uint32_t* reduce_entitiesId)
    {
        uint32_t readerId = 0, reader_entityKind = 0, writerId = 0, writer_entityKind = 0;

        if (reduce_entitiesId != nullptr)
        {
            fastcdr::FastBuffer read_buffer(reinterpret_cast<char*>(buffer_old), this->length);
            fastcdr::Cdr read_cdr(read_buffer, fastcdr::Cdr::Endianness::BIG_ENDIANNESS, fastcdr::CdrVersion::DDS_CDR);
            fastcdr::FastBuffer write_buffer(reinterpret_cast<char*>(buffer_old), this->length);
            fastcdr::Cdr write_cdr(write_buffer, fastcdr::Cdr::Endianness::BIG_ENDIANNESS,
                    fastcdr::CdrVersion::DDS_CDR);


            read_cdr >> readerId;
            reader_entityKind = ((readerId & 0x000000C0) >> 3) + (readerId & 0x00000007);
            read_cdr >> writerId;
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
                        EPROSIMA_LOG_ERROR(RTPS_HEADERREDUCTION_TRANSPORT,
                                "Reader RTPS object idenfier 0x" << std::hex << readerId << " is unknown");
                        readerId = 7 << 5;
                        break;
                }

                readerId += reader_entityKind;
            }
            else
            {
                readerId = ((readerId & 0xFFFFFF00) >> 3) + reader_entityKind;
            }

            write_cdr << readerId;

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
                        EPROSIMA_LOG_ERROR(RTPS_HEADERREDUCTION_TRANSPORT,
                                "Writer RTPS object idenfier 0x" << std::hex << writerId << " is unknown");
                        writerId = 7 << 5;
                        break;
                }

                writerId += writer_entityKind;
            }
            else
            {
                writerId = ((writerId & 0xFFFFFF00) >> 3) + writer_entityKind;
            }

            write_cdr << writerId;

            pack_bytes(2, 32, reduce_entitiesId, false);
        }
        else
        {
            EPROSIMA_LOG_ERROR(RTPS_HEADERREDUCTION_TRANSPORT, "Bad parameters");
        }
    }

    /*!
     * \brief Expands RTPS Submessage's EntitiesID that was previously reduced by pack_entity_id function.
     *
     * \param[in] Pointer where RTPS Submessage's EntitiesID begins.
     */
    void unpack_entity_id(
            const uint32_t* reduce_entitiesId)
    {
        uint32_t readerId = 0, reader_entityKind = 0, writerId = 0, writer_entityKind = 0;

        if (reduce_entitiesId != nullptr)
        {
            unpack_bytes(2, 32, reduce_entitiesId, false);

            auto buffer_point {reinterpret_cast<char*>(&(reinterpret_cast<uint32_t*>(buffer_new))[-2])};

            fastcdr::FastBuffer read_buffer(buffer_point, this->length);
            fastcdr::Cdr read_cdr(read_buffer, fastcdr::Cdr::Endianness::BIG_ENDIANNESS, fastcdr::CdrVersion::DDS_CDR);
            fastcdr::FastBuffer write_buffer(buffer_point, this->length);
            fastcdr::Cdr write_cdr(write_buffer, fastcdr::Cdr::Endianness::BIG_ENDIANNESS,
                    fastcdr::CdrVersion::DDS_CDR);

            read_cdr >> readerId;
            reader_entityKind = ((readerId & 0x00000018) << 3) + (readerId & 0x00000007);
            read_cdr >> writerId;
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
                        EPROSIMA_LOG_ERROR(RTPS_HEADERREDUCTION_TRANSPORT,
                                "Reader RTPS object idenfier 0x" << std::hex << readerId << " is unknown");
                        readerId = 0;
                        break;
                }

                readerId = readerId + reader_entityKind;
            }
            else
            {
                readerId = ((readerId & 0xFFFFFFE0) << 3) + reader_entityKind;
            }

            write_cdr << readerId;

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
                        EPROSIMA_LOG_ERROR(RTPS_HEADERREDUCTION_TRANSPORT,
                                "Writer RTPS object idenfier 0x" << std::hex << writerId << " is unknown");
                        writerId = 0;
                        break;
                }

                writerId = writerId + writer_entityKind;
            }
            else
            {
                writerId = ((writerId & 0xFFFFFFE0) << 3) + writer_entityKind;
            }

            write_cdr << writerId;
        }
        else
        {
            EPROSIMA_LOG_ERROR(RTPS_HEADERREDUCTION_TRANSPORT, "Bad parameters");
        }
    }

    /*!
     * Move the left content of the source buffer to the destination buffer.
     */
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
    fastdds::rtps::octet* buffer_new;
    /**
     * \brief Pointer to the position of the buffer where the data will be moved to other position of the buffer (msgNew)
     * or will be jumped without move the data..
     */
    fastdds::rtps::octet* buffer_old;
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
        fastdds::rtps::octet* dest_buffer,
        const fastdds::rtps::octet* src_buffer,
        uint32_t& buffer_length,
        const HeaderReductionOptions& options)
{
    if (dest_buffer != nullptr && src_buffer != nullptr && buffer_length >= RTPSMESSAGE_HEADER_SIZE &&
            memcmp(src_buffer, "RTPS", 4) == 0)
    {
        memcpy(dest_buffer, src_buffer, buffer_length);
        fastdds::rtps::octet* buffer = dest_buffer;

        HRBufferPointer buf(buffer, buffer_length);

        if (options.rtps_header.eliminate_protocol == true)
        {
            buf.pointer_jump(HRCONFIG_RTPS_HEADER_PROTOCOL_SIZE - 1);
            // Set 'L' to know that was reduced by this plugin.
            buf.buffer_old[0] = (fastdds::rtps::octet) 'L';
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

        if (options.rtps_header.reduce_guidPrefix[0] != 32 || options.rtps_header.reduce_guidPrefix[1] != 32 ||
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
            if (buf.length < 4)
            {
                return false;
            }
            fastdds::rtps::octet submessageId = buf.buffer_old[0];
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

            buf.pointer_move(HRCONFIG_SUBMESSAGE_HEADER_OCTETS_NEXT);

            if (buf.length < submessageSize)
            {
                EPROSIMA_LOG_ERROR(RTPS_HEADERREDUCTION_TRANSPORT,
                        "Bad submessage size: " << submessageSize << " remaining length: " << buf.length);
                return false;
            }

            if (submessageId == fastdds::rtps::DATA || submessageId == fastdds::rtps::DATA_FRAG)
            {
                if (buf.length >
                        HRCONFIG_SUBMESSAGE_BODY_EXTRAFLAGS_SIZE + HRCONFIG_SUBMESSAGE_BODY_OCTETSTOINLINEQOS_SIZE)
                {
                    if (options.submessage_body.eliminate_extraFlags == true)
                    {
                        buf.pointer_jump(HRCONFIG_SUBMESSAGE_BODY_EXTRAFLAGS_SIZE);
                        buf.pointer_move(HRCONFIG_SUBMESSAGE_BODY_OCTETSTOINLINEQOS_SIZE);
                    }
                    else
                    {
                        buf.pointer_move(HRCONFIG_SUBMESSAGE_BODY_EXTRAFLAGS_SIZE +
                                HRCONFIG_SUBMESSAGE_BODY_OCTETSTOINLINEQOS_SIZE);
                    }

                    submessageSize -= HRCONFIG_SUBMESSAGE_BODY_EXTRAFLAGS_SIZE +
                            HRCONFIG_SUBMESSAGE_BODY_OCTETSTOINLINEQOS_SIZE;
                }
                else
                {
                    EPROSIMA_LOG_ERROR(RTPS_HEADERREDUCTION_TRANSPORT,
                            "Bad DATA or DATA_FRAG submessage size: " << submessageSize);
                    return false;
                }
            }

            if (submessageId == fastdds::rtps::ACKNACK || submessageId == fastdds::rtps::HEARTBEAT ||
                    submessageId == fastdds::rtps::GAP || submessageId == fastdds::rtps::NACK_FRAG ||
                    submessageId == fastdds::rtps::HEARTBEAT_FRAG || submessageId == fastdds::rtps::DATA ||
                    submessageId == fastdds::rtps::DATA_FRAG)
            {
                if (buf.length >
                        HRCONFIG_SUBMESSAGE_BODY_ENTITIESID_SIZE + HRCONFIG_SUBMESSAGE_BODY_SEQUENCENUMBER_SIZE)
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

                    submessageSize -= HRCONFIG_SUBMESSAGE_BODY_ENTITIESID_SIZE;

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

                    submessageSize -= HRCONFIG_SUBMESSAGE_BODY_SEQUENCENUMBER_SIZE;     // SequenceNumber
                }
                else
                {
                    EPROSIMA_LOG_ERROR(RTPS_HEADERREDUCTION_TRANSPORT,
                            "Bad ACKNACK, HEARTBEAT, GAP, NACK_FRAG, HEARTBEAT_FRAG, DATA or DATA_FRAG submessage size: "
                            << submessageSize);
                    return false;
                }
            }

            buf.pointer_move(submessageSize);
        }

        buf.pointer_finalize();
        buffer_length = buf.final_length;
        return true;
    }

    EPROSIMA_LOG_ERROR(RTPS_HEADERREDUCTION_TRANSPORT, "Bad parameters");
    return false;
}

bool HeaderReduction_Recover(
        fastdds::rtps::octet* newBuffer,
        const fastdds::rtps::octet* oldBuffer,
        uint32_t& messageLength,
        const HeaderReductionOptions& options)
{
    if (newBuffer != nullptr && oldBuffer != nullptr)
    {
        HRBufferPointer buf(newBuffer, const_cast<fastdds::rtps::octet*>(oldBuffer), messageLength);

        // Check 'L' to know that was reduced by this plugin.
        if (buf.buffer_old[0] != 'L')
        {
            return false;
        }

        if (options.rtps_header.eliminate_protocol == true)
        {
            const fastdds::rtps::octet protocol[] {"RTPS"};
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
            const fastdds::rtps::octet version[2] = { fastdds::rtps::c_ProtocolVersion.m_major,
                                                      fastdds::rtps::c_ProtocolVersion.m_minor };
            buf.pointer_add(version, HRCONFIG_RTPS_HEADER_VERSION_SIZE);
        }
        else
        {
            buf.pointer_move(HRCONFIG_RTPS_HEADER_VERSION_SIZE);
        }

        if (options.rtps_header.eliminate_vendorId == true)
        {
            const fastdds::rtps::octet vendor[2] {fastdds::rtps::c_VendorId_eProsima[0],
                                                  fastdds::rtps::c_VendorId_eProsima[1]};
            buf.pointer_add(vendor, HRCONFIG_RTPS_HEADER_VENDORID_SIZE);
        }
        else
        {
            buf.pointer_move(HRCONFIG_RTPS_HEADER_VENDORID_SIZE);
        }

        if (options.rtps_header.reduce_guidPrefix[0] != 32 || options.rtps_header.reduce_guidPrefix[1] != 32 ||
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
            fastdds::rtps::octet submessageId = 0;
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

            buf.pointer_move(HRCONFIG_SUBMESSAGE_HEADER_OCTETS_NEXT);

            if (submessageId == fastdds::rtps::DATA || submessageId == fastdds::rtps::DATA_FRAG)
            {
                if (options.submessage_body.eliminate_extraFlags == true)
                {
                    const fastdds::rtps::octet extraFlags[2] { '\0', '\0' };
                    buf.pointer_add(extraFlags, HRCONFIG_SUBMESSAGE_BODY_EXTRAFLAGS_SIZE);
                    buf.pointer_move(HRCONFIG_SUBMESSAGE_BODY_OCTETSTOINLINEQOS_SIZE);
                }
                else
                {
                    buf.pointer_move(HRCONFIG_SUBMESSAGE_BODY_EXTRAFLAGS_SIZE +
                            HRCONFIG_SUBMESSAGE_BODY_OCTETSTOINLINEQOS_SIZE);
                }

                submessageSize -= HRCONFIG_SUBMESSAGE_BODY_EXTRAFLAGS_SIZE +
                        HRCONFIG_SUBMESSAGE_BODY_OCTETSTOINLINEQOS_SIZE;
            }

            if (submessageId == fastdds::rtps::ACKNACK || submessageId == fastdds::rtps::HEARTBEAT ||
                    submessageId == fastdds::rtps::GAP || submessageId == fastdds::rtps::NACK_FRAG ||
                    submessageId == fastdds::rtps::HEARTBEAT_FRAG || submessageId == fastdds::rtps::DATA ||
                    submessageId == fastdds::rtps::DATA_FRAG)
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

                submessageSize -= HRCONFIG_SUBMESSAGE_BODY_ENTITIESID_SIZE;

                if (options.submessage_body.reduce_sequenceNumber != 64)
                {
                    if (options.submessage_body.reduce_sequenceNumber > 32)
                    {
                        uint32_t reduction = options.submessage_body.reduce_sequenceNumber - 32;
                        buf.unpack_bytes(1, 32, &reduction, endianess);
                    }
                    else
                    {
                        const fastdds::rtps::octet seq_empty[4] { '\0', '\0', '\0', '\0' };
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

                submessageSize -= HRCONFIG_SUBMESSAGE_BODY_SEQUENCENUMBER_SIZE; // SequenceNumber
            }

            buf.pointer_move(submessageSize);
        }

        buf.pointer_finalize();
        messageLength = buf.final_length;
        return true;
    }

    EPROSIMA_LOG_ERROR(RTPS_HEADERREDUCTION_TRANSPORT, "Bad parameters");
    return false;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
