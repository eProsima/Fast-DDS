// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file BasicSubscriber.h
 *
 */

#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>

#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>

#include "types/HelloWorldPubSubTypes.h"

/**
 * Class used to group into a single working unit a Subscriber with a DataReader, its listener, and a TypeSupport member
 * corresponding to the HelloWorld datatype
 */
class BasicSubscriber : public eprosima::fastdds::dds::DataReaderListener
{
public:

    BasicSubscriber(
            const std::string& topic_name,
            uint32_t max_messages,
            uint32_t domain);

    virtual ~BasicSubscriber();

    //! RUN the subscriber until number samples are received
    void run(
            uint32_t number);

    ///////////////////
    // Signal related methods

    //! Return the current state of execution
    static bool is_stopped();

    //! Trigger the end of execution
    static void stop();

    ///////////////////
    // Listener methods

    //! Callback executed when a new sample is received
    void on_data_available(
            eprosima::fastdds::dds::DataReader* reader) override;

    //! Callback executed when a DataWriter is matched or unmatched
    void on_subscription_matched(
            eprosima::fastdds::dds::DataReader* reader,
            const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

private:

    ///////////////////
    // Fast DDS entities

    eprosima::fastdds::dds::DomainParticipant* participant_;

    eprosima::fastdds::dds::Subscriber* subscriber_;

    eprosima::fastdds::dds::Topic* topic_;

    eprosima::fastdds::dds::DataReader* reader_;

    eprosima::fastdds::dds::TypeSupport type_;

    ///////////////////
    // Data receive related variables

    //! Data type to read arrived data
    HelloWorld hello_;

    //! Number of samples received
    uint32_t samples_;

    //! Number of messages to be received before triggering termination of execution
    uint32_t max_messages_;

    ///////////////////
    // Async read

    void read_();

    ///////////////////
    // Stop related variables

    //! Member used for control flow purposes
    static std::atomic<bool> stop_;

    //! Protects terminate condition variable
    static std::mutex terminate_cv_mtx_;

    //! Waits during execution until SIGINT or max_messages_ samples are received
    static std::condition_variable terminate_cv_;
};
