/*************************************************************************
 * Copyright (c) 2015 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file MetaTestPublisher.h
 *
 */

#ifndef METATESTPUBLISHER_H_
#define METATESTPUBLISHER_H_

#include "fastrtps/fastrtps_fwd.h"
#include "fastrtps/publisher/PublisherListener.h"
#include "fastrtps/subscriber/SubscriberListener.h"
using namespace eprosima::fastrtps;

namespace eprosima {

class MetaTestPublisher {
public:
	MetaTestPublisher();
	virtual ~MetaTestPublisher();
	bool init();
	void run();

private:
	Participant* mp_par;
	Publisher* mp_pub;
	Subscriber* mp_sub;


	class MyPubListen : public PublisherListener
	{
	public:
		MyPubListen():n_matched(0){};
		~MyPubListen(){};
		void onPublicationMatched(Publisher* pub,MatchingInfo info)
		{
			if(info.status == MATCHED_MATCHING) n_matched++;
			else if(info.status == REMOVED_MATCHING) n_matched--;
		}
		int n_matched;
	}m_publisten;
	class MySubListen : public SubscriberListener
	{
	public:
		MySubListen():n_matched(0){};
		~MySubListen(){};
		void onSubscriptionMatched(Publisher* pub,MatchingInfo info)
		{
			if(info.status == MATCHED_MATCHING) n_matched++;
			else if(info.status == REMOVED_MATCHING) n_matched--;
		}
		void onNewDataMessage(Subscriber* sub);
		int n_matched;
	}m_sublisten;
};

} /* namespace eprosima */

#endif /* METATESTPUBLISHER_H_ */
