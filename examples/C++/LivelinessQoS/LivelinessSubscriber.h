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
 * @file LivelinessSubscriber.h
 *
 */

#ifndef LIVELINESSSUBSCRIBER_H_
#define LIVELINESSSUBSCRIBER_H_

#include "TopicPubSubTypes.h"
#include "Topic.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/subscriber/SampleInfo.h>
#include <fastrtps/participant/ParticipantListener.h>
#include <fastrtps/participant/Participant.h>

class LivelinessSubscriber
{
public:

    //! Constructor
    LivelinessSubscriber();

    //! Destructor
    virtual ~LivelinessSubscriber();

    //! Initialize the subscriber
    bool init(
            eprosima::fastrtps::LivelinessQosPolicyKind kind,
            int liveliness_ms);

    //! RUN the subscriber
    void run();

    //! Run the subscriber until number samples have been recevied.
    void run(
            uint32_t number);

private:

    TopicPubSubType type_;
    eprosima::fastrtps::Participant* participant_;
    eprosima::fastrtps::Subscriber* subscriber_;

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
                eprosima::fastrtps::rtps::MatchingInfo& info) override;

        void onNewDataMessage(
                eprosima::fastrtps::Subscriber* sub) override;

        void on_liveliness_changed(
                eprosima::fastrtps::Subscriber* sub,
                const eprosima::fastrtps::LivelinessChangedStatus& status) override;


        eprosima::fastrtps::SampleInfo_t m_info;
        int n_matched;
        uint32_t n_samples;
        Topic topic;
    };
    SubListener listener_;

    class PartListener : public eprosima::fastrtps::ParticipantListener
    {
        virtual void onParticipantDiscovery(
                eprosima::fastrtps::Participant* participant,
                eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info) override;
    };
    PartListener part_listener_;

};

#endif // ifndef LIVELINESSSUBSCRIBER_H_
