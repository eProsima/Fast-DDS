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
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastdds/publisher/Publisher.hpp>
#include <fastdds/publisher/qos/PublisherQos.hpp>
#include <fastdds/topic/DataWriter.hpp>
#include <fastdds/domain/DomainParticipantFactory.hpp>
#include <fastrtps/utils/eClock.h>

#include <thread>

using namespace eprosima::fastdds;
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
    PParam.rtps.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
    PParam.rtps.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
    PParam.rtps.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    PParam.rtps.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    PParam.rtps.builtin.domainId = 0;
    PParam.rtps.builtin.leaseDuration = c_TimeInfinite;
    PParam.rtps.setName("Participant_pub");
    mp_participant = DomainParticipantFactory::get_instance()->create_participant(PParam);

    if(mp_participant==nullptr)
        return false;

    //REGISTER THE TYPE
    mp_participant->register_type(&m_type);

    //CREATE THE PUBLISHER
    //PublisherQos qos;
    PublisherAttributes Wparam;
    Wparam.topic.topicKind = NO_KEY;
    Wparam.topic.topicDataType = "HelloWorld";
    Wparam.topic.topicName = "HelloWorldTopic";
    Wparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    Wparam.topic.historyQos.depth = 30;
    Wparam.topic.resourceLimitsQos.max_samples = 50;
    Wparam.topic.resourceLimitsQos.allocated_samples = 20;
    Wparam.times.heartbeatPeriod.seconds = 2;
    Wparam.times.heartbeatPeriod.nanosec = 200*1000*1000;
    Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    //mp_publisher = mp_participant->create_publisher(qos, Wparam, nullptr);
    mp_publisher = mp_participant->create_publisher(PUBLISHER_QOS_DEFAULT, Wparam, nullptr);

    if(mp_publisher == nullptr)
        return false;

    // CREATE THE WRITER
    writer_ = mp_publisher->create_datawriter(Wparam.topic, Wparam.qos, &m_listener);

    if (writer_ == nullptr)
    {
        return false;
    }

    return true;

}

HelloWorldPublisher::~HelloWorldPublisher()
{
    DomainParticipantFactory::get_instance()->delete_participant(mp_participant);
}

void HelloWorldPublisher::PubListener::on_publication_matched(
        eprosima::fastdds::DataWriter*,
        eprosima::fastrtps::rtps::MatchingInfo &info)
{
    if(info.status == MATCHED_MATCHING)
    {
        n_matched++;
        firstConnected = true;
        std::cout << "Publisher matched"<<std::endl;
    }
    else
    {
        n_matched--;
        std::cout << "Publisher unmatched"<<std::endl;
    }
}

void HelloWorldPublisher::runThread(uint32_t samples, uint32_t sleep)
{
    if (samples == 0)
    {
        while(!stop)
        {
            if(publish(false))
            {
                std::cout << "Message: "<<m_Hello.message()<< " with index: "<< m_Hello.index()<< " SENT"<<std::endl;
            }
            eClock::my_sleep(sleep);
        }
    }
    else
    {
        for(uint32_t i = 0;i<samples;++i)
        {
            if(!publish())
                --i;
            else
            {
                std::cout << "Message: "<<m_Hello.message()<< " with index: "<< m_Hello.index()<< " SENT"<<std::endl;
            }
            eClock::my_sleep(sleep);
        }
    }
}

void HelloWorldPublisher::run(uint32_t samples, uint32_t sleep)
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
    if(m_listener.firstConnected || !waitForListener || m_listener.n_matched>0)
    {
        m_Hello.index(m_Hello.index()+1);
        writer_->write(&m_Hello);
        return true;
    }
    return false;
}
