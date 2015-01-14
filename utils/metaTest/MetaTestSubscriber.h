/*************************************************************************
 * Copyright (c) 2015 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file MetaTestSubscriber.h
 *
 */

#ifndef METATESTSUBSCRIBER_H_
#define METATESTSUBSCRIBER_H_

#include "fastrtps/fastrtps_fwd.h"
#include "fastrtps/publisher/PublisherListener.h"
#include "fastrtps/subscriber/SubscriberListener.h"
#include "types/MetaTestTypes.h"
#include "types/MetaTestTypesPubSubType.h"

using namespace eprosima::fastrtps;

namespace eprosima {

class MetaTestSubscriber {
public:
	MetaTestSubscriber();
	virtual ~MetaTestSubscriber();
	bool init();
	void run();

private:
	Participant* mp_par;
	Publisher* mp_pub;
	Subscriber* mp_sub;
	MetaTestTypesPubSubType m_dataType;
	class MyPubListen : public PublisherListener
	{
	public:
		MyPubListen():n_matched(0){};
		~MyPubListen(){};
		void onPublicationMatched(Publisher* pub,MatchingInfo info)
		{

			if(info.status == MATCHED_MATCHING)
			{
				n_matched++;
				cout << "Publication Matched"<<endl;
			}
			else if(info.status == REMOVED_MATCHING) n_matched--;
		}
		int n_matched;
	}m_publisten;
	class MySubListen : public SubscriberListener
	{
	public:
		MySubListen():n_matched(0){};
		~MySubListen(){};
		void onSubscriptionMatched(Subscriber* pub,MatchingInfo info)
		{
			if(info.status == MATCHED_MATCHING){
				n_matched++;
				cout << "Subscription Matched"<<endl;
			}
			else if(info.status == REMOVED_MATCHING) n_matched--;
		}
		//void onNewDataMessage(Subscriber* sub);
		int n_matched;
	}m_sublisten;

	//TEST METHODS:
	void t_rtps_socket(MetaTestType& testinfo);
	void t_rtps_registered(MetaTestType& testinfo);
	void t_client_server(MetaTestType& testinfo);
	void t_hello_world(MetaTestType& testinfo);

};

} /* namespace eprosima */

#endif /* METATESTSUBSCRIBER_H_ */
