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
 * @file HelloWorldPublisher.cpp
 *
 */

#include "HelloWorldPublisher.h"
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/transport/TCPv4TransportDescriptor.h>
#include <fastrtps/transport/UDPv4TransportDescriptor.h>
#include <fastrtps/Domain.h>
#include <fastrtps/utils/eClock.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

HelloWorldPublisher::HelloWorldPublisher():mp_participant(nullptr),
mp_publisher(nullptr)
{


}

bool HelloWorldPublisher::init()
{
    m_Hello.index(0);
    m_Hello.message("HelloWorld");
    ParticipantAttributes PParam;
    PParam.rtps.use_IP6_to_send = true;

    // TCP CONNECTION PEER.
    uint32_t kind = LOCATOR_KIND_TCPv4;
    Locator_t initial_peer_locator;
    initial_peer_locator.kind = kind;
    initial_peer_locator.set_IP4_address("127.0.0.1");
    initial_peer_locator.set_port(5100);
    initial_peer_locator.set_logical_port(7401);
    PParam.rtps.builtin.initialPeersList.push_back(initial_peer_locator);
    PParam.rtps.defaultOutLocatorList.push_back(initial_peer_locator);

    Locator_t unicast_locator;
    unicast_locator.kind = kind;
    unicast_locator.set_IP4_address("127.0.0.1");
    unicast_locator.set_port(5100);
    unicast_locator.set_logical_port(7410);
    PParam.rtps.defaultUnicastLocatorList.push_back(unicast_locator);

    Locator_t meta_locator;
    meta_locator.kind = kind;
    meta_locator.set_IP4_address("127.0.0.1");
    meta_locator.set_port(5100);
    meta_locator.set_logical_port(7402);
    PParam.rtps.builtin.metatrafficUnicastLocatorList.push_back(meta_locator);

    //PParam.rtps.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
    //PParam.rtps.builtin.use_STATIC_EndpointDiscoveryProtocol = false;
    //PParam.rtps.builtin.setStaticEndpointXMLFilename("HelloWorldSubscriber.xml");

    //PParam.rtps.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
    //PParam.rtps.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    //PParam.rtps.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    PParam.rtps.builtin.domainId = 0;
    PParam.rtps.builtin.leaseDuration = c_TimeInfinite;
    PParam.rtps.builtin.leaseDuration_announcementperiod = Duration_t(5, 0);
    PParam.rtps.setName("Participant_pub");

    PParam.rtps.useBuiltinTransports = false;

    std::shared_ptr<TCPv4TransportDescriptor> descriptor = std::make_shared<TCPv4TransportDescriptor>();
    descriptor->sendBufferSize = 0;
    descriptor->receiveBufferSize = 0;
    descriptor->set_WAN_address("127.0.0.1");
    descriptor->add_listener_port(5100);
    PParam.rtps.userTransports.push_back(descriptor);

    mp_participant = Domain::createParticipant(PParam);

    if(mp_participant==nullptr)
        return false;
    //REGISTER THE TYPE

    Domain::registerType(mp_participant,&m_type);

    //CREATE THE PUBLISHER
    PublisherAttributes Wparam;
    Wparam.topic.topicKind = NO_KEY;
    Wparam.topic.topicDataType = "HelloWorld";
    Wparam.topic.topicName = "HelloWorldTopicTCP";
    Wparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    Wparam.topic.historyQos.depth = 30;
    Wparam.topic.resourceLimitsQos.max_samples = 50;
    Wparam.topic.resourceLimitsQos.allocated_samples = 20;
    Wparam.times.heartbeatPeriod.seconds = 2;
    Wparam.times.heartbeatPeriod.fraction = 200*1000*1000;
    Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    //Wparam.setUserDefinedID(1);
    //Wparam.setEntityID(2);
    mp_publisher = Domain::createPublisher(mp_participant,Wparam,(PublisherListener*)&m_listener);
    if(mp_publisher == nullptr)
        return false;

    return true;

}

HelloWorldPublisher::~HelloWorldPublisher()
{
    // TODO Auto-generated destructor stub
    Domain::removeParticipant(mp_participant);
}

void HelloWorldPublisher::PubListener::onPublicationMatched(Publisher* /*pub*/,MatchingInfo& info)
{
    if(info.status == MATCHED_MATCHING)
    {
        n_matched++;
        std::cout << "[RTCP] Publisher matched"<<std::endl;
    }
    else
    {
        n_matched--;
        std::cout << "[RTCP] Publisher unmatched"<<std::endl;
    }
}

void HelloWorldPublisher::run(uint32_t samples, long sleep_ms)
{
    if (samples == 0)
    {
        while(true)
        {
            if(publish(sleep_ms, false))
            {
                std::cout << "[RTCP] Message: "<<m_Hello.message()<< " with index: "<< m_Hello.index()<< " SENT"<<std::endl;
            }
            eClock::my_sleep(0);
        }
    }
    else
    {
        for(uint32_t i = 0;i<samples;++i)
        {
            if(!publish(sleep_ms))
                --i;
            else
            {
                std::cout << "[RTCP] Message: "<<m_Hello.message()<< " with index: "<< m_Hello.index()<< " SENT"<<std::endl;
            }
            eClock::my_sleep(0);
        }
    }
}

bool HelloWorldPublisher::publish(long sleep_ms, bool waitForListener)
{
    if(!waitForListener || m_listener.n_matched>0)
    {
        eClock::my_sleep(sleep_ms);
        m_Hello.index(m_Hello.index()+1);
        mp_publisher->write((void*)&m_Hello);
        return true;
    }
    return false;
}
