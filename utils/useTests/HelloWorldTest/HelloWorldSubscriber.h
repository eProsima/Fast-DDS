/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file HelloWorldSubscriber.h
 *
 */

#ifndef HELLOWORLDSUBSCRIBER_H_
#define HELLOWORLDSUBSCRIBER_H_

#include "fastrtps/rtps_all.h"
#include "HelloWorld.h"

class HelloWorldSubscriber {
public:
	HelloWorldSubscriber();
	virtual ~HelloWorldSubscriber();
private:
	RTPSParticipant* mp_RTPSParticipant;
	Subscriber* mp_subscriber;
	class SubListener:public SubscriberListener
	{
	public:
		SubListener():mp_sub(NULL),n_matched(0){};
		~SubListener(){};
		void onSubscriptionMatched(MatchingInfo info);
		void onNewDataMessage();
		HelloWorld m_Hello;
		SampleInfo_t m_info;
		Subscriber* mp_sub;
		int n_matched;
	}m_listener;

};

#endif /* HELLOWORLDSUBSCRIBER_H_ */
