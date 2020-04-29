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

#include "FilteringExamplePublisher.h"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;

FilteringExamplePublisher::FilteringExamplePublisher()
    : participant_(nullptr)
    , fast_publisher_(nullptr)
    , slow_publisher_(nullptr)
    , topic_(nullptr)
    , fast_writer_(nullptr)
    , slow_writer_(nullptr)
    , myType(new FilteringExamplePubSubType())
{
}

FilteringExamplePublisher::~FilteringExamplePublisher()
{
    if (fast_writer_ != nullptr)
    {
        fast_publisher_->delete_datawriter(fast_writer_);
    }
    if (slow_writer_ != nullptr)
    {
        slow_publisher_->delete_datawriter(slow_writer_);
    }
    if (fast_publisher_ != nullptr)
    {
        participant_->delete_publisher(fast_publisher_);
    }
    if (slow_publisher_ != nullptr)
    {
        participant_->delete_publisher(slow_publisher_);
    }
    if (topic_ != nullptr)
    {
        participant_->delete_topic(topic_);
    }
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

bool FilteringExamplePublisher::init()
{
    // Create Participant
    DomainParticipantQos pqos;
    pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
    pqos.name("Participant_publisher");  //You can put here the name you want

    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);

    if (participant_ == nullptr)
    {
        return false;
    }

    //Register the type
    myType.register_type(participant_);

    // Create "Fast" Publisher
    PublisherQos pfqos;
    pfqos.partition().push_back("Fast_Partition");

    fast_publisher_ = participant_->create_publisher(pfqos);
    if (fast_publisher_ == nullptr)
    {
        return false;
    }

    //Create Topic
    topic_ = participant_->create_topic("FilteringExamplePubSubTopic", myType.get_type_name(), TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        return false;
    }

    //Create "Fast" DataWriter
    fast_writer_ = fast_publisher_->create_datawriter(topic_, DATAWRITER_QOS_DEFAULT, &m_listener);

    if (fast_writer_ == nullptr)
    {
        return false;
    }

    std::cout << "Fast Publisher created, waiting for Subscribers." << std::endl;

    // Create "Slow" Publisher
    PublisherQos psqos;
    psqos.partition().push_back("Slow_Partition");

    slow_publisher_ = participant_->create_publisher(psqos);

    if (slow_publisher_ == nullptr)
    {
        return false;
    }

    // Create "Slow" DataWriter
    slow_writer_ = slow_publisher_->create_datawriter(topic_, DATAWRITER_QOS_DEFAULT, &m_listener);

    if (slow_writer_ == nullptr)
    {
        return false;
    }
    std::cout << "Slow Publisher created, waiting for Subscribers." << std::endl;

    return true;

}

void FilteringExamplePublisher::PubListener::on_publication_matched(
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
        fast_writer_->write(&st);
        if (sampleNumber % 5 == 0)
        { // slow publisher writes every 5 secs.
            slow_writer_->write(&st);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}
