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
	boost::asio::io_service::work work(sendService);
}

ThreadSend::~ThreadSend() {
	// TODO Auto-generated destructor stub
	sendService.stop();
}

void ThreadSend::sendSync(CDRMessage_t msg, Locator_t loc) {
	cout << "Mandando mensaje de longitud: " << msg.length << endl;
}


} /* namespace rtps */
} /* namespace eprosima */
