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
 * @file DiscoveryServerSubscriber.h
 *
 */

#ifndef _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_DISCOVERYSERVEREXAMPLE_DISCOVERYSERVERSUBSCRIBER_H_
#define _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_DISCOVERYSERVEREXAMPLE_DISCOVERYSERVERSUBSCRIBER_H_

#include <atomic>
#include <condition_variable>
#include <mutex>

#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>

#include "types/HelloWorldPubSubTypes.h"
#include "common.h"

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
            const std::string& topic_name,
            uint32_t max_messages,
            const std::string& server_address,
            unsigned short server_port,
            unsigned short server_id,
            TransportKind transport);

    //! RUN the subscriber until number samples are received
    void run(
            uint32_t number);

    //! Return the current state of execution
    static bool is_stopped();

    //! Trigger the end of execution
    static void stop();

private:

    eprosima::fastdds::dds::DomainParticipant* participant_;

    eprosima::fastdds::dds::Subscriber* subscriber_;

    eprosima::fastdds::dds::Topic* topic_;

    eprosima::fastdds::dds::DataReader* reader_;

    eprosima::fastdds::dds::TypeSupport type_;

    /**
     * Class handling discovery and dataflow events
     */
    class SubListener : public eprosima::fastdds::dds::DomainParticipantListener
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

        //! Callback executed when a DomainParticipant is discovered, dropped or removed
        void on_participant_discovery(
                eprosima::fastdds::dds::DomainParticipant* /*participant*/,
                eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info) override;

    private:

        using eprosima::fastdds::dds::DomainParticipantListener::on_participant_discovery;

        HelloWorld hello_;

        //! Number of DataWriters matched to the associated DataReader
        int matched_;

        //! Number of samples received
        uint32_t samples_;

        //! Number of messages to be received before triggering termination of execution
        uint32_t max_messages_;
    }
    listener_;

    //! Member used for control flow purposes
    static std::atomic<bool> stop_;

    //! Protects terminate condition variable
    static std::mutex terminate_cv_mtx_;

    //! Waits during execution until SIGINT or max_messages_ samples are received
    static std::condition_variable terminate_cv_;
};

#endif /* _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_DISCOVERYSERVEREXAMPLE_DISCOVERYSERVERSUBSCRIBER_H_ */
