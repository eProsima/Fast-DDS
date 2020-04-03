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

#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/attributes/SubscriberAttributes.h>

#include <fastrtps/Domain.h>

#include "FilteringExampleSubscriber.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

FilteringExampleSubscriber::FilteringExampleSubscriber()
    : mp_participant(nullptr)
    , mp_subscriber(nullptr)
{
}

FilteringExampleSubscriber::~FilteringExampleSubscriber()
{
    Domain::removeParticipant(mp_participant);
}

bool FilteringExampleSubscriber::init(
        int type)
{
    // Create RTPSParticipant

    ParticipantAttributes PParam;
    PParam.rtps.builtin.discovery_config.leaseDuration = c_TimeInfinite;
    PParam.rtps.setName("Participant_subscriber"); //You can put the name you want
    mp_participant = Domain::createParticipant(PParam);
    if (mp_participant == nullptr)
    {
        return false;
    }

    //Register the type

    Domain::registerType(mp_participant, (TopicDataType*) &myType);

    // Create Subscriber

    SubscriberAttributes Rparam;
    Rparam.topic.topicKind = NO_KEY;
    Rparam.topic.topicDataType = myType.getName(); //Must be registered before the creation of the subscriber
    Rparam.topic.topicName = "FilteringExamplePubSubTopic";
    if (type == 1)
    {
        Rparam.qos.m_partition.push_back("Fast_Partition");
    }
    else //2 = slow
    {
        Rparam.qos.m_partition.push_back("Slow_Partition");
    }

    mp_subscriber = Domain::createSubscriber(mp_participant, Rparam, (SubscriberListener*)&m_listener);
    if (mp_subscriber == nullptr)
    {
        return false;
    }
    return true;
}

void FilteringExampleSubscriber::SubListener::onSubscriptionMatched(
        Subscriber*,
        MatchingInfo& info)
{
    if (info.status == MATCHED_MATCHING)
    {
        n_matched++;
        std::cout << "Subscriber matched" << std::endl;
    }
    else
    {
        n_matched--;
        std::cout << "Subscriber unmatched" << std::endl;
    }
}

void FilteringExampleSubscriber::SubListener::onNewDataMessage(
        Subscriber* sub)
{
    // Take data
    FilteringExample st;

    if (sub->takeNextData(&st, &m_info))
    {
        if (m_info.sampleKind == ALIVE)
        {
            // Print your structure data here.
            ++n_msg;
            std::cout << "Sample received, count=" << n_msg << std::endl;
            std::cout << " sampleNumber=" << st.sampleNumber() << std::endl;
        }
    }
}

void FilteringExampleSubscriber::run()
{
    std::cout << "Waiting for Data, press Enter to stop the Subscriber. " << std::endl;
    std::cin.ignore();
    std::cout << "Shutting down the Subscriber." << std::endl;
}
