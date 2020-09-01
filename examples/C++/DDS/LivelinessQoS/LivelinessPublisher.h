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
 * @file LivelinessPublisher.h
 *
 */

#ifndef LivelinessPublisher_H_
#define LivelinessPublisher_H_

#include "TopicPubSubTypes.h"
#include "Topic.h"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/Topic.hpp>

#include <mutex>

class LivelinessPublisher
{
public:

    //! Constructor
    LivelinessPublisher();

    //! Destructor
    virtual ~LivelinessPublisher();

    //! Initialize
    bool init(
            eprosima::fastrtps::LivelinessQosPolicyKind kind,
            int liveliness_ms);

    //! Publish a sample
    bool publish(
            eprosima::fastdds::dds::DataWriter* writer,
            bool waitForListener = true);

    //! Run for number samples
    void run(
            uint32_t number,
            uint32_t sleep);

private:

    Topic topic;

    eprosima::fastdds::dds::TypeSupport type_;

    eprosima::fastdds::dds::DomainParticipant* participant_;

    eprosima::fastdds::dds::Publisher* publisher_;

    eprosima::fastdds::dds::Topic* topic_;

    eprosima::fastdds::dds::DataWriter* writer_;

    class PubListener : public eprosima::fastdds::dds::DataWriterListener
    {
    public:

        PubListener()
            : n_matched(0)
            , first_connected(false)
        {
        }

        ~PubListener() override
        {
        }

        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter* writer,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

        void on_liveliness_lost(
                eprosima::fastdds::dds::DataWriter* writer,
                const eprosima::fastrtps::LivelinessLostStatus& status) override;

        int n_matched;

        bool first_connected;

    };
    PubListener listener_;

    void runThread(
            eprosima::fastdds::dds::DataWriter* writer,
            uint32_t number,
            uint32_t sleep);
};

#endif /* LivelinessPublisher_H_ */
