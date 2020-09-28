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
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/transport/TCPv4TransportDescriptor.h>
#include <fastrtps/Domain.h>
#include <fastrtps/utils/IPLocator.h>
#include <fastrtps/utils/System.h>

#include <random>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

HelloWorldSubscriber::HelloWorldSubscriber()
    : mp_participant(nullptr)
    , mp_subscriber(nullptr)
{
}

bool HelloWorldSubscriber::init(
        Locator_t server_address,
        std::string topic_name)
{

    eprosima::fastdds::rtps::RemoteServerAttributes ratt;
    ratt.ReadguidPrefix("4D.49.47.55.45.4c.5f.42.41.52.52.4f");

    ParticipantAttributes PParam;
    PParam.rtps.builtin.discovery_config.discoveryProtocol = DiscoveryProtocol_t::CLIENT;
    PParam.rtps.builtin.discovery_config.leaseDuration = c_TimeInfinite;
    PParam.rtps.setName("Participant_sub");

    uint16_t default_port = IPLocator::getPhysicalPort(server_address.port);

    if (server_address.kind == LOCATOR_KIND_TCPv4 ||
            server_address.kind == LOCATOR_KIND_TCPv6)
    {
        if (!IsAddressDefined(server_address))
        {
            server_address.kind = LOCATOR_KIND_TCPv4;
            IPLocator::setIPv4(server_address, 127, 0, 0, 1);
        }

        // server logical port is not customizable in this example
        IPLocator::setLogicalPort(server_address, 65215);
        IPLocator::setPhysicalPort(server_address, default_port);

        ratt.metatrafficUnicastLocatorList.push_back(server_address);
        PParam.rtps.builtin.discovery_config.m_DiscoveryServers.push_back(ratt);

        PParam.rtps.useBuiltinTransports = false;
        std::shared_ptr<TCPv4TransportDescriptor> descriptor = std::make_shared<TCPv4TransportDescriptor>();

        // Generate a listening port for the client
        std::default_random_engine gen(System::GetPID());
        std::uniform_int_distribution<int> rdn(49152, 65535);
        descriptor->add_listener_port(rdn(gen)); // IANA ephemeral port number

        descriptor->wait_for_tcp_negotiation = false;
        PParam.rtps.userTransports.push_back(descriptor);
    }
    else
    {
        if (!IsAddressDefined(server_address))
        {
            server_address.kind = LOCATOR_KIND_UDPv4;
            server_address.port = default_port;
            IPLocator::setIPv4(server_address, 127, 0, 0, 1);
        }

        ratt.metatrafficUnicastLocatorList.push_back(server_address);
        PParam.rtps.builtin.discovery_config.m_DiscoveryServers.push_back(ratt);
    }

    mp_participant = Domain::createParticipant(PParam);

    if (mp_participant == nullptr)
    {
        return false;
    }

    //REGISTER THE TYPE

    Domain::registerType(mp_participant, &m_type);
    //CREATE THE SUBSCRIBER
    SubscriberAttributes Rparam;
    Rparam.topic.topicKind = NO_KEY;
    Rparam.topic.topicDataType = "HelloWorld";
    Rparam.topic.topicName = topic_name;
    Rparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    Rparam.topic.historyQos.depth = 30;
    Rparam.topic.resourceLimitsQos.max_samples = 50;
    Rparam.topic.resourceLimitsQos.allocated_samples = 20;
    Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Rparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    mp_subscriber = Domain::createSubscriber(mp_participant, Rparam, (SubscriberListener*)&m_listener);

    if (mp_subscriber == nullptr)
    {
        return false;
    }

    return true;
}

HelloWorldSubscriber::~HelloWorldSubscriber()
{
    Domain::removeParticipant(mp_participant);
}

void HelloWorldSubscriber::SubListener::onSubscriptionMatched(
        Subscriber* /*sub*/,
        MatchingInfo& info)
{
    if (info.status == MATCHED_MATCHING)
    {
        n_matched++;
        std::cout << "Subscriber matched" << std::endl;
    }
    else
    {
        n_matched--;
        std::cout << "Subscriber unmatched" << std::endl;
    }
}

void HelloWorldSubscriber::SubListener::onNewDataMessage(
        Subscriber* sub)
{
    if (sub->takeNextData((void*)&m_hello, &m_info))
    {
        if (m_info.sampleKind == ALIVE)
        {
            this->n_samples++;
            // Print your structure data here.
            std::cout << "Message " << m_hello.message() << " " << m_hello.index() << " RECEIVED" << std::endl;
        }
    }

}

void HelloWorldSubscriber::run()
{
    std::cout << "Subscriber running. Please press enter to stop the Subscriber" << std::endl;
    std::cin.ignore();
}

void HelloWorldSubscriber::run(
        uint32_t number)
{
    std::cout << "Subscriber running until " << number << "samples have been received" << std::endl;
    while (number > this->m_listener.n_samples)
    {
        std::this_thread::sleep_for(std::chrono::duration<uint32_t, std::milli>(500));
    }
}
