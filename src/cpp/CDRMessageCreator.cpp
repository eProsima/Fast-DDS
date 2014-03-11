/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * CDRMessageCreator.cpp
 *
 *  Created on: Feb 19, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#include "eprosimartps/CDRMessageCreator.h"
#include "eprosimartps/CDRMessage.h"
#include "eprosimartps/RTPSWriter.h"
#include "eprosimartps/ParameterListCreator.h"

namespace eprosima {
namespace rtps{


#if defined(__LITTLE_ENDIAN__)
	const Endianness_t DEFAULT_ENDIAN = LITTLEEND;
#elif defined (__BIG_ENDIAN__)
	const Endianness_t DEFAULT_ENDIAN = BIGEND;
#endif
};
};


namespace eprosima {
namespace rtps{


CDRMessageCreator::CDRMessageCreator() {
	// TODO Auto-generated constructor stub


}

CDRMessageCreator::~CDRMessageCreator() {
	// TODO Auto-generated destructor stub
}


bool CDRMessageCreator::createHeader(CDRMessage_t*msg, Header_t* header) {
	CDRMessage::initCDRMsg(msg,RTPSMESSAGE_HEADER_SIZE);
	try{
		CDRMessage::addOctet(msg,'R');
		CDRMessage::addOctet(msg,'T');
		CDRMessage::addOctet(msg,'P');
		CDRMessage::addOctet(msg,'S');

		CDRMessage::addOctet(msg,header->version.major);
		CDRMessage::addOctet(msg,header->version.minor);

		CDRMessage::addOctet(msg,header->vendorId[0]);
		CDRMessage::addOctet(msg,header->vendorId[1]);

		for (uint i = 0;i<12;i++){
			CDRMessage::addOctet(msg,header->guidPrefix.value[i]);
		}
		msg->length = msg->pos;
	}
	catch(int e)
	{
		RTPSLog::Error << B_RED << "Header creation fails: "<< e << DEF<< endl;
		RTPSLog::printError();
		return false;
	}

	return true;
}

bool CDRMessageCreator::createSubmessageHeader(CDRMessage_t* msg,
		SubmessageHeader_t* SubMH, uint16_t submsgsize) {
	CDRMessage::initCDRMsg(msg,RTPSMESSAGE_SUBMESSAGEHEADER_SIZE);
	try{
		CDRMessage::addOctet(msg,SubMH->submessageId);
		CDRMessage::addOctet(msg,SubMH->flags);
		CDRMessage::addUInt16(msg, submsgsize);
		msg->length = msg->pos;
	}
	catch(int e){
		RTPSLog::Error << B_RED << "Submessage Header creation fails: "<< e << DEF<< endl;
		RTPSLog::printError();
		return false;
	}

	return true;
}






}; /* namespace rtps */
}; /* namespace eprosima */


#include "submessages/DataMsg.hpp"
#include "submessages/HeartbeatMsg.hpp"
#include "submessages/AckNackMsg.hpp"
#include "submessages/GapMsg.hpp"
