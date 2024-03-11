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
 * @file HelloWorldSubscriberWaitset.cpp
 *
 */

#include "HelloWorldSubscriberWaitset.h"

#include <condition_variable>
#include <csignal>
#include <stdexcept>
#include <thread>

#include <fastdds/dds/core/condition/GuardCondition.hpp>
#include <fastdds/dds/core/condition/WaitSet.hpp>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>

using namespace eprosima::fastdds::dds;

std::atomic<bool> HelloWorldSubscriberWaitset::stop_(false);
GuardCondition HelloWorldSubscriberWaitset::terminate_condition_;

HelloWorldSubscriberWaitset::HelloWorldSubscriberWaitset()
    : participant_(nullptr)
    , subscriber_(nullptr)
    , topic_(nullptr)
    , reader_(nullptr)
    , type_(new HelloWorldPubSubType())
{
    stop_.store(false);

    // Create the participant
    auto factory = DomainParticipantFactory::get_instance();
    participant_ = factory->create_participant_with_default_profile();
    if (participant_ == nullptr)
    {
        throw std::runtime_error("Participant initialization failed");
    }

    // Register the type
    type_.register_type(participant_);

    // Create the subscriber
    SubscriberQos sub_qos = SUBSCRIBER_QOS_DEFAULT;
    participant_->get_default_subscriber_qos(sub_qos);
    subscriber_ = participant_->create_subscriber(sub_qos, nullptr);
    if (subscriber_ == nullptr)
    {
        throw std::runtime_error("Subscriber initialization failed");
    }

    // Create the topic
    TopicQos topic_qos = TOPIC_QOS_DEFAULT;
    participant_->get_default_topic_qos(topic_qos);
    topic_ = participant_->create_topic("Hello_world_topic", "HelloWorld", topic_qos);
    if (topic_ == nullptr)
    {
        throw std::runtime_error("Topic initialization failed");
    }

    // Create the reader
    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    subscriber_->get_default_datareader_qos(reader_qos);
    reader_ = subscriber_->create_datareader(topic_, reader_qos);
    if (reader_ == nullptr)
    {
        throw std::runtime_error("DataReader initialization failed");
    }

    // Prepare a wait-set
    wait_set_.attach_condition(reader_->get_statuscondition());
    wait_set_.attach_condition(terminate_condition_);
}

HelloWorldSubscriberWaitset::~HelloWorldSubscriberWaitset()
{
    wait_set_.detach_condition(reader_->get_statuscondition());
    if (reader_ != nullptr)
    {
        subscriber_->delete_datareader(reader_);
    }
    if (topic_ != nullptr)
    {
        participant_->delete_topic(topic_);
    }
    if (subscriber_ != nullptr)
    {
        participant_->delete_subscriber(subscriber_);
    }
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

void HelloWorldSubscriberWaitset::run()
{
    std::thread sub_thread([&]
            {
                while (!stop_.load())
                {
                    ConditionSeq triggered_conditions;
                    ReturnCode_t ret_code = wait_set_.wait(triggered_conditions, eprosima::fastrtps::c_TimeInfinite);
                    if (ReturnCode_t::RETCODE_OK != ret_code)
                    {
                        EPROSIMA_LOG_ERROR(SUBSCRIBER_WAITSET, "Error waiting for conditions");
                        continue;
                    }
                    for (Condition* cond : triggered_conditions)
                    {
                        StatusCondition* status_cond = dynamic_cast<StatusCondition*>(cond);
                        if (nullptr != status_cond)
                        {
                            Entity* entity = status_cond->get_entity();
                            StatusMask changed_statuses = entity->get_status_changes();
                            if (changed_statuses.is_active(StatusMask::subscription_matched()))
                            {
                                SubscriptionMatchedStatus status_;
                                reader_->get_subscription_matched_status(status_);
                                if (status_.current_count_change == 1)
                                {
                                    std::cout << "Waitset Subscriber matched." << std::endl;
                                }
                                else if (status_.current_count_change == -1)
                                {
                                    std::cout << "Waitset Subscriber unmatched." << std::endl;
                                }
                                else
                                {
                                    std::cout << status_.current_count_change <<
                                        " is not a valid value for SubscriptionMatchedStatus current count change" <<
                                        std::endl;
                                }
                            }
                            if (changed_statuses.is_active(StatusMask::data_available()))
                            {
                                SampleInfo info;
                                while ((reader_->take_next_sample(&hello_,
                                &info) == ReturnCode_t::RETCODE_OK) && !stop_.load())
                                {
                                    if (info.instance_state == ALIVE_INSTANCE_STATE)
                                    {
                                        // Print Hello world message data
                                        std::cout << "Message: '" << hello_.message() << "' with index: '"
                                                  << hello_.index() << "' RECEIVED" << std::endl;
                                    }
                                }
                            }
                        }
                    }
                }
            });
    std::cout << "Waitset Subscriber running. Please press Ctrl+C to stop the Waitset Subscriber at any time."
              << std::endl;
    signal(SIGINT, [](int signum)
            {
                std::cout << "SIGINT received, stopping Waitset Subscriber execution." << std::endl;
                static_cast<void>(signum); HelloWorldSubscriberWaitset::stop();
            });
    sub_thread.join();
}

bool HelloWorldSubscriberWaitset::is_stopped()
{
    return stop_;
}

void HelloWorldSubscriberWaitset::stop()
{
    stop_ = true;
    terminate_condition_.set_trigger_value(true);
}
