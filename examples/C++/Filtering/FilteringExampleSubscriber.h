// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _FILTERINGEXAMPLE_SUBSCRIBER_H_
#define _FILTERINGEXAMPLE_SUBSCRIBER_H_

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/subscriber/SampleInfo.h>
#include "FilteringExamplePubSubTypes.h"

using namespace eprosima::fastrtps;

class FilteringExampleSubscriber 
{
public:
	FilteringExampleSubscriber();
	virtual ~FilteringExampleSubscriber();
	bool init(int type);
	void run();
private:
	Participant *mp_participant;
	Subscriber *mp_subscriber;
	
	class SubListener : public SubscriberListener
	{
	public:
		SubListener() : n_matched(0),n_msg(0){};
		~SubListener(){};
		void onSubscriptionMatched(Subscriber* sub,MatchingInfo& info);
		void onNewDataMessage(Subscriber* sub);
		SampleInfo_t m_info;
		int n_matched;
		int n_msg;
	} m_listener;
	FilteringExamplePubSubType myType;
};

#endif // _FilteringExample_SUBSCRIBER_H_
