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
 * @file ContentFilteredTopicExampleSubscriber.hpp
 *
 */

#ifndef _CONTENTFILTEREDTOPICEXAMPLESUBSCRIBER_HPP_
#define _CONTENTFILTEREDTOPICEXAMPLESUBSCRIBER_HPP_

#include "HelloWorldPubSubTypes.h"

#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/ContentFilteredTopic.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "MyCustomFilterFactory.hpp"

class ContentFilteredTopicExampleSubscriber : public eprosima::fastdds::dds::DataReaderListener
{
public:

    ContentFilteredTopicExampleSubscriber() = default;

    virtual ~ContentFilteredTopicExampleSubscriber();

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

    //! Run the subscriber
    void run();

private:

    eprosima::fastdds::dds::DomainParticipant* participant_ = nullptr;

    eprosima::fastdds::dds::Subscriber* subscriber_ = nullptr;

    eprosima::fastdds::dds::Topic* topic_ = nullptr;

    eprosima::fastdds::dds::ContentFilteredTopic* filter_topic_ = nullptr;

    eprosima::fastdds::dds::DataReader* reader_ = nullptr;

    eprosima::fastdds::dds::TypeSupport type_ = eprosima::fastdds::dds::TypeSupport(new HelloWorldPubSubType());

    MyCustomFilterFactory filter_factory;

    HelloWorld hello_;

    int matched_ = 0;

    uint32_t samples_  = 0;

    void on_data_available(
            eprosima::fastdds::dds::DataReader* reader) override;

    void on_subscription_matched(
            eprosima::fastdds::dds::DataReader* reader,
            const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

};

#endif // _CONTENTFILTEREDTOPICEXAMPLESUBSCRIBER_HPP_
