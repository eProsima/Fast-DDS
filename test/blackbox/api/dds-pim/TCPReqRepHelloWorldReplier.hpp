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
 * @file TCPReqRepHelloWorldReplier.hpp
 *
 */

#ifndef _TEST_BLACKBOX_TCPREQREPHELLOWORLDREPLIER_HPP_
#define _TEST_BLACKBOX_TCPREQREPHELLOWORLDREPLIER_HPP_

#include <asio.hpp>
#include <condition_variable>
#include <list>
#include <thread>

#include <fastdds/dds/core/condition/GuardCondition.hpp>
#include <fastdds/dds/core/condition/WaitSet.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/qos/ReplierQos.hpp>
#include <fastdds/dds/rpc/Replier.hpp>
#include <fastdds/dds/rpc/Service.hpp>

#include "../../common/BlackboxTests.hpp"

#if defined(_WIN32)
#define GET_PID _getpid
#include <process.h>
#else
#define GET_PID getpid
#endif // if defined(_WIN32)

class TCPReqRepHelloWorldReplier
{
public:

    TCPReqRepHelloWorldReplier();

    virtual ~TCPReqRepHelloWorldReplier();

    void init(
            int participantId,
            int domainId,
            uint16_t listeningPort,
            uint32_t maxInitialPeer = 0,
            const char* certs_folder = nullptr);

    bool isInitialized() const
    {
        return initialized_;
    }

    void newNumber(
            const eprosima::fastdds::dds::rpc::RequestInfo& info,
            uint16_t number);

    void wait_discovery(
            std::chrono::seconds timeout = std::chrono::seconds::zero());

    void wait_unmatched(
            std::chrono::seconds timeout = std::chrono::seconds::zero());

    void matched();

    void unmatched();

    bool is_matched();

    eprosima::fastdds::dds::ReplierQos create_replier_qos();

private:

    TCPReqRepHelloWorldReplier& operator =(
            const TCPReqRepHelloWorldReplier&) = delete;

    void init_processing_thread();

    void process_status_changes();

    eprosima::fastdds::dds::rpc::Replier* replier_;
    eprosima::fastdds::dds::rpc::Service* service_;
    eprosima::fastdds::dds::DomainParticipant* participant_;
    eprosima::fastdds::dds::WaitSet wait_set_;

    bool initialized_;

    std::mutex mutexDiscovery_;
    std::condition_variable cvDiscovery_;
    std::atomic<unsigned int> matched_;

    // Entity status changes are managed using the WaitSet on a different thread
    // The main thread remains blocked until the requester matches with the replier
    std::thread processing_thread_;
    eprosima::fastdds::dds::GuardCondition stop_processing_thread_;
};

#endif // _TEST_BLACKBOX_TCPREQREPHELLOWORLDREPLIER_HPP_
