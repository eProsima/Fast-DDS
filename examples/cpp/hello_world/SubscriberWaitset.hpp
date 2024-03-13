// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file SubscriberWaitset.hpp
 *
 */

#ifndef _FASTDDS_HELLO_WORLD_SUBSCRIBER_WAITSET_HPP_
#define _FASTDDS_HELLO_WORLD_SUBSCRIBER_WAITSET_HPP_

#include <fastdds/dds/core/condition/GuardCondition.hpp>
#include <fastdds/dds/core/condition/WaitSet.hpp>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "cli_options.hpp"
#include "HelloWorldPubSubTypes.h"

using namespace eprosima::fastdds::dds;

class HelloWorldSubscriberWaitset
{
public:

    HelloWorldSubscriberWaitset(
            const CLIParser::hello_world_config& config);

    virtual ~HelloWorldSubscriberWaitset();

    //! Run subscriber
    void run();

    //! Return the current state of execution
    static bool is_stopped();

    //! Trigger the end of execution
    static void stop();

private:

    HelloWorld hello_;

    DomainParticipant* participant_;

    Subscriber* subscriber_;

    Topic* topic_;

    DataReader* reader_;

    TypeSupport type_;

    WaitSet wait_set_;

    uint16_t samples_;

    uint16_t received_samples_;

    static std::atomic<bool> stop_;

    static GuardCondition terminate_condition_;
};

#endif /* _FASTDDS_HELLO_WORLD_SUBSCRIBER_WAITSET_HPP_ */
