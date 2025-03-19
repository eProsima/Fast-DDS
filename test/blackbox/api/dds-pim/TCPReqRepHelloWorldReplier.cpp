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
 * @file TCPReqRepHelloWorldReplier.cpp
 *
 */

#include <gtest/gtest.h>

#include <fastdds/dds/core/condition/Condition.hpp>
#include <fastdds/dds/core/condition/StatusCondition.hpp>
#include <fastdds/dds/core/status/PublicationMatchedStatus.hpp>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/domain/qos/ReplierQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/rpc/RequestInfo.hpp>
#include <fastdds/dds/rpc/ServiceTypeSupport.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.hpp>
#include <fastdds/utils/IPLocator.hpp>

#include "TCPReqRepHelloWorldReplier.hpp"
#include "TCPReqRepHelloWorldService.hpp"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::dds::rpc;
using namespace eprosima::fastdds::rtps;

TCPReqRepHelloWorldReplier::TCPReqRepHelloWorldReplier()
    : replier_(nullptr)
    , service_(nullptr)
    , participant_(nullptr)
    , initialized_(false)
    , matched_(0)
{
}

TCPReqRepHelloWorldReplier::~TCPReqRepHelloWorldReplier()
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

void TCPReqRepHelloWorldReplier::init(
        int participantId,
        int domainId,
        uint16_t listeningPort,
        uint32_t maxInitialPeer,
        const char* certs_folder)
{
    ASSERT_NE(initialized_, true);

    DomainParticipantQos participant_qos;
    participant_qos.wire_protocol().participant_id = participantId;
    participant_qos.wire_protocol().builtin.discovery_config.leaseDuration_announcementperiod = Duration_t(1, 0);
    participant_qos.wire_protocol().builtin.discovery_config.leaseDuration = c_TimeInfinite;

    participant_qos.transport().use_builtin_transports = false;

    std::shared_ptr<TCPTransportDescriptor> descriptor;
    if (use_ipv6)
    {
        descriptor = std::make_shared<TCPv6TransportDescriptor>();
    }
    else
    {
        descriptor = std::make_shared<TCPv4TransportDescriptor>();
    }

    descriptor->sendBufferSize = 0;
    descriptor->receiveBufferSize = 0;
    if (maxInitialPeer > 0)
    {
        descriptor->maxInitialPeersRange = maxInitialPeer;
    }
    descriptor->add_listener_port(listeningPort);

    if (certs_folder != nullptr)
    {
        using TLSOptions = TCPTransportDescriptor::TLSConfig::TLSOptions;
        using TLSVerifyMode = TCPTransportDescriptor::TLSConfig::TLSVerifyMode;
        descriptor->apply_security = true;
        descriptor->tls_config.password = "testkey";
        descriptor->tls_config.cert_chain_file = std::string(certs_folder) + "/mainsubcert.pem";
        descriptor->tls_config.private_key_file = std::string(certs_folder) + "/mainsubkey.pem";
        descriptor->tls_config.verify_mode = TLSVerifyMode::VERIFY_PEER;
        descriptor->tls_config.add_option(TLSOptions::DEFAULT_WORKAROUNDS);
        descriptor->tls_config.add_option(TLSOptions::SINGLE_DH_USE);
        descriptor->tls_config.add_option(TLSOptions::NO_COMPRESSION);
        descriptor->tls_config.add_option(TLSOptions::NO_SSLV2);
    }

    participant_qos.transport().user_transports.push_back(descriptor);

    participant_ = DomainParticipantFactory::get_instance()->create_participant(
        domainId, participant_qos);
    ASSERT_NE(participant_, nullptr);
    ASSERT_TRUE(participant_->is_enabled());

    // Register service type and create service
    TCPReqRepHelloWorldService service;
    service_ = service.init(participant_);
    ASSERT_NE(service_, nullptr);

    // Create replier
    replier_ = participant_->create_service_replier(service_, create_replier_qos());
    ASSERT_NE(replier_, nullptr);
    ASSERT_EQ(replier_->is_enabled(), true);

    init_processing_thread();

    initialized_ = true;
}

void TCPReqRepHelloWorldReplier::newNumber(
        const RequestInfo& info,
        uint16_t number)
{
    HelloWorld hello;
    hello.index(number);
    hello.message("GoodBye");
    ASSERT_EQ(replier_->send_reply((void*)&hello, info), RETCODE_OK);
}

