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

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

#include "deadlinepayloadPublisher.h"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;

deadlinepayloadPublisher::deadlinepayloadPublisher()
    : mp_participant(nullptr)
    , mp_publisher(nullptr)
    , mp_writer(nullptr)
    , mp_topic(nullptr)
    , myType(new HelloMsgPubSubType())
    , double_time(false)
{
}

deadlinepayloadPublisher::~deadlinepayloadPublisher()
{
    if (mp_writer != nullptr)
    {
        mp_publisher->delete_datawriter(mp_writer);
    }
    if (mp_publisher != nullptr)
    {
        mp_participant->delete_publisher(mp_publisher);
    }
    if (mp_topic != nullptr)
    {
        mp_participant->delete_topic(mp_topic);
    }
    DomainParticipantFactory::get_instance()->delete_participant(mp_participant);
}

bool deadlinepayloadPublisher::init(
        double deadline_period_ms)
{
    // Create Participant
    DomainParticipantQos pqos;
    pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
    pqos.name("Participant_publisher");  //You can put here the name you want

    mp_participant = DomainParticipantFactory::get_instance()->create_participant(0, pqos);

    if (mp_participant == nullptr)
    {
        return false;
    }

    // Register the type
    myType.register_type(mp_participant);

    // Create Publisher
    mp_publisher = mp_participant->create_publisher(PUBLISHER_QOS_DEFAULT);

    if (mp_publisher == nullptr)
    {
        return false;
    }

    // Create topic
    mp_topic = mp_participant->create_topic("deadlinepayloadPubSubTopic", myType.get_type_name(), TOPIC_QOS_DEFAULT);

    if (mp_topic == nullptr)
    {
        return false;
    }

    // Create datawriter
    DataWriterQos wqos;
    wqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    wqos.deadline().period = deadline_period_ms * 1e-3;

    mp_writer = mp_publisher->create_datawriter(mp_topic, wqos, &m_listener);

    if (mp_writer == nullptr)
    {
        return false;
    }

    std::cout << "Publisher created, waiting for Subscribers." << std::endl;
    return true;
}

void deadlinepayloadPublisher::PubListener::on_publication_matched(
        eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        n_matched = info.total_count;
        std::cout << "Publisher matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        n_matched = info.total_count;
        std::cout << "Publisher unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
}

void deadlinepayloadPublisher::PubListener::on_offered_deadline_missed(
        DataWriter* writer,
        const OfferedDeadlineMissedStatus& status)
{
    (void)writer;
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
                    if (mp_writer->write(&st))
                    {
                        std::cout << "Message with key " << st.deadlinekey() << " sent" << std::endl;
                    }
                }
            }
            else
            {
                if (mp_writer->write(&st))
                {
                    std::cout << "Message with key " << st.deadlinekey() << " sent" << std::endl;
                }
            }
        }
    }
}
