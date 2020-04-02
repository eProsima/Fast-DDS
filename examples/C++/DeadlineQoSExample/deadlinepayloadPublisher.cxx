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
#include "deadlinepayloadPublisher.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

deadlinepayloadPublisher::deadlinepayloadPublisher()
    : mp_participant(nullptr)
    , mp_publisher(nullptr)
    , double_time(false)
{
}

deadlinepayloadPublisher::~deadlinepayloadPublisher()
{
    Domain::removeParticipant(mp_participant);
}

bool deadlinepayloadPublisher::init(
        double deadline_period_ms)
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

    // Create Publisher

    PublisherAttributes Wparam;
    Wparam.topic.topicKind = WITH_KEY;
    Wparam.topic.topicDataType = myType.getName();
    Wparam.topic.topicName = "deadlinepayloadPubSubTopic";
    Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Wparam.qos.m_deadline.period = deadline_period_ms * 1e-3;
    mp_publisher = Domain::createPublisher(mp_participant, Wparam, (PublisherListener*)&m_listener);
    if (mp_publisher == nullptr)
    {
        return false;
    }

    std::cout << "Publisher created, waiting for Subscribers." << std::endl;
    return true;
}

void deadlinepayloadPublisher::PubListener::onPublicationMatched(
        Publisher* /*pub*/,
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

void deadlinepayloadPublisher::PubListener::on_offered_deadline_missed(
        Publisher* pub,
        const OfferedDeadlineMissedStatus& status)
{
    (void)pub;
    std::cout << "Deadline missed for instance: " << status.last_instance_handle << std::endl;
}

void deadlinepayloadPublisher::run(
        uint32_t sleep_ms,
        int samples)
{
    while (m_listener.n_matched == 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    // Publication code

    HelloMsg st;
    std::stringstream stream;
    //Filling sample message
    stream << "generic payload";
    st.payload(stream.str());

    int sample = 0;
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));

        if (samples > 0)
        {
            if (sample == samples)
            {
                // Stop sending samples but keep the publisher running
                continue;
            }
            sample++;
        }

        // Send messages
        for (unsigned short i = 0; i < 3; i++)
        {
            // Set key
            st.deadlinekey(i);
            if (i == 2)
            {
                //Force key 2 message to be sent half of the times
                double_time = !double_time;
                if (double_time)
                {
                    if (mp_publisher->write(&st))
                    {
                        std::cout << "Message with key " << st.deadlinekey() << " sent" << std::endl;
                    }
                }
            }
            else
            {
                if (mp_publisher->write(&st))
                {
                    std::cout << "Message with key " << st.deadlinekey() << " sent" << std::endl;
                }
            }
        }
    }
}
