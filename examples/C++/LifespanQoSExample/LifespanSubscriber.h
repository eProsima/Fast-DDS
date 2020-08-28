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
 * @file LifespanSubscriber.h
 *
 */

#ifndef HELLOWORLDSUBSCRIBER_H_
#define HELLOWORLDSUBSCRIBER_H_

#include "LifespanPubSubTypes.h"
#include "Lifespan.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/subscriber/SampleInfo.h>

class LifespanSubscriber {
public:
    /**
     * @brief LifespanSubscriber
     */
	LifespanSubscriber();

    /**
     * @brief ~LifespanSubscriber
     */
	virtual ~LifespanSubscriber();

    /**
     * @brief Initialises the subscriber
     * @param lifespan_ms The lifespan in ms
     * @return True if initialisation was successful
     */
    bool init(uint32_t lifespan_ms);

    /**
     * @brief Runs the subscriber
     * @param number The number of samples the subscriber expects to receive
     * @param sleep_ms Time to sleep after receiving all the samples
     */
    void run(
            uint32_t number,
            uint32_t sleep_ms);

private:

    eprosima::fastrtps::Participant* participant_;
    eprosima::fastrtps::Subscriber* subscriber_;
    LifespanPubSubType type_;

public:
	class SubListener:public eprosima::fastrtps::SubscriberListener
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
		void onNewDataMessage(eprosima::fastrtps::Subscriber* sub);

        Lifespan hello;
        eprosima::fastrtps::SampleInfo_t info;
		int n_matched;
		uint32_t n_samples;
    }listener;
};

#endif /* HELLOWORLDSUBSCRIBER_H_ */
