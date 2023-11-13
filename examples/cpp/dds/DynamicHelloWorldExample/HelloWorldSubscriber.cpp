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

#include <chrono>
#include <mutex>
#include <thread>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicDataHelper.hpp>

using namespace eprosima::fastdds::dds;
using eprosima::fastrtps::types::ReturnCode_t;

HelloWorldSubscriber::HelloWorldSubscriber()
    : mp_participant(nullptr)
    , mp_subscriber(nullptr)
    , m_listener(this)
{
}

bool HelloWorldSubscriber::init()
{
    //Do not enable entities on creation
    DomainParticipantFactoryQos factory_qos;
    factory_qos.entity_factory().autoenable_created_entities = false;
    DomainParticipantFactory::get_instance()->set_qos(factory_qos);

    DomainParticipantQos pqos;
    pqos.name("Participant_sub");
    StatusMask par_mask = StatusMask::subscription_matched() << StatusMask::data_available();
    mp_participant = DomainParticipantFactory::get_instance()->create_participant(0, pqos, &m_listener, par_mask);

    if (mp_participant == nullptr)
    {
        return false;
    }
    if (mp_participant->enable() != ReturnCode_t::RETCODE_OK)
    {
        DomainParticipantFactory::get_instance()->delete_participant(mp_participant);
        return false;
    }

    // CREATE THE COMMON READER ATTRIBUTES
    qos_ = DATAREADER_QOS_DEFAULT;
    qos_.reliability().kind = RELIABLE_RELIABILITY_QOS;

    return true;
}

HelloWorldSubscriber::~HelloWorldSubscriber()
{
    for (const auto& it : topics_)
    {
        mp_subscriber->delete_datareader(it.first);
        mp_participant->delete_topic(it.second);
    }
    if (mp_subscriber != nullptr)
    {
        mp_participant->delete_subscriber(mp_subscriber);
    }

    DomainParticipantFactory::get_instance()->delete_participant(mp_participant);
    topics_.clear();
    readers_.clear();
    datas_.clear();
}

void HelloWorldSubscriber::SubListener::on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        n_matched = info.total_count;
        std::cout << "Subscriber matched" << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        n_matched = info.total_count;
        std::cout << "Subscriber unmatched" << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
}

void HelloWorldSubscriber::SubListener::on_data_available(
        DataReader* reader)
{
    auto dit = subscriber_->datas_.find(reader);

    if (dit != subscriber_->datas_.end())
    {
        eprosima::fastrtps::types::DynamicData_ptr data = dit->second;
        SampleInfo info;
        if (reader->take_next_sample(data.get(), &info) == ReturnCode_t::RETCODE_OK)
        {
            if (info.instance_state == ALIVE_INSTANCE_STATE)
            {
                eprosima::fastrtps::types::DynamicType_ptr type = subscriber_->readers_[reader];
                this->n_samples++;
                std::cout << "Received data of type " << type->get_name() << std::endl;
                eprosima::fastrtps::types::DynamicDataHelper::print(data);
            }
        }
    }
}

void HelloWorldSubscriber::SubListener::on_type_discovery(
        DomainParticipant*,
        const eprosima::fastrtps::rtps::SampleIdentity&,
        const eprosima::fastrtps::string_255& topic_name,
        const eprosima::fastrtps::types::TypeIdentifier*,
        const eprosima::fastrtps::types::TypeObject*,
        eprosima::fastrtps::types::DynamicType_ptr dyn_type)
{
    std::cout << "Discovered type: " << dyn_type->get_name() << " from topic " << topic_name << std::endl;
    received_type_ = dyn_type;
    reception_flag_.store(true);
    types_cv_.notify_one();
}

void HelloWorldSubscriber::initialize_entities()
{
    auto type = m_listener.received_type_;
    std::cout << "Initializing DDS entities for type: " << type->get_name() << std::endl;
    TypeSupport m_type(new eprosima::fastrtps::types::DynamicPubSubType(type));
    m_type.register_type(mp_participant);

    if (mp_subscriber == nullptr)
    {
        mp_subscriber = mp_participant->create_subscriber(
            SUBSCRIBER_QOS_DEFAULT, nullptr);

        if (mp_subscriber == nullptr)
        {
            return;
        }
    }

    //CREATE THE TOPIC
    Topic* topic = mp_participant->create_topic(
        "DDSDynHelloWorldTopic",
        m_type->getName(),
        TOPIC_QOS_DEFAULT);

    if (topic == nullptr)
    {
        return;
    }

    StatusMask sub_mask = StatusMask::subscription_matched() << StatusMask::data_available();
    DataReader* reader = mp_subscriber->create_datareader(
        topic,
        qos_,
        &m_listener,
        sub_mask);

    topics_[reader] = topic;
    readers_[reader] = type;
    eprosima::fastrtps::types::DynamicData_ptr data(
        eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(type));
    datas_[reader] = data;
}

void HelloWorldSubscriber::run()
{
    std::cout << "Subscriber running. Please press enter to stop the Subscriber" << std::endl;
    std::unique_lock<std::mutex> lock(m_listener.types_mx_);
    m_listener.types_cv_.wait(lock, [&]()
            {
                return m_listener.reception_flag_.exchange(false);
            });

    initialize_entities();

    std::cin.ignore();
}

void HelloWorldSubscriber::run(
        uint32_t number)
{
    std::cout << "Subscriber running until " << number << "samples have been received" << std::endl;
    std::unique_lock<std::mutex> lock(m_listener.types_mx_);
    m_listener.types_cv_.wait(lock, [&]()
            {
                return m_listener.reception_flag_.exchange(false);
            });

    initialize_entities();

    while (number > this->m_listener.n_samples)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
