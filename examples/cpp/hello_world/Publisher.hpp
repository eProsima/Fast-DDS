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
 * @file Publisher.hpp
 *
 */

#ifndef _FASTDDS_HELLO_WORLD_PUBLISHER_HPP_
#define _FASTDDS_HELLO_WORLD_PUBLISHER_HPP_

#include <condition_variable>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "HelloWorldPubSubTypes.h"

using namespace eprosima::fastdds::dds;

class HelloWorldPublisher : public DataWriterListener
{
public:

    HelloWorldPublisher();

    ~HelloWorldPublisher() override;

    //! Publisher matched method
    void on_publication_matched(
            DataWriter* writer,
            const PublicationMatchedStatus& info) override;

    //! Publish a sample
    bool publish();

    //! Run publisher
    void run();

    //! Trigger the end of execution
    static void stop();

private:

    //! Return the current state of execution
    bool is_stopped();

    HelloWorld hello_;

    DomainParticipant* participant_;

    Publisher* publisher_;

    Topic* topic_;

    DataWriter* writer_;

    TypeSupport type_;

    static std::atomic<bool> stop_;

    int16_t matched_;

    std::mutex mutex_;

    std::condition_variable matched_cv_;

    const uint8_t period_ = 100; // in ms
};



#endif /* _FASTDDS_HELLO_WORLD_PUBLISHER_HPP_ */
