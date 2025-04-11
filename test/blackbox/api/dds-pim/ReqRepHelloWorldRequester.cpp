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
 * @file ReqRepHelloWorldRequester.cpp
 *
 */

#include <gtest/gtest.h>

#include <fastdds/dds/core/condition/Condition.hpp>
#include <fastdds/dds/core/condition/StatusCondition.hpp>
#include <fastdds/dds/core/status/PublicationMatchedStatus.hpp>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/rpc/RequestInfo.hpp>
#include <fastdds/dds/rpc/ServiceTypeSupport.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>

#include "ReqRepHelloWorldRequester.hpp"
#include "ReqRepHelloWorldService.hpp"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::dds::rpc;
using namespace eprosima::fastdds::rtps;

ReqRepHelloWorldRequester::ReqRepHelloWorldRequester()
    : current_number_(std::numeric_limits<uint16_t>::max())
    , number_received_(std::numeric_limits<uint16_t>::max())
    , requester_(nullptr)
    , service_(nullptr)
    , participant_(nullptr)
    , initialized_(false)
    , matched_(0)
{
}

ReqRepHelloWorldRequester::~ReqRepHelloWorldRequester()
{
    stop_processing_thread_.set_trigger_value(true);

    // Stop the processing thread
    if (processing_thread_.joinable())
    {
        processing_thread_.join();
    }

    if (participant_)
    {
        if (service_)
        {
            if (requester_)
            {
                participant_->delete_service_requester(service_->get_service_name(), requester_);
            }

            participant_->delete_service(service_);
        }

        participant_->delete_contained_entities();
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void ReqRepHelloWorldRequester::init(
        bool use_volatile /* = false */)
{
    init_with_custom_qos(create_requester_qos(use_volatile));
}

void ReqRepHelloWorldRequester::init_with_custom_qos(
        const RequesterQos& requester_qos)
{
    ASSERT_NE(initialized_, true);

    // Create participant
    participant_ = DomainParticipantFactory::get_instance()->create_participant(
        (uint32_t)GET_PID() % 230, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant_, nullptr);
    ASSERT_TRUE(participant_->is_enabled());

    // Register service type and create service
    ReqRepHelloWorldService service;
    service_ = service.init(participant_);
    ASSERT_NE(service_, nullptr);

    // Create requester
    requester_ = participant_->create_service_requester(service_, requester_qos);
    ASSERT_NE(requester_, nullptr);
    ASSERT_EQ(requester_->is_enabled(), true);

    init_processing_thread();

    initialized_ = true;
}

void ReqRepHelloWorldRequester::init_with_latency(
        const Duration_t& latency_budget_duration_pub,
        const Duration_t& latency_budget_duration_sub)
{
    RequesterQos requester_qos = create_requester_qos();
    requester_qos.writer_qos.latency_budget().duration = latency_budget_duration_pub;
    requester_qos.reader_qos.latency_budget().duration = latency_budget_duration_sub;

    init_with_custom_qos(requester_qos);
}

void ReqRepHelloWorldRequester::newNumber(
        const RequestInfo& info,
        uint16_t number)
{
    std::unique_lock<std::mutex> lock(mutex_);
    received_sample_identity_ = info.related_sample_identity;
    number_received_ = number;
    ASSERT_EQ(current_number_, number_received_);
    if (current_number_ == number_received_)
    {
        cv_.notify_one();
    }
}

void ReqRepHelloWorldRequester::block(
        const std::chrono::seconds& seconds)
{
    std::unique_lock<std::mutex> lock(mutex_);

    bool timeout = cv_.wait_for(lock, seconds, [&]() -> bool
                    {
                        return current_number_ == number_received_;
                    });

    ASSERT_TRUE(timeout);
    ASSERT_EQ(current_number_, number_received_);
    ASSERT_EQ(related_sample_identity_, received_sample_identity_);
}

void ReqRepHelloWorldRequester::wait_discovery()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);

    std::cout << "Requester is waiting discovery..." << std::endl;

    cvDiscovery_.wait(lock, [&]()
            {
                return matched_ > 1;
            });

    std::cout << "Requester discovery finished..." << std::endl;
}

void ReqRepHelloWorldRequester::matched()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);
    ++matched_;
    if (matched_ > 1)
    {
        cvDiscovery_.notify_one();
    }
}

void ReqRepHelloWorldRequester::send(
        const uint16_t number)
{
    RequestInfo info;
    HelloWorld hello;
    hello.index(number);
    hello.message("HelloWorld");

    {
        std::unique_lock<std::mutex> lock(mutex_);
        current_number_ = number;
    }

    ASSERT_EQ(requester_->send_request((void*)&hello, info), RETCODE_OK);
    related_sample_identity_ = info.related_sample_identity;

    ASSERT_NE(related_sample_identity_.sequence_number(), SequenceNumber_t());
}

void ReqRepHelloWorldRequester::send(
        const uint16_t number,
        const eprosima::fastdds::rtps::SampleIdentity& related_sample_identity)
{
    RequestInfo info;
    info.related_sample_identity = related_sample_identity;
    HelloWorld hello;
    hello.index(number);
    hello.message("HelloWorld");

    {
        std::unique_lock<std::mutex> lock(mutex_);
        current_number_ = number;
    }

    ASSERT_EQ(requester_->send_request((void*)&hello, info), RETCODE_OK);
    related_sample_identity_ = info.related_sample_identity;

    ASSERT_NE(related_sample_identity_.sequence_number(), SequenceNumber_t());

    if (eprosima::fastdds::rtps::GUID_t::unknown() != related_sample_identity.writer_guid())
    {
        ASSERT_EQ(related_sample_identity_.writer_guid(), related_sample_identity.writer_guid());
    }

    if (eprosima::fastdds::rtps::SequenceNumber_t::unknown() != related_sample_identity.sequence_number())
    {
        ASSERT_EQ(related_sample_identity_.sequence_number(), related_sample_identity.sequence_number());
    }
}

