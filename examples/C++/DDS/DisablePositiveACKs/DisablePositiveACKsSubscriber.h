// Copyright 20196 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DisablePositiveACKsSubscriber.h
 *
 */

#ifndef DisablePositiveACKsSubscriber_H_
#define DisablePositiveACKsSubscriber_H_

#include "TopicTypes.h"
#include "Topic.h"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/topic/Topic.hpp>

class DisablePositiveACKsSubscriber
{
public:

    /**
     * @brief DisablePositiveACKsSubscriber
     */
    DisablePositiveACKsSubscriber();

    /**
     * @brief ~DisablePositiveACKsSubscriber
     */
    virtual ~DisablePositiveACKsSubscriber();

    /**
     * @brief Initialises the subscriber
     * @param disable_positive_acks True to disable positive acks
     * @return True if initialisation was successful
     */
    bool init(
            bool disable_positive_acks);

    /**
     * @brief Runs the subscriber
     * @param number The number of samples the subscriber expects to receive
     */
    void run(
            uint32_t number);

private:

    eprosima::fastdds::dds::DomainParticipant* participant_;

    eprosima::fastdds::dds::Subscriber* subscriber_;

    eprosima::fastdds::dds::DataReader* reader_;

    eprosima::fastdds::dds::Topic* topic_;

    eprosima::fastdds::dds::TypeSupport type_;

public:

    class SubListener : public eprosima::fastdds::dds::DataReaderListener
    {
public:

        SubListener()
            : n_matched(0)
            , n_samples(0)
        {
        }

        ~SubListener() override
        {
        }

        void on_data_available(
                eprosima::fastdds::dds::DataReader* reader) override;

        void on_subscription_matched(
                eprosima::fastdds::dds::DataReader* reader,
                const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

        Topic hello;

        int n_matched;

        uint32_t n_samples;

    } listener;
};

#endif /* DisablePositiveACKsSubscriber_H_ */
