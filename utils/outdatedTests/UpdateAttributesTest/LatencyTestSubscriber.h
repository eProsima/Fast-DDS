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

#include "LatencyTestTypes.h"

class LatencyTestSubscriber {
public:
	LatencyTestSubscriber();
	virtual ~LatencyTestSubscriber();

	RTPSParticipant* mp_RTPSParticipant;
	Publisher* mp_datapub;
	Publisher* mp_commandpub;
	Subscriber* mp_datasub;
	Subscriber* mp_commandsub;
	LatencyType* mp_latency;
	SampleInfo_t m_sampleinfo;
	boost::interprocess::interprocess_semaphore m_disc_sema;
	boost::interprocess::interprocess_semaphore m_comm_sema;
	boost::interprocess::interprocess_semaphore m_data_sema;
	int m_status;
	int n_received;
	int n_samples;
	bool init(bool echo,int nsam);
	void run();
	bool test(uint32_t datasize);
	class DataPubListener : public PublisherListener
	{
	public:
		DataPubListener(LatencyTestSubscriber* up):mp_up(up){}
		~DataPubListener(){}
		void onPublicationMatched(MatchingInfo info);
		LatencyTestSubscriber* mp_up;
	}m_datapublistener;
	class DataSubListener : public SubscriberListener
	{
	public:
		DataSubListener(LatencyTestSubscriber* up):mp_up(up){}
		~DataSubListener(){}
		void onSubscriptionMatched(MatchingInfo into);
		void onNewDataMessage();
		LatencyTestSubscriber* mp_up;
	}m_datasublistener;
	class CommandPubListener : public PublisherListener
	{
	public:
		CommandPubListener(LatencyTestSubscriber* up):mp_up(up){}
		~CommandPubListener(){}
		void onPublicationMatched(MatchingInfo info);
		LatencyTestSubscriber* mp_up;
	}m_commandpublistener;
	class CommandSubListener : public SubscriberListener
	{
	public:
		CommandSubListener(LatencyTestSubscriber* up):mp_up(up){}
		~CommandSubListener(){}
		void onSubscriptionMatched(MatchingInfo into);
		void onNewDataMessage();
		LatencyTestSubscriber* mp_up;
	}m_commandsublistener;
	bool m_echo;

};

#endif /* LATENCYTESTSUBSCRIBER_H_ */
