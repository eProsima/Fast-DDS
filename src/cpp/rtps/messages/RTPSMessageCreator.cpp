/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * @file CDRMessageCreator.cpp
 *
 */

#include "fastrtps/rtps/messages/RTPSMessageCreator.h"
#include "fastrtps/rtps/messages/CDRMessage.h"
#include "fastrtps/qos/ParameterList.h"
#include "fastrtps/utils/eClock.h"
#include "fastrtps/rtps/messages/CDRMessagePool.h"
//#include "fastrtps/common/RTPS_messages.h"

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"

#include "fastrtps/utils/RTPSLog.h"

using namespace boost::posix_time;



using namespace eprosima::fastrtps;

namespace eprosima {
namespace fastrtps{
namespace rtps{

// Auxiliary message to avoid creation of new messages each time.
CDRMessagePool g_pool_submsg(100);
eClock g_clock;

static const char* const CLASS_NAME = "RTPSMessageCreator";

RTPSMessageCreator::RTPSMessageCreator() {

}

RTPSMessageCreator::~RTPSMessageCreator() {
	const char* const METHOD_NAME = "~RTPSMessageCreator";
	logInfo(RTPS_CDR_MSG,"RTPSMessageCreator destructor"<<endl;);
}


bool RTPSMessageCreator::addHeader(CDRMessage_t*msg, const GuidPrefix_t& guidPrefix,
		ProtocolVersion_t version,VendorId_t vendorId)
{
	const char* const METHOD_NAME = "addHeader";
	try{
		CDRMessage::addOctet(msg,'R');
		CDRMessage::addOctet(msg,'T');
		CDRMessage::addOctet(msg,'P');
		CDRMessage::addOctet(msg,'S');

		CDRMessage::addOctet(msg,version.m_major);
		CDRMessage::addOctet(msg,version.m_minor);

		CDRMessage::addOctet(msg,vendorId[0]);
		CDRMessage::addOctet(msg,vendorId[1]);

		for (uint8_t i = 0;i<12;i++){
			CDRMessage::addOctet(msg,guidPrefix.value[i]);
		}
		msg->length = msg->pos;
	}
	catch(int e)
	{
		logError(RTPS_CDR_MSG,"Header creation fails. "<< e <<endl);
		return false;
	}

	return true;
}

bool RTPSMessageCreator::addHeader(CDRMessage_t*msg, const GuidPrefix_t& guidPrefix)
{
	ProtocolVersion_t prot;
	PROTOCOLVERSION(prot);
	VendorId_t vend;
	VENDORID_EPROSIMA(vend);
	return RTPSMessageCreator::addHeader(msg,guidPrefix,prot,vend);
}


bool RTPSMessageCreator::addSubmessageHeader(CDRMessage_t* msg,
		octet id,octet flags,uint16_t size) {
	const char* const METHOD_NAME = "addSubmessageHeader";
	try{
		CDRMessage::addOctet(msg,id);
		CDRMessage::addOctet(msg,flags);
		CDRMessage::addUInt16(msg, size);
		msg->length = msg->pos;
	}
	catch(int e){

		logError(RTPS_CDR_MSG,"Submessage Header creation fails. "<< e <<endl);
		return false;
	}

	return true;
}

bool RTPSMessageCreator::addSubmessageInfoTS(CDRMessage_t* msg,Time_t& time,bool invalidateFlag)
{
	const char* const METHOD_NAME = "addSubmessageInfoTS";
	octet flags = 0x0;
	uint16_t size = 8;
	if(EPROSIMA_ENDIAN == LITTLEEND)
	{
		flags = flags | BIT(0);
		msg->msg_endian  = LITTLEEND;
	}
	else
	{
		msg->msg_endian = BIGEND;
	}
	if(invalidateFlag)
	{
		flags = flags | BIT(1);
		size = 0;
	}
	try{
		CDRMessage::addOctet(msg,INFO_TS);
		CDRMessage::addOctet(msg,flags);
		CDRMessage::addUInt16(msg, size);
		if(!invalidateFlag)
		{
			CDRMessage::addInt32(msg,time.seconds);
			CDRMessage::addUInt32(msg,time.fraction);
		}
	}
	catch(int e)
	{
		logError(RTPS_CDR_MSG,"Submessage Header creation fails."<<e<<endl);
		return false;
	}
	return true;
}

bool RTPSMessageCreator::addSubmessageInfoDST(CDRMessage_t* msg, GuidPrefix_t guidP)
{
	const char* const METHOD_NAME = "addSubmessageInfoDST";
	octet flags = 0x0;
	uint16_t size = 12;
	if(EPROSIMA_ENDIAN == LITTLEEND)
	{
		flags = flags | BIT(0);
		msg->msg_endian  = LITTLEEND;
	}
	else
	{
		msg->msg_endian = BIGEND;
	}
	try{
		CDRMessage::addOctet(msg,INFO_DST);
		CDRMessage::addOctet(msg,flags);
		CDRMessage::addUInt16(msg, size);
		CDRMessage::addData(msg,guidP.value,12);
	}
	catch(int e)
	{
		logError(RTPS_CDR_MSG,"Submessage Header creation fails."<<e<<endl);
		return false;
	}
	return true;
}



bool RTPSMessageCreator::addSubmessageInfoTS_Now(CDRMessage_t* msg,bool invalidateFlag)
{
	Time_t time_now;
	g_clock.setTimeNow(&time_now);
	return RTPSMessageCreator::addSubmessageInfoTS(msg,time_now,invalidateFlag);
}
}
}; /* namespace rtps */
}; /* namespace eprosima */


#include "submessages/DataMsg.hpp"
#include "submessages/HeartbeatMsg.hpp"
#include "submessages/AckNackMsg.hpp"
#include "submessages/GapMsg.hpp"
