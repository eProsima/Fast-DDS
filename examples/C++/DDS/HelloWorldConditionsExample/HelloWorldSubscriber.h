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

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/topic/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>

class HelloWorldSubscriber
{
public:

    HelloWorldSubscriber();

    virtual ~HelloWorldSubscriber();

    //!Initialize the subscriber
    bool init(
            int domain_id);

    //!RUN the subscriber
    void run();

    //!Run the subscriber until number samples have been recevied.
    void run(
            uint32_t number);

    HelloWorld hello_;

    eprosima::fastdds::dds::SampleInfo_t info_;

    int matched_ = 0;

    uint32_t samples_ = 0;

    bool enable_ = true;

private:

    eprosima::fastdds::dds::DomainParticipant* participant_;

    eprosima::fastdds::dds::Subscriber* subscriber_;

    eprosima::fastdds::dds::DataReader* reader_;

    eprosima::fastdds::dds::TypeSupport type_;

    void data_available_handler(
            eprosima::fastdds::dds::DataReader* reader);

    void liveliness_changed_handler();

    void requested_deadline_missed_handler();

    void requested_incompatible_qos_handler();

    void subscription_matched_handler();

    void sample_rejected_handler();

    void sample_lost_handler();
};

#endif /* HELLOWORLDSUBSCRIBER_H_ */
