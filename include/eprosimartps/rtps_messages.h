/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * rtps_messages.h
 *
 *  Created on: Feb 18, 2014
 *      Author: Gonzalo Rodriguez Canosa
 */

#ifndef RTPS_MESSAGES_H_
#define RTPS_MESSAGES_H_

#include <vector>

#include "eprosimartps/rtps_common.h"

namespace eprosima{
namespace rtps{

//!@brief RTPS Header Structure
typedef struct Header_t{
			ProtocolVersion_t version;
			VendorId_t vendorId;
			GuidPrefix_t guidPrefix;
			Header_t(){
				PROTOCOLVERSION(version);
				vendorId = VENDORID_UNKNOWN;
				guidPrefix = GUIDPREFIX_UNKNOWN;
			}
}Header_t;

//!@brief Enumeration of the different Submessages types
typedef enum {
	PAD=0x01,
	ACKNACK=0x06,
	HEARTBEAT=0x07,
	GAP=0x08,
	INFO_TS=0x09,
	INFO_SRC=0x0c,
	INFO_REPLY_IP4=0x0d,
	INFO_DST=0x0e,
	INFO_REPLY=0x0f,
	NACK_FRAG=0x12,
	HEARTBEAT_FRAG=0x13,
	DATA=0x15,
	DATA_FRAG=0x16
}SubmessageKind;

//!@brief RTPS SubmessageHeader Structure
typedef struct{
	SubmessageKind submessageId;
	unsigned short submessageLength;
	SubmessageFlag flags[8];
}SubmessageHeader_t;

// SUBMESSAGE types definition

//!@brief RTPS Data Submessage
typedef struct{
	SubmessageHeader_t SubmessageHeader;
	EntityId_t readerId;
	EntityId_t writerId;
	std::vector<Parameter_t> inlineQos;
	SerializedPayload_t serializedPayload;
}DataSubM_t;



}
}



#endif /* RTPS_MESSAGES_H_ */
