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

/**
 * @file HelloWorldPublisher.h
 *
 */

#ifndef HELLOWORLDPUBLISHER_H_
#define HELLOWORLDPUBLISHER_H_

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/participant/ParticipantListener.h>
#include <fastrtps/participant/Participant.h>
#include <fastrtps/types/DynamicData.h>
#include <fastrtps/types/DynamicPubSubType.h>


class HelloWorldPublisher
{
    public:

        HelloWorldPublisher();

        virtual ~HelloWorldPublisher();

        //!Initialize
        bool init();

        //!Publish a sample
        bool publish(
                bool waitForListener = true);

        //!Run for number samples
        void run(
                uint32_t number,
                uint32_t sleep);

    private:

        eprosima::fastrtps::Participant* mp_participant;

        eprosima::fastrtps::Publisher* mp_publisher;

        bool stop;

        class PubListener : public eprosima::fastrtps::PublisherListener
        {
            public:
                PubListener()
                    : n_matched(0)
                    , firstConnected(false)
                {}

                ~PubListener() override {}

                void onPublicationMatched(
                        eprosima::fastrtps::Publisher* pub,
                        eprosima::fastrtps::rtps::MatchingInfo& info) override;

                int n_matched;

                bool firstConnected;

        } m_listener;

        class PartListener : public eprosima::fastrtps::ParticipantListener
        {
            void onParticipantDiscovery(
                    eprosima::fastrtps::Participant* participant,
                    eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info) override;
        } m_part_list;

        void runThread(
                uint32_t number,
                uint32_t sleep);

        // Dynamic Types
        eprosima::fastrtps::types::DynamicData* m_DynHello;

        eprosima::fastrtps::types::DynamicPubSubType m_DynType;
};



#endif /* HELLOWORLDPUBLISHER_H_ */
