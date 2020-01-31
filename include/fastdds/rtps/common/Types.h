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
 * @file Types.h
 */

#ifndef _FASTDDS_RTPS_COMMON_TYPES_H_
#define _FASTDDS_RTPS_COMMON_TYPES_H_

#include <stddef.h>
#include <iostream>
#include <cstdint>
#include <stdint.h>

#include <fastrtps/fastrtps_dll.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

/*!
 * @brief This enumeration represents endianness types.
 * @ingroup COMMON_MODULE
 */
enum Endianness_t{
    //! @brief Big endianness.
    BIGEND = 0x1,
    //! @brief Little endianness.
    LITTLEEND = 0x0
};

//!Reliability enum used for internal purposes
//!@ingroup COMMON_MODULE
typedef enum ReliabilityKind_t
{
    RELIABLE,
    BEST_EFFORT
}ReliabilityKind_t;

//!Durability kind
//!@ingroup COMMON_MODULE
typedef enum DurabilityKind_t
{
    VOLATILE,        //!< Volatile Durability
    TRANSIENT_LOCAL, //!< Transient Local Durability
    TRANSIENT,       //!< Transient Durability.
    PERSISTENT       //!< NOT IMPLEMENTED.
}DurabilityKind_t;

//!Endpoint kind
//!@ingroup COMMON_MODULE
typedef enum EndpointKind_t
{
    READER,
    WRITER
}EndpointKind_t;

//!Topic kind
typedef enum TopicKind_t
{
    NO_KEY,
    WITH_KEY
}TopicKind_t;

#if __BIG_ENDIAN__
const Endianness_t DEFAULT_ENDIAN = BIGEND;
#else
const Endianness_t DEFAULT_ENDIAN = LITTLEEND;
#endif

using octet = unsigned char;
//typedef unsigned int uint;
//typedef unsigned short ushort;
using SubmessageFlag = unsigned char;
using BuiltinEndpointSet_t = uint32_t;
using Count_t = uint32_t;

#define BIT0 0x01u
#define BIT1 0x02u
#define BIT2 0x04u
#define BIT3 0x08u
#define BIT4 0x10u
#define BIT5 0x20u
#define BIT6 0x40u
#define BIT7 0x80u

#define BIT(i) (1U << static_cast<unsigned>(i))

//!@brief Structure ProtocolVersion_t, contains the protocol version.
struct RTPS_DllAPI ProtocolVersion_t
{
    octet m_major;
    octet m_minor;
    ProtocolVersion_t():
#if HAVE_SECURITY
        // As imposed by DDSSEC11-93
        ProtocolVersion_t(2,3)
#else
        ProtocolVersion_t(2,2)
#endif
    {

    };

    ProtocolVersion_t(octet maj,octet min)
        : m_major(maj)
        , m_minor(min)
    {

    }

    bool operator==(const ProtocolVersion_t &v) const
    {
        return m_major == v.m_major && m_minor == v.m_minor;
    }

    bool operator!=(const ProtocolVersion_t &v) const
    {
        return m_major != v.m_major || m_minor != v.m_minor;
    }
};

/**
 * Prints a ProtocolVersion
 * @param output Output Stream
 * @param pv ProtocolVersion
 * @return OStream.
 */
inline std::ostream& operator<<(std::ostream& output, const ProtocolVersion_t& pv){
    return output << static_cast<int>(pv.m_major) << "." << static_cast<int>(pv.m_minor);
}

const ProtocolVersion_t c_ProtocolVersion_2_0(2,0);
const ProtocolVersion_t c_ProtocolVersion_2_1(2,1);
const ProtocolVersion_t c_ProtocolVersion_2_2(2,2);
const ProtocolVersion_t c_ProtocolVersion_2_3(2,3);

const ProtocolVersion_t c_ProtocolVersion;

//!@brief Structure VendorId_t, specifying the vendor Id of the implementation.
class VendorId_t
{
public:
    VendorId_t(const VendorId_t &v)// : m_vendor{v[0], v[1]}
    {
        // There isn't a explicit constructor because VS2013 doesn't support it.
        m_vendor[0] = v[0];
        m_vendor[1] = v[1];
    }

    VendorId_t(const octet id0, const octet id1)// : m_vendor{id0, id1}
    {
        // There isn't a explicit constructor because VS2013 doesn't support it.
        m_vendor[0] = id0;
        m_vendor[1] = id1;
    }

    VendorId_t& operator=(const VendorId_t &v)
    {
        m_vendor[0] = v[0];
        m_vendor[1] = v[1];
        return *this;
    }

    octet& operator[](const int index)
    {
        return m_vendor[index];
    }

    octet operator[](const int index) const
    {
        return m_vendor[index];
    }

    bool operator==(const VendorId_t &v) const
    {
        return m_vendor[0] == v[0] && m_vendor[1] == v[1];
    }
private:
    octet m_vendor[2];
};
//typedef octet VendorId_t[2];

const VendorId_t c_VendorId_Unknown = {0x00,0x00};
const VendorId_t c_VendorId_eProsima = {0x01,0x0F};

}
}
}

#endif /* _FASTDDS_RTPS_COMMON_TYPES_H_ */
