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

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.h>

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

    eprosima::fastdds::dds::TypeSupport type_;

    eprosima::fastdds::dds::DomainParticipant* participant_;

    eprosima::fastdds::dds::Subscriber* subscriber_;

    eprosima::fastdds::dds::Topic* topic_;

    eprosima::fastdds::dds::DataReader* reader_;

    class SubListener : public eprosima::fastdds::dds::DataReaderListener
    {
public:

        SubListener()
            : n_matched(0)
            , n_samples(0)
        {
        }

        ~SubListener() override
        {
        }

        void on_data_available(
                eprosima::fastdds::dds::DataReader* reader) override;

        void on_subscription_matched(
                eprosima::fastdds::dds::DataReader* reader,
                const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

        void on_liveliness_changed(
                eprosima::fastdds::dds::DataReader* reader,
                const eprosima::fastdds::dds::LivelinessChangedStatus& status) override;

        int n_matched;

        uint32_t n_samples;

        Topic topic;
    };

    SubListener listener_;

    class PartListener : public eprosima::fastdds::dds::DomainParticipantListener
    {
        virtual void on_participant_discovery(
                eprosima::fastdds::dds::DomainParticipant* participant,
                eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info) override;
    };

    PartListener part_listener_;

};

#endif
