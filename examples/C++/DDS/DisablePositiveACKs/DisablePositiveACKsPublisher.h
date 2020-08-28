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
 * @file DisablePositiveACKsPublisher.h
 *
 */

#ifndef DisablePositiveACKsPublisher_H_
#define DisablePositiveACKsPublisher_H_

#include "Topic.h"
#include "TopicPubSubTypes.h"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/Topic.hpp>

class DisablePositiveACKsPublisher
{
public:

    /**
     * @brief DisablePositiveACKsPublisher
     */
    DisablePositiveACKsPublisher();

    /**
     * @brief ~DisablePositiveACKsPublisher
     */
    virtual ~DisablePositiveACKsPublisher();

    /**
     * @brief Initialises the publisher
     * @param disable_positive_acks True to disable positive acks
     * @param keep_duration_ms The keep duration in ms
     * @return True if initialization was successful
     */
    bool init(
            bool disable_positive_acks,
            uint32_t keep_duration_ms);

    /**
     * @brief Publishes a new sample
     * @param waitForListener True to wait until a listener exists
     * @return True if sample was written succesfully
     */
    bool publish(
            bool waitForListener = true);

    /**
     * @brief Runs the publisher
     * @param number The number of samples to write
     * @param write_sleep_ms Time to sleep between samples
     */
    void run(
            uint32_t number,
            uint32_t write_sleep_ms);

private:

    Topic hello_;

    eprosima::fastdds::dds::DomainParticipant* participant_;

    eprosima::fastdds::dds::Publisher* publisher_;

    eprosima::fastdds::dds::Topic* topic_;

    eprosima::fastdds::dds::DataWriter* writer_;

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

        int n_matched;

    } listener;

    eprosima::fastdds::dds::TypeSupport type_;
};



#endif /* DisablePositiveACKsPublisher_H_ */
