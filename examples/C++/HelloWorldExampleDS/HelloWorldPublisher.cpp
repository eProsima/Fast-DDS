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
#include <fastrtps/Domain.h>
#include <fastrtps/utils/IPLocator.h>
#include <fastrtps/utils/System.h>

#include <thread>
#include <random>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

HelloWorldPublisher::HelloWorldPublisher()
    : mp_participant(nullptr)
    , mp_publisher(nullptr)
{


}

bool HelloWorldPublisher::init(
        Locator_t server_address,
        std::string topic_name)
{
    m_hello.index(0);
    std::string msg = "HelloWorld-" + topic_name;
    m_hello.message(msg);

    eprosima::fastdds::rtps::RemoteServerAttributes ratt;
    ratt.ReadguidPrefix("4D.49.47.55.45.4c.5f.42.41.52.52.4f");

    ParticipantAttributes PParam;
    PParam.rtps.builtin.discovery_config.discoveryProtocol = DiscoveryProtocol_t::CLIENT;
    PParam.rtps.builtin.discovery_config.leaseDuration = c_TimeInfinite;
    PParam.rtps.setName("Participant_pub");

    uint16_t default_port = IPLocator::getPhysicalPort(server_address.port);

    if(server_address.kind == LOCATOR_KIND_TCPv4 ||
        server_address.kind == LOCATOR_KIND_TCPv6)
    {
        if(!IsAddressDefined(server_address))
        {
            server_address.kind = LOCATOR_KIND_TCPv4;
            IPLocator::setIPv4(server_address, 127, 0, 0, 1);
        }

        // server logical port is not customizable in this example
        IPLocator::setPhysicalPort(server_address, default_port);
        IPLocator::setLogicalPort(server_address, 65215);

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
        if(!IsAddressDefined(server_address))
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
    Domain::registerType(mp_participant,&m_type);

    //CREATE THE PUBLISHER
    PublisherAttributes Wparam;
    Wparam.topic.topicKind = NO_KEY;
    Wparam.topic.topicDataType = "HelloWorld";
    Wparam.topic.topicName = topic_name;
    Wparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    Wparam.topic.historyQos.depth = 30;
    Wparam.topic.resourceLimitsQos.max_samples = 50;
    Wparam.topic.resourceLimitsQos.allocated_samples = 20;
    Wparam.times.heartbeatPeriod.seconds = 2;
    Wparam.times.heartbeatPeriod.nanosec = 0;
    Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    mp_publisher = Domain::createPublisher(mp_participant,Wparam,(PublisherListener*)&m_listener);

    if (mp_publisher == nullptr)
    {
        return false;
    }

    return true;

}

HelloWorldPublisher::~HelloWorldPublisher()
{
    Domain::removeParticipant(mp_participant);
}

void HelloWorldPublisher::PubListener::onPublicationMatched(
    Publisher* /*pub*/,
    MatchingInfo& info)
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
        std::cout << "Publisher unmatched" << std::endl;
    }
}

void HelloWorldPublisher::runThread(
    uint32_t samples,
    uint32_t sleep)
{
    if (samples == 0)
    {
        while(!stop)
        {
            if (publish(false))
            {
                std::cout << "Message: " << m_hello.message() << " with index: " << m_hello.index() << " SENT" << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::duration<uint32_t, std::milli>(sleep));
        }
    }
    else
    {
        for(uint32_t i = 0;i<samples;++i)
        {
            if (!publish())
            {
                --i;
            }
            else
            {
                std::cout << "Message: " << m_hello.message() << " with index: " << m_hello.index() << " SENT" << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::duration<uint32_t,std::milli>(sleep));
        }
    }
}

void HelloWorldPublisher::run(
    uint32_t samples,
    uint32_t sleep)
{
    stop = false;
    std::thread thread(&HelloWorldPublisher::runThread, this, samples, sleep);
    if (samples == 0)
    {
        std::cout << "Publisher running. Please press enter to stop the Publisher at any time." << std::endl;
        std::cin.ignore();
        stop = true;
    }
    else
    {
        std::cout << "Publisher running " << samples << " samples." << std::endl;
    }
    thread.join();
}

bool HelloWorldPublisher::publish(bool waitForListener)
{
    if (m_listener.firstConnected || !waitForListener || m_listener.n_matched>0)
    {
        m_hello.index(m_hello.index()+1);
        mp_publisher->write((void*)&m_hello);
        return true;
    }
    return false;
}
