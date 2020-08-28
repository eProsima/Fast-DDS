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

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/PublisherListener.h>

#include <mutex>

class LivelinessPublisher {
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
            eprosima::fastrtps::Publisher* pub,
            bool waitForListener = true);

    //! Run for number samples
	void run(uint32_t number, uint32_t sleep);

private:

    Topic topic_;
    TopicPubSubType type_;

    eprosima::fastrtps::Participant* participant_;
    eprosima::fastrtps::Publisher* publisher_;

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
                eprosima::fastrtps::rtps::MatchingInfo& info) override;

        void on_liveliness_lost(
                eprosima::fastrtps::Publisher* pub,
                const eprosima::fastrtps::LivelinessLostStatus& status) override;

		int n_matched;
        bool first_connected;
    };
    PubListener listener_;

    void runThread(
            eprosima::fastrtps::Publisher *pub,
            uint32_t number,
            uint32_t sleep);
};



#endif /* LivelinessPublisher_H_ */
