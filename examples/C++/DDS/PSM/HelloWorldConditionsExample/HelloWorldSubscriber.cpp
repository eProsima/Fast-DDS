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
using namespace ::dds::core::status;

HelloWorldSubscriber::HelloWorldSubscriber()
    : participant_(nullptr)
    , subscriber_(dds::core::null)
    , reader_(dds::core::null)
    , type_(new HelloWorldPubSubType())
    , topic_(dds::core::null)
{
}

bool HelloWorldSubscriber::init(
        int domain_id)
{
    participant_ = dds::domain::DomainParticipant(domain_id);

    if (participant_ == dds::core::null)
    {
        return false;
    }

    waitset_.attach_condition(participant_.status_condition());

    //REGISTER THE TYPE
    type_.register_type(participant_.delegate().get(), type_.get_type_name());

    //CREATE THE SUBSCRIBER
    subscriber_ = dds::sub::Subscriber(participant_, SUBSCRIBER_QOS_DEFAULT, nullptr);

    if (subscriber_ == dds::core::null)
    {
        return false;
    }

    //Set the handler associated to the subscriber status condition
    subscriber_.status_condition()->set_handler([this]() -> void {
        subscriber_.notify_datareaders();
        subscriber_.status_condition()->set_status_as_read(StatusMask::data_on_readers());
    });

    waitset_.attach_condition(subscriber_.status_condition());

    // TopicQos
    dds::topic::qos::TopicQos topicQos
        = participant_.default_topic_qos()
            << dds::core::policy::Reliability::Reliable();

    topic_ = dds::topic::Topic<HelloWorld>(participant_, "HelloWorldTopic", "HelloWorld", topicQos);

    topic_.status_condition()->set_handler([this]() -> void {
        StatusMask triggered_status = topic_->get_status_changes();
        if (triggered_status.is_active(StatusMask::inconsistent_topic()))
        {
            eprosima::fastdds::dds::InconsistentTopicStatus status;
            topic_->get_inconsistent_topic_status(
                status);
            if (status.total_count_change == 1)
            {
                std::cout << "The discovered topic is inconsistent with topic " << topic_->get_instance_handle() << std::endl;
            }
        }
    });

    waitset_.attach_condition(topic_.status_condition());

    dds::sub::qos::DataReaderQos drqos = topicQos;

    // CREATE THE READER
    reader_ = dds::sub::DataReader<HelloWorld>(subscriber_, topic_, drqos, nullptr);

    if (reader_ == dds::core::null)
    {
        return false;
    }

    //Set the handler associated to the reader status condition
    reader_.status_condition()->set_handler([this]() -> void {
        StatusMask triggered_status = reader_->get_status_changes();
        if (triggered_status.is_active(StatusMask::subscription_matched()))
        {
            subscription_matched_handler();
        }
        if (triggered_status.is_active(StatusMask::requested_incompatible_qos()))
        {
            requested_incompatible_qos_handler();
        }
        if (triggered_status.is_active(StatusMask::data_available()))
        {
            data_available_handler(&reader_);
        }
        if (triggered_status.is_active(StatusMask::sample_rejected()))
        {
            sample_rejected_handler();
        }
        if (triggered_status.is_active(StatusMask::liveliness_changed()))
        {
            liveliness_changed_handler();
        }
        if (triggered_status.is_active(StatusMask::requested_deadline_missed()))
        {
            requested_deadline_missed_handler();
        }
        if (triggered_status.is_active(StatusMask::sample_lost()))
        {
            sample_lost_handler();
        }

    });

    waitset_.attach_condition(reader_.status_condition());

    return true;
}

HelloWorldSubscriber::~HelloWorldSubscriber()
{
    participant_.delete_participant();
}

void HelloWorldSubscriber::run()
{
    std::cout << "Subscriber running." << std::endl;
    ConditionSeq active_conditions;
    while (true)
    {
        if (waitset_.wait(active_conditions, Duration_t(0, 0)) != ReturnCode_t::RETCODE_TIMEOUT)
        {
            for (auto cond: active_conditions)
            {
                cond->call_handler();
            }
        }
        //waitset_.dispatch(Duration_t(0, 0));
    }
}

void HelloWorldSubscriber::run(
        uint32_t number)
{
    std::cout << "Subscriber running until " << number << "samples have been received" << std::endl;
    while (number > samples_)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void HelloWorldSubscriber::data_available_handler(
        dds::sub::DataReader<HelloWorld>* reader)
{
    dds::sub::LoanedSamples<HelloWorld> samples = reader->take();
    for (auto sample = samples.begin(); sample < samples.end(); ++sample)
    {
        if (sample->info().valid())
        {
            samples_++;
            hello_ = sample->data();
            // Print your structure data here.
            std::cout << "Message " << hello_.message() << " " << hello_.index() << " RECEIVED" <<
                std::endl;
        }
    }
    reader->status_condition()->set_status_as_read(StatusMask::data_available());
}

void HelloWorldSubscriber::liveliness_changed_handler()
{
    eprosima::fastdds::dds::LivelinessChangedStatus status;
    status = reader_.liveliness_changed_status();
    if (status.alive_count_change == 1)
    {
        std::cout << "Publisher " << status.last_publication_handle << " recovered liveliness" << std::endl;
    }
    else if (status.not_alive_count_change == 1)
    {
        std::cout << "Publisher " << status.last_publication_handle << " lost liveliness" << std::endl;
    }
}

void HelloWorldSubscriber::requested_deadline_missed_handler()
{
    std::cout << "Condition" << std::endl;
    eprosima::fastdds::dds::RequestedDeadlineMissedStatus status;
    status = reader_.requested_deadline_missed_status();
    std::cout << "Deadline missed for instance: " << status.last_instance_handle << std::endl;
}

void HelloWorldSubscriber::requested_incompatible_qos_handler()
{
    eprosima::fastdds::dds::RequestedIncompatibleQosStatus status;
    status = reader_.requested_incompatible_qos_status();
    QosPolicy qos;
    std::cout << "The Requested Qos is incompatible with the Offered one." << std::endl;
    std::cout << "The Qos causing this incompatibility is " << qos.search_qos_by_id(
        status.last_policy_id) << "." << std::endl;
}

void HelloWorldSubscriber::subscription_matched_handler()
{
    eprosima::fastdds::dds::SubscriptionMatchedStatus info;
    info = reader_.subscription_matched_status();
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

void HelloWorldSubscriber::sample_rejected_handler()
{
    eprosima::fastdds::dds::SampleRejectedStatus status;
    status = reader_.sample_rejected_status();
    std::cout << "The sample " << status.last_seq_num << " is " << status.reason_to_string() <<
        std::endl;
}

void HelloWorldSubscriber::sample_lost_handler()
{
    eprosima::fastdds::dds::SampleLostStatus status;
    status = reader_.sample_lost_status();
    std::cout << "There are " << status.total_count << " samples lost" << std::endl;
}
