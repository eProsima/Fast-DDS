// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _DEADLINEPAYLOAD_PUBLISHER_H_
#define _DEADLINEPAYLOAD_PUBLISHER_H_

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/Topic.hpp>

#include "deadlinepayloadPubSubTypes.h"

class deadlinepayloadPublisher
{
public:

    /**
     * @brief Constructor
     */
    deadlinepayloadPublisher();

    /**
     * @brief Destructor
     */
    virtual ~deadlinepayloadPublisher();

    /**
     * @brief Initialises publisher
     * @param deadline_period_ms The deadline period in milliseconds
     * @return True if initialised correctly
     */
    bool init(
            double deadline_period_ms);

    /**
     * @brief Run the publisher
     * @param sleep_ms A time period to sleep for before sending the new sample
     * @param samples The number of samples per instance to send. If set to 0 sends sample indefinitely
     */
    void run(
            uint32_t sleep_ms,
            int samples);

private:

    eprosima::fastdds::dds::DomainParticipant* mp_participant;

    eprosima::fastdds::dds::Publisher* mp_publisher;

    eprosima::fastdds::dds::DataWriter* mp_writer;

    eprosima::fastdds::dds::Topic* mp_topic;

    eprosima::fastdds::dds::TypeSupport myType;

    class PubListener : public eprosima::fastdds::dds::DataWriterListener
    {
public:

        PubListener()
            : n_matched(0)
        {
        }

        ~PubListener() override
        {
        }

        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter* writer,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

        void on_offered_deadline_missed(
                eprosima::fastdds::dds::DataWriter* writer,
                const eprosima::fastdds::dds::OfferedDeadlineMissedStatus& status) override;

        int n_matched;

    } m_listener;

    //!Boolean used to force a period double on a certain key
    bool double_time;
};

#endif // _DEADLINEPAYLOAD_PUBLISHER_H_
