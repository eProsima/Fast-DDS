/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * Subscriber.h
 *
 *  Created on: Feb 27, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "rtps_all.h"


#ifndef SUBSCRIBER_H_
#define SUBSCRIBER_H_

namespace eprosima {
namespace dds {

class Subscriber {
public:
	Subscriber();
	virtual ~Subscriber();
};

} /* namespace dds */
} /* namespace eprosima */

#endif /* SUBSCRIBER_H_ */
