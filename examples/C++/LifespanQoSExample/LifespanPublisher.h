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
 * @file LifespanPublisher.h
 *
 */

#ifndef HELLOWORLDPUBLISHER_H_
#define HELLOWORLDPUBLISHER_H_

#include "Lifespan.h"
#include "LifespanTypes.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/PublisherListener.h>

class LifespanPublisher {
public:

    /**
     * @brief LifespanPublisher
     */
	LifespanPublisher();

    /**
     * @brief ~LifespanPublisher
     */
	virtual ~LifespanPublisher();

    /**
     * @brief Initialises the publisher
     * @param lifespan_ms The lifespan in ms
     * @return True if initialization was successful
     */
	bool init(uint32_t lifespan_ms);

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
     * @param sleep_ms Time to sleep after sending all the samples
     */
    void run(
            uint32_t number,
            uint32_t write_sleep_ms,
            uint32_t sleep_ms);
private:

    Lifespan hello_;
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

    LifespanType type_;
};



#endif /* HELLOWORLDPUBLISHER_H_ */
