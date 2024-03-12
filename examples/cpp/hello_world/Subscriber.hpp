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
 * @file Subscriber.hpp
 *
 */

#ifndef _FASTDDS_HELLO_WORLD_SUBSCRIBER_HPP_
#define _FASTDDS_HELLO_WORLD_SUBSCRIBER_HPP_

#include <condition_variable>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "HelloWorldPubSubTypes.h"

using namespace eprosima::fastdds::dds;

class HelloWorldSubscriber : public DataReaderListener
{
public:

    HelloWorldSubscriber();

    virtual ~HelloWorldSubscriber();

    //! Subscription callback
    void on_data_available(
            DataReader* reader) override;

    //! Subscriber matched method
    void on_subscription_matched(
            DataReader* reader,
            const SubscriptionMatchedStatus& info) override;

    //! Run subscriber
    void run();

    //! Trigger the end of execution
    static void stop();

private:

    //! Return the current state of execution
    static bool is_stopped();

    HelloWorld hello_;

    DomainParticipant* participant_;

    Subscriber* subscriber_;

    Topic* topic_;

    DataReader* reader_;

    TypeSupport type_;

    static std::atomic<bool> stop_;

    mutable std::mutex terminate_cv_mtx_;

    static std::condition_variable terminate_cv_;
};

#endif /* _FASTDDS_HELLO_WORLD_SUBSCRIBER_HPP_ */