const Duration_t ReqRepHelloWorldRequester::datawriter_latency_budget_duration() const
{
    return requester_->get_requester_writer()->get_qos().latency_budget().duration;
}

const Duration_t ReqRepHelloWorldRequester::datareader_latency_budget_duration() const
{
    return requester_->get_requester_reader()->get_qos().latency_budget().duration;
}

const eprosima::fastdds::rtps::GUID_t& ReqRepHelloWorldRequester::get_reader_guid() const
{
    return requester_->get_requester_reader()->guid();
}

const eprosima::fastdds::rtps::SampleIdentity& ReqRepHelloWorldRequester::get_last_related_sample_identity() const
{
    return related_sample_identity_;
}

void ReqRepHelloWorldRequester::init_processing_thread()
{
    wait_set_.attach_condition(stop_processing_thread_);
    wait_set_.attach_condition(requester_->get_requester_writer()->get_statuscondition());
    wait_set_.attach_condition(requester_->get_requester_reader()->get_statuscondition());

    processing_thread_ = std::thread(&ReqRepHelloWorldRequester::process_status_changes, this);
}

void ReqRepHelloWorldRequester::process_status_changes()
{
    while (!stop_processing_thread_.get_trigger_value())
    {
        ReturnCode_t retcode;
        ConditionSeq triggered_conditions;

        retcode = wait_set_.wait(triggered_conditions, c_TimeInfinite);

        if (RETCODE_OK != retcode)
        {
            std::cout << "Requester: Error processing status changes" << std::endl;
            continue;
        }

        for (Condition* condition : triggered_conditions)
        {
            // Process reader/writer status changes
            StatusCondition* status_condition = dynamic_cast<StatusCondition*>(condition);

            // Check if the triggered condition is a status condition.
            // If it is, process it and notify the changes to the main thread
            if (status_condition)
            {
                Entity* entity = status_condition->get_entity();
                StatusMask status_changes = entity->get_status_changes();

                if (status_changes.is_active(StatusMask::publication_matched()))
                {
                    std::cout << "Requester: Processing publication matched status" << std::endl;

                    DataWriter* writer = dynamic_cast<DataWriter*>(entity);

                    ASSERT_NE(writer, nullptr);
                    ASSERT_EQ(writer, requester_->get_requester_writer());
                    PublicationMatchedStatus status;
                    if (RETCODE_OK != writer->get_publication_matched_status(status))
                    {
                        std::cout << "Requester: Error processing publication matched status" << std::endl;
                        continue;
                    }

                    if (status.current_count_change > 0)
                    {
                        matched();
                    }
                }
                else if (status_changes.is_active(StatusMask::subscription_matched()))
                {
                    std::cout << "Requester: Processing subscription matched status" << std::endl;

                    DataReader* reader = dynamic_cast<DataReader*>(entity);
                    ASSERT_NE(reader, nullptr);
                    ASSERT_EQ(reader, requester_->get_requester_reader());

                    SubscriptionMatchedStatus status;
                    if (RETCODE_OK != reader->get_subscription_matched_status(status))
                    {
                        std::cout << "Requester: Error processing subscription matched status" << std::endl;
                        continue;
                    }

                    if (status.current_count_change > 0)
                    {
                        matched();
                    }
                }
                else if (status_changes.is_active(StatusMask::data_available()))
                {
                    std::cout << "Requester: Processing data available status" << std::endl;

                    DataReader* reader = dynamic_cast<DataReader*>(entity);
                    ASSERT_NE(reader, nullptr);
                    ASSERT_EQ(reader, requester_->get_requester_reader());

                    HelloWorld hello;
                    RequestInfo info;

                    while (RETCODE_OK == requester_->take_reply((void*)&hello, info))
                    {
                        if (info.valid_data)
                        {
                            ASSERT_EQ(hello.message().compare("GoodBye"), 0);
                            newNumber(info, hello.index());
                        }
                    }
                }
            }
        }
    }
}

RequesterQos ReqRepHelloWorldRequester::create_requester_qos(
        bool volatile_durability_qos)
{
    RequesterQos requester_qos;
    ReqRepHelloWorldService service;

    DataWriterQos& writer_qos = requester_qos.writer_qos;
    DataReaderQos& reader_qos = requester_qos.reader_qos;
    // Requester/Replier DataWriter QoS configuration
    reader_qos.endpoint().history_memory_policy = PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    writer_qos.endpoint().history_memory_policy = PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

    if (volatile_durability_qos)
    {
        reader_qos.durability().kind = VOLATILE_DURABILITY_QOS;
        writer_qos.durability().kind = VOLATILE_DURABILITY_QOS;
    }

    writer_qos.reliable_writer_qos().times.heartbeat_period.seconds = 1;
    writer_qos.reliable_writer_qos().times.heartbeat_period.nanosec = 0;

    if (enable_datasharing)
    {
        reader_qos.data_sharing().automatic();
        writer_qos.data_sharing().automatic();
    }
    else
    {
        reader_qos.data_sharing().off();
        writer_qos.data_sharing().off();
    }

    if (use_pull_mode)
    {
        writer_qos.properties().properties().emplace_back("fastdds.push_mode", "false");
    }

    return requester_qos;
}
