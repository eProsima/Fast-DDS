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
 * @file PositiveACKsPublisher.h
 *
 */

#ifndef POSITIVEACKSPUBLISHER_H_
#define POSITIVEACKSPUBLISHER_H_

#include "Topic.h"
#include "TopicTypes.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/PublisherListener.h>

class PositiveACKsPublisher {
public:

    /**
     * @brief PositiveACKsPublisher
     */
    PositiveACKsPublisher();

    /**
     * @brief ~PositiveACKsPublisher
     */
    virtual ~PositiveACKsPublisher();

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
    bool publish(bool waitForListener = true);

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
    eprosima::fastrtps::Participant* participant_;
    eprosima::fastrtps::Publisher* publisher_;
	bool stop;
	class PubListener:public eprosima::fastrtps::PublisherListener
	{
	public:
        PubListener()
            : n_matched(0)
            , first_connected(false)
        {
        }

        ~PubListener()
        {
        }

        void onPublicationMatched(
                eprosima::fastrtps::Publisher* pub,
                eprosima::fastrtps::rtps::MatchingInfo& info);

		int n_matched;
        bool first_connected;
    }listener;

    void runThread(
            uint32_t number,
            uint32_t sleep);

    TopicType type_;
};



#endif /* POSITIVEACKSPUBLISHER_H_ */
