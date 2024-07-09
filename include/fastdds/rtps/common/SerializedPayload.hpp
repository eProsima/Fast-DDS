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
 * @file SerializedPayload.hpp
 */

#ifndef FASTDDS_RTPS_COMMON__SERIALIZEDPAYLOAD_HPP
#define FASTDDS_RTPS_COMMON__SERIALIZEDPAYLOAD_HPP

#include <cstring>
#include <new>
#include <stdexcept>
#include <cassert>
#include <stdint.h>
#include <stdlib.h>

#include <fastdds/fastdds_dll.hpp>
#include <fastdds/rtps/common/Types.hpp>
#include <fastdds/rtps/history/IPayloadPool.hpp>

/*!
 * @brief Maximum payload is maximum of UDP packet size minus 536bytes (RTPSMESSAGE_COMMON_RTPS_PAYLOAD_SIZE)
 * With those 536 bytes (RTPSMESSAGE_COMMON_RTPS_PAYLOAD_SIZE) bytes is posible to send RTPS Header plus RTPS Data submessage plus RTPS Heartbeat submessage.
 */

namespace eprosima {
namespace fastdds {
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
struct FASTDDS_EXPORTED_API SerializedPayload_t
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
    //!Pool that created the payload
    IPayloadPool* payload_owner = nullptr;

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

    //!Copy constructor
    SerializedPayload_t(
            const SerializedPayload_t& other) = delete;
    //!Copy operator
    SerializedPayload_t& operator = (
            const SerializedPayload_t& other) = delete;

    //!Move constructor
    SerializedPayload_t(
            SerializedPayload_t&& other) noexcept
    {
        *this = std::move(other);
    }

    //!Move operator
    SerializedPayload_t& operator = (
            SerializedPayload_t&& other) noexcept;

    /*!
     * Destructor
     * It is expected to release the payload if the payload owner is not nullptr before destruction
     */
    ~SerializedPayload_t();

    bool operator == (
            const SerializedPayload_t& other) const;

    /*!
     * Copy another structure (including allocating new space for the data).
     * @param [in] serData Pointer to the structure to copy
     * @param with_limit if true, the function will fail when providing a payload too big
     * @return True if correct
     */
    bool copy(
            const SerializedPayload_t* serData,
            bool with_limit = true);

    /*!
     * Allocate new space for fragmented data
     * @param [in] serData Pointer to the structure to copy
     * @return True if correct
     */
    bool reserve_fragmented(
            SerializedPayload_t* serData);

    /*!
     * Empty the payload
     * @pre payload_owner must be nullptr
     */
    void empty();

    void reserve(
            uint32_t new_size);

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_COMMON__SERIALIZEDPAYLOAD_HPP
