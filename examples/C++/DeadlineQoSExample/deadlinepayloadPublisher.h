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

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/publisher/PublisherListener.h>

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
    bool init(double deadline_period_ms);

    /**
     * @brief Run the publisher
     * @param sleep_ms A time period to sleep for before sending the new sample
     */
    void run(double sleep_ms);

private:

	eprosima::fastrtps::Participant *mp_participant;
	eprosima::fastrtps::Publisher *mp_publisher;
    HelloMsgPubSubType myType;
	class PubListener : public eprosima::fastrtps::PublisherListener
	{
	public:
		PubListener() : n_matched(0){};
		~PubListener(){};
        void onPublicationMatched(eprosima::fastrtps::Publisher* pub, eprosima::fastrtps::rtps::MatchingInfo& info) override;
        void on_offered_deadline_missed(eprosima::fastrtps::rtps::InstanceHandle_t& handle) override;
		int n_matched;
	} m_listener;

    //!Boolean used to force a period double on a certain key
    bool double_time;
};

#endif // _DEADLINEPAYLOAD_PUBLISHER_H_
