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

#include <dds/domain/DomainParticipant.hpp>
#include <dds/pub/Publisher.hpp>
#include <dds/pub/DataWriter.hpp>
#include <dds/pub/DataWriterListener.hpp>
#include <dds/topic/Topic.hpp>
#include <dds/core/status/Status.hpp>

#include <fastdds/dds/topic/DataWriter.hpp>
#include <fastdds/dds/core/status/PublicationMatchedStatus.hpp>

class HelloWorldPublisher
{
public:

    HelloWorldPublisher();

    virtual ~HelloWorldPublisher();

    //!Initialize
    bool init();

    //!Publish a sample
    bool publish(
            bool waitForListener = true);

    //!Run for number samples
    void run(
            uint32_t number,
            uint32_t sleep);

private:

    HelloWorld hello_;

    dds::domain::DomainParticipant participant_;

    dds::pub::Publisher publisher_;

    dds::pub::DataWriter<HelloWorld> writer_;

    bool stop_;

    class PubListener : public dds::pub::NoOpDataWriterListener<HelloWorld>
    {
public:

        PubListener()
            : matched_(0)
            , firstConnected_(false)
        {
        }

        ~PubListener() override
        {
        }

        void on_publication_matched(
                dds::pub::DataWriter<HelloWorld>& writer,
                const dds::core::status::PublicationMatchedStatus& status) override;

        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter* writer,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

        void on_offered_incompatible_qos(
                eprosima::fastdds::dds::DataWriter* writer,
                const eprosima::fastdds::dds::OfferedIncompatibleQosStatus& status) override;

        int matched_;

        bool firstConnected_;
    } listener_;

    void runThread(
            uint32_t number,
            uint32_t sleep);

    HelloWorldTypeSupport type_;
    dds::topic::Topic<HelloWorld> topic_;
};



#endif /* HELLOWORLDPUBLISHER_H_ */
