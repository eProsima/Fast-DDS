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
 * @file ThroughputSubscriber.h
 *
 */

#ifndef THROUGHPUTSUBSCRIBER_H_
#define THROUGHPUTSUBSCRIBER_H_

#include <asio.hpp>

#include "ThroughputTypes.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/subscriber/SampleInfo.h>


using namespace eprosima::fastrtps;

#include <condition_variable>
#include <chrono>

#include <fstream>
#include <iostream>

using namespace std;

class ThroughputSubscriber
{
public:
	ThroughputSubscriber(bool reliable, uint32_t pid, bool hostname);
	virtual ~ThroughputSubscriber();
	Participant* mp_par;
	Subscriber* mp_datasub;
	Publisher* mp_commandpubli;
	Subscriber* mp_commandsub;
    std::chrono::steady_clock::time_point t_start_, t_end_;
    std::chrono::duration<double, std::micro> t_overhead_;
    std::mutex mutex_;
    int disc_count_;
    std::condition_variable disc_cond_;
    //! 0 - Continuing test, 1 - End of a test, 2 - Finish application
    int stop_count_;
    std::condition_variable stop_cond_;
	class DataSubListener:public SubscriberListener
	{
	public:
		DataSubListener(ThroughputSubscriber& up);
		virtual ~DataSubListener();
		ThroughputSubscriber& m_up;
		void reset();
		uint32_t lastseqnum,saved_lastseqnum;
		uint32_t lostsamples,saved_lostsamples;
		bool first;
		ThroughputType* throughputin;
		SampleInfo_t info;
		void onSubscriptionMatched(Subscriber* sub,MatchingInfo& info);
		void onNewDataMessage(Subscriber* sub);
		void saveNumbers();
		std::ofstream myfile;
	}m_DataSubListener;

	class CommandSubListener:public SubscriberListener
	{
	public:
		CommandSubListener(ThroughputSubscriber& up);
		virtual ~CommandSubListener();
		ThroughputSubscriber& m_up;
		ThroughputCommandType m_commandin;
		SampleInfo_t info;
		void onSubscriptionMatched(Subscriber* sub,MatchingInfo& info);
		void onNewDataMessage(Subscriber* sub);
		void saveNumbers();

	private:

		CommandSubListener& operator=(const CommandSubListener&);
	}m_CommandSubListener;
	class CommandPubListener:public PublisherListener
	{
	public:
		CommandPubListener(ThroughputSubscriber& up);
		virtual ~CommandPubListener();
		ThroughputSubscriber& m_up;
		void onPublicationMatched(Publisher* pub,MatchingInfo& info);

	private:

		CommandPubListener& operator=(const CommandPubListener&);
	}m_CommandPubListener;
	bool ready;
	uint32_t m_datasize;
	uint32_t m_demand;
	void run();
	ThroughputDataType throughput_t;
	ThroughputCommandDataType throuputcommand_t;
};



#endif /* THROUGHPUTSUBSCRIBER_H_ */
