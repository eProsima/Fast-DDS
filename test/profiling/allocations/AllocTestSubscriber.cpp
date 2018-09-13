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
 * @file AllocTestSubscriber.cpp
 *
 */

#include "AllocTestSubscriber.h"
#include "AllocTestCommon.h"
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/Domain.h>
#include <fastrtps/utils/eClock.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

AllocTestSubscriber::AllocTestSubscriber():mp_participant(nullptr),
mp_subscriber(nullptr)
{
}

bool AllocTestSubscriber::init(const char* profile)
{
    Domain::loadXMLProfilesFile("test_xml_profiles.xml");
    mp_participant = Domain::createParticipant("test_participant_profile");
    if(mp_participant==nullptr)
        return false;

    //REGISTER THE TYPE
    Domain::registerType(mp_participant,&m_type);

    //CREATE THE SUBSCRIBER
    std::string prof("test_subscriber_profile_");
    prof.append(profile);
    mp_subscriber = Domain::createSubscriber(mp_participant, prof, &m_listener);

    if(mp_subscriber == nullptr)
        return false;

    // Breakpoint placed here. First snapshot taken.
    return true;
}

AllocTestSubscriber::~AllocTestSubscriber() {
    Domain::removeParticipant(mp_participant);
}

void AllocTestSubscriber::SubListener::onSubscriptionMatched(Subscriber* /*sub*/,MatchingInfo& info)
{
    if(info.status == MATCHED_MATCHING)
    {
        n_matched++;
        std::cout << "Subscriber matched"<<std::endl;
    }
    else
    {
        n_matched--;
        std::cout << "Subscriber unmatched"<<std::endl;
    }
}

void AllocTestSubscriber::SubListener::onNewDataMessage(Subscriber* sub)
{
    if(sub->takeNextData((void*)&m_Hello, &m_info))
    {
        if(m_info.sampleKind == ALIVE)
        {
            this->n_samples++;
            // Print your structure data here.
            std::cout << "Message " << m_Hello.index() << " RECEIVED" << std::endl;
        }
    }

}


void AllocTestSubscriber::run()
{
  run(60);
}

void AllocTestSubscriber::run(uint32_t number)
{
    // Restart callgrind graph
    callgrind_zero_count();

    std::cout << "Subscriber waiting for publisher..." << std::endl;
    while (m_listener.n_matched <= 0)
    {
        eClock::my_sleep(25);
    }

    // Flush callgrind graph
    callgrind_dump();

    std::cout << "Subscriber matched. Waiting for first sample..." << std::endl;
    while (this->m_listener.n_samples < 1)
    {
        eClock::my_sleep(25);
    }

    // Flush callgrind graph
    callgrind_dump();

    std::cout << "First sample received. Waiting for rest of samples..." << std::endl;
    while (this->m_listener.n_samples < number)
    {
        eClock::my_sleep(25);
    }

    // Flush callgrind graph
    callgrind_dump();

    std::cout << "All messages received. Press enter to stop subscriber" << std::endl;
    std::cin.ignore();

    // Flush callgrind graph
    callgrind_dump();
}
