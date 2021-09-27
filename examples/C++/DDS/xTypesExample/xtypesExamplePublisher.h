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
 * @file xtypesExamplePublisher.h
 *
 */

#ifndef xtypesExamplePUBLISHER_H_
#define xtypesExamplePUBLISHER_H_

#include <atomic>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#define SLEEP_TIME 1000
#define TOPIC_NAME "xtypesExampleTopic"
#define TYPE_NAME_PUB "xTypesExampleType"
#define IDL_FILE_NAME "xtypesExample.idl"

class xtypesExamplePublisher
{
public:

    xtypesExamplePublisher();

    virtual ~xtypesExamplePublisher();

    //! Initialize publisher
    bool init();

    //! Run publisher
    void run();

private:

    //! Run the publication thread until \c stop_ has changed
    void runThread();

    eprosima::fastdds::dds::DomainParticipant* participant_;

    eprosima::fastdds::dds::Publisher* publisher_;

    eprosima::fastdds::dds::Topic* topic_;

    eprosima::fastdds::dds::DataWriter* writer_;

    eprosima::fastdds::dds::TypeSupport type_;

    uint32_t samples_sent_;

    // Variable to handle the stop of the publish thread
    std::atomic<bool> stop_;

    // DataWriter Listener class to handle callback of matching
    class PubListener : public eprosima::fastdds::dds::DataWriterListener
    {
    public:

        PubListener()
        {
        }

        ~PubListener() override
        {
        }

        // Callback called when a subscriber has matched or unmatched
        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter* writer,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;
    } listener_;
};



#endif /* xtypesExamplePUBLISHER_H_ */
