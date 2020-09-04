// Copyright 20196 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DisablePositiveACKsSubscriber.h
 *
 */

#ifndef DisablePositiveACKsSubscriber_H_
#define DisablePositiveACKsSubscriber_H_

#include "TopicPubSubTypes.h"
#include "Topic.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/subscriber/SampleInfo.h>

class DisablePositiveACKsSubscriber
{
public:

    /**
     * @brief DisablePositiveACKsSubscriber
     */
    DisablePositiveACKsSubscriber();

    /**
     * @brief ~DisablePositiveACKsSubscriber
     */
    virtual ~DisablePositiveACKsSubscriber();

    /**
     * @brief Initialises the subscriber
     * @param disable_positive_acks True to disable positive acks
     * @return True if initialisation was successful
     */
    bool init(
            bool disable_positive_acks);

    /**
     * @brief Runs the subscriber
     * @param number The number of samples the subscriber expects to receive
     */
    void run(
            uint32_t number);

private:

    eprosima::fastrtps::Participant* participant_;
    eprosima::fastrtps::Subscriber* subscriber_;
    TopicPubSubType type_;

public:

    class SubListener : public eprosima::fastrtps::SubscriberListener
    {
    public:

        SubListener()
            : n_matched(0)
            , n_samples(0)
        {
        }

        ~SubListener()
        {
        }

        void onSubscriptionMatched(
                eprosima::fastrtps::Subscriber* sub,
                eprosima::fastrtps::rtps::MatchingInfo& info);
        void onNewDataMessage(
                eprosima::fastrtps::Subscriber* sub);

        Topic hello;
        eprosima::fastrtps::SampleInfo_t info;
        int n_matched;
        uint32_t n_samples;
    }
    listener;
};

#endif /* DisablePositiveACKsSubscriber_H_ */
