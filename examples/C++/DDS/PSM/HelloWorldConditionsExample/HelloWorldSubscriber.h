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
#include <dds/core/cond/WaitSet.hpp>
#include <dds/sub/SampleInfo.hpp>
#include <dds/core/status/State.hpp>

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

    int matched_ = 0;

    uint32_t samples_ = 0;

    dds::sub::SampleInfo info_;

    bool enable_ = true;

private:

    dds::domain::DomainParticipant participant_;

    dds::sub::Subscriber subscriber_;

    dds::sub::DataReader<HelloWorld> reader_;

    HelloWorldTypeSupport type_;

    dds::topic::Topic<HelloWorld> topic_;

    dds::core::cond::WaitSet waitset_;

    void data_available_handler(
            dds::sub::DataReader<HelloWorld>* reader);

    void liveliness_changed_handler();

    void requested_deadline_missed_handler();

    void requested_incompatible_qos_handler();

    void subscription_matched_handler();

    void sample_rejected_handler();

    void sample_lost_handler();

};

#endif /* HELLOWORLDSUBSCRIBER_H_ */
