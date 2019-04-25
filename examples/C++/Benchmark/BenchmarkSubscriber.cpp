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
//#include "DynamicTypesHelper.h"
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/transport/UDPv4TransportDescriptor.h>
#include <fastrtps/transport/TCPv4TransportDescriptor.h>
#include <fastrtps/transport/UDPv6TransportDescriptor.h>
#include <fastrtps/transport/TCPv6TransportDescriptor.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/subscriber/Subscriber.h>
//#include <fastrtps/types/DynamicData.h>


#include <fastrtps/Domain.h>
#include <fastrtps/utils/eClock.h>
#include <fastrtps/utils/IPLocator.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

BenchMarkSubscriber::BenchMarkSubscriber()
    : mp_participant(nullptr)
    , mp_publisher(nullptr)
    , mp_subscriber(nullptr)
	, m_pubListener(this)
	, m_subListener(this)
    //, m_bDynamicTypes(false)
{
}

bool BenchMarkSubscriber::init(int transport, ReliabilityQosPolicyKind reliabilityKind, const std::string& topicName,
    int domain, int size/*, bool dynamicTypes*/)
{
    //m_bDynamicTypes = dynamicTypes;
	m_iSize = size;

    ParticipantAttributes PParam;
    PParam.rtps.builtin.domainId = domain;
    PParam.rtps.builtin.leaseDuration = c_TimeInfinite;
    PParam.rtps.builtin.leaseDuration_announcementperiod = Duration_t(1, 0);
    PParam.rtps.setName("Participant_sub");

    if (transport == 1)
    {
        PParam.rtps.useBuiltinTransports = true;
    }
    else if (transport == 2)
    {
        int32_t kind = LOCATOR_KIND_TCPv4;

        Locator_t initial_peer_locator;
        initial_peer_locator.kind = kind;
        IPLocator::setIPv4(initial_peer_locator, "127.0.0.1");
        initial_peer_locator.port = 5100;
        PParam.rtps.builtin.initialPeersList.push_back(initial_peer_locator); // Publisher's channel

        PParam.rtps.useBuiltinTransports = false;
        std::shared_ptr<TCPv4TransportDescriptor> descriptor = std::make_shared<TCPv4TransportDescriptor>();
		descriptor->sendBufferSize = 8912896; // 8.5Mb
		descriptor->receiveBufferSize = 8912896; // 8.5Mb
        PParam.rtps.userTransports.push_back(descriptor);
    }
    else if (transport == 3)
    {
        //uint32_t kind = LOCATOR_KIND_UDPv6;
    }
    else if (transport == 4)
    {
        uint32_t kind = LOCATOR_KIND_TCPv6;
        PParam.rtps.useBuiltinTransports = false;

        Locator_t initial_peer_locator;
        initial_peer_locator.kind = kind;
        IPLocator::setIPv6(initial_peer_locator, "::1");
        initial_peer_locator.port = 5100;
        PParam.rtps.builtin.initialPeersList.push_back(initial_peer_locator); // Publisher's channel

        PParam.rtps.useBuiltinTransports = false;
        std::shared_ptr<TCPv6TransportDescriptor> descriptor = std::make_shared<TCPv6TransportDescriptor>();
		descriptor->sendBufferSize = 8912896; // 8.5Mb
		descriptor->receiveBufferSize = 8912896; // 8.5Mb
        PParam.rtps.userTransports.push_back(descriptor);
    }

    mp_participant = Domain::createParticipant(PParam);
    if(mp_participant==nullptr)
        return false;

    //CREATE THE SUBSCRIBER
    SubscriberAttributes Rparam;
    Rparam.topic.topicKind = NO_KEY;

    //CREATE THE PUBLISHER
    PublisherAttributes Wparam;
    Wparam.topic.topicKind = NO_KEY;

    //REGISTER THE TYPE
    /*if (m_bDynamicTypes)
    {
        switch (m_iSize)
        {
        default:
        case 0:
            m_DynamicData = DynamicTypesHelper::CreateData();
            Rparam.topic.topicDataType = "Dyn_BenchMark";
            Wparam.topic.topicDataType = "Dyn_BenchMark";
            m_dynType.SetDynamicType(m_DynamicData);
            Domain::registerType(mp_participant, &m_dynType);
            break;
        case 1:
            m_DynamicData = DynamicTypesHelper::CreateSmallData();
            Rparam.topic.topicDataType = "Dyn_BenchMarkSmall";
            Wparam.topic.topicDataType = "Dyn_BenchMarkSmall";
            m_dynType.SetDynamicType(m_DynamicData);
            Domain::registerType(mp_participant, &m_dynType);
            break;
        case 2:
            m_DynamicData = DynamicTypesHelper::CreateMediumData();
            Rparam.topic.topicDataType = "Dyn_BenchMarkMedium";
            Wparam.topic.topicDataType = "Dyn_BenchMarkMedium";
            m_dynType.SetDynamicType(m_DynamicData);
            Domain::registerType(mp_participant, &m_dynType);
            break;
        case 3:
            m_DynamicData = DynamicTypesHelper::CreateBigData();
            Rparam.topic.topicDataType = "Dyn_BenchMarkBig";
            Wparam.topic.topicDataType = "Dyn_BenchMarkBig";
            m_dynType.SetDynamicType(m_DynamicData);
            Domain::registerType(mp_participant, &m_dynType);
            break;
        }
        m_DynamicData->set_uint32_value(0, 0);
    }
    else*/
    {
        switch (m_iSize)
        {
        default:
        case 0:
            Rparam.topic.topicDataType = "BenchMark";
            Wparam.topic.topicDataType = "BenchMark";
            Domain::registerType(mp_participant, &m_type);
            break;
        case 1:
            Rparam.topic.topicDataType = "BenchMarkSmall";
            Wparam.topic.topicDataType = "BenchMarkSmall";
            Domain::registerType(mp_participant, &m_typeSmall);
            break;
        case 2:
            Rparam.topic.topicDataType = "BenchMarkMedium";
            Wparam.topic.topicDataType = "BenchMarkMedium";
            Domain::registerType(mp_participant, &m_typeMedium);
            break;
        case 3:
            Rparam.topic.topicDataType = "BenchMarkBig";
            Wparam.topic.topicDataType = "BenchMarkBig";
            Domain::registerType(mp_participant, &m_typeBig);
            break;
        }
    }

	Rparam.topic.topicName = topicName + "_1";
    Rparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    //Rparam.topic.historyQos.depth = 30;
    //Rparam.topic.resourceLimitsQos.max_samples = 50;
    //Rparam.topic.resourceLimitsQos.allocated_samples = 20;
    Rparam.topic.historyQos.depth = 30;
    Rparam.topic.resourceLimitsQos.max_samples = 50;
    Rparam.topic.resourceLimitsQos.allocated_samples = 20;
    Rparam.qos.m_reliability.kind = reliabilityKind;
    //Rparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
    Rparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    //Rparam.setUserDefinedID(3);
    //Rparam.setEntityID(4);
    mp_subscriber = Domain::createSubscriber(mp_participant,Rparam,(SubscriberListener*)&m_subListener);

    if (mp_subscriber == nullptr)
    {
        return false;
    }

	Wparam.topic.topicName = topicName + "_2";
    Wparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    //Wparam.topic.historyQos.depth = 30;
    //Wparam.topic.resourceLimitsQos.max_samples = 50;
    //Wparam.topic.resourceLimitsQos.allocated_samples = 20;
    Wparam.topic.historyQos.depth = 1;
    Wparam.topic.resourceLimitsQos.max_samples = 1;
    Wparam.topic.resourceLimitsQos.allocated_samples = 1;
    Wparam.times.heartbeatPeriod.seconds = 2;
    Wparam.times.heartbeatPeriod.fraction = 200 * 1000 * 1000;
    Wparam.qos.m_reliability.kind = reliabilityKind;
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
        std::cout << "Publisher matched" << std::endl;
    }
    else
    {
        n_matched--;
		std::cin.clear();
        std::cout << "Publisher unmatched" << std::endl;
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
    /*if (mParent->m_bDynamicTypes)
    {
        if (sub->takeNextData((void*)mParent->m_DynamicData.get(), &m_info))
        {
            if (m_info.sampleKind == ALIVE)
            {
                mParent->m_DynamicData->set_uint32_value(mParent->m_DynamicData->get_uint32_value(0) + 1, 0);
                mParent->mp_publisher->write((void*)mParent->m_DynamicData.get());
            }
        }
    }
    else*/
    {
        switch (mParent->m_iSize)
        {
        default:
        case 0:
        {
            if (sub->takeNextData((void*)&mParent->m_Hello, &m_info))
            {
                if (m_info.sampleKind == ALIVE)
                {
                    mParent->m_Hello.index(mParent->m_Hello.index() + 1);
                    mParent->mp_publisher->write((void*)&mParent->m_Hello);
                }
            }
        }
        break;
        case 1:
        {
            if (sub->takeNextData((void*)&mParent->m_HelloSmall, &m_info))
            {
                if (m_info.sampleKind == ALIVE)
                {
                    mParent->m_HelloSmall.index(mParent->m_HelloSmall.index() + 1);
                    mParent->mp_publisher->write((void*)&mParent->m_HelloSmall);
                }
            }
        }
        break;
        case 2:
        {
            if (sub->takeNextData((void*)&mParent->m_HelloMedium, &m_info))
            {
                if (m_info.sampleKind == ALIVE)
                {
                    mParent->m_HelloMedium.index(mParent->m_HelloMedium.index() + 1);
                    mParent->mp_publisher->write((void*)&mParent->m_HelloMedium);
                }
            }
        }
        break;
        case 3:
        {
            if (sub->takeNextData((void*)&mParent->m_HelloBig, &m_info))
            {
                if (m_info.sampleKind == ALIVE)
                {
                    mParent->m_HelloBig.index(mParent->m_HelloBig.index() + 1);
                    mParent->mp_publisher->write((void*)&mParent->m_HelloBig);
                }
            }
        }
        break;
        }
    }
}

void BenchMarkSubscriber::run()
{
    std::cout << "Subscriber running..." << std::endl;
	std::cin.ignore();
}
