/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * ThreadSend.cpp
 *
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#include "eprosimartps/ThreadSend.h"

namespace eprosima {
namespace rtps {

ThreadSend::ThreadSend() {
	// TODO Auto-generated constructor stub
	//Create socket

	//Found out about my IP
	//Fake IP for now.
	sendLocator.kind = LOCATOR_KIND_UDPv4;
	sendLocator.port = 4243;
	LOCATOR_ADDRESS_INVALID(sendLocator.address);
	sendLocator.address[12] = 192;
	sendLocator.address[13] = 168;
	sendLocator.address[14] = 1;
	sendLocator.address[15] = 18;


	//boost::asio::io_service::work work(sendService);
}

ThreadSend::~ThreadSend() {
	// TODO Auto-generated destructor stub
	//sendService.stop();
}

void ThreadSend::sendSync(CDRMessage_t msg, Locator_t loc) {
	cout << "Mandando mensaje de longitud: " << msg.length << endl;
}


} /* namespace rtps */
} /* namespace eprosima */
