// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file BasicConfigurationSubscriber.h
 *
 */

#ifndef _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_BASICCONFIGURATIONEXAMPLE_BASICCONFIGURATIONSUBSCRIBER_H_
#define _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_BASICCONFIGURATIONEXAMPLE_BASICCONFIGURATIONSUBSCRIBER_H_

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <vector>

#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>

#include "HelloWorldPubSubTypes.h"
#include "types.hpp"

/**
 * Class handling discovery and dataflow events
 */
class SubListener : public eprosima::fastdds::dds::DataReaderListener
{
public:

    SubListener()
        : matched_(0)
        , samples_(0)
        , max_messages_(0)
    {
    }

    ~SubListener() override
    {
    }

    //! Set the maximum number of messages to receive before exiting
    void set_max_messages(
            uint32_t max_messages);

    //! Callback executed when a new sample is received
    void on_data_available(
            eprosima::fastdds::dds::DataReader* reader) override;

    //! Callback executed when a DataWriter is matched or unmatched
    void on_subscription_matched(
            eprosima::fastdds::dds::DataReader* reader,
            const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

    std::string topic_name_;

private:

    HelloWorld hello_;

    //! Number of DataWriters matched to the associated DataReader
    int matched_;

    //! Number of samples received
    uint32_t samples_;

    //! Number of messages to be received before triggering termination of execution
    uint32_t max_messages_;
};

/**
 * Class used to group into a single working unit a Subscriber with a DataReader, its listener, and a TypeSupport member
 * corresponding to the HelloWorld datatype
 */
class HelloWorldSubscriber
{
public:

    HelloWorldSubscriber();

    virtual ~HelloWorldSubscriber();

    //! Initialize the subscriber
    bool init(
            std::vector<std::string> topic_names,
            uint32_t max_messages,
            uint32_t domain,
            TransportType transport,
            bool reliable,
            bool transient);

    //! RUN the subscriber until number samples are received
    void run(
            uint32_t number);

    //! Return the current state of execution
    static bool is_stopped();

    //! Trigger the end of execution
    static void stop(
            bool force = false);

private:

    eprosima::fastdds::dds::DomainParticipant* participant_;

    eprosima::fastdds::dds::Subscriber* subscriber_;

    std::vector<eprosima::fastdds::dds::Topic*> topics_;

    std::vector<eprosima::fastdds::dds::DataReader*> readers_;

    static std::atomic<uint32_t> n_topics_;

    eprosima::fastdds::dds::TypeSupport type_;

    std::vector<std::shared_ptr<SubListener>> listeners_;

    //! Member used for control flow purposes
    static std::atomic<bool> stop_;

    static std::atomic<uint32_t> n_stopped_;

    //! Protects terminate condition variable
    static std::mutex terminate_cv_mtx_;

    //! Waits during execution until SIGINT or max_messages_ samples are received
    static std::condition_variable terminate_cv_;
};

#endif /* _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_BASICCONFIGURATIONEXAMPLE_BASICCONFIGURATIONSUBSCRIBER_H_ */
