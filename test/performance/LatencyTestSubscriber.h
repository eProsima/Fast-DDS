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
 * @file LatencyTestSubscriber.h
 *
 */

#ifndef LATENCYTESTSUBSCRIBER_H_
#define LATENCYTESTSUBSCRIBER_H_

#include <asio.hpp>
#include <condition_variable>
#include "LatencyTestTypes.h"

class LatencyTestSubscriber {
public:
	LatencyTestSubscriber();
	virtual ~LatencyTestSubscriber();

	Participant* mp_participant;
	Publisher* mp_datapub;
	Publisher* mp_commandpub;
	Subscriber* mp_datasub;
	Subscriber* mp_commandsub;
	LatencyType* mp_latency;
	SampleInfo_t m_sampleinfo;
    std::mutex mutex_;
    int disc_count_;
	std::condition_variable disc_cond_;
    int comm_count_;
	std::condition_variable comm_cond_;
    int data_count_;
	std::condition_variable data_cond_;
	int m_status;
	int n_received;
	int n_samples;
	bool init(bool echo, int nsam, bool reliable, uint32_t pid, bool hostname);
	void run();
	bool test(uint32_t datasize);
	class DataPubListener : public PublisherListener
	{
	public:
		DataPubListener(LatencyTestSubscriber* up):mp_up(up){}
		~DataPubListener(){}
		void onPublicationMatched(Publisher* pub,MatchingInfo& info);
		LatencyTestSubscriber* mp_up;
	}m_datapublistener;
	class DataSubListener : public SubscriberListener
	{
	public:
		DataSubListener(LatencyTestSubscriber* up):mp_up(up){}
		~DataSubListener(){}
		void onSubscriptionMatched(Subscriber* sub,MatchingInfo& into);
		void onNewDataMessage(Subscriber* sub);
		LatencyTestSubscriber* mp_up;
	}m_datasublistener;
	class CommandPubListener : public PublisherListener
	{
	public:
		CommandPubListener(LatencyTestSubscriber* up):mp_up(up){}
		~CommandPubListener(){}
		void onPublicationMatched(Publisher* pub,MatchingInfo& info);
		LatencyTestSubscriber* mp_up;
	}m_commandpublistener;
	class CommandSubListener : public SubscriberListener
	{
	public:
		CommandSubListener(LatencyTestSubscriber* up):mp_up(up){}
		~CommandSubListener(){}
		void onSubscriptionMatched(Subscriber* sub,MatchingInfo& into);
		void onNewDataMessage(Subscriber* sub);
		LatencyTestSubscriber* mp_up;
	}m_commandsublistener;
	bool m_echo;
	LatencyDataType latency_t;
	TestCommandDataType command_t;
};

#endif /* LATENCYTESTSUBSCRIBER_H_ */
