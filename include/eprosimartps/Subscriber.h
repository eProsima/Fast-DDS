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

#include <iostream>

#include "rtps_all.h"
#include <boost/signals2.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>


#ifndef SUBSCRIBER_H_
#define SUBSCRIBER_H_

namespace eprosima {

namespace rtps{
class RTPSReader;
}

using namespace rtps;

namespace dds {

class Subscriber {
	friend class DomainParticipant;
	friend class RTPSReader;
public:
	Subscriber();
	Subscriber(RTPSReader* Rin);
	virtual ~Subscriber();

	const std::string& getTopicDataType() const {
		return topicDataType;
	}

	const std::string& getTopicName() const {
		return topicName;
	}

	void blockUntilNewMessage();

	void assignNewMessageCallback(void(*fun)());

	void read(void**);
	void take(void**);

//	void processMsgs();
//
//	void blockUntilNewMessage();

	ParameterList ParamList;
private:

	RTPSReader* R;
	//bool initialized;
	std::string topicName;
	std::string topicDataType;
	TypeReg_t type;

	//boost::signals2::signal<void()> msg_signal;

};

} /* namespace dds */
} /* namespace eprosima */

#endif /* SUBSCRIBER_H_ */
