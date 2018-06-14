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
 * @file BenchMarkPublisher.cpp
 *
 */

#include "BenchMarkPublisher.h"
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/transport/TCPv4TransportDescriptor.h>
#include <fastrtps/transport/UDPv4TransportDescriptor.h>
#include <fastrtps/Domain.h>
#include <fastrtps/utils/eClock.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

static bool g_bBenchmarkFinished = false;
static std::chrono::time_point<std::chrono::system_clock> g_bTestStartTime;
static int g_iTestTimeMs = 10000;
static int g_iCount = 0;

BenchMarkPublisher::BenchMarkPublisher()
    : mp_participant(nullptr)
    , mp_publisher(nullptr)
    , mp_subscriber(nullptr)
	, m_pubListener(this)
	, m_subListener(this)
	, m_iSize(0)
{


}

bool BenchMarkPublisher::init(int transport, ReliabilityQosPolicyKind kind, int time, const std::string& topicName, int domain, int size)
{
	g_iTestTimeMs = time;
	m_iSize = size;
	switch (m_iSize)
	{
	default:
	case 0:
		m_Hello.index(0);
		break;
	case 1:
		m_HelloSmall.index(0);
		break;
	case 2:
		m_HelloMedium.index(0);
		break;
	case 3:
		m_HelloBig.index(0);
		break;
	}

    ParticipantAttributes PParam;
    PParam.rtps.builtin.domainId = domain;
    PParam.rtps.builtin.leaseDuration = c_TimeInfinite;
    PParam.rtps.builtin.leaseDuration_announcementperiod = Duration_t(5, 0);
    PParam.rtps.setName("Participant_pub");

    if (transport == 1)
    {
        PParam.rtps.useBuiltinTransports = true;
    }
    else
    {
        uint32_t kind = LOCATOR_KIND_TCPv4;
        PParam.rtps.useBuiltinTransports = false;

        Locator_t unicast_locator;
        unicast_locator.kind = kind;
        unicast_locator.set_IP4_address("127.0.0.1");
        unicast_locator.set_port(5100);
        unicast_locator.set_logical_port(7410);
        PParam.rtps.defaultUnicastLocatorList.push_back(unicast_locator); // Publisher's data channel

        Locator_t meta_locator;
        meta_locator.kind = kind;
        meta_locator.set_IP4_address("127.0.0.1");
        meta_locator.set_port(5100);
        meta_locator.set_logical_port(7402);
        PParam.rtps.builtin.metatrafficUnicastLocatorList.push_back(meta_locator);  // Publisher's meta channel

        std::shared_ptr<TCPv4TransportDescriptor> descriptor = std::make_shared<TCPv4TransportDescriptor>();
        descriptor->sendBufferSize = 8912896; // 8.5Mb
        descriptor->receiveBufferSize = 8912896; // 8.5Mb
        descriptor->set_WAN_address("127.0.0.1");
        descriptor->add_listener_port(5100);
        PParam.rtps.userTransports.push_back(descriptor);
    }

    mp_participant = Domain::createParticipant(PParam);

    if (mp_participant == nullptr)
    {
        return false;
    }

	//CREATE THE PUBLISHER
	PublisherAttributes Wparam;
	Wparam.topic.topicKind = NO_KEY;

    //REGISTER THE TYPE
	switch (m_iSize)
	{
	default:
	case 0:
		Wparam.topic.topicDataType = "BenchMark";
		Domain::registerType(mp_participant, &m_type);
		break;
	case 1:
		Wparam.topic.topicDataType = "BenchMarkSmall";
		Domain::registerType(mp_participant, &m_typeSmall);
		break;
	case 2:
		Wparam.topic.topicDataType = "BenchMarkMedium";
		Domain::registerType(mp_participant, &m_typeMedium);
		break;
	case 3:
		Wparam.topic.topicDataType = "BenchMarkBig";
		Domain::registerType(mp_participant, &m_typeBig);
		break;
	}

	Wparam.topic.topicName = topicName + "_1";
    Wparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    //Wparam.topic.historyQos.depth = 30;
    //Wparam.topic.resourceLimitsQos.max_samples = 50;
    //Wparam.topic.resourceLimitsQos.allocated_samples = 20;
    Wparam.topic.historyQos.depth = 30;
    Wparam.topic.resourceLimitsQos.max_samples = 50;
    Wparam.topic.resourceLimitsQos.allocated_samples = 20;
    Wparam.times.heartbeatPeriod.seconds = 2;
    Wparam.times.heartbeatPeriod.fraction = 200*1000*1000;
    Wparam.qos.m_reliability.kind = kind;
	Wparam.qos.m_publishMode.kind = ASYNCHRONOUS_PUBLISH_MODE;
    //Wparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
    //Wparam.setUserDefinedID(1);
    //Wparam.setEntityID(2);

    mp_publisher = Domain::createPublisher(mp_participant,Wparam,(PublisherListener*)&m_pubListener);
    if (mp_publisher == nullptr)
    {
        return false;
    }

    //CREATE THE SUBSCRIBER
    SubscriberAttributes Rparam;
    Rparam.topic.topicKind = NO_KEY;
	switch (m_iSize)
	{
	default:
	case 0:
		Rparam.topic.topicDataType = "BenchMark";
		break;
	case 1:
		Rparam.topic.topicDataType = "BenchMarkSmall";
		break;
	case 2:
		Rparam.topic.topicDataType = "BenchMarkMedium";
		break;
	case 3:
		Rparam.topic.topicDataType = "BenchMarkBig";
		break;
	}
    Rparam.topic.topicName = topicName + "_2";
    Rparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    //Rparam.topic.historyQos.depth = 30;
    //Rparam.topic.resourceLimitsQos.max_samples = 50;
    //Rparam.topic.resourceLimitsQos.allocated_samples = 20;
    Rparam.topic.historyQos.depth = 30;
    Rparam.topic.resourceLimitsQos.max_samples = 50;
    Rparam.topic.resourceLimitsQos.allocated_samples = 20;
    Rparam.qos.m_reliability.kind = kind;
    //Rparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
    Rparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    mp_subscriber = Domain::createSubscriber(mp_participant, Rparam, (SubscriberListener*)&m_subListener);

    if (mp_subscriber == nullptr)
    {
        return false;
    }
    return true;
}

