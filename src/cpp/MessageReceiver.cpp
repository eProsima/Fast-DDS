/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * MessageReceiver.cpp
 *
 *  Created on: Feb 20, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#include "eprosimartps/MessageReceiver.h"

namespace eprosima {
namespace rtps {

MessageReceiver::MessageReceiver() {
	// TODO Auto-generated constructor stub
	PROTOCOLVERSION(sourceVersion);
	VENDORID_UNKNOWN(sourceVendorId);
	GUIDPREFIX_UNKNOWN(sourceGuidPrefix);
	GUIDPREFIX_UNKNOWN(sourceGuidPrefix);
	haveTimestamp = false;
	TIME_INVALID(timestamp);
	unicastReplyLocatorList.clear();
	multicastReplyLocatorList.clear();
}

MessageReceiver::~MessageReceiver() {
	// TODO Auto-generated destructor stub
}

void MessageReceiver::reset(){
	MessageReceiver();
	Locator_t defUniL;
	defUniL.kind = LOCATOR_KIND_UDPv4;
	LOCATOR_ADDRESS_INVALID(defUniL.address);
	defUniL.port = LOCATOR_PORT_INVALID;
	unicastReplyLocatorList.push_back(defUniL);
	multicastReplyLocatorList.push_back(defUniL);
}

void MessageReceiver::processMsg(GuidPrefix_t participantguidprefix,
								octet address[],
								void*data,short length){
	reset();
	destGuidPrefix = participantguidprefix;
	unicastReplyLocatorList[0].address = address;
	msg.buffer = (octet*)data;
	msg.max_size = length;
	msg.w_pos = 0;





}

bool MessageReceiver::extractHeader(CDRMessage_t* msg, Header_t* H) {
	if(msg->buffer[0] != 'R' ||  msg->buffer[1] != 'T' ||
       msg->buffer[2] != 'P' ||  msg->buffer[3] != 'S')
		return false;

	if()


	return true;
}

} /* namespace rtps */
} /* namespace eprosima */
