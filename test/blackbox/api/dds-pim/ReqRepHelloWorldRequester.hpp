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
 * @file ReqRepHelloWorldRequester.hpp
 *
 */

#ifndef _TEST_BLACKBOX_REQREPHELLOWORLDREQUESTER_HPP_
#define _TEST_BLACKBOX_REQREPHELLOWORLDREQUESTER_HPP_

#include <asio.hpp>
#include <condition_variable>
#include <list>
#include <thread>

#include <fastdds/dds/core/condition/GuardCondition.hpp>
#include <fastdds/dds/core/condition/WaitSet.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/qos/RequesterQos.hpp>
#include <fastdds/dds/rpc/Requester.hpp>
#include <fastdds/dds/rpc/RequestInfo.hpp>
#include <fastdds/dds/rpc/Service.hpp>
#include <fastdds/rtps/common/SampleIdentity.hpp>

#include "../../common/BlackboxTests.hpp"

#if defined(_WIN32)
#include <process.h>
#define GET_PID _getpid
#else
#define GET_PID getpid
#endif // if defined(_WIN32)

class ReqRepHelloWorldRequester
{

public:

    ReqRepHelloWorldRequester();

    virtual ~ReqRepHelloWorldRequester();

    void init(
            bool use_volatile = false);

    void init_with_custom_qos(
            const eprosima::fastdds::dds::RequesterQos& requester_qos);

    void init_with_latency(
            const eprosima::fastdds::dds::Duration_t& latency_budget_duration_pub,
            const eprosima::fastdds::dds::Duration_t& latency_budget_duration_sub);

    bool isInitialized() const
    {
        return initialized_;
    }

    void newNumber(
            const eprosima::fastdds::dds::rpc::RequestInfo& info,
            uint16_t number);

    void block(
            const std::chrono::seconds& seconds);

    void wait_discovery();

    void matched();

    void send(
            const uint16_t number);

    void send(
            const uint16_t number,
            const eprosima::fastdds::rtps::SampleIdentity& related_sample_identity);

    const eprosima::fastdds::dds::Duration_t datawriter_latency_budget_duration() const;

    const eprosima::fastdds::dds::Duration_t datareader_latency_budget_duration() const;

    eprosima::fastdds::dds::RequesterQos create_requester_qos(
            bool volatile_durability_qos = false);

    const eprosima::fastdds::rtps::GUID_t& get_reader_guid() const;

    const eprosima::fastdds::rtps::SampleIdentity& get_last_related_sample_identity() const;

private:

    ReqRepHelloWorldRequester& operator =(
            const ReqRepHelloWorldRequester&) = delete;

    void init_processing_thread();

    void process_status_changes();

    uint16_t current_number_;
    uint16_t number_received_;

    eprosima::fastdds::dds::rpc::Requester* requester_;
    eprosima::fastdds::dds::rpc::Service* service_;
    eprosima::fastdds::dds::DomainParticipant* participant_;
    eprosima::fastdds::dds::WaitSet wait_set_;

    bool initialized_;
    unsigned int matched_;
    eprosima::fastdds::rtps::SampleIdentity related_sample_identity_;
    eprosima::fastdds::rtps::SampleIdentity received_sample_identity_;

    std::mutex mutex_;
    std::condition_variable cv_;
    std::mutex mutexDiscovery_;
    std::condition_variable cvDiscovery_;

    // Entity status changes are managed using the WaitSet on a different thread
    // The main thread remains blocked until the requester matches with the replier
    std::thread processing_thread_;
    eprosima::fastdds::dds::GuardCondition stop_processing_thread_;
};

#endif // _TEST_BLACKBOX_REQREPHELLOWORLDREQUESTER_HPP_
