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
 * @file SerializedPayload.h
 */

#ifndef _FASTDDS_RTPS_SERIALIZEDPAYLOAD_H_
#define _FASTDDS_RTPS_SERIALIZEDPAYLOAD_H_
#include <fastrtps/fastrtps_dll.h>
#include <fastdds/rtps/common/Types.h>
#include <cstring>
#include <new>
#include <stdexcept>
#include <stdint.h>
#include <stdlib.h>

/*!
 * @brief Maximum payload is maximum of UDP packet size minus 536bytes (RTPSMESSAGE_COMMON_RTPS_PAYLOAD_SIZE)
 * With those 536 bytes (RTPSMESSAGE_COMMON_RTPS_PAYLOAD_SIZE) bytes is posible to send RTPS Header plus RTPS Data submessage plus RTPS Heartbeat submessage.
 */

namespace eprosima {
namespace fastrtps {
namespace rtps {

//Pre define data encapsulation schemes
#define CDR_BE 0x0000
#define CDR_LE 0x0001
#define PL_CDR_BE 0x0002
#define PL_CDR_LE 0x0003

#if FASTDDS_IS_BIG_ENDIAN_TARGET
#define DEFAULT_ENCAPSULATION CDR_BE
#define PL_DEFAULT_ENCAPSULATION PL_CDR_BE
#else
#define DEFAULT_ENCAPSULATION CDR_LE
#define PL_DEFAULT_ENCAPSULATION PL_CDR_LE
#endif  // FASTDDS_IS_BIG_ENDIAN_TARGET

//!@brief Structure SerializedPayload_t.
//!@ingroup COMMON_MODULE
struct RTPS_DllAPI SerializedPayload_t
{
    //!Size in bytes of the representation header as specified in the RTPS 2.3 specification chapter 10.
    static constexpr size_t representation_header_size = 4u;

    //!Encapsulation of the data as suggested in the RTPS 2.1 specification chapter 10.
    uint16_t encapsulation;
    //!Actual length of the data
    uint32_t length;
    //!Pointer to the data.
    octet* data;
    //!Maximum size of the payload
    uint32_t max_size;
    //!Position when reading
    uint32_t pos;

    //!Default constructor
    SerializedPayload_t()
        : encapsulation(CDR_BE)
        , length(0)
        , data(nullptr)
        , max_size(0)
        , pos(0)
    {
    }

    /**
     * @param len Maximum size of the payload
     */
    explicit SerializedPayload_t(
            uint32_t len)
        : SerializedPayload_t()
    {
        this->reserve(len);
    }

    ~SerializedPayload_t()
    {
        this->empty();
    }

    bool operator == (
            const SerializedPayload_t& other) const
    {
        return ((encapsulation == other.encapsulation) &&
               (length == other.length) &&
               (0 == memcmp(data, other.data, length)));
    }

    /*!
     * Copy another structure (including allocating new space for the data.)
     * @param[in] serData Pointer to the structure to copy
     * @param with_limit if true, the function will fail when providing a payload too big
     * @return True if correct
     */
    bool copy(
            const SerializedPayload_t* serData,
            bool with_limit = true)
    {
        length = serData->length;

        if (serData->length > max_size)
        {
            if (with_limit)
            {
                return false;
            }
            else
            {
                this->reserve(serData->length);
            }
        }
        encapsulation = serData->encapsulation;
        if (length == 0)
        {
            return true;
        }
        memcpy(data, serData->data, length);
        return true;
    }

    /*!
     * Allocate new space for fragmented data
     * @param[in] serData Pointer to the structure to copy
     * @return True if correct
     */
    bool reserve_fragmented(
            SerializedPayload_t* serData)
    {
        length = serData->length;
        max_size = serData->length;
        encapsulation = serData->encapsulation;
        data = (octet*)calloc(length, sizeof(octet));
        return true;
    }

    //! Empty the payload
    void empty()
    {
        length = 0;
        encapsulation = CDR_BE;
        max_size = 0;
        if (data != nullptr)
        {
            free(data);
        }
        data = nullptr;
    }

    void reserve(
            uint32_t new_size)
    {
        if (new_size <= this->max_size)
        {
            return;
        }
        if (data == nullptr)
        {
            data = (octet*)calloc(new_size, sizeof(octet));
            if (!data)
            {
                throw std::bad_alloc();
            }
        }
        else
        {
            void* old_data = data;
            data = (octet*)realloc(data, new_size);
            if (!data)
            {
                free(old_data);
                throw std::bad_alloc();
            }
            memset(data + max_size, 0, (new_size - max_size) * sizeof(octet));
        }
        max_size = new_size;
    }

};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* _FASTDDS_RTPS_SERIALIZEDPAYLOAD_H_ */
