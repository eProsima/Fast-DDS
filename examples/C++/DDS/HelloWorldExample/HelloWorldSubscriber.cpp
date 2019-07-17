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
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastdds/subscriber/Subscriber.hpp>
#include <fastdds/domain/DomainParticipantFactory.hpp>
#include <fastrtps/utils/eClock.h>

using namespace eprosima::fastdds;
using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

HelloWorldSubscriber::HelloWorldSubscriber():mp_participant(nullptr),
mp_subscriber(nullptr)
{
}

bool HelloWorldSubscriber::init()
{
    ParticipantAttributes PParam;
    PParam.rtps.builtin.discovery_config.discoveryProtocol = SIMPLE;
    PParam.rtps.builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol = true;
    PParam.rtps.builtin.discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    PParam.rtps.builtin.discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    PParam.rtps.builtin.domainId = 0;
    PParam.rtps.builtin.discovery_config.leaseDuration = c_TimeInfinite;
    PParam.rtps.setName("Participant_sub");
    mp_participant = DomainParticipantFactory::get_instance()->create_participant(PParam);
    if(mp_participant==nullptr)
        return false;

    //REGISTER THE TYPE
    mp_participant->register_type(&m_type);

    //CREATE THE SUBSCRIBER
    //SubscriberQos qos;
    SubscriberAttributes att;
    //mp_subscriber = mp_participant->create_subscriber(qos, att, nullptr);
    mp_subscriber = mp_participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT, att, nullptr);

    if(mp_subscriber == nullptr)
        return false;

    // CREATE THE READER
    ReaderQos rqos;
    rqos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    rqos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    TopicAttributes topic_att;
    topic_att.topicKind = NO_KEY;
    topic_att.topicDataType = "HelloWorld";
    topic_att.topicName = "HelloWorldTopic";
    topic_att.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    topic_att.historyQos.depth = 30;
    topic_att.resourceLimitsQos.max_samples = 50;
    topic_att.resourceLimitsQos.allocated_samples = 20;
    reader_ = mp_subscriber->create_datareader(topic_att, rqos, &m_listener);

    if (reader_ == nullptr)
        return false;

    return true;
}

HelloWorldSubscriber::~HelloWorldSubscriber() {
    DomainParticipantFactory::get_instance()->delete_participant(mp_participant);
}

void HelloWorldSubscriber::SubListener::on_subscription_matched(
        eprosima::fastdds::DataReader*,
        eprosima::fastdds::SubscriptionMatchedStatus& info)
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

void HelloWorldSubscriber::SubListener::on_data_available(eprosima::fastdds::DataReader* reader)
{
    if (reader->take_next_sample(&m_Hello, &m_info))
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
