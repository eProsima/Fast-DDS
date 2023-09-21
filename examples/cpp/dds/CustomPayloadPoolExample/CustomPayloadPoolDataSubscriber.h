// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file CustomPayloadPoolDataSubscriber.h
 *
 */

#ifndef CUSTOM_PAYLOAD_POOL_DATA_SUBSCRIBER_H_
#define CUSTOM_PAYLOAD_POOL_DATA_SUBSCRIBER_H_

#include <condition_variable>
#include <mutex>

#include "CustomPayloadPoolDataPubSubTypes.h"
#include "CustomPayloadPool.hpp"

#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastrtps/subscriber/SampleInfo.h>

class CustomPayloadPoolDataSubscriber : private eprosima::fastdds::dds::DataReaderListener
{
public:

    CustomPayloadPoolDataSubscriber(
            std::shared_ptr<CustomPayloadPool> payload_pool);

    virtual ~CustomPayloadPoolDataSubscriber();

    //!Initialize the subscriber
    bool init();

    //!Run the subscriber until all samples have been received.
    bool run(
            uint32_t samples);

private:

    void on_data_available(
            eprosima::fastdds::dds::DataReader* reader) override;

    void on_subscription_matched(
            eprosima::fastdds::dds::DataReader* reader,
            const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

    //! Return the current state of execution
    static bool is_stopped();

    //! Trigger the end of execution
    static void stop();

    CustomPayloadPoolData hello_;

    std::shared_ptr<CustomPayloadPool> payload_pool_;

    eprosima::fastdds::dds::DomainParticipant* participant_;

    eprosima::fastdds::dds::Subscriber* subscriber_;

    eprosima::fastdds::dds::Topic* topic_;

    eprosima::fastdds::dds::DataReader* reader_;

    eprosima::fastdds::dds::TypeSupport type_;

    int32_t matched_;

    uint32_t samples_;

    uint32_t max_samples_;

    //! Member used for control flow purposes
    static std::atomic<bool> stop_;

    //! Protects terminate condition variable
    static std::mutex terminate_cv_mtx_;

    //! Waits during execution until SIGINT or max_messages_ samples are received
    static std::condition_variable terminate_cv_;
};

#endif /* CUSTOM_PAYLOAD_POOL_DATA_SUBSCRIBER_H_ */