BenchMarkPublisher::~BenchMarkPublisher()
{
    // TODO Auto-generated destructor stub
    Domain::removeParticipant(mp_participant);
}

BenchMarkPublisher::SubListener::SubListener(BenchMarkPublisher* parent)
    : mParent(parent)
{
};

void BenchMarkPublisher::SubListener::onSubscriptionMatched(Subscriber* /*sub*/, MatchingInfo& info)
{
    if (info.status == MATCHED_MATCHING)
    {
        //std::cout << "Subscriber matched" << std::endl;
    }
    else
    {
        //std::cout << "Subscriber unmatched" << std::endl;
    }
}

void BenchMarkPublisher::SubListener::onNewDataMessage(Subscriber* sub)
{
	if (!g_bBenchmarkFinished)
	{
		switch (mParent->m_iSize)
		{
		default:
		case 0:
		{
			if (sub->takeNextData((void*)&m_Hello, &m_info))
			{
				if (m_info.sampleKind == ALIVE)
				{
					m_Hello.index(m_Hello.index() + 1);
					g_iCount = m_Hello.index();
					mParent->mp_publisher->write((void*)&m_Hello);
				}
			}
		}
		break;
		case 1:
		{
			if (sub->takeNextData((void*)&m_HelloSmall, &m_info))
			{
				if (m_info.sampleKind == ALIVE)
				{
					m_HelloSmall.index(m_HelloSmall.index() + 1);
					g_iCount = m_HelloSmall.index();
					mParent->mp_publisher->write((void*)&m_HelloSmall);
				}
			}
		}
		break;
		case 2:
		{
			if (sub->takeNextData((void*)&m_HelloMedium, &m_info))
			{
				if (m_info.sampleKind == ALIVE)
				{
					m_HelloMedium.index(m_HelloMedium.index() + 1);
					g_iCount = m_HelloMedium.index();
					mParent->mp_publisher->write((void*)&m_HelloMedium);
				}
			}
		}
		break;
		case 3:
		{
			if (sub->takeNextData((void*)&m_HelloBig, &m_info))
			{
				if (m_info.sampleKind == ALIVE)
				{
					m_HelloBig.index(m_HelloBig.index() + 1);
					g_iCount = m_HelloBig.index();
					mParent->mp_publisher->write((void*)&m_HelloBig);
				}
			}
		}
		break;
		}
	}
}

BenchMarkPublisher::PubListener::PubListener(BenchMarkPublisher* parent)
    : mParent(parent)
	, n_matched(0)
{
};

void BenchMarkPublisher::PubListener::onPublicationMatched(Publisher* /*pub*/,MatchingInfo& info)
{
    if(info.status == MATCHED_MATCHING)
    {
        std::cout << "Publisher matched. Test starts..." << std::endl;
        if (n_matched == 0)
        {
            g_bTestStartTime = std::chrono::system_clock::now();
        }
        n_matched++;
    }
    else
    {
        g_bBenchmarkFinished = true;
        std::cout << "Publisher unmatched. Test Aborted"<<std::endl;
    }
}

void BenchMarkPublisher::runThread()
{
    while (!publish())
    {
		eClock::my_sleep(10);
	}

    while(!g_bBenchmarkFinished)
    {
        auto end = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - g_bTestStartTime);
        if (elapsed.count() > g_iTestTimeMs)
        {
            g_bBenchmarkFinished = true;
        }
        else
        {
            // WAIT
            eClock::my_sleep(1000);
        }
    }
}

void BenchMarkPublisher::run()
{
    std::thread thread(&BenchMarkPublisher::runThread, this);
    //std::cout << "Publisher running..." << std::endl;
    thread.join();

    std::cout << "TEST RESULTS after " << g_iTestTimeMs << " milliseconds. Count: " << g_iCount << std::endl;
}

bool BenchMarkPublisher::publish()
{
    if (m_pubListener.n_matched > 0)
    {
		switch (m_iSize)
		{
		default:
		case 0:
		{
			m_Hello.index(m_Hello.index() + 1);
			mp_publisher->write((void*)&m_Hello);
			return true;
		}
		case 1:
		{
			m_HelloSmall.index(m_HelloSmall.index() + 1);
			mp_publisher->write((void*)&m_HelloSmall);
			return true;
		}
		case 2:
		{
			m_HelloMedium.index(m_HelloMedium.index() + 1);
			mp_publisher->write((void*)&m_HelloMedium);
			return true;
		}
		case 3:
		{
			m_HelloBig.index(m_HelloBig.index() + 1);
			mp_publisher->write((void*)&m_HelloBig);
			return true;
		}
		}
    }
    return false;
}
