/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file HelloWorldPublisher.h
 *
 */

#ifndef HELLOWORLDPUBLISHER_H_
#define HELLOWORLDPUBLISHER_H_

#include "eprosimartps/rtps_all.h"
#include "HelloWorld.h"

class HelloWorldPublisher {
public:
	HelloWorldPublisher();
	virtual ~HelloWorldPublisher();
	bool publish();
private:
	HelloWorld m_Hello;
	RTPSParticipant* mp_RTPSParticipant;
	Publisher* mp_publisher;
	class PubListener:public PublisherListener
	{
	public:
		PubListener():n_matched(0){};
		~PubListener(){};
		void onPublicationMatched(MatchingInfo info);
		int n_matched;
	}m_listener;

};



#endif /* HELLOWORLDPUBLISHER_H_ */
