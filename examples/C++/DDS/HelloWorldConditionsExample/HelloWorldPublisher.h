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
 * @file HelloWorldPublisher.h
 *
 */

#ifndef HELLOWORLDPUBLISHER_H_
#define HELLOWORLDPUBLISHER_H_

#include "HelloWorldPubSubTypes.h"

#include <fastdds/dds/topic/DataWriterListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>

class HelloWorldPublisher
{
public:

    HelloWorldPublisher();

    virtual ~HelloWorldPublisher();

    //!Initialize
    bool init(
            int domain_id);

    //!Publish a sample
    bool publish(
            bool waitForListener = true);

    //!Run for number samples
    void run(
            uint32_t number,
            uint32_t sleep);

private:

    HelloWorld hello_;

    eprosima::fastdds::dds::DomainParticipant* participant_;

    eprosima::fastdds::dds::Publisher* publisher_;

    eprosima::fastdds::dds::DataWriter* writer_;

    eprosima::fastdds::dds::Topic* topic_;

    bool stop_;

    int matched_ = 0;

    bool firstConnected_ = false;

    void runThread(
            uint32_t number,
            uint32_t sleep);

    void check_reader_matched();

    eprosima::fastdds::dds::TypeSupport type_;

    std::mutex mtx_;

    std::condition_variable cv_;

    void liveliness_lost_handler();

    void offered_deadline_missed_handler();

    void offered_incompatible_qos_handler();

    void publication_matched_handler();
};

#endif /* HELLOWORLDPUBLISHER_H_ */
