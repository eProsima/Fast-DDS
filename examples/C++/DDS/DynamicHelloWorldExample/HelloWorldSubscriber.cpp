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
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastrtps/utils/eClock.h>

#include <fastrtps/types/DynamicDataHelper.hpp>
#include <fastrtps/types/DynamicDataFactory.h>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

HelloWorldSubscriber::HelloWorldSubscriber():mp_participant(nullptr),
    mp_subscriber(nullptr), m_listener(this)
{
}

bool HelloWorldSubscriber::init()
{
    ParticipantAttributes PParam;
    PParam.rtps.setName("Participant_sub");
    mp_participant = DomainParticipantFactory::get_instance()->create_participant(PParam, &m_listener);
    if(mp_participant==nullptr)
        return false;

    // CREATE THE COMMON READER ATTRIBUTES
    qos_.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    topic_.topicKind = NO_KEY;
    topic_.topicDataType = "HelloWorld";
    topic_.topicName = "HelloWorldTopic";

    return true;
}

HelloWorldSubscriber::~HelloWorldSubscriber() {
    DomainParticipantFactory::get_instance()->delete_participant(mp_participant);
    readers_.clear();
    datas_.clear();
}

void HelloWorldSubscriber::SubListener::on_subscription_matched(
        eprosima::fastdds::dds::DataReader*,
        eprosima::fastrtps::rtps::MatchingInfo& info)
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

void HelloWorldSubscriber::SubListener::on_data_available(eprosima::fastdds::dds::DataReader* reader)
{
    auto dit = subscriber_->datas_.find(reader);

    if (dit != subscriber_->datas_.end())
    {
        types::DynamicData_ptr data = dit->second;
        if (reader->take_next_sample(data.get(), &m_info))
        {
            if(m_info.sampleKind == ALIVE)
            {
                types::DynamicType_ptr type = subscriber_->readers_[reader];
                this->n_samples++;
                std::cout << "Received data of type " << type->get_name() << std::endl;
                types::DynamicDataHelper::print(data);
            }
        }
    }
}

void HelloWorldSubscriber::SubListener::on_type_discovery(
        DomainParticipant*,
        const string_255& topic,
        const types::TypeIdentifier*,
        const types::TypeObject*,
        types::DynamicType_ptr dyn_type)
{
    TypeSupport m_type(new types::DynamicPubSubType(dyn_type));
    subscriber_->mp_participant->register_type(m_type);

    std::cout << "Discovered type: " << m_type->getName() << " from topic " << topic << std::endl;

    if (subscriber_->mp_subscriber == nullptr)
    {
        SubscriberAttributes Rparam;
        Rparam = subscriber_->att_;
        Rparam.topic = subscriber_->topic_;
        Rparam.topic.topicName = topic;
        Rparam.qos = subscriber_->qos_;
        subscriber_->mp_subscriber = subscriber_->mp_participant->create_subscriber(
            SUBSCRIBER_QOS_DEFAULT, Rparam, nullptr);

        if (subscriber_->mp_subscriber == nullptr)
        {
            return;
        }
    }
    subscriber_->topic_.topicDataType = m_type->getName();
    DataReader* reader = subscriber_->mp_subscriber->create_datareader(
        subscriber_->topic_,
        subscriber_->qos_,
        &subscriber_->m_listener);

    subscriber_->readers_[reader] = dyn_type;
    types::DynamicData_ptr data = types::DynamicDataFactory::get_instance()->create_data(dyn_type);
    subscriber_->datas_[reader] = data;
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
