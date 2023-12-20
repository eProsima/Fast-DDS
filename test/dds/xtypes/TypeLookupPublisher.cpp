// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file TypeLookupPublisher.cpp
 */

#include "TypeLookupPublisher.h"

#include <chrono>
#include <fstream>
#include <string>
#include <thread>

#include <asio.hpp>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;

static int DOMAIN_ID_ = 10;

TypeLookupPublisher::~TypeLookupPublisher()
{
    if (nullptr != writer_)
    {
        publisher_->delete_datawriter(writer_);
    }

    if (nullptr != publisher_)
    {
        participant_->delete_publisher(publisher_);
    }

    if (nullptr != topic_)
    {
        participant_->delete_topic(topic_);
    }

    if (nullptr != participant_)
    {
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

bool TypeLookupPublisher::init()
{
    std::cout << "TypeLookupPublisher::init" << std::endl;

    participant_ = DomainParticipantFactory::get_instance()
                    ->create_participant(DOMAIN_ID_, PARTICIPANT_QOS_DEFAULT, this);

    if (participant_ == nullptr)
    {
        std::cout << "ERROR create_participant" << std::endl;
        return false;
    }

    if (!create_type())
    {
        std::cout << "ERROR create_type" << std::endl;
        return false;
    }

    return true;
}

bool TypeLookupPublisher::create_type()
{

    auto obj = BasicStruct();
    type_.reset(new BasicStructPubSubType());
    type_.register_type(participant_);


    //CREATE THE PUBLISHER
    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, this);
    if (publisher_ == nullptr)
    {
        std::cout << "ERROR create_publisher" << std::endl;
        return false;
    }

    //CREATE THE TOPIC
    std::ostringstream topic_name;
    topic_name << "BasicStructPubSubType" << "_" << asio::ip::host_name() << "_" << DOMAIN_ID_;
    topic_ = participant_->create_topic(topic_name.str(), type_.get_type_name(), TOPIC_QOS_DEFAULT);
    if (topic_ == nullptr)
    {
        std::cout << "ERROR create_topic" << std::endl;
        return false;
    }

    //CREATE THE DATAWRITER
    DataWriterQos wqos = publisher_->get_default_datawriter_qos();
    writer_ = publisher_->create_datawriter(topic_, wqos, this);
    if (writer_ == nullptr)
    {
        std::cout << "ERROR create_datawriter" << std::endl;
        return false;
    }

    return true;
}

void TypeLookupPublisher::wait_discovery(
        uint32_t how_many)
{
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [&]
            {
                return matched_ >= how_many;
            });
}

void TypeLookupPublisher::run(
        uint32_t samples)
{
    uint32_t current_sample = 0;
    uint16_t index = 0;
    void* sample = nullptr;

    while (samples > current_sample)
    {
        sample = type_.create_data();

        BasicStruct* data = static_cast<BasicStruct*>(sample);
        data->index(index);
        data->message("Message" + std::to_string(index));

        std::cout << "Publisher writting index: " << index << std::endl;
        writer_->write(sample);

        ++current_sample;
        ++index;

        type_.delete_data(sample);

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
}

void TypeLookupPublisher::on_publication_matched(
        DataWriter* /*publisher*/,
        const PublicationMatchedStatus& info)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (info.current_count_change == 1)
    {
        matched_ = info.total_count;
        std::cout << "(TypeLookupPublisher) matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        matched_ = info.total_count;
        std::cout << "(TypeLookupPublisher) unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
    cv_.notify_all();
}
