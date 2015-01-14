/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file HelloWorldPublisher.h
 *
 */

#ifndef HELLOWORLDPUBLISHER_H_
#define HELLOWORLDPUBLISHER_H_

#include "HelloWorldType.h"

#include "fastrtps/fastrtps_fwd.h"
#include "fastrtps/attributes/PublisherAttributes.h"
#include "fastrtps/publisher/PublisherListener.h"
using namespace eprosima::fastrtps;

#include "HelloWorld.h"

class HelloWorldPublisher {
public:
	HelloWorldPublisher();
	virtual ~HelloWorldPublisher();
	//!Initialize
	bool init();
	//!Publish a sample
	bool publish();
	//!Run for number samples
	void run(uint32_t number);
private:
	HelloWorld m_Hello;
	Participant* mp_participant;
	Publisher* mp_publisher;
	class PubListener:public PublisherListener
	{
	public:
		PubListener():n_matched(0){};
		~PubListener(){};
		void onPublicationMatched(Publisher* pub,MatchingInfo info);
		int n_matched;
	}m_listener;
	HelloWorldType m_type;
};



#endif /* HELLOWORLDPUBLISHER_H_ */
