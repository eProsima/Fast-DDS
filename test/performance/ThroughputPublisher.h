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
 * @file ThroughputPublisher.h
 *
 */

#ifndef THROUGHPUTPUBLISHER_H_
#define THROUGHPUTPUBLISHER_H_

#include <asio.hpp>

#include "ThroughputTypes.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/subscriber/SubscriberListener.h>

using namespace eprosima::fastrtps;

#include <condition_variable>
#include <chrono>
#include <map>
#include <vector>
#include <string>

using namespace std;

class ThroughputPublisher
{
public:
	ThroughputPublisher(bool reliable, uint32_t pid, bool hostname, bool export_csv);
	virtual ~ThroughputPublisher();
	Participant* mp_par;
	Publisher* mp_datapub;
	Publisher* mp_commandpub;
	Subscriber* mp_commandsub;
    std::chrono::steady_clock::time_point t_start_, t_end_;
    std::chrono::duration<double, std::micro> t_overhead_;
    std::mutex mutex_;
    int disc_count_;
    std::condition_variable disc_cond_;
	class DataPubListener:public PublisherListener
	{
	public:
		DataPubListener(ThroughputPublisher& up);
		virtual ~DataPubListener();
		ThroughputPublisher& m_up;
		void onPublicationMatched(Publisher* pub,MatchingInfo& info);

	private:

		DataPubListener& operator=(const DataPubListener&);
	}m_DataPubListener;

	class CommandSubListener:public SubscriberListener
	{
	public:
		CommandSubListener(ThroughputPublisher& up);
		virtual ~CommandSubListener();
		ThroughputPublisher& m_up;
		void onSubscriptionMatched(Subscriber* sub,MatchingInfo& info);

	private:

		CommandSubListener& operator=(const CommandSubListener&);
	}m_CommandSubListener;
	class CommandPubListener:public PublisherListener
	{
	public:
		CommandPubListener(ThroughputPublisher& up);
		virtual ~CommandPubListener();
		ThroughputPublisher& m_up;
		void onPublicationMatched(Publisher* pub,MatchingInfo& info);

	private:

		CommandPubListener& operator=(const CommandPubListener&);
	}m_CommandPubListener;


	bool ready;

	void run(uint32_t test_time, uint32_t recovery_time_ms, int demand, int msg_size);
	bool test(uint32_t test_time, uint32_t recovery_time_ms, uint32_t demand, uint32_t size);
	std::vector<TroughputResults> m_timeStats;
	ThroughputDataType latency_t;
    ThroughputCommandDataType throuputcommand_t;

	bool loadDemandsPayload();
	std::map<uint32_t,std::vector<uint32_t>> m_demand_payload;

	std::string m_file_name;
	bool m_export_csv;
	std::stringstream output_file;
	uint32_t payload;
    bool reliable_;
};



#endif /* THROUGHPUTPUBLISHER_H_ */
