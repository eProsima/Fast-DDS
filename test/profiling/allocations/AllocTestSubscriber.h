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

/**
 * @file AllocTestSubscriber.h
 *
 */

#ifndef ALLOCTESTSUBSCRIBER_H_
#define ALLOCTESTSUBSCRIBER_H_

#include "AllocTestTypePubSubTypes.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/subscriber/SampleInfo.h>
#include <string>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include "AllocTestType.h"

class AllocTestSubscriber {
public:
	AllocTestSubscriber();
	virtual ~AllocTestSubscriber();
	//!Initialize the subscriber
	bool init(const char* profile, int domainId, const std::string& outputFile);
	//!RUN the subscriber
	void run(bool wait_unmatch=false);
	//!Run the subscriber until number samples have been recevied.
	void run(uint32_t number, bool wait_unmatch = false);
private:
	eprosima::fastrtps::Participant* mp_participant;
	eprosima::fastrtps::Subscriber* mp_subscriber;
	std::string m_profile;
	std::string m_outputFile;
public:
	class SubListener:public eprosima::fastrtps::SubscriberListener
	{
	public:
		SubListener():n_matched(0),n_samples(0){}
		~SubListener(){}

		void onSubscriptionMatched(eprosima::fastrtps::Subscriber* sub, eprosima::fastrtps::rtps::MatchingInfo& info);
		void onNewDataMessage(eprosima::fastrtps::Subscriber* sub);
        
        void wait_match();
        void wait_unmatch();
        void wait_until_total_received_at_least(uint32_t n);

    private:
        AllocTestType m_Hello;
        eprosima::fastrtps::SampleInfo_t m_info;
        int n_matched;
        uint32_t n_samples;
        std::mutex mtx;
        std::condition_variable cv;
    }m_listener;
private:
	AllocTestTypePubSubType m_type;
};

#endif /* ALLOCTESTSUBSCRIBER_H_ */
