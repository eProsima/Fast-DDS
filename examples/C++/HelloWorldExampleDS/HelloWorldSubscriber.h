// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/subscriber/SampleInfo.h>




#include "HelloWorld.h"

class HelloWorldSubscriber
{
public:

    HelloWorldSubscriber();
    virtual ~HelloWorldSubscriber();
    //!Initialize the subscriber
    bool init(
            eprosima::fastrtps::rtps::Locator_t server_address,
            std::string topic_name);
    //!RUN the subscriber
    void run();
    //!Run the subscriber until number samples have been recevied.
    void run(
            uint32_t number);

private:

    eprosima::fastrtps::Participant* mp_participant;
    eprosima::fastrtps::Subscriber* mp_subscriber;

public:

    class SubListener : public eprosima::fastrtps::SubscriberListener
    {
    public:

        SubListener()
            : n_matched(0)
            , n_samples(0)
        {
        }

        ~SubListener()
        {
        }

        void onSubscriptionMatched(
                eprosima::fastrtps::Subscriber* sub,
                eprosima::fastrtps::rtps::MatchingInfo& info);
        void onNewDataMessage(
                eprosima::fastrtps::Subscriber* sub);
        HelloWorld m_hello;
        eprosima::fastrtps::SampleInfo_t m_info;
        int n_matched;
        uint32_t n_samples;
    }
    m_listener;

private:

    HelloWorldPubSubType m_type;
};

#endif /* HELLOWORLDSUBSCRIBER_H_ */
