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
 * @file LivelinessPublishers.h
 *
 */

#ifndef LivelinessPublishers_H_
#define LivelinessPublishers_H_

#include "TopicType.h"
#include "Topic.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/PublisherListener.h>

#include <mutex>

class LivelinessPublishers {
public:

    //! Constructor
    LivelinessPublishers();

    //! Destructor
    virtual ~LivelinessPublishers();

    /**
     * @brief Initialises the participant
     * @param first_kind Liveliness kind of the first publisher
     * @param second_kind Liveliness kind of the second publisher
     * @param first_liveliness_ms Lease duration of first publisher
     * @param second_liveliness_ms Lease duration of second publisher
     * @return
     */
    bool init(
            eprosima::fastrtps::LivelinessQosPolicyKind first_kind,
            eprosima::fastrtps::LivelinessQosPolicyKind second_kind,
            int first_liveliness_ms,
            int second_liveliness_ms);

    //! Publish a sample
    bool publish(
            eprosima::fastrtps::Publisher* pub,
            bool waitForListener = true);

    //! Run for number samples
	void run(uint32_t number, uint32_t sleep);

private:

    Topic topic_;
    TopicType type_;

    eprosima::fastrtps::Participant* participant_;
    eprosima::fastrtps::Publisher* publisher_1_;
    eprosima::fastrtps::Publisher* publisher_2_;

    class PubListener : public eprosima::fastrtps::PublisherListener
	{
	public:
        PubListener()
            : n_matched(0)
            , first_connected(false)
        {}

        ~PubListener()
        {}

        void onPublicationMatched(
                eprosima::fastrtps::Publisher* pub,
                eprosima::fastrtps::rtps::MatchingInfo& info);
		int n_matched;
        bool first_connected;
        std::recursive_mutex listener_mutex_;
    };
    PubListener listener_;

    void runThread(
            eprosima::fastrtps::Publisher *pub,
            uint32_t number,
            uint32_t sleep);

    std::recursive_mutex pub_mutex;
};



#endif /* LivelinessPublishers_H_ */
