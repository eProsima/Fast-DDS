/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file common_types.h	
 */

#ifndef COMMON_TYPES_H_
#define COMMON_TYPES_H_

#include "eprosimartps\eprosimartps_dll.h"

#include <stddef.h>
#include <iostream>
#include <cstdint>

/**
 * @namespace eprosima
 * @ingroup EPROSIMARTPSAPIREFERENCE
 * eProsima namespace. It contains everything.
 */
namespace eprosima{

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
enum ReliabilityKind_t{
	RELIABLE,
	BEST_EFFORT
};

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

#define PROTOCOLVERSION_1_0(pv) {pv.m_major=1;pv.m_minor=0;}
#define PROTOCOLVERSION_1_1(pv) {pv.m_major=1;pv.m_minor=1;}
#define PROTOCOLVERSION_2_0(pv) {pv.m_major=2;pv.m_minor=0;}
#define PROTOCOLVERSION_2_1(pv) {pv.m_major=2;pv.m_minor=1;}
#define PROTOCOLVERSION PROTOCOLVERSION_2_1

const ProtocolVersion_t c_ProtocolVersion(2,1);

#define VENDORID_UNKNOWN(vi) {vi[0]=0;vi[1]=0;}
#define VENDORID_EPROSIMA(vi) {vi[0]=0x01;vi[1]=0x0F;}
//!@brief Structure VendorId_t, specifying the vendor Id of the implementation.
typedef octet VendorId_t[2];

const VendorId_t c_eProsimaVendorId={0x01,0x0F};

}
}
#include "colors.h"

#include "eprosimartps/eprosimartps_dll.h"

using std::cout;
using std::endl;

#endif /* COMMON_TYPES_H_ */