void TCPReqRepHelloWorldReplier::wait_discovery(
        std::chrono::seconds timeout)
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);

    std::cout << "Replier waiting for discovery..." << std::endl;

    if (timeout == std::chrono::seconds::zero())
    {
        cvDiscovery_.wait(lock, [&]()
                {
                    return matched_ > 1;
                });
    }
    else
    {
        cvDiscovery_.wait_for(lock, timeout, [&]()
                {
                    return matched_ > 1;
                });
    }

    std::cout << "Replier discovery phase finished" << std::endl;
}

void TCPReqRepHelloWorldReplier::wait_unmatched(
        std::chrono::seconds timeout)
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);

    std::cout << "Replier waiting until being unmatched..." << std::endl;

    if (timeout == std::chrono::seconds::zero())
    {
        cvDiscovery_.wait(lock, [&]()
                {
                    return !is_matched();
                });
    }
    else
    {
        cvDiscovery_.wait_for(lock, timeout, [&]()
                {
                    return !is_matched();
                });
    }

    std::cout << "Replier unmatched" << std::endl;
}

void TCPReqRepHelloWorldReplier::matched()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);
    ++matched_;
    if (matched_ > 1)
    {
        cvDiscovery_.notify_one();
    }
}

void TCPReqRepHelloWorldReplier::unmatched()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);
    --matched_;
    if (!is_matched())
    {
        cvDiscovery_.notify_one();
    }
}

bool TCPReqRepHelloWorldReplier::is_matched()
{
    return matched_ > 1;
}

void TCPReqRepHelloWorldReplier::init_processing_thread()
{
    wait_set_.attach_condition(stop_processing_thread_);
    wait_set_.attach_condition(replier_->get_replier_writer()->get_statuscondition());
    wait_set_.attach_condition(replier_->get_replier_reader()->get_statuscondition());

    processing_thread_ = std::thread(&TCPReqRepHelloWorldReplier::process_status_changes, this);
}

void TCPReqRepHelloWorldReplier::process_status_changes()
{
    while (!stop_processing_thread_.get_trigger_value())
    {
        ReturnCode_t retcode;
        ConditionSeq triggered_conditions;

        retcode = wait_set_.wait(triggered_conditions, c_TimeInfinite);

        if (RETCODE_OK != retcode)
        {
            std::cout << "TCPReplier: Error processing status changes" << std::endl;
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
                    std::cout << "TCPReplier: Processing publication matched status" << std::endl;

                    DataWriter* writer = dynamic_cast<DataWriter*>(entity);
                    ASSERT_NE(writer, nullptr);
                    ASSERT_EQ(writer, replier_->get_replier_writer());

                    PublicationMatchedStatus status;
                    if (RETCODE_OK != writer->get_publication_matched_status(status))
                    {
                        std::cout << "TCPReplier: Error processing publication matched status" << std::endl;
                        continue;
                    }

                    if (status.current_count_change > 0)
                    {
                        matched();
                    }
                    else if (status.current_count_change < 0)
                    {
                        unmatched();
                    }
                }
                else if (status_changes.is_active(StatusMask::subscription_matched()))
                {
                    std::cout << "TCPReplier: Processing subscription matched status" << std::endl;

                    DataReader* reader = dynamic_cast<DataReader*>(entity);
                    ASSERT_NE(reader, nullptr);
                    ASSERT_EQ(reader, replier_->get_replier_reader());

                    SubscriptionMatchedStatus status;
                    if (RETCODE_OK != reader->get_subscription_matched_status(status))
                    {
                        std::cout << "TCPReplier: Error processing subscription matched status" << std::endl;
                        continue;
                    }

                    if (status.current_count_change > 0)
                    {
                        matched();
                    }
                    else if (status.current_count_change < 0)
                    {
                        unmatched();
                    }
                }
                else if (status_changes.is_active(StatusMask::data_available()))
                {
                    std::cout << "TCPReplier: Processing data available status" << std::endl;

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

ReplierQos TCPReqRepHelloWorldReplier::create_replier_qos()
{
    ReplierQos replier_qos;
    TCPReqRepHelloWorldService service;

    DataWriterQos& writer_qos = replier_qos.writer_qos;
    DataReaderQos& reader_qos = replier_qos.reader_qos;

    reader_qos.endpoint().history_memory_policy = PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    writer_qos.endpoint().history_memory_policy = PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

    //Increase default max_blocking_time to 1s in case the CPU is overhead
    reader_qos.reliability().max_blocking_time = Duration_t(1, 0);
    writer_qos.reliability().max_blocking_time = Duration_t(1, 0);

    return replier_qos;
}