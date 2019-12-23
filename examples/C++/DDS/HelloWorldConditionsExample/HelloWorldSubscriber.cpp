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
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/DataReader.hpp>
#include <fastdds/dds/topic/qos/DataReaderQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastrtps/attributes/TopicAttributes.h>

#include <fastdds/dds/core/conditions/WaitSet.hpp>
#include <fastdds/dds/core/conditions/StatusCondition.hpp>

using namespace eprosima::fastdds::dds;

HelloWorldSubscriber::HelloWorldSubscriber()
    : participant_(nullptr)
    , subscriber_(nullptr)
    , type_(new HelloWorldPubSubType())
{
}

bool HelloWorldSubscriber::init(
        int domain_id)
{
    eprosima::fastrtps::ParticipantAttributes participant_att;
    participant_att.rtps.builtin.domainId = domain_id;
    participant_att.rtps.setName("Participant_sub");
    participant_ = DomainParticipantFactory::get_instance()->create_participant(participant_att, nullptr);

    if (participant_ == nullptr)
    {
        return false;
    }

    //REGISTER THE TYPE
    type_.register_type(participant_, type_.get_type_name());

    //CREATE THE SUBSCRIBER
    eprosima::fastrtps::SubscriberAttributes sub_att;
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, sub_att, nullptr);

    if (subscriber_ == nullptr)
    {
        return false;
    }

    //Set the handler associated to the subscriber status condition
    subscriber_->get_statuscondition()->set_handler([this](StatusCondition* subs_cond) -> void {
        (void) subs_cond;
        while (reader_->take_next_sample(&hello_, &info_) == ReturnCode_t::RETCODE_OK)
        {
            if (info_.instance_state == ::dds::sub::status::InstanceState::alive())
            {
                samples_++;
                // Print your structure data here.
                std::cout << "Message " << hello_.message() << " " << hello_.index() << " RECEIVED" << std::endl;
            }
        }
    });

    // CREATE THE READER
    DataReaderQos rqos;
    rqos.reliability.kind = RELIABLE_RELIABILITY_QOS;
    rqos.history.depth = 20;
    rqos.history.kind = KEEP_ALL_HISTORY_QOS;
    rqos.durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    //Topic topic(participant_, "HelloWorldTopic", "HelloWorld", TopicQos()); //PSM
    TopicDescription topic_desc(participant_, "HelloWorldTopic", "HelloWorld"); //PIM
    reader_ = subscriber_->create_datareader(topic_desc, rqos, nullptr);

    if (reader_ == nullptr)
    {
        return false;
    }

    //Set the handler associated to the reader status condition
    reader_->get_statuscondition()->set_handler([this](StatusCondition* reader_cond) -> void {
        eprosima::fastdds::dds::SubscriptionMatchedStatus info;
        static_cast<DataReader*>(reader_cond->get_entity())->get_subscription_matched_status(info);
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
    });

    return true;
}

HelloWorldSubscriber::~HelloWorldSubscriber()
{
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

void HelloWorldSubscriber::run()
{
    std::cout << "Subscriber running. Please press enter to stop the Subscriber" << std::endl;
    std::cin.ignore();
}

void HelloWorldSubscriber::run(
        uint32_t number)
{
    std::cout << "Subscriber running until " << number << " samples have been received" << std::endl;

    //Creation of waitset and attachment of the conditions
    WaitSet waitset;
    waitset.attach_condition(participant_->get_statuscondition());
    waitset.attach_condition(subscriber_->get_statuscondition());
    waitset.attach_condition(reader_->get_statuscondition());
    ConditionSeq active_conditions;
    while (number > samples_)
    {
        if (waitset.wait(active_conditions, Duration_t(0, 500)) != ReturnCode_t::RETCODE_TIMEOUT)
        {
            for (auto cond: active_conditions)
            {
                if (reader_->get_statuscondition() == cond)
                {
                    reader_->get_statuscondition()->call_handler(reader_->get_statuscondition());
                }
                else if (subscriber_->get_statuscondition() == cond)
                {
                    subscriber_->get_statuscondition()->call_handler(subscriber_->get_statuscondition());
                }
            }
        }
    }
}
