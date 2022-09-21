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
 * @file BasicPublisher.h
 *
 */

#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "types/HelloWorldPubSubTypes.h"

/**
 * Class used to group into a single working unit a Publisher with a DataWriter, its listener, and a TypeSupport member
 * corresponding to the HelloWorld datatype
 */
class BasicPublisher: public eprosima::fastdds::dds::DataWriterListener
{
public:

    BasicPublisher(
            const std::string& topic_name,
            uint32_t domain);

    virtual ~BasicPublisher();

    //! Run for number samples, publish every sleep seconds
    void run(
            uint32_t number,
            uint32_t sleep);

    ///////////////////
    // Signal related methods

    //! Return the current state of execution
    static bool is_stopped();

    //! Trigger the end of execution
    static void stop();

    ///////////////////
    // Listener methods

    void on_publication_matched(
            eprosima::fastdds::dds::DataWriter* writer,
            const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

private:

    ///////////////////
    // Fast DDS entities

    eprosima::fastdds::dds::DomainParticipant* participant_;

    eprosima::fastdds::dds::Publisher* publisher_;

    eprosima::fastdds::dds::Topic* topic_;

    eprosima::fastdds::dds::DataWriter* writer_;

    eprosima::fastdds::dds::TypeSupport type_;

    ///////////////////
    // Data send

    //! Publish a sample
    void publish_();

    //! Data to send. This is related to the class so it is not created and destroyed each iteration.
    HelloWorld hello_;

    ///////////////////
    // Stop related variables

    //! Member used for control flow purposes
    static std::atomic<bool> stop_;
};
