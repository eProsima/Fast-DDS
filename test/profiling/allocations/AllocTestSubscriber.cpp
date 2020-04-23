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
#include <fastrtps/xmlparser/XMLProfileManager.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

AllocTestSubscriber::AllocTestSubscriber():mp_participant(nullptr),
mp_subscriber(nullptr)
{
}

bool AllocTestSubscriber::init(const char* profile, int domainId, const std::string& outputFile)
{
    m_profile = profile;
    m_outputFile = outputFile;
    Domain::loadXMLProfilesFile("test_xml_profiles.xml");

    ParticipantAttributes participant_att;
    if (eprosima::fastrtps::xmlparser::XMLP_ret::XML_OK ==
        eprosima::fastrtps::xmlparser::XMLProfileManager::fillParticipantAttributes("test_participant_profile",
            participant_att))
    {
        participant_att.domainId = domainId;
        mp_participant = Domain::createParticipant(participant_att);
    }

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

    eprosima_profiling::entities_created();
    return true;
}

AllocTestSubscriber::~AllocTestSubscriber() {
    Domain::removeParticipant(mp_participant);
}

void AllocTestSubscriber::SubListener::onSubscriptionMatched(Subscriber* /*sub*/,MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(mtx);
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
    cv.notify_all();
}

void AllocTestSubscriber::SubListener::onNewDataMessage(Subscriber* sub)
{
    if(sub->takeNextData((void*)&m_Hello, &m_info))
    {
        if(m_info.sampleKind == ALIVE)
        {
            std::unique_lock<std::mutex> lock(mtx);
            this->n_samples++;
            // Print your structure data here.
            std::cout << "Message " << m_Hello.index() << " RECEIVED" << std::endl;
            cv.notify_all();
        }
    }

}

void AllocTestSubscriber::SubListener::wait_match()
{
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this]() { return n_matched > 0; });
}

void AllocTestSubscriber::SubListener::wait_unmatch()
{
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this]() { return n_matched <= 0; });
}

void AllocTestSubscriber::SubListener::wait_until_total_received_at_least(uint32_t n)
{
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this, n]() { return n_samples >= n; });
}

void AllocTestSubscriber::run(bool wait_unmatch)
{
  run(60, wait_unmatch);
}

void AllocTestSubscriber::run(uint32_t number, bool wait_unmatch)
{
    // Restart callgrind graph
    eprosima_profiling::callgrind_zero_count();

    std::cout << "Subscriber waiting for publisher..." << std::endl;
    m_listener.wait_match();

    // Flush callgrind graph
    eprosima_profiling::callgrind_dump();
    eprosima_profiling::discovery_finished();

    std::cout << "Subscriber matched. Waiting for first sample..." << std::endl;
    m_listener.wait_until_total_received_at_least(1ul);

    // Flush callgrind graph
    eprosima_profiling::callgrind_dump();
    eprosima_profiling::first_sample_exchanged();

    std::cout << "First sample received. Waiting for rest of samples..." << std::endl;
    m_listener.wait_until_total_received_at_least(number);

    // Flush callgrind graph
    eprosima_profiling::callgrind_dump();
    eprosima_profiling::all_samples_exchanged();

    if (wait_unmatch)
    {
        std::cout << "All messages received. Waiting for publisher to stop." << std::endl;
        m_listener.wait_unmatch();
    }
    else
    {
        std::cout << "All messages received. Waiting a bit to let publisher receive acks." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Flush callgrind graph
    eprosima_profiling::callgrind_dump();
    eprosima_profiling::undiscovery_finished();
    eprosima_profiling::print_results(m_outputFile, "subscriber", m_profile);
}
