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
 * @file BenchMarkPublisher.h
 *
 */

#ifndef BENCHMARKPUBLISHER_H_
#define BENCHMARKPUBLISHER_H_

#include "BenchMarkPubSubTypes.h"
#include "BenchMark_smallPubSubTypes.h"
#include "BenchMark_mediumPubSubTypes.h"
#include "BenchMark_bigPubSubTypes.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/subscriber/SampleInfo.h>


#include "BenchMark.h"
#include "BenchMark_small.h"
#include "BenchMark_medium.h"
#include "BenchMark_big.h"

class BenchMarkPublisher {
public:
	BenchMarkPublisher();
	virtual ~BenchMarkPublisher();
	//!Initialize
	bool init(int transport, eprosima::fastrtps::ReliabilityQosPolicyKind kind, int time, const std::string& topicName, int domain, int size);
	//!Publish a sample
	bool publish();
	//!Run for number samples
	void run();
private:
	int m_iSize;
	BenchMark m_Hello;
	BenchMarkSmall m_HelloSmall;
	BenchMarkMedium m_HelloMedium;
	BenchMarkBig m_HelloBig;

	eprosima::fastrtps::Participant* mp_participant;
	eprosima::fastrtps::Publisher* mp_publisher;
    eprosima::fastrtps::Subscriber* mp_subscriber;

	class PubListener:public eprosima::fastrtps::PublisherListener
	{
	public:
		PubListener() {}
		PubListener(BenchMarkPublisher* parent);

		~PubListener(){};

        void onPublicationMatched(eprosima::fastrtps::Publisher* pub, eprosima::fastrtps::rtps::MatchingInfo& info);

		BenchMarkPublisher* mParent;
        int n_matched;
	}m_pubListener;

    class SubListener :public eprosima::fastrtps::SubscriberListener
    {
    public:
        SubListener() {}
        SubListener(BenchMarkPublisher* parent);

        ~SubListener() {};

        void onSubscriptionMatched(eprosima::fastrtps::Subscriber* sub, eprosima::fastrtps::rtps::MatchingInfo& info);

        void onNewDataMessage(eprosima::fastrtps::Subscriber* sub);

        BenchMarkPublisher* mParent;
		BenchMark m_Hello;
		BenchMarkSmall m_HelloSmall;
		BenchMarkMedium m_HelloMedium;
		BenchMarkBig m_HelloBig;
		eprosima::fastrtps::SampleInfo_t m_info;
    }m_subListener;

	BenchMarkPubSubType m_type;
	BenchMarkSmallPubSubType m_typeSmall;
	BenchMarkMediumPubSubType m_typeMedium;
	BenchMarkBigPubSubType m_typeBig;
	void runThread();
};



#endif /* BENCHMARKPUBLISHER_H_ */
