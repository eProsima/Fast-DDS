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
 * @file AllocTestPublisher.cpp
 *
 */

#include "AllocTestPublisher.h"
#include "AllocTestCommon.h"
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/Domain.h>
#include <fastrtps/utils/eClock.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

AllocTestPublisher::AllocTestPublisher():mp_participant(nullptr),
mp_publisher(nullptr)
{


}

bool AllocTestPublisher::init(const char* profile)
{
    m_data.index(0);

    Domain::loadXMLProfilesFile("test_xml_profiles.xml");
    mp_participant = Domain::createParticipant("test_participant_profile");
    if (mp_participant == nullptr)
        return false;

    //REGISTER THE TYPE
    Domain::registerType(mp_participant, &m_type);

    //CREATE THE PUBLISHER
    std::string prof("test_publisher_profile_");
    prof.append(profile);
    mp_publisher = Domain::createPublisher(mp_participant,prof,(PublisherListener*)&m_listener);
    if(mp_publisher == nullptr)
        return false;

    return true;

}

AllocTestPublisher::~AllocTestPublisher()
{
    Domain::removeParticipant(mp_participant);
}

void AllocTestPublisher::PubListener::onPublicationMatched(Publisher* /*pub*/,MatchingInfo& info)
{
    if(info.status == MATCHED_MATCHING)
    {
        n_matched++;
        std::cout << "Publisher matched"<<std::endl;
    }
    else
    {
        n_matched--;
        std::cout << "Publisher unmatched"<<std::endl;
    }
}

void AllocTestPublisher::run(uint32_t samples)
{
    // Restart callgrind graph
    callgrind_zero_count();

    std::cout << "Publisher waiting for subscriber..." << std::endl;
    while (m_listener.n_matched <= 0)
    {
        eClock::my_sleep(25);
    }

    // Flush callgrind graph
    callgrind_dump();

    std::cout << "Publisher matched. Press enter to start sending samples" << std::endl;
    std::cin.ignore();

    for(uint32_t i = 0;i<samples;++i)
    {
        if(!publish())
            --i;
        else
        {
            std::cout << "Message with index: "<< m_data.index()<< " SENT"<<std::endl;
        }
        eClock::my_sleep(500);

        if (i == 0)
        {
            // Flush callgrind graph
            callgrind_dump();

            std::cout << "First message has been sent. Press enter to continue sending samples" << std::endl;
            std::cin.ignore();
        }
    }

    // Flush callgrind graph
    callgrind_dump();

    std::cout << "All messages have been sent. Press enter to stop publisher" << std::endl;
    std::cin.ignore();

    // Flush callgrind graph
    callgrind_dump();
}

bool AllocTestPublisher::publish()
{
    if(m_listener.n_matched>0)
    {
        m_data.index(m_data.index()+1);
        mp_publisher->write((void*)&m_data);
        return true;
    }
    return false;
}

