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
 * @file TCPReqRepHelloWorldRequester.cpp
 *
 */

#include <gtest/gtest.h>

#include <fastdds/dds/core/condition/Condition.hpp>
#include <fastdds/dds/core/condition/StatusCondition.hpp>
#include <fastdds/dds/core/status/PublicationMatchedStatus.hpp>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/RequesterQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/rpc/RequestInfo.hpp>
#include <fastdds/dds/rpc/ServiceTypeSupport.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.hpp>
#include <fastdds/utils/IPFinder.hpp>
#include <fastdds/utils/IPLocator.hpp>

#include "TCPReqRepHelloWorldRequester.hpp"
#include "TCPReqRepHelloWorldService.hpp"
#include "../../types/HelloWorld.hpp"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::dds::rpc;
using namespace eprosima::fastdds::rtps;

TCPReqRepHelloWorldRequester::TCPReqRepHelloWorldRequester()
    : current_number_(std::numeric_limits<uint16_t>::max())
    , number_received_(std::numeric_limits<uint16_t>::max())
    , participant_(nullptr)
    , service_(nullptr)
    , requester_(nullptr)
    , initialized_(false)
    , matched_(0)
{
}

TCPReqRepHelloWorldRequester::~TCPReqRepHelloWorldRequester()
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

void TCPReqRepHelloWorldRequester::init(
        int participantId,
        int domainId,
        uint16_t listeningPort,
        uint32_t maxInitialPeer,
        const char* certs_folder,
        bool force_localhost)
{
    ASSERT_NE(initialized_, true);

    DomainParticipantQos participant_qos;

    int32_t kind;
    LocatorList_t loc;

    if (use_ipv6)
    {
        kind = LOCATOR_KIND_TCPv6;
        IPFinder::getIP6Address(&loc);

    }
    else
    {
        kind = LOCATOR_KIND_TCPv4;
        IPFinder::getIP4Address(&loc);

    }


    Locator_t initial_peer_locator;
    initial_peer_locator.kind = kind;
    if (!force_localhost && loc.size() > 0)
    {
        if (use_ipv6)
        {
            IPLocator::setIPv6(initial_peer_locator, *(loc.begin()));
        }
        else
        {
            IPLocator::setIPv4(initial_peer_locator, *(loc.begin()));
        }
    }
    else
    {
        if (use_ipv6)
        {
            IPLocator::setIPv6(initial_peer_locator, "::1");
        }
        else
        {
            IPLocator::setIPv4(initial_peer_locator, "127.0.0.1");
        }
    }

    initial_peer_locator.port = listeningPort;
    participant_qos.wire_protocol().builtin.initialPeersList.push_back(initial_peer_locator);
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

    if (maxInitialPeer > 0)
    {
        descriptor->maxInitialPeersRange = maxInitialPeer;
    }

    if (certs_folder != nullptr)
    {
        using TLSOptions = TCPTransportDescriptor::TLSConfig::TLSOptions;
        using TLSVerifyMode = TCPTransportDescriptor::TLSConfig::TLSVerifyMode;
        descriptor->apply_security = true;
        descriptor->tls_config.verify_file = std::string(certs_folder) + "/maincacert.pem";
        descriptor->tls_config.verify_mode = TLSVerifyMode::VERIFY_PEER;
        descriptor->tls_config.add_option(TLSOptions::DEFAULT_WORKAROUNDS);
        descriptor->tls_config.add_option(TLSOptions::SINGLE_DH_USE);
        descriptor->tls_config.add_option(TLSOptions::NO_COMPRESSION);
        descriptor->tls_config.add_option(TLSOptions::NO_SSLV2);
    }

    participant_qos.transport().user_transports.push_back(descriptor);

    participant_qos.wire_protocol().participant_id = participantId;
    participant_qos.wire_protocol().builtin.discovery_config.leaseDuration = c_TimeInfinite;
    participant_qos.wire_protocol().builtin.discovery_config.leaseDuration_announcementperiod = Duration_t(1, 0);

    participant_ = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
        domainId, participant_qos);
    ASSERT_NE(participant_, nullptr);
    ASSERT_TRUE(participant_->is_enabled());

    // Register service type and create service
    TCPReqRepHelloWorldService service;
    service_ = service.init(participant_);
    ASSERT_NE(service_, nullptr);

    // Create requester
    requester_ = participant_->create_service_requester(service_, create_requester_qos());
    ASSERT_NE(requester_, nullptr);
    ASSERT_EQ(requester_->is_enabled(), true);

    init_processing_thread();

    initialized_ = true;
}

