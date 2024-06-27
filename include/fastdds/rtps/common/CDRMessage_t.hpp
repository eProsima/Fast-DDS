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

/**
 * @file CDRMessage_t.hpp
 */

#ifndef FASTDDS_RTPS_COMMON__CDRMESSAGE_T_HPP
#define FASTDDS_RTPS_COMMON__CDRMESSAGE_T_HPP
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/rtps/common/SerializedPayload.hpp>
#include <fastdds/rtps/common/Types.hpp>
#include <cassert>
#include <cstdlib>
#include <cstring>

namespace eprosima {
namespace fastdds {
namespace rtps {

//!Max size of RTPS message in bytes.
#define RTPSMESSAGE_DEFAULT_SIZE 10500  //max size of rtps message in bytes
#define RTPSMESSAGE_COMMON_RTPS_PAYLOAD_SIZE 536 //common payload a rtps message has TODO(Ricardo) It is necessary?
#define RTPSMESSAGE_COMMON_DATA_PAYLOAD_SIZE 10000 //common data size
#define RTPSMESSAGE_HEADER_SIZE 20  //header size in bytes
#define RTPSMESSAGE_SUBMESSAGEHEADER_SIZE 4
#define RTPSMESSAGE_DATA_EXTRA_INLINEQOS_SIZE 4
#define RTPSMESSAGE_INFOTS_SIZE 12

#define RTPSMESSAGE_OCTETSTOINLINEQOS_DATASUBMSG 16 //may change in future versions
#define RTPSMESSAGE_OCTETSTOINLINEQOS_DATAFRAGSUBMSG 28 //may change in future versions
#define RTPSMESSAGE_DATA_MIN_LENGTH 24

/**
 * @brief Structure CDRMessage_t, contains a serialized message.
 * @ingroup COMMON_MODULE
 */
struct FASTDDS_EXPORTED_API CDRMessage_t final
{
    // TODO(Miguel C): Deprecate when not used in mocks
    CDRMessage_t()
        : CDRMessage_t(RTPSMESSAGE_DEFAULT_SIZE)
    {
    }

    ~CDRMessage_t()
    {
        if (buffer != nullptr && !wraps)
        {
            free(buffer);
        }
    }

    /**
     * Constructor with maximum size
     * @param size Maximum size
     */
    explicit CDRMessage_t(
            uint32_t size)
    {
        wraps = false;
        pos = 0;
        length = 0;

        if (size != 0)
        {
            buffer = (octet*)malloc(size);
        }
        else
        {
            buffer = nullptr;
        }

        max_size = size;
        reserved_size = size;
        msg_endian = DEFAULT_ENDIAN;
    }

    /**
     * Constructor to wrap a serialized payload
     * @param payload Payload to wrap
     */
    explicit CDRMessage_t(
            const SerializedPayload_t& payload)
        : wraps(true)
    {
        msg_endian = LITTLEEND;
        if (payload.encapsulation == PL_CDR_BE || payload.encapsulation == CDR_BE)
        {
            msg_endian = BIGEND;
        }
        pos = payload.pos;
        length = payload.length;
        buffer = payload.data;
        max_size = payload.max_size;
        reserved_size = payload.max_size;
    }

    CDRMessage_t(
            const CDRMessage_t& message)
    {
        wraps = false;
        pos = 0;
        length = message.length;
        max_size = message.max_size;
        msg_endian = message.msg_endian;

        reserved_size = max_size;
        if (max_size != 0)
        {
            buffer =  (octet*)malloc(max_size);
            memcpy(buffer, message.buffer, length);
        }
        else
        {
            buffer = nullptr;
        }
    }

    CDRMessage_t(
            CDRMessage_t&& message)
    {
        wraps = message.wraps;
        message.wraps = false;
        pos = message.pos;
        message.pos = 0;
        length = message.length;
        message.length = 0;
        max_size = message.max_size;
        message.max_size = 0;
        reserved_size = message.reserved_size;
        message.reserved_size = 0;
        msg_endian = message.msg_endian;
        message.msg_endian = DEFAULT_ENDIAN;
        buffer = message.buffer;
        message.buffer = nullptr;
    }

    CDRMessage_t& operator =(
            CDRMessage_t&& message)
    {
        wraps = message.wraps;
        message.wraps = false;
        pos = message.pos;
        message.pos = 0;
        length = message.length;
        message.length = 0;
        max_size = message.max_size;
        message.max_size = 0;
        reserved_size = message.reserved_size;
        message.reserved_size = 0;
        msg_endian = message.msg_endian;
        message.msg_endian = DEFAULT_ENDIAN;
        buffer = message.buffer;
        message.buffer = nullptr;

        return *(this);
    }

    void init(
            octet* buffer_ptr,
            uint32_t size)
    {
        assert(buffer == nullptr);
        wraps = true;
        pos = 0;
        length = 0;
        buffer = buffer_ptr;
        max_size = size;
        reserved_size = size;
        msg_endian = DEFAULT_ENDIAN;
    }

    void reserve(
            uint32_t size)
    {
        assert(wraps == false);
        if (size > reserved_size)
        {
            octet* new_buffer = (octet*) realloc(buffer, size);
            if (new_buffer == nullptr)
            {
                // TODO: Exception? Assertion?
            }
            else
            {
                buffer = new_buffer;
                reserved_size = size;
            }
        }

        max_size = size;
    }

    //!Pointer to the buffer where the data is stored.
    octet* buffer;
    //!Read or write position.
    uint32_t pos;
    //!Max size of the message.
    uint32_t max_size;
    //!Size allocated on buffer. May be higher than max_size.
    uint32_t reserved_size;
    //!Current length of the message.
    uint32_t length;
    //!Endianness of the message.
    Endianness_t msg_endian;
    //Whether this message is wrapping a buffer managed elsewhere.
    bool wraps;
};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif // DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif // FASTDDS_RTPS_COMMON__CDRMESSAGE_T_HPP
