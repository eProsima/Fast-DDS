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

#include <fastdds/domain/DomainParticipant.hpp>
#include <fastdds/topic/DataReader.hpp>
#include <fastdds/topic/DataReaderListener.hpp>
#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/subscriber/SampleInfo.h>

class HelloWorldSubscriber {
public:
    HelloWorldSubscriber();
    virtual ~HelloWorldSubscriber();
    //!Initialize the subscriber
    bool init();
    //!RUN the subscriber
    void run();
    //!Run the subscriber until number samples have been recevied.
    void run(uint32_t number);
private:
    eprosima::fastdds::DomainParticipant* mp_participant;
    eprosima::fastdds::Subscriber* mp_subscriber;
    eprosima::fastdds::DataReader* reader_;
public:
    class SubListener:public eprosima::fastdds::DataReaderListener
    {
    public:
        SubListener():n_matched(0),n_samples(0){}
        ~SubListener() override {}
        void on_data_available(eprosima::fastdds::DataReader* reader) override;
        void on_subscription_matched(eprosima::fastdds::DataReader* reaer,
                                     eprosima::fastrtps::SubscriptionMatchedStatus& info) override;
        HelloWorld m_Hello;
        eprosima::fastrtps::SampleInfo_t m_info;
        int n_matched;
        uint32_t n_samples;
    }m_listener;
private:
    HelloWorldPubSubType m_type;
};

#endif /* HELLOWORLDSUBSCRIBER_H_ */
