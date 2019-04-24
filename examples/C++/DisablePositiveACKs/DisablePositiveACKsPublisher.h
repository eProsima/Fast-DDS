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
#include "TopicTypes.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/PublisherListener.h>

class DisablePositiveACKsPublisher {
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
    class PubListener:public eprosima::fastrtps::PublisherListener
    {
    public:
        PubListener()
            : n_matched(0)
        {
        }

        ~PubListener()
        {
        }

        void onPublicationMatched(
                eprosima::fastrtps::Publisher* pub,
                eprosima::fastrtps::rtps::MatchingInfo& info);

        int n_matched;
    }listener;

    TopicType type_;
};



#endif /* DisablePositiveACKsPublisher_H_ */
