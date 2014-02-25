/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * Participant.cpp
 *
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#include "eprosimartps/Participant.h"

namespace eprosima {
namespace rtps {

Participant::Participant() {
	// TODO Auto-generated constructor stub
	//So the the service nevers stops until we say so
	boost::asio::io_service::work work(sendService);
}

Participant::~Participant() {
	// TODO Auto-generated destructor stub
	sendService.stop();
}

void Participant::sendSync(CDRMessage_t msg, Locator_t loc) {
	cout << "Mandando mensaje de longitud: " << msg.length << endl;
}



} /* namespace rtps */
} /* namespace eprosima */

