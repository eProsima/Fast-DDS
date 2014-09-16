/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SimplePublisher.h
 *
 */

#ifndef SIMPLEPUBLISHER_H
#define SIMPLEPUBLISHER_H

#include "eprosimartps/rtps_all.h"


class SimplePublisher {
public:
	SimplePublisher();
	virtual ~SimplePublisher();
	bool init();
	void run();
private:
	Participant* mp_participant;
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



#endif /* SIMPLEPUBLISHER_H */
