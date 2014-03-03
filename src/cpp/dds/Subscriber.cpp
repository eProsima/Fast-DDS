/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * Subscriber.cpp
 *
 *  Created on: Feb 27, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/Subscriber.h"
#include "eprosimartps/RTPSReader.h"

namespace eprosima {
namespace dds {

Subscriber::Subscriber() {
	// TODO Auto-generated constructor stub

}

Subscriber::Subscriber(RTPSReader* Rin) {
	R = Rin;
}


Subscriber::~Subscriber() {
	// TODO Auto-generated destructor stub
}

void Subscriber::read(void**) {
}

void Subscriber::take(void**) {
}

void Subscriber::newMessage() {
}

} /* namespace dds */
} /* namespace eprosima */

