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
#include "eprosimartps/ParameterListCreator.h"

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"

using namespace boost::posix_time;

namespace eprosima {
namespace rtps{


#if defined(__LITTLE_ENDIAN__)
	const Endianness_t DEFAULT_ENDIAN = LITTLEEND;
#elif defined (__BIG_ENDIAN__)
	const Endianness_t DEFAULT_ENDIAN = BIGEND;
#endif
};
};

using namespace eprosima::dds;

namespace eprosima {
namespace rtps{


CDRMessageCreator::CDRMessageCreator() {
	// TODO Auto-generated constructor stub


}

CDRMessageCreator::~CDRMessageCreator() {
	// TODO Auto-generated destructor stub
}


bool CDRMessageCreator::createHeader(CDRMessage_t*msg, GuidPrefix_t guidPrefix,
		ProtocolVersion_t version,VendorId_t vendorId)
{

	try{
		CDRMessage::addOctet(msg,'R');
		CDRMessage::addOctet(msg,'T');
		CDRMessage::addOctet(msg,'P');
		CDRMessage::addOctet(msg,'S');

		CDRMessage::addOctet(msg,version.major);
		CDRMessage::addOctet(msg,version.minor);

		CDRMessage::addOctet(msg,vendorId[0]);
		CDRMessage::addOctet(msg,vendorId[1]);

		for (uint i = 0;i<12;i++){
			CDRMessage::addOctet(msg,guidPrefix.value[i]);
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

bool CDRMessageCreator::createHeader(CDRMessage_t*msg, GuidPrefix_t guidPrefix)
{
	ProtocolVersion_t prot;
	PROTOCOLVERSION(prot);
	VendorId_t vend;
	VENDORID_EPROSIMA(vend);
	return createHeader(msg,guidPrefix,prot,vend);
}


bool CDRMessageCreator::createSubmessageHeader(CDRMessage_t* msg,
		octet id,octet flags,uint16_t size) {

	try{
		CDRMessage::addOctet(msg,id);
		CDRMessage::addOctet(msg,flags);
		CDRMessage::addUInt16(msg, size);
		msg->length = msg->pos;
	}
	catch(int e){
		RTPSLog::Error << B_RED << "Submessage Header creation fails: "<< e << DEF<< endl;
		RTPSLog::printError();
		return false;
	}

	return true;
}

bool CDRMessageCreator::createSubmessageInfoTS(CDRMessage_t* msg,Time_t time,bool invalidateFlag)
{
	CDRMessage::initCDRMsg(msg,RTPSMESSAGE_INFOTS_SIZE);
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
		RTPSLog::Error << B_RED << "Submessage creation fails: "<< e << DEF<< endl;
		RTPSLog::printError();
		return false;
	}
	return true;
}

bool CDRMessageCreator::createSubmessageInfoTS_Now(CDRMessage_t* msg,bool invalidateFlag)
{
	Time_t time_now;
	boost::posix_time::ptime t(microsec_clock::local_time());
	boost::posix_time::ptime t_epoch(boost::gregorian::date(1900,1,1),boost::posix_time::time_duration(0,0,0));

	time_now.seconds = (int32_t)(t-t_epoch).total_seconds();
	time_now.fraction = (t-t_epoch).fractional_seconds()*(int32_t)(pow(2,32)*pow(10,-boost::posix_time::time_duration::num_fractional_digits()));
//	cout << t << endl;
//	cout << (t-t_epoch) << endl;
//	cout << time_now.seconds << endl;
//	cout << time_now.fraction << endl;
	return CDRMessageCreator::createSubmessageInfoTS(msg,time_now,invalidateFlag);
}

}; /* namespace rtps */
}; /* namespace eprosima */


#include "submessages/DataMsg2.hpp"
#include "submessages/HeartbeatMsg2.hpp"
#include "submessages/AckNackMsg2.hpp"
#include "submessages/GapMsg2.hpp"
