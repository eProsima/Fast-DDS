/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * rtps_all.h
 *
 *  Created on: Feb 19, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#include <cstdlib>
#include <cstring>
#include <math.h>
#include <vector>
#include <iostream>
 #include <bitset>
 #include <string>
#include <sstream>
#include <stdint.h>
#include <iomanip>

#include "eprosimartps/eprosimartps_dll.h"


#ifndef RTPS_ALL_H_
#define RTPS_ALL_H_
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
typedef enum Endianness_t{
	//! @brief Big endianness.
	BIGEND = 0x1,
	//! @brief Little endianness.
	LITTLEEND = 0x0
}Endianness_t;

#define EPROSIMA_ENDIAN LITTLEEND //ERROR CAMBIAR A LITTLEEND

//!Max size of RTPS message in bytes.
#define RTPSMESSAGE_MAX_SIZE 2048  //max size of ftps message in bytes
#define RTPSMESSAGE_HEADER_SIZE 20  //header size in bytes
#define RTPSMESSAGE_SUBMESSAGEHEADER_SIZE 4
#define RTPSMESSAGE_DATA_EXTRA_INLINEQOS_SIZE 4

#define RTPSMESSAGE_OCTETSTOINLINEQOS_DATASUBMSG 16 //may change in future versions

#define DEFAULT_HISTORY_SIZE 10


#define BIT0 0x1
#define BIT1 0x2
#define BIT2 0x4
#define BIT3 0x8
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80

#define BIT(i) ((i==0) ? BIT0 : (i==1) ? BIT1 :(i==2)?BIT2:(i==3)?BIT3:(i==4)?BIT4:(i==5)?BIT5:(i==6)?BIT6:(i==7)?BIT7:0x0)

typedef unsigned char octet;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef octet SubmessageFlag;


}
}



using namespace std;

#include "common/colors.h"

#include "common/rtps_elem_guid.h"
#include "common/rtps_elem_seqnum.h"
#include "common/rtps_elem_locator.h"
#include "common/rtps_common.h"

//#include "ParameterList.h"


#include "common/rtps_error_codes.h"
#include "common/CacheChange.h"


namespace eprosima{

using namespace rtps;

/**
 * Structure TypeReg_t used to save the name, size and serialization functions of a user-defined type.
 */
typedef struct TypeReg_t{
	std::string dataType;
	void (*serialize)(SerializedPayload_t*,void*);
	void (*deserialize)(SerializedPayload_t*,void*);
	int32_t byte_size;
}TypeReg_t;

}

//#include "CDRMessage.h"
//#include "Endpoint.h"
//
//#include "MessageReceiver.h"
//#include "ReaderLocator.h"
//#include "RTPSWriter.h"
//#include "HistoryCache.h"
//#include "StatelessWriter.h"





#endif /* RTPS_ALL_H_ */
