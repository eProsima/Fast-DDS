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

#include "CustomPayloadPoolDataPubSubTypes.h"
#include "CustomPayloadPool.hpp"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastrtps/subscriber/SampleInfo.h>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>

class CustomPayloadPoolDataSubscriber : public eprosima::fastdds::dds::DataReaderListener
{
public:

    CustomPayloadPoolDataSubscriber(
            std::shared_ptr<CustomPayloadPool> payload_pool);

    virtual ~CustomPayloadPoolDataSubscriber();

    //!Initialize the subscriber
    bool init();

    //!RUN the subscriber
    void run();

    //!Run the subscriber until number samples have been received.
    void run(
            uint32_t number);

private:

    eprosima::fastdds::dds::DomainParticipant* participant_;

    eprosima::fastdds::dds::Subscriber* subscriber_;

    eprosima::fastdds::dds::Topic* topic_;

    eprosima::fastdds::dds::DataReader* reader_;

    eprosima::fastdds::dds::TypeSupport type_;

    std::shared_ptr<CustomPayloadPool> payload_pool_;

    void on_data_available(
            eprosima::fastdds::dds::DataReader* reader) override;

    void on_subscription_matched(
            eprosima::fastdds::dds::DataReader* reader,
            const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

    CustomPayloadPoolData hello_;

    int matched_;

    uint32_t samples_;
};

#endif /* CUSTOM_PAYLOAD_POOL_DATA_SUBSCRIBER_H_ */
