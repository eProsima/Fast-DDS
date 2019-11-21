// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file HelloWorldSubscriber.h
 *
 */

#ifndef HELLOWORLDSUBSCRIBER_H_
#define HELLOWORLDSUBSCRIBER_H_

#include "HelloWorldPubSubTypes.h"

#include <dds/domain/DomainParticipant.hpp>
#include <dds/sub/Subscriber.hpp>
#include <dds/sub/DataReader.hpp>
#include <dds/sub/DataReaderListener.hpp>
#include <fastdds/dds/topic/DataReaderListener.hpp>

class HelloWorldSubscriber
{
public:
    HelloWorldSubscriber();

    virtual ~HelloWorldSubscriber();

    //!Initialize the subscriber
    bool init();

    //!RUN the subscriber
    void run();

    //!Run the subscriber until number samples have been recevied.
    void run(
            uint32_t number);

private:
    dds::domain::DomainParticipant participant_;

    dds::sub::Subscriber subscriber_;

    dds::sub::DataReader<HelloWorld> reader_;

    HelloWorldTypeSupport type_;

    dds::topic::Topic<HelloWorld> topic_;

    class SubListener : public dds::sub::DataReaderListener<HelloWorld>
    {
    public:
        SubListener()
            : matched_(0)
            , samples_(0)
        {}

        ~SubListener() override
        {}

        void on_data_available(
                dds::sub::DataReader<HelloWorld>& reader) override;

        void on_subscription_matched(
                eprosima::fastdds::dds::DataReader* reader,
                const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

        void on_subscription_matched(
                dds::sub::DataReader<HelloWorld>& writer,
                const dds::core::status::SubscriptionMatchedStatus& status) override;

        HelloWorld hello_;

        int matched_;

        uint32_t samples_;
    }listener_;
};

#endif /* HELLOWORLDSUBSCRIBER_H_ */
