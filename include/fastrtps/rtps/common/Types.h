/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file common_types.h	
 */

#ifndef COMMON_TYPES_H_
#define COMMON_TYPES_H_

#include <stddef.h>
#include <iostream>
#include <cstdint>
#include <stdint.h>

#include "fastrtps/config/fastrtps_dll.h"

/**
 * @namespace eprosima
 * @ingroup fastrtpsAPIREFERENCE
 * eProsima namespace. It contains everything.
 */
namespace eprosima{
namespace fastrtps{

/**
 * RTPS namespace. Functions and structures dedicated to implement the RTPS protocol.
 * @ingroup RTPSMODULE
 */
namespace rtps{

/*!
 * @brief This enumeration represents endianness types.
 */
enum Endianness_t{
	//! @brief Big endianness.
	BIGEND = 0x1,
	//! @brief Little endianness.
	LITTLEEND = 0x0
};

//!Reliability enum used for internal purposes
typedef enum ReliabilityKind_t{
	RELIABLE,
	BEST_EFFORT
}ReliabilityKind_t;

typedef enum DurabilityKind_t
{
	VOLATILE,
	TRANSIENT_LOCAL
}DurabilityKind_t;

typedef enum EndpointKind_t{
	READER,
	WRITER
}EndpointKind_t;

typedef enum TopicKind_t{
	NO_KEY,
	WITH_KEY
}TopicKind_t;


#if defined(__LITTLE_ENDIAN__)
const Endianness_t DEFAULT_ENDIAN = LITTLEEND;
#else
const Endianness_t DEFAULT_ENDIAN = BIGEND;
#endif

#define EPROSIMA_ENDIAN LITTLEEND


typedef unsigned char octet;
//typedef unsigned int uint;
//typedef unsigned short ushort;
typedef unsigned char SubmessageFlag;
typedef uint32_t BuiltinEndpointSet_t;
typedef uint32_t Count_t;

#define BIT0 0x1
#define BIT1 0x2
#define BIT2 0x4
#define BIT3 0x8
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80

#define BIT(i) ((i==0) ? BIT0 : (i==1) ? BIT1 :(i==2)?BIT2:(i==3)?BIT3:(i==4)?BIT4:(i==5)?BIT5:(i==6)?BIT6:(i==7)?BIT7:0x0)

//!@brief Structure ProtocolVersion_t, contains the protocol version.
struct RTPS_DllAPI ProtocolVersion_t{
	octet m_major;
	octet m_minor;
	ProtocolVersion_t():
		m_major(2),
		m_minor(1)
	{

	};
	ProtocolVersion_t(octet maj,octet min):
		m_major(maj),
		m_minor(min)
	{

	}
};


const ProtocolVersion_t c_ProtocolVersion_2_0(2,0);
const ProtocolVersion_t c_ProtocolVersion_2_1(2,1);
const ProtocolVersion_t c_ProtocolVersion_2_2(2,2);

const ProtocolVersion_t c_ProtocolVersion(2,1);

//!@brief Structure VendorId_t, specifying the vendor Id of the implementation.
typedef octet VendorId_t[2];

const VendorId_t c_VendorId_Unknown={0x00,0x00};
const VendorId_t c_VendorId_eProsima={0x01,0x0F};


static inline void set_VendorId_Unknown(VendorId_t& id)
{
	id[0]=0x0;id[1]=0x0;
}

static inline void set_VendorId_eProsima(VendorId_t& id)
{
	id[0]=0x01;id[1]=0x0F;
}

}
}
}

using std::cout;
using std::endl;

#endif /* COMMON_TYPES_H_ */
