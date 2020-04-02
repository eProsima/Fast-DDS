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
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/attributes/PublisherAttributes.h>

#include <fastrtps/Domain.h>
#include "FilteringExamplePublisher.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

FilteringExamplePublisher::FilteringExamplePublisher()
    : mp_participant(nullptr)
    , mp_fast_publisher(nullptr)
    , mp_slow_publisher(nullptr)
{
}

FilteringExamplePublisher::~FilteringExamplePublisher()
{
    Domain::removeParticipant(mp_participant);
}

bool FilteringExamplePublisher::init()
{
    // Create RTPSParticipant

    ParticipantAttributes PParam;
    PParam.rtps.builtin.discovery_config.leaseDuration = c_TimeInfinite;
    PParam.rtps.setName("Participant_publisher");  //You can put here the name you want
    mp_participant = Domain::createParticipant(PParam);
    if (mp_participant == nullptr)
    {
        return false;
    }

    //Register the type

    Domain::registerType(mp_participant, (TopicDataType*) &myType);

    // Create Publishers

    // Create "Fast" Publisher
    PublisherAttributes Wparam_fast;
    Wparam_fast.topic.topicKind = NO_KEY;
    Wparam_fast.topic.topicDataType = myType.getName();  //This type MUST be registered
    Wparam_fast.topic.topicName = "FilteringExamplePubSubTopic";
    Wparam_fast.qos.m_partition.push_back("Fast_Partition");
    mp_fast_publisher = Domain::createPublisher(mp_participant, Wparam_fast, (PublisherListener*)&m_listener);
    if (mp_fast_publisher == nullptr)
    {
        return false;
    }
    std::cout << "Fast Publisher created, waiting for Subscribers." << std::endl;

    // Create "Slow" Publisher
    PublisherAttributes Wparam_slow;
    Wparam_slow.topic.topicKind = NO_KEY;
    Wparam_slow.topic.topicDataType = myType.getName();  //This type MUST be registered
    Wparam_slow.topic.topicName = "FilteringExamplePubSubTopic";
    Wparam_slow.qos.m_partition.push_back("Slow_Partition");
    mp_slow_publisher = Domain::createPublisher(mp_participant, Wparam_slow, (PublisherListener*)&m_listener);
    if (mp_slow_publisher == nullptr)
    {
        return false;
    }
    std::cout << "Slow Publisher created, waiting for Subscribers." << std::endl;

    return true;

}

void FilteringExamplePublisher::PubListener::onPublicationMatched(
        Publisher*,
        MatchingInfo& info)
{
    if (info.status == MATCHED_MATCHING)
    {
        n_matched++;
        std::cout << "Publisher matched" << std::endl;
    }
    else
    {
        n_matched--;
        std::cout << "Publisher unmatched" << std::endl;
    }
}

void FilteringExamplePublisher::run()
{
    while (m_listener.n_matched == 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    // Publication code

    FilteringExample st;

    int sampleNumber = 0;

    while (1)
    {
        sampleNumber++;
        st.sampleNumber(sampleNumber);
        mp_fast_publisher->write(&st);
        if (sampleNumber % 5 == 0)
        { // slow publisher writes every 5 secs.
            mp_slow_publisher->write(&st);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}
