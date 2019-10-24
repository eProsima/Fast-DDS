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

using namespace eprosima::fastdds::dds;

HelloWorldSubscriber::HelloWorldSubscriber()
    : participant_(nullptr)
    , subscriber_(dds::core::null)
    , reader_(dds::core::null)
    , topic_(dds::core::null)
{
}

bool HelloWorldSubscriber::init()
{
    /*
    eprosima::fastrtps::ParticipantAttributes participant_att;
    participant_att.rtps.builtin.domainId = 0;
    participant_att.rtps.setName("Participant_sub");
    */
    //participant_ = DomainParticipantFactory::get_instance()->create_participant(participant_att);
    participant_ = dds::domain::DomainParticipant(0);

    //if (participant_ == nullptr)
    if (participant_ == dds::core::null)
    {
        return false;
    }

    //REGISTER THE TYPE
    //type_.register_type(participant_, type_->getName());
    type_.register_type(participant_.delegate().get());

    //CREATE THE SUBSCRIBER
    //eprosima::fastrtps::SubscriberAttributes sub_att;
    //subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, sub_att, nullptr);
    subscriber_ = dds::sub::Subscriber(participant_, SUBSCRIBER_QOS_DEFAULT, nullptr);

    if (subscriber_ == dds::core::null)
    {
        return false;
    }

    // CREATE THE READER
    /*
    ReaderQos rqos;
    rqos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    eprosima::fastrtps::TopicAttributes topic_att;
    topic_att.topicDataType = "HelloWorld";
    topic_att.topicName = "HelloWorldTopic";
    reader_ = subscriber_->create_datareader(topic_att, rqos, &listener_);
    */

    // TopicQos
    dds::topic::qos::TopicQos topicQos
        = participant_.default_topic_qos()
        << dds::core::policy::Reliability::Reliable();

    topic_ = dds::topic::Topic<HelloWorld>(participant_, "HelloWorldTopic", "HelloWorld", topicQos);

    dds::sub::qos::DataReaderQos drqos = topicQos;

    // CREATE THE WRITER
    reader_ = dds::sub::DataReader<HelloWorld>(subscriber_, topic_, drqos, &listener_);

    if (reader_ == dds::core::null)
    {
        return false;
    }

    return true;
}

HelloWorldSubscriber::~HelloWorldSubscriber()
{
    //DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

void HelloWorldSubscriber::SubListener::on_subscription_matched(
        eprosima::fastdds::dds::DataReader*,
        const eprosima::fastdds::dds::SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_ = info.total_count;
        std::cout << "Subscriber matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        matched_ = info.total_count;
        std::cout << "Subscriber unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
}
/*
void HelloWorldSubscriber::SubListener::on_data_available(
        eprosima::fastdds::dds::DataReader* reader)
{
    if (!!reader->take_next_sample(&hello_, info_))
    {
        if (info_->instance_state == ::dds::sub::status::InstanceState::alive())
        {
            samples_++;
            // Print your structure data here.
            std::cout << "Message " << hello_.message() << " " << hello_.index() << " RECEIVED" << std::endl;
        }
    }
}
*/


void HelloWorldSubscriber::SubListener::on_data_available(
        dds::sub::DataReader<HelloWorld>& reader)
{
    dds::sub::LoanedSamples<HelloWorld> samples = reader.take();
    //for (auto sample : samples)
    for (auto sample = samples.begin(); sample < samples.end(); ++sample)
    {
        if (sample->info().valid())
        {
            samples_++;
            hello_ = sample->data();
            // Print your structure data here.
            std::cout << "Message " << hello_.message() << " " << hello_.index() << " RECEIVED" << std::endl;
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
    while (number > listener_.samples_)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
