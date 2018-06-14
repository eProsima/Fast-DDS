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
 * @file BenchMarkSubscriber.cpp
 *
 */

#include "BenchmarkSubscriber.h"
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/transport/UDPv4TransportDescriptor.h>
#include <fastrtps/transport/TCPv4TransportDescriptor.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/Domain.h>
#include <fastrtps/utils/eClock.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

static bool g_bBenchmarkFinished = false;

BenchMarkSubscriber::BenchMarkSubscriber()
    : mp_participant(nullptr)
    , mp_publisher(nullptr)
    , mp_subscriber(nullptr)
	, m_pubListener(this)
	, m_subListener(this)
{
}

bool BenchMarkSubscriber::init(int transport, ReliabilityQosPolicyKind kind, const std::string& topicName, int domain, int size)
{
	m_iSize = size;

    ParticipantAttributes PParam;
    PParam.rtps.builtin.domainId = domain;
    PParam.rtps.builtin.leaseDuration = c_TimeInfinite;
    PParam.rtps.builtin.leaseDuration_announcementperiod = Duration_t(5, 0);
    PParam.rtps.setName("Participant_sub");

    if (transport == 1)
    {
        PParam.rtps.useBuiltinTransports = true;
    }
    else
    {
        int32_t kind = LOCATOR_KIND_TCPv4;

        Locator_t initial_peer_locator;
        initial_peer_locator.kind = kind;
        initial_peer_locator.set_IP4_address("127.0.0.1");
        initial_peer_locator.set_port(5100);
        initial_peer_locator.set_logical_port(7402);
        PParam.rtps.builtin.initialPeersList.push_back(initial_peer_locator); // Publisher's meta channel
        initial_peer_locator.set_logical_port(7410);
        PParam.rtps.builtin.initialPeersList.push_back(initial_peer_locator); // Publisher's meta channel

        Locator_t unicast_locator;
        unicast_locator.kind = kind;
        unicast_locator.set_IP4_address("127.0.0.1");
        unicast_locator.set_port(5100);
        unicast_locator.set_logical_port(7411);
        PParam.rtps.defaultUnicastLocatorList.push_back(unicast_locator); // Subscriber's data channel

        Locator_t meta_locator;
        meta_locator.kind = kind;
        meta_locator.set_IP4_address("127.0.0.1");
        meta_locator.set_port(5100);
        meta_locator.set_logical_port(7403);
        PParam.rtps.builtin.metatrafficUnicastLocatorList.push_back(meta_locator); // Subscriber's meta channel

        //PParam.rtps.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
        //PParam.rtps.builtin.use_STATIC_EndpointDiscoveryProtocol = false;
        //PParam.rtps.builtin.setStaticEndpointXMLFilename("BenchMarkPublisher.xml");
        //PParam.rtps.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
        //PParam.rtps.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
        //PParam.rtps.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;

        PParam.rtps.useBuiltinTransports = false;
        std::shared_ptr<TCPv4TransportDescriptor> descriptor = std::make_shared<TCPv4TransportDescriptor>();
		descriptor->sendBufferSize = 8912896; // 8.5Mb
		descriptor->receiveBufferSize = 8912896; // 8.5Mb
		descriptor->set_metadata_logical_port(7403);
        //descriptor->set_WAN_address("192.168.1.47");
        //descriptor->set_WAN_address("192.168.1.55");
        PParam.rtps.userTransports.push_back(descriptor);
    }
    mp_participant = Domain::createParticipant(PParam);
    if(mp_participant==nullptr)
        return false;

    //REGISTER THE TYPE
	switch(m_iSize)
	{
	default:
	case 0:
		Domain::registerType(mp_participant, &m_type);
		break;
	case 1:
		Domain::registerType(mp_participant, &m_typeSmall);
		break;
	case 2:
		Domain::registerType(mp_participant, &m_typeMedium);
		break;
	case 3:
		Domain::registerType(mp_participant, &m_typeBig);
		break;
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
	Rparam.topic.topicName = topicName + "_1";
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
    //Rparam.setUserDefinedID(3);
    //Rparam.setEntityID(4);
    mp_subscriber = Domain::createSubscriber(mp_participant,Rparam,(SubscriberListener*)&m_subListener);

    if (mp_subscriber == nullptr)
    {
        return false;
    }

    //CREATE THE PUBLISHER
    PublisherAttributes Wparam;
    Wparam.topic.topicKind = NO_KEY;
	switch (m_iSize)
	{
	default:
	case 0:
		Wparam.topic.topicDataType = "BenchMark";
		break;
	case 1:
		Wparam.topic.topicDataType = "BenchMarkSmall";
		break;
	case 2:
		Wparam.topic.topicDataType = "BenchMarkMedium";
		break;
	case 3:
		Wparam.topic.topicDataType = "BenchMarkBig";
		break;
	}
	Wparam.topic.topicName = topicName + "_2";
    Wparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    //Wparam.topic.historyQos.depth = 30;
    //Wparam.topic.resourceLimitsQos.max_samples = 50;
    //Wparam.topic.resourceLimitsQos.allocated_samples = 20;
    Wparam.topic.historyQos.depth = 30;
    Wparam.topic.resourceLimitsQos.max_samples = 50;
    Wparam.topic.resourceLimitsQos.allocated_samples = 20;
    Wparam.times.heartbeatPeriod.seconds = 2;
    Wparam.times.heartbeatPeriod.fraction = 200 * 1000 * 1000;
    Wparam.qos.m_reliability.kind = kind;
	Wparam.qos.m_publishMode.kind = ASYNCHRONOUS_PUBLISH_MODE;
    //Wparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
    //Wparam.setUserDefinedID(1);
    //Wparam.setEntityID(2);

    mp_publisher = Domain::createPublisher(mp_participant, Wparam, (PublisherListener*)&m_pubListener);
    if (mp_publisher == nullptr)
    {
        return false;
    }

    return true;
}

BenchMarkSubscriber::~BenchMarkSubscriber()
{
    // TODO Auto-generated destructor stub
    Domain::removeParticipant(mp_participant);
}

BenchMarkSubscriber::PubListener::PubListener(BenchMarkSubscriber* parent)
    : mParent(parent)
	, n_matched(0)
    , firstConnected(false)
{
}

void BenchMarkSubscriber::PubListener::onPublicationMatched(Publisher* /*pub*/, MatchingInfo& info)
{
    if (info.status == MATCHED_MATCHING)
    {
        n_matched++;
        firstConnected = true;
        //std::cout << "Publisher matched" << std::endl;
    }
    else
    {
        n_matched--;
        //std::cout << "Publisher unmatched" << std::endl;
    }
}

BenchMarkSubscriber::SubListener::SubListener(BenchMarkSubscriber* parent)
    : mParent(parent)
    , n_matched(0)
    , n_samples(0)
{
}

void BenchMarkSubscriber::SubListener::onSubscriptionMatched(Subscriber* /*sub*/,MatchingInfo& info)
{
    if(info.status == MATCHED_MATCHING)
    {
        n_matched++;
        std::cout << "Subscriber matched"<<std::endl;
    }
    else
    {
        n_matched--;
        std::cout << "Subscriber unmatched"<<std::endl;
    }
}

void BenchMarkSubscriber::SubListener::onNewDataMessage(Subscriber* sub)
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
				mParent->mp_publisher->write((void*)&m_HelloBig);
			}
		}
	}
	break;
	}
}

void BenchMarkSubscriber::run()
{
    std::cout << "Subscriber running..." << std::endl;
    while (!g_bBenchmarkFinished)
    {
        eClock::my_sleep(500);
    }
}
