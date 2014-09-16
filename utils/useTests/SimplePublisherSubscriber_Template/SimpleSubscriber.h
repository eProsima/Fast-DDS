/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SimpleSubscriber.h
 *
 */

#ifndef SIMPLESUBSCRIBER_H_
#define SIMPLESUBSCRIBER_H_

#include "eprosimartps/rtps_all.h"


class SimpleSubscriber {
public:
	SimpleSubscriber();
	virtual ~SimpleSubscriber();
	bool init();
	void run();
private:
	Participant* mp_participant;
	Subscriber* mp_subscriber;
	//LISTENER
	class SubListener:public SubscriberListener
	{
	public:
		SubListener():mp_sub(NULL),n_matched(0){};
		~SubListener(){};
		void onSubscriptionMatched(MatchingInfo info);
		void onNewDataMessage();
		SampleInfo_t m_info;
		Subscriber* mp_sub;
		int n_matched;
	}m_listener;
};

#endif /* SIMPLESUBSCRIBER_H_ */
