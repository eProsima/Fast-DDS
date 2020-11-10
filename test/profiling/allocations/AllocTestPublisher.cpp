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
#include <fastrtps/xmlparser/XMLProfileManager.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

AllocTestPublisher::AllocTestPublisher()
    : mp_participant(nullptr)
    , mp_publisher(nullptr)
{


}

bool AllocTestPublisher::init(
        const char* profile,
        int domainId,
        const std::string& outputFile)
{
    m_data.index(0);

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

    if (mp_participant == nullptr)
    {
        return false;
    }

    //REGISTER THE TYPE
    Domain::registerType(mp_participant, &m_type);

    //CREATE THE PUBLISHER
    std::string prof("test_publisher_profile_");
    prof.append(profile);
    mp_publisher = Domain::createPublisher(mp_participant, prof, (PublisherListener*)&m_listener);
    if (mp_publisher == nullptr)
    {
        return false;
    }

    bool show_allocation_traces = std::getenv("FASTDDS_PROFILING_PRINT_TRACES") != nullptr;
    eprosima_profiling::entities_created(show_allocation_traces);
    return true;

}

AllocTestPublisher::~AllocTestPublisher()
{
    Domain::removeParticipant(mp_participant);
}

void AllocTestPublisher::PubListener::onPublicationMatched(
        Publisher* /*pub*/,
        MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(mtx);
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
    cv.notify_all();
}

bool AllocTestPublisher::PubListener::is_matched()
{
    std::unique_lock<std::mutex> lock(mtx);
    return n_matched > 0;
}

void AllocTestPublisher::PubListener::wait_match()
{
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this]()
            {
                return n_matched > 0;
            });
}

void AllocTestPublisher::PubListener::wait_unmatch()
{
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this]()
            {
                return n_matched <= 0;
            });
}

void AllocTestPublisher::run(
        uint32_t samples,
        bool wait_unmatch)
{
    // Restart callgrind graph
    eprosima_profiling::callgrind_zero_count();

    std::cout << "Publisher waiting for subscriber..." << std::endl;
    m_listener.wait_match();

    // Flush callgrind graph
    eprosima_profiling::callgrind_dump();
    eprosima_profiling::discovery_finished();

    std::cout << "Publisher matched. Sending samples" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    for (uint32_t i = 0; i < samples; ++i)
    {
        if (!publish())
        {
            --i;
        }
        else
        {
            std::cout << "Message with index: " << m_data.index() << " SENT" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        if (i == 0)
        {
            // Flush callgrind graph
            eprosima_profiling::callgrind_dump();
            eprosima_profiling::first_sample_exchanged();

            std::cout << "First message has been sent" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    // Flush callgrind graph
    eprosima_profiling::callgrind_dump();
    eprosima_profiling::all_samples_exchanged();

    if (wait_unmatch)
    {
        std::cout << "All messages have been sent. Waiting for subscriber to stop." << std::endl;
        m_listener.wait_unmatch();
    }
    else
    {
        std::cout << "All messages have been sent. Waiting a bit to let subscriber receive samples." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Flush callgrind graph
    eprosima_profiling::callgrind_dump();
    eprosima_profiling::undiscovery_finished();
    eprosima_profiling::print_results(m_outputFile, "publisher", m_profile);
}

bool AllocTestPublisher::publish()
{
    if (m_listener.is_matched())
    {
        m_data.index(m_data.index() + 1);
        mp_publisher->write((void*)&m_data);
        return true;
    }
    return false;
}