void TCPReqRepHelloWorldRequester::newNumber(
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

void TCPReqRepHelloWorldRequester::block()
{
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this]() -> bool
            {
                return current_number_ == number_received_;
            });

    ASSERT_EQ(related_sample_identity_, received_sample_identity_);
}

void TCPReqRepHelloWorldRequester::wait_discovery(
        std::chrono::seconds timeout)
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);

    std::cout << "Requester waiting for discovery..." << std::endl;

    if (timeout == std::chrono::seconds::zero())
    {
        cvDiscovery_.wait(lock, [&]()
                {
                    return is_matched();
                });
    }
    else
    {
        cvDiscovery_.wait_for(lock, timeout, [&]()
                {
                    return is_matched();
                });
    }

    std::cout << "Requester discovery phase finished" << std::endl;
}

void TCPReqRepHelloWorldRequester::matched()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);
    ++matched_;
    if (is_matched())
    {
        cvDiscovery_.notify_one();
    }
}

void TCPReqRepHelloWorldRequester::unmatched()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);
    --matched_;
}

bool TCPReqRepHelloWorldRequester::is_matched()
{
    return matched_ > 1;
}

void TCPReqRepHelloWorldRequester::send(
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

void TCPReqRepHelloWorldRequester::init_processing_thread()
{
    ASSERT_NE(requester_, nullptr);
    wait_set_.attach_condition(stop_processing_thread_);
    wait_set_.attach_condition(requester_->get_requester_writer()->get_statuscondition());
    wait_set_.attach_condition(requester_->get_requester_reader()->get_statuscondition());

    processing_thread_ = std::thread(&TCPReqRepHelloWorldRequester::process_status_changes, this);
}

void TCPReqRepHelloWorldRequester::process_status_changes()
{
    while (!stop_processing_thread_.get_trigger_value())
    {
        ReturnCode_t retcode;
        ConditionSeq triggered_conditions;

        retcode = wait_set_.wait(triggered_conditions, c_TimeInfinite);

        if (RETCODE_OK != retcode)
        {
            std::cout << "TCPRequester: Error processing status changes" << std::endl;
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
                    std::cout << "TCPRequester: Processing publication matched status" << std::endl;

                    DataWriter* writer = dynamic_cast<DataWriter*>(entity);
                    ASSERT_NE(writer, nullptr);
                    ASSERT_EQ(writer, requester_->get_requester_writer());

                    PublicationMatchedStatus status;
                    if (RETCODE_OK != writer->get_publication_matched_status(status))
                    {
                        std::cout << "TCPRequester: Error processing publication matched status" << std::endl;
                        continue;
                    }

                    if (status.current_count_change > 0)
                    {
                        matched();
                    }
                }
                else if (status_changes.is_active(StatusMask::subscription_matched()))
                {
                    std::cout << "TCPRequester: Processing subscription matched status" << std::endl;

                    DataReader* reader = dynamic_cast<DataReader*>(entity);
                    ASSERT_NE(reader, nullptr);
                    ASSERT_EQ(reader, requester_->get_requester_reader());

                    SubscriptionMatchedStatus status;
                    if (RETCODE_OK != reader->get_subscription_matched_status(status))
                    {
                        std::cout << "TCPRequester: Error processing subscription matched status" << std::endl;
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
                    std::cout << "TCPRequester: Processing data available status" << std::endl;

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

RequesterQos TCPReqRepHelloWorldRequester::create_requester_qos()
{
    RequesterQos requester_qos;
    TCPReqRepHelloWorldService service;

    DataWriterQos& writer_qos = requester_qos.writer_qos;
    DataReaderQos& reader_qos = requester_qos.reader_qos;

    reader_qos.endpoint().history_memory_policy = PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    writer_qos.endpoint().history_memory_policy = PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    //Increase default max_blocking_time to 1s in case the CPU is overhead
    reader_qos.reliability().max_blocking_time = Duration_t(1, 0);
    writer_qos.reliability().max_blocking_time = Duration_t(1, 0);

    return requester_qos;
}
