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
 * @file CustomFilterSubscriber.hpp
 *
 */

#ifndef _DEFAULTSQLFILTERSUBSCRIBER_HPP_
#define _DEFAULTSQLFILTERSUBSCRIBER_HPP_

#include "HelloWorldPubSubTypes.h"
#include "MyCustomFilterFactory.hpp"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastrtps/subscriber/SampleInfo.h>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>

class CustomFilterSubscriber
{
public:

    CustomFilterSubscriber() = default;

    virtual ~CustomFilterSubscriber();

    /**
     * @brief Initialize the subscriber
     * 
     * @param custom_filter Whether the default SQL filter or the custom defined filter is used.
     *                      By default the SQL filter is used.
     * @return true if correctly initialized.
     * @return false otherwise.
     */
    bool init(
            bool custom_filter = false);

    //!RUN the subscriber
    void run();

    //!Run the subscriber until number samples have been received.
    void run(
            uint32_t number);

private:

    eprosima::fastdds::dds::DomainParticipant* participant_ = nullptr;

    eprosima::fastdds::dds::Subscriber* subscriber_ = nullptr;

    eprosima::fastdds::dds::Topic* topic_ = nullptr;

    eprosima::fastdds::dds::ContentFilteredTopic* filter_topic_ = nullptr;

    eprosima::fastdds::dds::DataReader* reader_ = nullptr;

    eprosima::fastdds::dds::TypeSupport type_ = eprosima::fastdds::dds::TypeSupport(new HelloWorldPubSubType());

    MyCustomFilterFactory filter_factory;

    class SubListener : public eprosima::fastdds::dds::DataReaderListener
    {
    public:

        ~SubListener() override
        {
        }

        void on_data_available(
                eprosima::fastdds::dds::DataReader* reader) override;

        void on_subscription_matched(
                eprosima::fastdds::dds::DataReader* reader,
                const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

        HelloWorld hello_;

        int matched_ = 0;

        uint32_t samples_  = 0;
    }
    listener_;
};

#endif // _DEFAULTSQLFILTERSUBSCRIBER_HPP_
