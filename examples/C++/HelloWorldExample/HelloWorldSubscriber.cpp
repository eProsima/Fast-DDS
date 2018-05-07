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
 * @file HelloWorldSubscriber.cpp
 *
 */

#include "HelloWorldSubscriber.h"
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/transport/TCPv4TransportDescriptor.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/Domain.h>
#include <fastrtps/utils/eClock.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

HelloWorldSubscriber::HelloWorldSubscriber():mp_participant(nullptr),
mp_subscriber(nullptr)
{
}

bool HelloWorldSubscriber::init()
{
    ParticipantAttributes PParam;

	// TCP CONNECTION PEER.
	//Locator_t initial_peer_locator;
	//initial_peer_locator.kind = LOCATOR_KIND_TCPv4;
	//initial_peer_locator.set_IP4_address("127.0.0.1");
	//initial_peer_locator.port = 7401;
	//PParam.rtps.builtin.initialPeersList.push_back(initial_peer_locator);

	// TCP CONNECTION PEER.
	Locator_t initial_peer_locator;
	initial_peer_locator.kind = LOCATOR_KIND_TCPv4;
	initial_peer_locator.set_IP4_address("127.0.0.1");
	initial_peer_locator.set_TCP_port(7400);
	PParam.rtps.builtin.initialPeersList.push_back(initial_peer_locator);
	PParam.rtps.defaultOutLocatorList.push_back(initial_peer_locator);

	Locator_t unicast_locator;
	unicast_locator.kind = LOCATOR_KIND_TCPv4;
	unicast_locator.set_IP4_address("127.0.0.1");
	unicast_locator.set_TCP_port(7401);
	PParam.rtps.defaultUnicastLocatorList.push_back(unicast_locator);

	Locator_t meta_locator;
	meta_locator.kind = LOCATOR_KIND_TCPv4;
	meta_locator.set_IP4_address("127.0.0.1");
	meta_locator.set_TCP_port(7402);
	PParam.rtps.builtin.metatrafficUnicastLocatorList.push_back(meta_locator);

	PParam.rtps.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
    PParam.rtps.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
    PParam.rtps.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    PParam.rtps.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    PParam.rtps.builtin.domainId = 0;
    PParam.rtps.builtin.leaseDuration = c_TimeInfinite;
    PParam.rtps.setName("Participant_sub");

    //ARCE:
    PParam.rtps.useBuiltinTransports = false;
    std::shared_ptr<TCPv4TransportDescriptor> descriptor = std::make_shared<TCPv4TransportDescriptor>();
    descriptor->sendBufferSize = 0;
    descriptor->receiveBufferSize = 0;
    PParam.rtps.userTransports.emplace_back(descriptor);

    mp_participant = Domain::createParticipant(PParam);
    if(mp_participant==nullptr)
        return false;

    //REGISTER THE TYPE

    Domain::registerType(mp_participant,&m_type);
    //CREATE THE SUBSCRIBER
    SubscriberAttributes Rparam;
    Rparam.topic.topicKind = NO_KEY;
    Rparam.topic.topicDataType = "HelloWorld";
    Rparam.topic.topicName = "HelloWorldTopic1";
    Rparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    Rparam.topic.historyQos.depth = 30;
    Rparam.topic.resourceLimitsQos.max_samples = 50;
    Rparam.topic.resourceLimitsQos.allocated_samples = 20;
    Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Rparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    mp_subscriber = Domain::createSubscriber(mp_participant,Rparam,(SubscriberListener*)&m_listener);

    if(mp_subscriber == nullptr)
        return false;


    return true;
}

HelloWorldSubscriber::~HelloWorldSubscriber() {
    // TODO Auto-generated destructor stub
    Domain::removeParticipant(mp_participant);
}

void HelloWorldSubscriber::SubListener::onSubscriptionMatched(Subscriber* /*sub*/,MatchingInfo& info)
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

void HelloWorldSubscriber::SubListener::onNewDataMessage(Subscriber* sub)
{
    if(sub->takeNextData((void*)&m_Hello, &m_info))
    {
        if(m_info.sampleKind == ALIVE)
        {
            this->n_samples++;
            // Print your structure data here.
            std::cout << "Message "<<m_Hello.message()<< " "<< m_Hello.index()<< " RECEIVED"<<std::endl;
        }
    }

}


void HelloWorldSubscriber::run()
{
    std::cout << "Subscriber running. Please press enter to stop the Subscriber" << std::endl;
    std::cin.ignore();
}

void HelloWorldSubscriber::run(uint32_t number)
{
    std::cout << "Subscriber running until "<< number << "samples have been received"<<std::endl;
    while(number > this->m_listener.n_samples)
        eClock::my_sleep(500);
}
