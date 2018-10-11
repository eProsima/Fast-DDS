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
 * @file BenchmarkSubscriber.h
 *
 */

#ifndef BENCHMARK_SUBSCRIBER_H_
#define BENCHMARK_SUBSCRIBER_H_

#include "BenchmarkPubSubTypes.h"
#include "Benchmark_smallPubSubTypes.h"
#include "Benchmark_mediumPubSubTypes.h"
#include "Benchmark_bigPubSubTypes.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/subscriber/SampleInfo.h>
//#include <fastrtps/types/DynamicDataPtr.h>
//#include <fastrtps/types/DynamicPubSubType.h>

#include "Benchmark.h"
#include "Benchmark_small.h"
#include "Benchmark_medium.h"
#include "Benchmark_big.h"

class BenchMarkSubscriber
{
public:
	BenchMarkSubscriber();

	virtual ~BenchMarkSubscriber();

    //!Initialize the subscriber
	bool init(int transport, eprosima::fastrtps::ReliabilityQosPolicyKind kind, const std::string& topicName,
        int domain, int size/*, bool dynamicTypes*/);

    //!RUN the subscriber
	void run();

private:

    BenchMarkBig m_HelloBig;
    BenchMarkMedium m_HelloMedium;
    BenchMarkSmall m_HelloSmall;
    BenchMark m_Hello;
    //eprosima::fastrtps::types::DynamicData_ptr m_DynamicData;

	int m_iSize;
    eprosima::fastrtps::Participant* mp_participant;
    eprosima::fastrtps::Publisher* mp_publisher;
    eprosima::fastrtps::Subscriber* mp_subscriber;

public:

    class PubListener :public eprosima::fastrtps::PublisherListener
    {
    public:
        PubListener(BenchMarkSubscriber* parent);

        ~PubListener() {};

        void onPublicationMatched(eprosima::fastrtps::Publisher* pub, eprosima::fastrtps::rtps::MatchingInfo& info);

		BenchMarkSubscriber* mParent;
		int n_matched;
        bool firstConnected;
    }m_pubListener;

    class SubListener :public eprosima::fastrtps::SubscriberListener
    {
    public:
        SubListener() {}
        SubListener(BenchMarkSubscriber* parent);

        ~SubListener() {};

        void onSubscriptionMatched(eprosima::fastrtps::Subscriber* sub, eprosima::fastrtps::rtps::MatchingInfo& info);

        void onNewDataMessage(eprosima::fastrtps::Subscriber* sub);

        BenchMarkSubscriber* mParent;
		BenchMark m_Hello;
		BenchMarkSmall m_HelloSmall;
		BenchMarkMedium m_HelloMedium;
		BenchMarkBig m_HelloBig;

		eprosima::fastrtps::SampleInfo_t m_info;
        int n_matched;
        uint32_t n_samples;
    }m_subListener;

private:
    //eprosima::fastrtps::types::DynamicPubSubType m_dynType;
	BenchMarkPubSubType m_type;
	BenchMarkSmallPubSubType m_typeSmall;
	BenchMarkMediumPubSubType m_typeMedium;
	BenchMarkBigPubSubType m_typeBig;
    //bool m_bDynamicTypes;
};

#endif /* BENCHMARK_SUBSCRIBER_H_ */
