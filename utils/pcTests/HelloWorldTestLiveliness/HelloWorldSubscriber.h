/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file HelloWorldSubscriber.h
 *
 */

#ifndef HELLOWORLDSUBSCRIBER_H_
#define HELLOWORLDSUBSCRIBER_H_

#include "HelloWorldType.h"

#include "fastrtps/fastrtps_fwd.h"
#include "fastrtps/attributes/SubscriberAttributes.h"
#include "fastrtps/subscriber/SubscriberListener.h"
#include "fastrtps/subscriber/SampleInfo.h"


using namespace eprosima::fastrtps;

#include "HelloWorld.h"

class HelloWorldSubscriber {
public:
	HelloWorldSubscriber();
	virtual ~HelloWorldSubscriber();
	//!Initialize the subscriber
	bool init();
	//!RUN the subscriber
	void run();
	//!Run the subscriber until number samples have been recevied.
	void run(uint32_t number);
private:
	Participant* mp_participant;
	Subscriber* mp_subscriber;
public:
	class SubListener:public SubscriberListener
	{
	public:
		SubListener():n_matched(0),n_samples(0){};
		~SubListener(){};
		void onSubscriptionMatched(Subscriber* sub,MatchingInfo& info);
		void onNewDataMessage(Subscriber* sub);
		HelloWorld m_Hello;
		SampleInfo_t m_info;
		int n_matched;
		uint32_t n_samples;
	}m_listener;
private:
	HelloWorldType m_type;
};

#endif /* HELLOWORLDSUBSCRIBER_H_ */
