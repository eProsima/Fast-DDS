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
 * @file ContentFilteredTopicExamplePublisher.hpp
 *
 */

#ifndef _CONTENTFILTEREDTOPICEXAMPLEPUBLISHER_H_
#define _CONTENTFILTEREDTOPICEXAMPLEPUBLISHER_H_

#include <atomic>

#include <fastdds/dds/core/status/PublicationMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "HelloWorldPubSubTypes.h"

//! Publisher application class
class ContentFilteredTopicExamplePublisher : public eprosima::fastdds::dds::DataWriterListener
{
public:

    //! Constructor
    ContentFilteredTopicExamplePublisher() = default;

    //! Destructor
    virtual ~ContentFilteredTopicExamplePublisher();

    //! Initialize
    bool init();

    //! Publish a sample
    bool publish(
            bool waitForListener = true);

    //! Run for the given number of samples (0 => infinite samples)
    void run(
            uint32_t number,
            uint32_t sleep);

private:

    //! Data type
    HelloWorld hello_;

    //! DDS DomainParticipant pointer
    eprosima::fastdds::dds::DomainParticipant* participant_ = nullptr;

    //! DDS Publisher pointer
    eprosima::fastdds::dds::Publisher* publisher_ = nullptr;

    //! DDS Topic pointer
    eprosima::fastdds::dds::Topic* topic_ = nullptr;

    //! DDS DataWriter pointer
    eprosima::fastdds::dds::DataWriter* writer_ = nullptr;

    //! DDS TypeSupport pointer
    eprosima::fastdds::dds::TypeSupport type_ = eprosima::fastdds::dds::TypeSupport(new HelloWorldPubSubType());

    //! Flag to terminate application
    std::atomic<bool> stop_;

    //! Number of DataReaders matched with the publisher application
    std::atomic<int> matched_;

    //! Flag set once the first subscriber is discovered and matched
    std::atomic<bool> first_connected_;

    //! Discovery callback specialization when the DataWriter received discovery information from a remote DataReader
    void on_publication_matched(
            eprosima::fastdds::dds::DataWriter* writer,
            const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

    //! Publisher application thread
    void runThread(
            uint32_t number,
            uint32_t sleep);

};

#endif // _CONTENTFILTEREDTOPICEXAMPLEPUBLISHER_H_
