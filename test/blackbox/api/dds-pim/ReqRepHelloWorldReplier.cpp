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
 * @file ReqRepHelloWorldReplier.cpp
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

#include "ReqRepHelloWorldReplier.hpp"
#include "ReqRepHelloWorldService.hpp"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::dds::rpc;
using namespace eprosima::fastdds::rtps;

ReqRepHelloWorldReplier::ReqRepHelloWorldReplier()
    : replier_(nullptr)
    , service_(nullptr)
    , participant_(nullptr)
    , initialized_(false)
    , matched_(0)
{
}

ReqRepHelloWorldReplier::~ReqRepHelloWorldReplier()
{
    stop_processing_thread_.set_trigger_value(true);

    // stop the processing thread
    if (processing_thread_.joinable())
    {
        processing_thread_.join();
    }

    if (participant_)
    {
        if (service_)
        {
            if (replier_)
            {
                participant_->delete_service_replier(service_->get_service_name(), replier_);
            }

            participant_->delete_service(service_);
        }

        participant_->delete_contained_entities();
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void ReqRepHelloWorldReplier::init()
{
    init_with_custom_qos(create_replier_qos());
}

void ReqRepHelloWorldReplier::init_with_custom_qos(
        const ReplierQos& qos)
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

    // Create replier
    replier_ = participant_->create_service_replier(service_, qos);
    ASSERT_NE(replier_, nullptr);
    ASSERT_EQ(replier_->is_enabled(), true);

    init_processing_thread();

    initialized_ = true;
}

void ReqRepHelloWorldReplier::newNumber(
        const RequestInfo& info,
        uint16_t number)
{
    HelloWorld hello;
    hello.index(number);
    hello.message("GoodBye");
    ASSERT_EQ(replier_->send_reply((void*)&hello, info), RETCODE_OK);
}

void ReqRepHelloWorldReplier::wait_discovery()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);

    std::cout << "Replier is waiting discovery..." << std::endl;

    cvDiscovery_.wait(lock, [&]()
            {
                return matched_ > 1;
            });

    std::cout << "Replier discovery finished..." << std::endl;
}

void ReqRepHelloWorldReplier::matched()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);
    ++matched_;
    if (matched_ > 1)
    {
        cvDiscovery_.notify_one();
    }
}

void ReqRepHelloWorldReplier::init_processing_thread()
{
    wait_set_.attach_condition(stop_processing_thread_);
    wait_set_.attach_condition(replier_->get_replier_writer()->get_statuscondition());
    wait_set_.attach_condition(replier_->get_replier_reader()->get_statuscondition());

    processing_thread_ = std::thread(&ReqRepHelloWorldReplier::process_status_changes, this);
}

void ReqRepHelloWorldReplier::process_status_changes()
{
    while (!stop_processing_thread_.get_trigger_value())
    {
        ReturnCode_t retcode;
        ConditionSeq triggered_conditions;

        retcode = wait_set_.wait(triggered_conditions, c_TimeInfinite);

        if (RETCODE_OK != retcode)
        {
            std::cout << "Replier: Error processing status changes" << std::endl;
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
                    std::cout << "Replier: Processing publication matched status" << std::endl;

                    DataWriter* writer = dynamic_cast<DataWriter*>(entity);
                    ASSERT_NE(writer, nullptr);
                    ASSERT_EQ(writer, replier_->get_replier_writer());

                    PublicationMatchedStatus status;
                    if (RETCODE_OK != writer->get_publication_matched_status(status))
                    {
                        std::cout << "Replier: Error processing publication matched status" << std::endl;
                        continue;
                    }

                    if (status.current_count_change > 0)
                    {
                        matched();
                    }
                }
                else if (status_changes.is_active(StatusMask::subscription_matched()))
                {
                    std::cout << "Replier: Processing subscription matched status" << std::endl;

                    DataReader* reader = dynamic_cast<DataReader*>(entity);
                    ASSERT_NE(reader, nullptr);
                    ASSERT_EQ(reader, replier_->get_replier_reader());

                    SubscriptionMatchedStatus status;
                    if (RETCODE_OK != reader->get_subscription_matched_status(status))
                    {
                        std::cout << "Replier: Error processing subscription matched status" << std::endl;
                        continue;
                    }

                    if (status.current_count_change > 0)
                    {
                        matched();
                    }
                }
                else if (status_changes.is_active(StatusMask::data_available()))
                {
                    std::cout << "Replier: Processing data available status" << std::endl;

                    DataReader* reader = dynamic_cast<DataReader*>(entity);
                    ASSERT_NE(reader, nullptr);
                    ASSERT_EQ(reader, replier_->get_replier_reader());

                    HelloWorld hello;
                    RequestInfo info;

                    while (RETCODE_OK == replier_->take_request((void*)&hello, info))
                    {
                        if (info.valid_data)
                        {
                            ASSERT_EQ(hello.message().compare("HelloWorld"), 0);
                            newNumber(info, hello.index());
                        }
                    }
                }
            }
        }
    }
}

ReplierQos ReqRepHelloWorldReplier::create_replier_qos()
{
    ReplierQos replier_qos;
    ReqRepHelloWorldService service;

    DataWriterQos& writer_qos = replier_qos.writer_qos;
    DataReaderQos& reader_qos = replier_qos.reader_qos;

    // Requester/Replier DataWriter QoS configuration
    reader_qos.endpoint().history_memory_policy = PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    writer_qos.endpoint().history_memory_policy = PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

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

    return replier_qos;
}
