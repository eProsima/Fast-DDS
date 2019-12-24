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
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/topic/DataWriter.hpp>
#include <fastdds/dds/topic/qos/DataWriterQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>

#include <fastdds/dds/core/conditions/WaitSet.hpp>
#include <fastdds/dds/core/conditions/StatusCondition.hpp>

#include <thread>

using namespace eprosima::fastdds::dds;
using namespace ::dds::core::status;

HelloWorldPublisher::HelloWorldPublisher()
    : participant_(nullptr)
    , publisher_(nullptr)
    , type_(new HelloWorldPubSubType())
{
}

bool HelloWorldPublisher::init(
        int domain_id)
{
    hello_.index(0);
    hello_.message("HelloWorld");
    eprosima::fastrtps::ParticipantAttributes participant_att;
    participant_att.rtps.builtin.domainId = domain_id;
    participant_att.rtps.setName("Participant_pub");
    participant_ = DomainParticipantFactory::get_instance()->create_participant(participant_att, nullptr);

    if (participant_ == nullptr)
    {
        return false;
    }

    //REGISTER THE TYPE
    type_.register_type(participant_, type_.get_type_name());

    //CREATE THE PUBLISHER
    eprosima::fastrtps::PublisherAttributes pub_att;
    pub_att.topic.topicDataType = "HelloWorld";
    pub_att.topic.topicName = "HelloWorldTopic";
    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, pub_att, nullptr);

    Topic* topic = participant_->create_topic(pub_att.topic.topicName.c_str(),
                    pub_att.topic.topicDataType.c_str(), TOPIC_QOS_DEFAULT);

    DataWriterQos qos;
    qos.reliability.kind = RELIABLE_RELIABILITY_QOS;

    if (publisher_ == nullptr)
    {
        return false;
    }

    // CREATE THE WRITER
    writer_ = publisher_->create_datawriter(*topic, qos, nullptr);

    if (writer_ == nullptr)
    {
        return false;
    }

    writer_->get_statuscondition()->set_handler([this]() -> void {
        StatusMask triggered_status = writer_->get_status_changes();
        if (triggered_status.is_active(StatusMask::publication_matched()))
        {
            publication_matched_handler();
        }
        if (triggered_status.is_active(StatusMask::offered_deadline_missed()))
        {
            offered_deadline_missed_handler();
        }
        if (triggered_status.is_active(StatusMask::offered_incompatible_qos()))
        {
            offered_incompatible_qos_handler();
        }
        if (triggered_status.is_active(StatusMask::liveliness_lost()))
        {
            liveliness_lost_handler();
        }
    });

    return true;
}

HelloWorldPublisher::~HelloWorldPublisher()
{
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

void HelloWorldPublisher::check_reader_matched()
{
    WaitSet waitset;
    waitset.attach_condition(writer_->get_statuscondition());
    ConditionSeq active_conditions;
    std::unique_lock<std::mutex> lock(mtx_);

    while (matched_ == 0)
    {
        waitset.wait(active_conditions, Duration_t(0, 500));
        for (auto cond: active_conditions)
        {
            if (writer_->get_statuscondition() == cond)
            {
                writer_->get_statuscondition()->call_handler();
            }
        }
    }
}

void HelloWorldPublisher::runThread(
        uint32_t samples,
        uint32_t sleep)
{
    check_reader_matched();

    if (samples == 0)
    {
        while (!stop_)
        {
            if (publish(false))
            {
                std::cout << "Message: " << hello_.message() << " with index: " << hello_.index()
                          << " SENT" << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
        }
    }
    else
    {
        for (uint32_t i = 0; i < samples; ++i)
        {
            if (!publish())
            {
                --i;
            }
            else
            {
                std::cout << "Message: " << hello_.message() << " with index: " << hello_.index()
                          << " SENT" << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
        }
    }
}

void HelloWorldPublisher::run(
        uint32_t samples,
        uint32_t sleep)
{
    stop_ = false;
    std::thread thread(&HelloWorldPublisher::runThread, this, samples, sleep);
    if (samples == 0)
    {
        std::cout << "Publisher running. Please press enter to stop the Publisher at any time." << std::endl;
        std::cin.ignore();
        stop_ = true;
    }
    else
    {
        std::cout << "Publisher running " << samples << " samples." << std::endl;
    }
    thread.join();
}

bool HelloWorldPublisher::publish(
        bool waitForListener)
{
    if (firstConnected_ || !waitForListener || matched_ > 0)
    {
        hello_.index(hello_.index() + 1);
        ReturnCode_t code = writer_->write(&hello_);
        if (code == ReturnCode_t::RETCODE_OK)
        {
            return true;
        }
        else if (code == ReturnCode_t::RETCODE_NOT_ENABLED)
        {
            std::cout << "Writer not enabled." << std::endl;
            return false;
        }
    }
    return false;
}

void HelloWorldPublisher::liveliness_lost_handler()
{
    eprosima::fastdds::dds::LivelinessLostStatus status;
    writer_->get_liveliness_lost_status(status);
    std::cout << "Writer " << writer_->get_instance_handle() << " lost liveliness: " << status.total_count << std::endl;
}

void HelloWorldPublisher::offered_deadline_missed_handler()
{
    eprosima::fastdds::dds::OfferedDeadlineMissedStatus status;
    writer_->get_offered_deadline_missed_status(status);
    std::cout << "Deadline missed for instance: " << status.last_instance_handle << std::endl;
}

void HelloWorldPublisher::offered_incompatible_qos_handler()
{
    eprosima::fastdds::dds::OfferedIncompatibleQosStatus status;
    writer_->get_offered_incompatible_qos_status(status);
    QosPolicy qos;
    std::cout << "The Offered Qos is incompatible with the Requested one." << std::endl;
    std::cout << "The Qos causing this incompatibility is " << qos.search_qos_by_id(
        status.last_policy_id) << "." << std::endl;
}

void HelloWorldPublisher::publication_matched_handler()
{
    eprosima::fastdds::dds::PublicationMatchedStatus info;
    writer_->get_publication_matched_status(info);
    if (info.current_count_change == 1)
    {
        matched_ = info.total_count;
        firstConnected_ = true;
        std::cout << "Publisher matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        matched_ = info.total_count;
        std::cout << "Publisher unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change." << std::endl;
    }
}
